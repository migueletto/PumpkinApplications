/*
 *	file:		fm_10-copy.c
 *	project:	GentleMan
 *	content:	copy/move files/databases
 *	updated:	Sep. 17. 2001
 *
 * copyright: Collin R. Mulliner <palm@mulliner.org>
 * web: www.mulliner.org/palm/
 *
 */

/*
 *  This file is part of GentleMan.
 *
 *  GentleMan is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  GentleMan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GentleMan; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "fm.h"

/*
 *	init the copy/move form
 */
void setUpCopyForm(global_data_type *glb)
{
	FormPtr form;
	char *namePtr, *dP;
	char drawStr[200];
	UInt16 infoLeng, volInfoPos, dirInfoPos, namePos;
	const char multifile[] = {"Multiple Files\0"};
	
	
	form = FrmGetActiveForm();
	
	FntSetFont(glb->cfg.font0);
	
	if (glb->cfg.screenState == screen160x160) {
		infoLeng = 150;
		volInfoPos = 30;
		dirInfoPos = 45;
		namePos = 65;
	}
	else {
		infoLeng = BIG(150);
		volInfoPos = BIG(30);
		dirInfoPos = BIG(45);
		namePos = BIG(65);
	}
		
	MemSet(drawStr, 200, 0);
	// make vol info string
	makeVolInfo(&glb->vLst.list[glb->brwLst[glb->copy->destBrowser].volIndex], drawStr, infoLeng);
	
	if (!glb->copy->single || !glb->copy->changeName) {	// only one file / database to copy
		FrmHideObject(form, FrmGetObjectIndex(form, name_copy_fld));
	}
	else if (glb->copy->single && glb->copy->changeName) {
		FldSetTextHandle(GetObjectPtr(name_copy_fld), glb->copy->destName);
		FrmSetFocus(form, FrmGetObjectIndex(form, name_copy_fld));
	}
	
	// draw form
	FrmDrawForm(form);
	
	// draw destination info
	WinDrawChars(drawStr, StrLen(drawStr), 5, volInfoPos);
	
	// draw directory info
	if (glb->brwLst[glb->copy->destBrowser].brwMode == brwModeVfs) {
		
		dP = glb->brwLst[glb->copy->destBrowser].lst.data.vol.currentDir;
	
		if (FntLineWidth(dP, StrLen(dP)) > infoLeng) {
			do {
				dP++;
			} while (FntLineWidth(dP, StrLen(dP)) > infoLeng);
		}
		
		WinDrawChars(dP, StrLen(dP), 5, dirInfoPos);
	}
			
	if (!glb->copy->single) {
		if (glb->cfg.screenState == screen160x160) {
			FntSetFont(1);
		}
/*
		else {
			FntSetFont(VgaBaseToVgaFont(1));
		}
*/
		
		WinDrawChars(multifile, StrLen(multifile), 5, namePos);

		FntSetFont(glb->cfg.font0);		
	}
	else if (!glb->copy->changeName) {
		namePtr = MemHandleLock(glb->copy->destName);
				
		WinDrawChars(namePtr, StrLen(namePtr), 5, namePos);
				
		MemHandleUnlock(glb->copy->destName);
	}
	
}

/*
 *	call init copy
 */
void doCopy(global_data_type *glb)
{
	Err error;
	
	
	error = initCopy(glb);
	
	switch (error) {
	case	errNone:
		break;
	case	1:	// readonly
		FrmAlert(alertID_volReadOnly);
		break;
	case	2:	// can't copy file to this destination
		FrmAlert(alertID_volSrcDestIncompatible);
		break;
	case	3:	// no files selected
		if (glb->brwLst[glb->currentBrw].brwMode == brwModeVfs) FrmAlert(alertID_volNoSelFile);
		else if (glb->brwLst[glb->currentBrw].brwMode == brwModeCard) FrmAlert(alertID_cardNoSelDB);
		break;
	case	4:	// .pdb/.prc file on vfs card is broken
		FrmAlert(alertID_volPDBBroken);
		break;
	}
}

