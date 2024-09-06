/*
 *	file:		fm_9-files.c
 *	project:	GentleMan
 *	content:	file stuff
 *	updated:	Oct. 03. 2001
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
 *	select / UnSelect all files
 */
void doUnSelect(brwLstType *brw, Boolean select)
{
	UInt32 i;
	
	
	if (brw->brwMode == brwModeVfs) {
		brw->lst.data.vol.numSelFiles = 0;
		brw->lst.data.vol.sizeSelFiles = 0;
		
		i = 0;
		while (i < brw->lst.data.vol.numFiles) {
			brw->lst.data.vol.fileList[i].selected = select;
			brw->lst.data.vol.numSelFiles++;
			brw->lst.data.vol.sizeSelFiles += brw->lst.data.vol.fileList[i].size;
			i++;
		}
		if (select) {
			if (StrCompare(brw->lst.data.vol.fileList[0].name, ".") == 0) brw->lst.data.vol.fileList[0].selected = false;
			if (1 < brw->lst.data.vol.numFiles) {
				if (StrCompare(brw->lst.data.vol.fileList[1].name, "..") == 0) brw->lst.data.vol.fileList[1].selected = false;
			}
		}
		else {
			brw->lst.data.vol.numSelFiles = 0;
			brw->lst.data.vol.sizeSelFiles = 0;
		}
	}
	else if (brw->brwMode == brwModeCard) {
		brw->lst.data.card.numSelDBs = 0;
		brw->lst.data.card.sizeSelDBs = 0;
		
		i = 0;
		while (i < brw->lst.data.card.numDBs) {
			brw->lst.data.card.dbList[i].selected = select;
			brw->lst.data.card.numSelDBs++;
			brw->lst.data.card.sizeSelDBs += brw->lst.data.card.dbList[i].size;
			i++;
		}
		if (!select) {
			brw->lst.data.card.numSelDBs = 0;
			brw->lst.data.card.sizeSelDBs = 0;
		}
	}	
}

/*
 *	make a new directory !
 */
Err doMkDir(global_data_type *glb)
{
	char *fullname;
	char *textPtr;
	Err retVal = 45;	// just something that is !errNone
	
	
	// volume is readonly !
	if ((glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volInfo.attributes & vfsVolumeAttrReadOnly) == vfsVolumeAttrReadOnly) {
		return(1);
	}
	
	textPtr = FldGetTextPtr(GetObjectPtr(name_mkdir_fld));
	if (textPtr != NULL) {
		
		if (StrLen(textPtr) > 0) {
		
			fullname = MemPtrNew(StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.currentDir) + StrLen(textPtr) + 2);
		
			StrCopy(fullname, glb->brwLst[glb->currentBrw].lst.data.vol.currentDir);
			if (fullname[StrLen(fullname)-1] != '/') StrCat(fullname, "/");
			StrCat(fullname, textPtr);
		
			retVal = VFSDirCreate(glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, fullname);
			
			if (fullname != NULL) MemPtrFree(fullname);
		}
		else {
			// dir name length == 0
			return(2);
		}
	}
	else {
		// dir name length == 0
		return(2);
	}
	
	if (retVal == errNone) {
		// scan the directory
		getFilesInDir(&glb->brwLst[glb->currentBrw].lst.data.vol);
		// resort files
		sort(&glb->brwLst[glb->currentBrw]);
	}		
	
	return(retVal);
}

/*
 *	delete the selected files/directory's/databases
 * - accesses globals
 */
void doDelete(brwLstType *brw, prefsType *prefs)
{
	Err error;
	
	
	error = deleteStuff(brw, prefs);
	
	if (brw->brwMode == brwModeVfs) {
		
		switch (error) {
		case	errNone:
			// refresh volume list
			refreshVolumeInfo(&glob->vLst);
			// scan dir
			getFilesInDir(&brw->lst.data.vol);
			// sort the files
			sort(brw);
			break;
			
		case	1:
			FrmAlert(alertID_volReadOnly);
			break;
		case	vfsErrFilePermissionDenied:
			FrmAlert(alertID_volPerms);
			break;
		case	vfsErrFileGeneric:
		default:
			FrmAlert(alertID_volUnknownError);
			break;
		}
	}
	else if (brw->brwMode == brwModeCard) {
		
		switch (error) {
		case	errNone:
			// get all db's
			getDBList(&brw->lst.data.card);
			// sort the dbs
			sort(brw);
			break;
			
		case	1:
		case	dmErrROMBased:
			FrmAlert(alertID_cardDBRomBased);
			break;
		}
	}
}