/*
 * now copy the files
 */
Boolean doFileCopy(global_data_type *glb)
{
	Err error;
	
	
	error = copyFiles(glb);
	
	switch (error) {
	case	errNone:
		// refresh volume info
		refreshVolumeInfo(&glb->vLst);
		break;
		
	case	1:
		// src == dest
		FrmAlert(alertID_volSrcEqDest);
		break;
		
	case	3:
		// can't open dest
	case	2:
		// can't open src
	case	vfsErrFileGeneric:
	case	vfsErrBadName:
	default:
		FrmAlert(alertID_volUnknownError);
		break;
		
	case	vfsErrFileAlreadyExists:
	case	4:
		// dest exists
		break;
		
	case	vfsErrVolumeFull:
		FrmAlert(alertID_volFull);
		break;
	}
	
	if (error == errNone) return(true);
	else return(false);
}

/*
 * init copy function
 */
Err initCopy(global_data_type *glb)
{
	// get memory
	glb->copy = MemPtrNew(sizeof(copyType));

	// init this with NULL
	glb->copy->destName = NULL;
	
	// get destination browser
	glb->copy->destBrowser = ((glb->currentBrw == 0) ? 1 : 0);
	
	// check for "readOnly" and for "destType"
	if (glb->brwLst[glb->copy->destBrowser].brwMode == brwModeVfs) {
		if ((glb->vLst.list[glb->brwLst[glb->copy->destBrowser].volIndex].data.vol.volInfo.attributes & vfsVolumeAttrReadOnly) == vfsVolumeAttrReadOnly) {
			// can't use destination, it is set to ReadOnly !
			
			if (glb->copy != NULL) MemPtrFree(glb->copy);
			return(1);
		}
		else {
			// this vol can hold stuff from other vol's and from rom/ram cards
			glb->copy->destType = destCardVfs;
			
			// the user may change the destination file name !
			glb->copy->changeName = true;
		}
	}
	else if (glb->brwLst[glb->copy->destBrowser].brwMode == brwModeCard) {
		if (glb->vLst.list[glb->brwLst[glb->copy->destBrowser].volIndex].data.card.typeId  == cardTypeRom) {
			// can't use destination, it is set to ReadOnly !
			
			if (glb->copy != NULL) MemPtrFree(glb->copy);
			return(1);
		}
		else {
			// this card can only hold .pdb/.prc/.pqa files or stuff from ram/rom cards
			glb->copy->destType = destCard;
			
			// the user can't change the destination file name !
			glb->copy->changeName = false;
		}
	}

	// check some stuff of the source file(s) / db(s)
	if (glb->brwLst[glb->currentBrw].brwMode == brwModeVfs) {
		if (glb->brwLst[glb->currentBrw].lst.data.vol.numSelFiles == 1) {
			glb->copy->single = true;
		}
		else if (glb->brwLst[glb->currentBrw].lst.data.vol.numSelFiles > 1) {
			glb->copy->single = false;
			glb->copy->changeName = false;
		}
		else {
			if (glb->copy != NULL) MemPtrFree(glb->copy);
			return(3);
		}
	}
	else if (glb->brwLst[glb->currentBrw].brwMode == brwModeCard) {
		if (glb->brwLst[glb->currentBrw].lst.data.card.numSelDBs == 1) {
			glb->copy->single = true;
		}
		else if (glb->brwLst[glb->currentBrw].lst.data.card.numSelDBs > 1) {
			glb->copy->single = false;
		}
		else {
			
			if (glb->copy != NULL) MemPtrFree(glb->copy);
			return(3);
		}
	}

	if (glb->copy->destType == destCard) {
		// get source file type
		glb->copy->sourceType = analyzeSourceFiles(&glb->brwLst[glb->currentBrw]);
	}
	else {
		glb->copy->sourceType = destCardVfs;
	}
	
	// check source and destination
	if (glb->copy->sourceType != glb->copy->destType && glb->copy->destType != destCardVfs) {
		if ((glb->copy->sourceType & destCard) != destCard) {
			// source and destination are NOT compatible !!!
			
			if (glb->copy != NULL) MemPtrFree(glb->copy);
			return(2);
		}
	}

	if (glb->copy->single) {
	
	// copy from card to card
	if (glb->brwLst[glb->currentBrw].brwMode == brwModeCard && glb->brwLst[glb->copy->destBrowser].brwMode == brwModeCard) {
		
		glb->copy->destName = MemHandleNew(33);
		// find selected db and copy the name as destination name !
		{
			char *namePtr;
			UInt32 i;
				
			i = 0;
			while (i < glb->brwLst[glb->currentBrw].lst.data.card.numDBs && !glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].selected) {
				i++;
			}
								
			namePtr = MemHandleLock(glb->copy->destName);
			StrCopy(namePtr, glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].name);
			MemHandleUnlock(glb->copy->destName);
		}
	}
	else if (glb->brwLst[glb->currentBrw].brwMode == brwModeCard && glb->brwLst[glb->copy->destBrowser].brwMode == brwModeVfs) {
		
		glb->copy->destName = MemHandleNew(256);
		// find selected file and copy the name as destination name !
		{
			char *namePtr;
			UInt32 i;
				
			i = 0;
			while (i < glb->brwLst[glb->currentBrw].lst.data.card.numDBs && !glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].selected) {
				i++;
			}
								
			namePtr = MemHandleLock(glb->copy->destName);
			StrCopy(namePtr, glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].name);
					
			// resource databases get the ".PRC" extension
			if ((glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].attribs & dmHdrAttrResDB) == dmHdrAttrResDB) {
				StrCat(namePtr, ".prc");
			}
			else {	// all others get the ".PDB" extension
				StrCat(namePtr, ".pdb");
			}
			
			// PQA database are currently UN handled !!!
			
			MemHandleUnlock(glb->copy->destName);
		}
	}
	// copy from card to card
	else if (glb->brwLst[glb->currentBrw].brwMode == brwModeVfs && glb->brwLst[glb->copy->destBrowser].brwMode == brwModeVfs) {
		
		glb->copy->destName = MemHandleNew(256);
		// find selected file and copy the name as destination name !
		{
			char *namePtr;
			UInt32 i;
				
			i = 0;
			while (i < glb->brwLst[glb->currentBrw].lst.data.vol.numFiles && !glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].selected) {
				i++;
			}
								
			namePtr = MemHandleLock(glb->copy->destName);
			StrCopy(namePtr, glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name);
			MemHandleUnlock(glb->copy->destName);
		}
	}
	// copy from card to card
	else if (glb->brwLst[glb->currentBrw].brwMode == brwModeVfs && glb->brwLst[glb->copy->destBrowser].brwMode == brwModeCard) {
			
		glb->copy->destName = MemHandleNew(256);
		// find selected file and copy the name as destination name !
		{
			char *namePtr;
			char dbName[32];
			UInt32 i;
			char *fullpath;
			FileRef fp;
			
			
			i = 0;
			while (i < glb->brwLst[glb->currentBrw].lst.data.vol.numFiles && !glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].selected) {
				i++;
			}
			
			// make fullpath name to current file
			fullpath = MemPtrNew(StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.currentDir) + StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name) + 2);
			StrCopy(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.currentDir);
			if (fullpath[StrLen(fullpath)-1] != '/') StrCat(fullpath, "/");
			StrCat(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name);
			
			if (VFSFileOpen(glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, fullpath, vfsModeRead, &fp) != errNone) {
				
				if (fullpath != NULL) MemPtrFree(fullpath);
				if (glb->copy != NULL) MemPtrFree(glb->copy);
				
				// can't open file
				return(4);
			}
			else {
				// get database name
				if (VFSFileDBInfo(fp, dbName, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != errNone) {
				
					if (fullpath != NULL) MemPtrFree(fullpath);
					if (glb->copy != NULL) MemPtrFree(glb->copy);
					
					// can't get info
					return(4);
				}
			
				VFSFileClose(fp);
			
				if (fullpath != NULL) MemPtrFree(fullpath);
			}
								
			namePtr = MemHandleLock(glb->copy->destName);
			StrCopy(namePtr, dbName);
			MemHandleUnlock(glb->copy->destName);
		}
	}
	
	}
			
	// popup the copy(move) form
	FrmPopupForm(formID_copy);
	
	return(errNone);
}