/*
 *	delete the selected files/directory's/databases
 */
Err deleteStuff(brwLstType *brw, prefsType *prefs)
{
	Err retVal;
	FileRef fp2;
	UInt32 i;
	char *fullname;
	char numStr[20];
		
		
	if (brw->brwMode == brwModeVfs) {
		
		if ((brw->lst.data.vol.volInfo.volInfo.attributes & vfsVolumeAttrReadOnly) == vfsVolumeAttrReadOnly) {
			// card is readOnly
			return(1);
		}
		else {		
			if (brw->lst.data.vol.numSelFiles == 1) {
				i = 0;
				while (i < brw->lst.data.vol.numFiles && !brw->lst.data.vol.fileList[i].selected) {
					i++;
				}
				if (i < brw->lst.data.vol.numFiles) {
					if (StrCompare(".", brw->lst.data.vol.fileList[i].name) != 0 && StrCompare("..", brw->lst.data.vol.fileList[i].name) != 0) {
						
						// delete if ask is of or if the user taped YES !
						if ((prefs->askDelete && FrmCustomAlert(alertID_volDeleteFile, brw->lst.data.vol.fileList[i].name, NULL, NULL) == 0) || !prefs->askDelete) {
							
							fullname = MemPtrNew(StrLen(brw->lst.data.vol.currentDir) + StrLen(brw->lst.data.vol.fileList[i].name) + 2);
		
							StrCopy(fullname, brw->lst.data.vol.currentDir);
							if (fullname[StrLen(fullname)-1] != '/') StrCat(fullname, "/");
							StrCat(fullname, brw->lst.data.vol.fileList[i].name);
							
							// delete content of directory first!
							if ((brw->lst.data.vol.fileList[i].attribs & vfsFileAttrDirectory) == vfsFileAttrDirectory) {
								doDeleteDir(fullname, brw->lst.data.vol.volInfo.volRefNum);
							}
							
							// clear readOnly flag !
							if ((brw->lst.data.vol.fileList[i].attribs & vfsFileAttrReadOnly) == vfsFileAttrReadOnly) {
								if ((retVal = VFSFileOpen(brw->lst.data.vol.volInfo.volRefNum, fullname, vfsModeReadWrite, &fp2)) == errNone) {
									
									// clear all attributes
									if ((retVal = VFSFileSetAttributes(fp2, 0)) != errNone) {
										if (FrmCustomAlert(alertID_volCantRemoveRO, brw->lst.data.vol.fileList[i].name, NULL, NULL) != 0) {
											// user doesn't want to continue
											
											if (fullname != NULL) MemPtrFree(fullname);
											VFSFileClose(fp2);
											
											// but we don't return an error!
											return(errNone);
										}
									}
						
									VFSFileClose(fp2);
								}
								else {
									if (fullname != NULL) MemPtrFree(fullname);
									return(retVal);
								}
							}
							
							// try to delete the file !
							retVal = VFSFileDelete(brw->lst.data.vol.volInfo.volRefNum, fullname);

							if (fullname != NULL) MemPtrFree(fullname);
						}
					}
				}
			}
			else if (brw->lst.data.vol.numSelFiles > 1) {
				if (prefs->askDeleteOnlyOnce) {
					StrIToA(numStr, brw->lst.data.vol.numSelFiles);
					
					// just jump out if user presses "No"
					if (FrmCustomAlert(alertID_volDeleteFiles, numStr, NULL, NULL) != 0) return(0);
				}
				
				// start with 2nd file !
				i = 1;
				while (i < brw->lst.data.vol.numFiles) {
					if (brw->lst.data.vol.fileList[i].selected) {
						// don't allow "." or ".."
						if (StrCompare(".", brw->lst.data.vol.fileList[i].name) != 0 && StrCompare("..", brw->lst.data.vol.fileList[i].name) != 0) {

							// if askOnlyOnce OR if user taps "YES"
							if (prefs->askDeleteOnlyOnce || (prefs->askDelete && FrmCustomAlert(alertID_volDeleteFile, brw->lst.data.vol.fileList[i].name, NULL, NULL) == 0)) {
								
								fullname = MemPtrNew(StrLen(brw->lst.data.vol.currentDir) + StrLen(brw->lst.data.vol.fileList[i].name) + 2);
								StrCopy(fullname, brw->lst.data.vol.currentDir);
								if (fullname[StrLen(fullname)-1] != '/') StrCat(fullname, "/");
								StrCat(fullname, brw->lst.data.vol.fileList[i].name);
							
								// delete content of directory first!
								if ((brw->lst.data.vol.fileList[i].attribs & vfsFileAttrDirectory) == vfsFileAttrDirectory) {
									if ((retVal = doDeleteDir(fullname, brw->lst.data.vol.volInfo.volRefNum)) != errNone) {
										// directory can't be deleted
										
										if (fullname != NULL) MemPtrFree(fullname);
										return(retVal);
									}
								}
							
								// clear readOnly flag !
								if ((brw->lst.data.vol.fileList[i].attribs & vfsFileAttrReadOnly) == vfsFileAttrReadOnly) {
									if ((retVal = VFSFileOpen(brw->lst.data.vol.volInfo.volRefNum, fullname, vfsModeReadWrite, &fp2)) == errNone) {
										
										// clear all attributes
										if ((retVal = VFSFileSetAttributes(fp2, 0)) != errNone) {
											if (FrmCustomAlert(alertID_volCantRemoveRO, brw->lst.data.vol.fileList[i].name, NULL, NULL) != 0) {
												// user doesn't want to continue
											
												if (fullname != NULL) MemPtrFree(fullname);
												VFSFileClose(fp2);
											
												// but we don't return an error!
												return(errNone);
											}
										}
												
										VFSFileClose(fp2);
									}
									else {
										if (fullname != NULL) MemPtrFree(fullname);
										return(retVal);
									}
								}
								
								// try to delete 
								VFSFileDelete(brw->lst.data.vol.volInfo.volRefNum, fullname);
							
								if (fullname != NULL) MemPtrFree(fullname);
							}
						}
					}
					// next file !
					i++;
				}
			}
			else {
				// error no files selected
				FrmAlert(alertID_volNoSelFile);
			}
		}
	}
	else if (brw->brwMode == brwModeCard) {
		if (brw->lst.data.card.cardInfo.typeId == cardTypeRam) {
			
			if (brw->lst.data.card.numSelDBs == 1) {
				i = 0;
				while (i < brw->lst.data.card.numDBs && !brw->lst.data.card.dbList[i].selected) {
					i++;
				}
				if (i < brw->lst.data.card.numDBs) {
									
					// delete if ask is of or if the user taped YES !
					if ((prefs->askDelete && FrmCustomAlert(alertID_volDeleteFile, brw->lst.data.card.dbList[i].name, NULL, NULL) == 0) || !prefs->askDelete) {
						
						// clear all attribs	
						if ((brw->lst.data.card.dbList[i].attribs & dmHdrAttrReadOnly) == dmHdrAttrReadOnly) {
							DmSetDatabaseInfo(brw->lst.data.card.dbList[i].cardNo, brw->lst.data.card.dbList[i].lID, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
						}
											
						if ((retVal = DmDeleteDatabase(brw->lst.data.card.dbList[i].cardNo, brw->lst.data.card.dbList[i].lID)) != errNone) {
							return(retVal);
						}
					}
				}
			}
			else if (brw->lst.data.card.numSelDBs > 1) {
				if (prefs->askDeleteOnlyOnce) {
					StrIToA(numStr, brw->lst.data.card.numSelDBs);
					
					// just jump out if user presses "No"
					if (FrmCustomAlert(alertID_cardDeleteFiles, numStr, NULL, NULL) != 0) return(0);
				}
				
				// start with 2nd file !
				i = 1;
				while (i < brw->lst.data.card.numDBs) {
					if (brw->lst.data.card.dbList[i].selected) {

						// if askOnlyOnce OR if user taps "YES"
						if (prefs->askDeleteOnlyOnce || (prefs->askDelete && FrmCustomAlert(alertID_volDeleteFile, brw->lst.data.card.dbList[i].name, NULL, NULL) == 0)) {
							
							// clear all attribs	
							if ((brw->lst.data.card.dbList[i].attribs & dmHdrAttrReadOnly) == dmHdrAttrReadOnly) {
								DmSetDatabaseInfo(brw->lst.data.card.dbList[i].cardNo, brw->lst.data.card.dbList[i].lID, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
							}
											
							if ((retVal = DmDeleteDatabase(brw->lst.data.card.dbList[i].cardNo, brw->lst.data.card.dbList[i].lID)) != errNone) {
								return(retVal);
							}		
						}
					}
					// next file !
					i++;
				}
			}
			else {
				// error no selected databases found
				FrmAlert(alertID_cardNoSelDB);
			}
		}
		else if (brw->lst.data.card.cardInfo.typeId == cardTypeRom) {
			return(1);
		}
	}
	
	return(errNone);
}

/*
 *	delete everything below "pathname" ! "pathname" will NOT be deleted !
 */
Err doDeleteDir(char *pathname, UInt16 volRefNum)
{
	FileRef fp, fp2;
	UInt32 iterator;
	FileInfoType info;
	char *filepath = NULL;
	char fileName[256];
	Err retVal;
	
	
	info.nameP = fileName;
	info.nameBufLen = 255;
	
	if (VFSFileOpen(volRefNum, pathname, vfsModeRead, &fp) == errNone) {
		iterator = vfsIteratorStart;

		while (iterator != vfsIteratorStop) {

			if (VFSDirEntryEnumerate(fp, &iterator, &info) == errNone) {

				if (filepath != NULL) MemPtrFree(filepath);
				filepath = MemPtrNew(StrLen(pathname) + StrLen(info.nameP) + 2);
				StrCopy(filepath, pathname);
				if (filepath[StrLen(filepath)-1] != '/') StrCat(filepath, "/");
				StrCat(filepath, info.nameP);
				
				if ((info.attributes & vfsFileAttrDirectory) == vfsFileAttrDirectory) {
					if ((retVal = doDeleteDir(filepath, volRefNum)) != errNone) {
						VFSFileClose(fp);
						MemPtrFree(filepath);
						return(retVal);
					}
				}
				
				if ((info.attributes & vfsFileAttrReadOnly) == vfsFileAttrReadOnly) {
					if (VFSFileOpen(volRefNum, filepath, vfsModeReadWrite, &fp2) == errNone) {
						// clear all attributes
						retVal = VFSFileSetAttributes(fp2, 0);
						VFSFileClose(fp2);
						if (retVal != errNone) {	// looks like we can't change the files RW/RO bit
							VFSFileClose(fp);
							MemPtrFree(filepath);
							return(retVal);
						}
					}
				}
				
				// delete file or dir.
				if ((retVal = VFSFileDelete(volRefNum, filepath)) != errNone) {
					VFSFileClose(fp);
					MemPtrFree(filepath);
					return(retVal);
				}
				else {
					// reinit iterator, otherwise we don't find the next files
					iterator = vfsIteratorStart;
				}
			}
		} // while
		// close dir
		VFSFileClose(fp);
	}
	
	if (filepath != NULL) MemPtrFree(filepath);
	
	return(errNone);
}

/*
 *	beam the selected database or file
 * - errors will be handled internly
 */
void doBeam(brwLstType *brw)
{
	char *fullpath;
	UInt32 i;
	//Err error;
	
	
	if (brw->brwMode == brwModeVfs) {
		if (brw->lst.data.vol.numSelFiles == 0) {
			FrmAlert(alertID_volNoSelFile);
		}
		else if (brw->lst.data.vol.numSelFiles > 1) {
			FrmAlert(alertID_volSingleFileOnly);
		}
		else if (brw->lst.data.vol.numSelFiles == 1) {
			i = 0;
			while (i < brw->lst.data.vol.numFiles && !brw->lst.data.vol.fileList[i].selected) {
				i++;
			}
			
			fullpath = MemPtrNew(StrLen(brw->lst.data.vol.currentDir) + StrLen(brw->lst.data.vol.fileList[i].name) + 2);
			StrCopy(fullpath, brw->lst.data.vol.currentDir);
			if (fullpath[StrLen(fullpath)-1] != '/') StrCat(fullpath, "/");
			StrCat(fullpath, brw->lst.data.vol.fileList[i].name);
			
			/*error =*/ beamFile(brw->lst.data.vol.volInfo.volRefNum, fullpath, brw->lst.data.vol.fileList[i].name, brw->lst.data.vol.fileList[i].size);
			
			MemPtrFree(fullpath);
		}		
	}
	else if (brw->brwMode == brwModeCard) {
		if (brw->lst.data.card.numSelDBs == 0) {
			FrmAlert(alertID_cardNoSelDB);
		}
		else if (brw->lst.data.card.numSelDBs > 1) {
			FrmAlert(alertID_cardSingleDBOnly);
		}
		else if (brw->lst.data.card.numSelDBs == 1) {
			i = 0;
			while (i < brw->lst.data.card.numDBs && !brw->lst.data.card.dbList[i].selected) {
				i++;
			}
			
			fullpath = MemPtrNew(StrLen(brw->lst.data.card.dbList[i].name) + 5);
			if ((brw->lst.data.card.dbList[i].attribs & dmHdrAttrResDB) == dmHdrAttrResDB) StrCat(fullpath, ".prc");
			else StrCat(fullpath, ".pdb");
			
			/*error =*/ beamDB(brw->lst.data.card.dbList[i].cardNo, brw->lst.data.card.dbList[i].lID, fullpath, brw->lst.data.card.dbList[i].name);

			MemPtrFree(fullpath);			
		}
	}
}

/*
 *	ExgDbWrite callback
 */
Err myDBWrite(const void* data, UInt32 *size, void *userdata)
{
	Err error;
	
	
	//CALLBACK_PROLOGUE
	
	*size = ExgSend((ExgSocketPtr) userdata, (void*) data, *size, &error);
	
	//CALLBACK_EPILOGUE
	
	return(error);
}

/*
 *	just beam the database identified by cNo and lID use name as the name and description
 */
Err beamDB(UInt16 cNo, LocalID lID, char *name, char *displayname)
{	
	ExgSocketType eSock;
	Err error; //, error2;
	
	
	MemSet(&eSock, sizeof(eSock), 0);
	eSock.description = displayname;
	eSock.name = name;
	
	error = ExgPut(&eSock);
	if (!error) {
		/*error2 =*/ ExgDBWrite(myDBWrite, &eSock, NULL, lID, cNo);
		error = ExgDisconnect(&eSock, error);
	}
	
	return(error);
}

/*
 *	beam a file
 */
Err beamFile(UInt16 refNum, char *fullpath, char *name, UInt32 size)
{
	ExgSocketType eSock;
	Err error, error2;
	FileRef fp;
	UInt32 readBytes, sendLength, transLength;
	unsigned char *buffer, *bufferP;
	
	
	MemSet(&eSock, sizeof(eSock), 0);
	eSock.description = name;
	eSock.name = name;
	
	error = ExgPut(&eSock);
	if (!error) {
		
		if ((error2 = VFSFileOpen(refNum, fullpath, vfsModeRead, &fp)) == errNone) {	
			
			while ((buffer = MemPtrNew(size)) == NULL) {
				size = size / 2;
			}
						
			do {
				error2 = VFSFileRead(fp, size, buffer, &readBytes);

				sendLength = readBytes;
				transLength = 0;
				bufferP = buffer;
				
				do {
					sendLength = sendLength - transLength;
					bufferP = bufferP + transLength;
					transLength = ExgSend(&eSock, bufferP, sendLength, &error);					
				} while (error == errNone && transLength < sendLength);
				
			} while (error == errNone && error2 != vfsErrFileEOF);
			
			VFSFileClose(fp);
			
			MemPtrFree(buffer);
		}
		
		error = ExgDisconnect(&eSock, error);
	}
	
	return(error);
}