/*
 *	free the memory used by the copy form
 */
void freeCopy(global_data_type *glb)
{
	if (glb->copy->single && glb->copy->changeName) FldSetTextHandle(GetObjectPtr(name_copy_fld), NULL);
	
	if (glb->copy->destName != NULL) MemHandleFree(glb->copy->destName);
	
	MemPtrFree(glb->copy);
	glb->copy = NULL;
}

/*
 *	is the source a palm database (prc/pdb/pqa) if yes allow user to select the ram as destination for copy or move
 * - ignore directory's (is this good?)
 */
UInt16 analyzeSourceFiles(brwLstType *brw)
{
	const char palmFileExts[3][5] = {{".pdb\0"}, {".prc\0"}, {".pqa\0"}};
	char ext[6];
	UInt32 i, i3;
	UInt8 i2, ok;
	UInt16 dest;
	
	
	if (brw->brwMode == brwModeVfs) {
		// init dest type
		dest = destCardVfs;
		
		i = 0;
		i3 = 0;
		// check all files
		while (i < brw->lst.data.vol.numFiles && i3 < brw->lst.data.vol.numSelFiles) {
	
			if (brw->lst.data.vol.fileList[i].selected && (brw->lst.data.vol.fileList[i].attribs & vfsFileAttrDirectory) != vfsFileAttrDirectory) {
				i3++;
				// get file extension
				for (i2 = StrLen(brw->lst.data.vol.fileList[i].name); i2 > 0; i2--) {
					if (brw->lst.data.vol.fileList[i].name[i2] == '.') {
						StrNCopy(ext, &brw->lst.data.vol.fileList[i].name[i2], 5);
						break;
					}
				}
			
				// check if the extension is pdb/prc/pqa
				ok = 0;
				for (i2 = 0; i2 < 3; i2++) {
					if (StrCaselessCompare(ext, palmFileExts[i2]) == 0) {
						ok = 1;
						break;
					}
				}
			
				if (ok == 0) {
					dest = destVfs;
					break;
				}
			}
			// next file
			i++;
		}
	}
	else if (brw->brwMode == brwModeCard) {
		// can always be copied to card and vfs
		dest = destCardVfs;
	}
	
	return(dest);
}

/*
 *	go thru the list of selected files/dbs and copy them to the destination
 */
Err copyFiles(global_data_type *glb)
{
	UInt32 i, i2;
	Err retVal;
	char *fullpath = NULL;
	char *fullpath2 = NULL;
	char *namePtr;
	LocalID lID;
	UInt16 cardNo;
	
	
	if (glb->brwLst[glb->currentBrw].brwMode == brwModeCard && glb->brwLst[glb->copy->destBrowser].brwMode == brwModeCard) {
		FrmAlert(alertID_stupid);
		retVal = errNone;
	}
	else if (glb->brwLst[glb->currentBrw].brwMode == brwModeCard && glb->brwLst[glb->copy->destBrowser].brwMode == brwModeVfs) {
		if (glb->copy->single) {
			
			namePtr = MemHandleLock(glb->copy->destName);
			
			fullpath = MemPtrNew(StrLen(glb->brwLst[glb->copy->destBrowser].lst.data.vol.currentDir) + StrLen(namePtr) + 2);
			StrCopy(fullpath, glb->brwLst[glb->copy->destBrowser].lst.data.vol.currentDir);
			if (fullpath[StrLen(fullpath)-1] != '/') StrCat(fullpath, "/");
			StrCat(fullpath, namePtr);
			
			MemHandleUnlock(glb->copy->destName);
			
			i = 0;
			while (i < glb->brwLst[glb->currentBrw].lst.data.card.numDBs && !glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].selected) {
				i++;
			}
			
			// this should not happen
			ErrFatalDisplayIf(i >= glb->brwLst[glb->currentBrw].lst.data.card.numDBs, "ERROR: can't find selected database!");
			
			if ((retVal = VFSExportDatabaseToFile(glb->brwLst[glb->copy->destBrowser].lst.data.vol.volInfo.volRefNum, fullpath, glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].cardNo, 
					glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].lID)) != errNone) {
			}
			
			MemPtrFree(fullpath);
		}
		else {
			i = 0;
			i2 = 0;
			
			while (i < glb->brwLst[glb->currentBrw].lst.data.card.numDBs && i2 < glb->brwLst[glb->currentBrw].lst.data.card.numSelDBs) {
				if (glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].selected) {
					
					fullpath = MemPtrNew(StrLen(glb->brwLst[glb->copy->destBrowser].lst.data.vol.currentDir) + StrLen(glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].name) + 2);
					StrCopy(fullpath, glb->brwLst[glb->copy->destBrowser].lst.data.vol.currentDir);
					if (fullpath[StrLen(fullpath)-1] != '/') StrCat(fullpath, "/");
					StrCat(fullpath, glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].name);
					
					if ((retVal = VFSExportDatabaseToFile(glb->brwLst[glb->copy->destBrowser].lst.data.vol.volInfo.volRefNum, fullpath, glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].cardNo, 
							glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].lID)) != errNone) {
						
						// error
						
						break;
					}
					else {
						MemPtrFree(fullpath);
						fullpath = NULL;
					}
					
					i2++;
				}
				
				// next db
				i++;
			}
			
			if (fullpath != NULL) MemPtrFree(fullpath);
		}
		
		if (retVal == errNone) {
			// scan dir
			getFilesInDir(&glb->brwLst[glb->copy->destBrowser].lst.data.vol);
			// sort the dbs/files
			sort(&glb->brwLst[glb->copy->destBrowser]);
		}
	}
	else if (glb->brwLst[glb->currentBrw].brwMode == brwModeVfs && glb->brwLst[glb->copy->destBrowser].brwMode == brwModeCard) {
		if (glb->copy->single) {
			
			i = 0;
			while (i < glb->brwLst[glb->currentBrw].lst.data.vol.numFiles && !glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].selected) {
				i++;
			}
			
			// hope this never ever happens
			ErrFatalDisplayIf(i >= glb->brwLst[glb->currentBrw].lst.data.vol.numFiles, "ERROR: can't find selected file!");
			
			fullpath = MemPtrNew(StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.currentDir) + StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name) + 2);
			StrCopy(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.currentDir);
			if (fullpath[StrLen(fullpath)-1] != '/') StrCat(fullpath, "/");
			StrCat(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name);
			
			if ((retVal = VFSImportDatabaseFromFile(glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, fullpath, &cardNo, &lID)) != errNone) {
				// nothing to do
			}
			
			MemPtrFree(fullpath);
		}
		else {
			i = 0;
			i2 = 0;
			
			while (i < glb->brwLst[glb->currentBrw].lst.data.vol.numFiles && i2 < glb->brwLst[glb->currentBrw].lst.data.vol.numSelFiles) {
				if (glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].selected) {
										
					fullpath = MemPtrNew(StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.currentDir) + StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name) + 2);
					StrCopy(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.currentDir);
					if (fullpath[StrLen(fullpath)-1] != '/') StrCat(fullpath, "/");
					StrCat(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name);
					
					if ((retVal = VFSImportDatabaseFromFile(glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, fullpath, &cardNo, &lID)) != errNone) {
						
						break;
					}
					else {
						MemPtrFree(fullpath);
						fullpath = NULL;
					}
					
					i2++;
				}
				// next file
				i++;
			}
			
			if (fullpath != NULL) MemPtrFree(fullpath);
		}
		
		if (retVal == errNone) {
			// get all db's
			getDBList(&glb->brwLst[glb->copy->destBrowser].lst.data.card);
			// sort the dbs/files
			sort(&glb->brwLst[glb->copy->destBrowser]);
		}
	}
	else if (glb->brwLst[glb->currentBrw].brwMode == brwModeVfs && glb->brwLst[glb->copy->destBrowser].brwMode == brwModeVfs) {
		if (glb->copy->single) {
						
			i = 0;
			while (i < glb->brwLst[glb->currentBrw].lst.data.vol.numFiles && !glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].selected) {
				i++;
			}
			
			// this should never happen !!!
			ErrFatalDisplayIf(i >= glb->brwLst[glb->currentBrw].lst.data.vol.numFiles, "ERROR: can't find selected file!");
			
			// source path
			fullpath = MemPtrNew(StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.currentDir) + StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name) + 2);
			StrCopy(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.currentDir);
			if (fullpath[StrLen(fullpath)-1] != '/') StrCat(fullpath, "/");
			StrCat(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name);
						
			// make destination fullpath (in fullpath2)
			namePtr = MemHandleLock(glb->copy->destName);
			fullpath2 = MemPtrNew(StrLen(glb->brwLst[glb->copy->destBrowser].lst.data.vol.currentDir) + StrLen(namePtr) + 2);
			StrCopy(fullpath2, glb->brwLst[glb->copy->destBrowser].lst.data.vol.currentDir);
			if (fullpath2[StrLen(fullpath2)-1] != '/') StrCat(fullpath2, "/");
			StrCat(fullpath2, namePtr);
			
			if ((glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].attribs & vfsFileAttrDirectory) == vfsFileAttrDirectory) {				
				retVal = VFSDirCreate(glb->brwLst[glb->copy->destBrowser].lst.data.vol.volInfo.volRefNum, fullpath2);
				if (retVal == errNone) {
					retVal = copyVfsDirToVfs(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, fullpath2, 
							glb->brwLst[glb->copy->destBrowser].lst.data.vol.volInfo.volRefNum);
				}
			}
			else {
				// copy file
				retVal = copyVfsToVfs(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].size,
					fullpath2, glb->brwLst[glb->copy->destBrowser].lst.data.vol.volInfo.volRefNum, false);
			
				if (retVal == 4) {
					// ask for overwrite
					retVal = FrmCustomAlert(alertID_volOverwriteQuestion, namePtr, NULL, NULL);
				
					if (retVal == 0) {	// yes to overwrite
						retVal = copyVfsToVfs(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].size,
						fullpath2, glb->brwLst[glb->copy->destBrowser].lst.data.vol.volInfo.volRefNum, true);
					}
				}
			}
			
			if (fullpath != NULL) MemPtrFree(fullpath);
			if (fullpath2 != NULL) MemPtrFree(fullpath2);
			
			MemHandleUnlock(glb->copy->destName);
		}
		else {
			i = 0;
			i2 = 0;
			while (i < glb->brwLst[glb->currentBrw].lst.data.vol.numFiles && i2 < glb->brwLst[glb->currentBrw].lst.data.vol.numSelFiles) {
				if (glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].selected) {
				
					if (fullpath != NULL) MemPtrFree(fullpath);
					if (fullpath2 != NULL) MemPtrFree(fullpath2);
				
					// source path
					fullpath = MemPtrNew(StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.currentDir) + StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name) + 2);
					StrCopy(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.currentDir);
					if (fullpath[StrLen(fullpath)-1] != '/') StrCat(fullpath, "/");
					StrCat(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name);
			
					// make destination fullpath (in fullpath2)
					fullpath2 = MemPtrNew(StrLen(glb->brwLst[glb->copy->destBrowser].lst.data.vol.currentDir) + StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name) + 2);
					StrCopy(fullpath2, glb->brwLst[glb->copy->destBrowser].lst.data.vol.currentDir);
					if (fullpath2[StrLen(fullpath2)-1] != '/') StrCat(fullpath2, "/");
					StrCat(fullpath2, glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name);
					
					if ((glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].attribs & vfsFileAttrDirectory) == vfsFileAttrDirectory) {
					
						retVal = VFSDirCreate(glb->brwLst[glb->copy->destBrowser].lst.data.vol.volInfo.volRefNum, fullpath2);
						if (retVal == errNone) {
							retVal = copyVfsDirToVfs(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, fullpath2, 
									glb->brwLst[glb->copy->destBrowser].lst.data.vol.volInfo.volRefNum);
						
							if (retVal != errNone) {
								break;
							}
						}
						else {
							break;
						}
					}
					else {			
						// copy file
						retVal = copyVfsToVfs(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].size,
							fullpath2, glb->brwLst[glb->copy->destBrowser].lst.data.vol.volInfo.volRefNum, false);
			
						if (retVal == 4) {
							// ask for overwrite
							retVal = FrmCustomAlert(alertID_volOverwriteQuestion, glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name, NULL, NULL);
				
							if (retVal == 0) {	// yes to overwrite
								retVal = copyVfsToVfs(fullpath, glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].size,
								fullpath2, glb->brwLst[glb->copy->destBrowser].lst.data.vol.volInfo.volRefNum, true);
								
								if (retVal != errNone && retVal != 4) {
									break;
								}
							}
						}
						else if (retVal != errNone) {
							break;
						}
					}
				
					// inc number of files that have been copied
					i2++;
				}
			
				// next file
				i++;
			}
		
			if (fullpath != NULL) MemPtrFree(fullpath);
			if (fullpath2 != NULL) MemPtrFree(fullpath2);
		}
		
		if (retVal == errNone) {
			// scan dir
			getFilesInDir(&glb->brwLst[glb->copy->destBrowser].lst.data.vol);
			// sort the dbs/files
			sort(&glb->brwLst[glb->copy->destBrowser]);
		}
	}
	
	return(retVal);
}

/*
 *	copy a file from vfs to vfs
 * - doesn't work with directory's !
 * - set srcSize to 0 NULL if you don't know the size
 * 
 * errors: 1 src = dest, 2 src error, 3 dest error, 4 dest all ready exists
 */
Err copyVfsToVfs(char *src, UInt16 srcRefNum, UInt32 srcSize, char *dest, UInt16 destRefNum, Boolean overwrite)
{
	FileRef in_fp, out_fp;
	Err error;
	unsigned char *buffer;
	UInt32 readBytes, writeBytes;
	
	
	// src and dest are the same !
	if (StrCompare(src, dest) == 0 && srcRefNum == destRefNum) return(1);
	
	error = VFSFileCreate(destRefNum, dest);
	
	if (error == errNone || (error == vfsErrFileAlreadyExists && overwrite)) {
		
		// if file exists delete it !
		if (error == vfsErrFileAlreadyExists && overwrite) {
			VFSFileDelete(destRefNum, dest);
			VFSFileCreate(destRefNum, dest);
		}
		
		// open file
		error = VFSFileOpen(destRefNum, dest, vfsModeReadWrite, &out_fp);
		if (error == errNone) {
			
			// just in case - clear all attributes of destination
			VFSFileSetAttributes(out_fp, 0);
			
			// open source file
			error = VFSFileOpen(srcRefNum, src, vfsModeRead, &in_fp);
			
			if (error == errNone) {
				
				// get source size
				if (srcSize == 0) VFSFileSize(in_fp, &srcSize);
				
				// try to buffer the complete file in memory
				do {
					buffer = MemPtrNew(srcSize);
					srcSize /= 2;
				} while (buffer == NULL);
				
				// copy bytes
				do {
					VFSFileRead(in_fp, srcSize, buffer, &readBytes);
					error = VFSFileWrite(out_fp, readBytes, buffer, &writeBytes);
				} while (readBytes == srcSize || error == vfsErrVolumeFull);
				
				// free the read/write buffer
				MemPtrFree(buffer);
				
				// close in/out file
				VFSFileClose(in_fp);
				VFSFileClose(out_fp);
			}
			else {
				// can't open source
				error = 2;
				
				// close dest
				VFSFileClose(out_fp);
			}
		}
		else {
			
			{
				char test[20];
				StrIToA(test, error);
				WinDrawChars(test, StrLen(test), 0, 10);
				SysTaskDelay(500);
			}
			
			// can't open destination
			error = 3;
		}
	}
	else {
		// destination all ready exists and overwrite is set to FALSE
		error = 4;
	}
	
	return(error);
}

/*
 *		copy the content of the directory "src" to "dest"
 *		+ also copies sub directory's
 */
Err copyVfsDirToVfs(char *src, UInt16 srcRefNum, char *dest, UInt16 destRefNum)
{
	FileRef fp;
	UInt32 iterator;
	FileInfoType info;
	char *filepath = NULL;
	char *filepath2 = NULL;
	char fileName[256];
	Err retVal;
	
	
	info.nameP = fileName;
	info.nameBufLen = 255;
		
	if (VFSFileOpen(srcRefNum, src, vfsModeRead, &fp) == errNone) {
		iterator = vfsIteratorStart;

		while (iterator != vfsIteratorStop) {

			if (VFSDirEntryEnumerate(fp, &iterator, &info) == errNone) {
				
				// make new filepath src
				if (filepath != NULL) MemPtrFree(filepath);
				filepath = MemPtrNew(StrLen(src) + StrLen(info.nameP) + 2);
				StrCopy(filepath, src);
				if (filepath[StrLen(filepath)-1] != '/') StrCat(filepath, "/");
				StrCat(filepath, info.nameP);
				
				// make new filepath2 dest
				if (filepath2 != NULL) MemPtrFree(filepath2);
				filepath2 = MemPtrNew(StrLen(dest) + StrLen(info.nameP) + 2);
				StrCopy(filepath2, dest);
				if (filepath2[StrLen(filepath2)-1] != '/') StrCat(filepath2, "/");
				StrCat(filepath2, info.nameP);
				
				// this is a directory
				if ((info.attributes & vfsFileAttrDirectory) == vfsFileAttrDirectory) {
								
					retVal = VFSDirCreate(destRefNum, filepath2);
					
					if (retVal == errNone) {
						retVal = copyVfsDirToVfs(filepath, srcRefNum, filepath2, destRefNum);
						if (retVal != errNone) {
							VFSFileClose(fp);
							if (filepath != NULL) MemPtrFree(filepath);
							if (filepath2 != NULL) MemPtrFree(filepath2);
							return(retVal);
						}
					}
					else {
						VFSFileClose(fp);
						if (filepath != NULL) MemPtrFree(filepath);
						if (filepath2 != NULL) MemPtrFree(filepath2);
						return(retVal);
					}
				}
				else {					
					retVal = copyVfsToVfs(filepath, srcRefNum, 0, filepath2, destRefNum, false);
					
					if (retVal == 4) {
						// ask for overwrite
						retVal = FrmCustomAlert(alertID_volOverwriteQuestion, info.nameP, NULL, NULL);
						
						if (retVal == 0) {	
							retVal = copyVfsToVfs(filepath, srcRefNum, 0, filepath2, destRefNum, true);
						}
					}
					else if (retVal != errNone) {
						VFSFileClose(fp);
						if (filepath != NULL) MemPtrFree(filepath);
						if (filepath2 != NULL) MemPtrFree(filepath2);
						return(retVal);
					}
				}
			}
		} // while
		// close dir
		VFSFileClose(fp);
	}
	
	if (filepath != NULL) MemPtrFree(filepath);
	if (filepath2 != NULL) MemPtrFree(filepath2);
	
	return(errNone);
}
