/*
 *	file:		fm_2-file_db.c
 *	project:	GentleMan
 *	content:	volumes, file, database lists
 *	updated:	Oct. 29. 2001
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
 *	make file list for current dir on current volume
 */
UInt8 getFilesInDir(fileListType *flst)
{
	FileInfoType info;
	FileRef fp, fp2;
	UInt32 iterator, i;
	char fileName[256];
	void *tmpPtr;
	char *filepath;
	UInt16 num, num2;
	WinHandle backGround;

	
	// save bg
	inProgress(0, 1, &backGround, glob->cfg.screenState, "scanning...");
	
	num = 0;
	// show first image
	inProgress(num, 0, &backGround, glob->cfg.screenState, NULL);
	
	// clear values
	flst->numSelFiles = 0;
	flst->sizeSelFiles = 0;
	
	// check if list pointer had memory
	if (flst->fileList != NULL) {
		MemPtrFree(flst->fileList);
		flst->fileList = NULL;
	}

	// free fileNames memory
	if (flst->fileNames != NULL) {
		MemPtrFree(flst->fileNames);
		flst->fileNames = NULL;
		flst->fileNamesSize = 0;
	}
			
	// check for root of volume
	if (StrCompare(flst->currentDir, "/") != 0) {	// this is not the root dir
		flst->numFiles = 2;
		flst->fileList = MemPtrNew(sizeof(fileInfoType) * flst->numFiles);
		// clear memory !
		MemSet(flst->fileList, sizeof(fileInfoType) * flst->numFiles, 0);
		
		flst->fileNames = MemPtrNew(5);
		flst->fileNamesSize = 5;
		
		// relative offset in fileNames list
		flst->fileList[1].name = (char *)2;
		
		// own level above
		StrCopy(flst->fileNames + (UIntPtr)flst->fileList[1].name, "..");
		flst->fileList[1].attribs = vfsFileAttrDirectory;
		flst->fileList[1].selected = false;
		
		// init the date values for ".."
		flst->fileList[1].crDate = 0;
		flst->fileList[1].moDate = 0;
		flst->fileList[1].acDate = 0;
	}
	else {	// this is the root dir
		
		flst->fileNames = MemPtrNew(2);
		flst->fileNamesSize = 2;
		
		flst->numFiles = 1;
		flst->fileList = MemPtrNew(sizeof(fileInfoType) * flst->numFiles);
		// clear memory !
		MemSet(flst->fileList, sizeof(fileInfoType) * flst->numFiles, 0);
	}
		
	// relative offset in fileNames list
	flst->fileList[0].name = 0;
	
	// current dir
	StrCopy(flst->fileNames + (UIntPtr)flst->fileList[0].name, ".");
	flst->fileList[0].attribs = vfsFileAttrDirectory;
	flst->fileList[0].selected = false;
	
	info.nameP = fileName;
	info.nameBufLen = 255;
	
	// open directory for reading !
	if (VFSFileOpen(flst->volInfo.volRefNum, flst->currentDir, vfsModeRead, &fp) == errNone) {
	
		// get dir dates (for current dir)
		VFSFileGetDate(fp, vfsFileDateCreated, &flst->fileList[0].crDate);
		VFSFileGetDate(fp, vfsFileDateModified, &flst->fileList[0].moDate);
		VFSFileGetDate(fp, vfsFileDateAccessed, &flst->fileList[0].acDate);		

	/*	// use this in POSE
		flst->fileList[0].crDate += 2082844800;
		flst->fileList[0].moDate += 2082844800;
		flst->fileList[0].acDate += 2082844800;
	*/	

		// init progress counter		
		num2 = 0;
		
		iterator = vfsIteratorStart;
		while (iterator != vfsIteratorStop) {

			if (VFSDirEntryEnumerate(fp, &iterator, &info) == errNone) {
							
				if (MemPtrResize(flst->fileList, sizeof(fileInfoType) * (flst->numFiles+1)) != 0) {
					// get more memory for file list
					tmpPtr = flst->fileList;
					flst->fileList = MemPtrNew(sizeof(fileInfoType) * (flst->numFiles+1));

					MemMove(flst->fileList, tmpPtr, sizeof(fileInfoType) * flst->numFiles);
					MemPtrFree(tmpPtr);
				}
				// init structure with 0s
				MemSet(&flst->fileList[flst->numFiles], sizeof(fileInfoType), 0);
			
				if (MemPtrResize(flst->fileNames, flst->fileNamesSize + StrLen(info.nameP) + 1) != 0) {
					// save old ptr
					tmpPtr = flst->fileNames;
			
					// get more memory
					flst->fileNames = MemPtrNew(flst->fileNamesSize + StrLen(info.nameP) + 1);
						
					MemMove(flst->fileNames, tmpPtr, flst->fileNamesSize);
					MemPtrFree(tmpPtr);
				}
			
				ErrFatalDisplayIf(flst->fileList == NULL, "flst->fileList == NULL");
				ErrFatalDisplayIf(flst->fileNames == NULL, "flst->fileNames == NULL");
				
				// calc new rel. pointer
				flst->fileList[flst->numFiles].name = (char *)flst->fileNamesSize;
			
				// clear string
				MemSet(flst->fileNames + (UIntPtr)flst->fileList[flst->numFiles].name, (StrLen(info.nameP) + 1), 0);
				
				// copy file name
				StrCopy(flst->fileNames + (UIntPtr)flst->fileList[flst->numFiles].name, info.nameP);
			
				// inc filenamesLength
				flst->fileNamesSize += (StrLen(info.nameP) + 1);
			
				// copy attributes
				flst->fileList[flst->numFiles].attribs = info.attributes;
				// set the file to not selected
				flst->fileList[flst->numFiles].selected = false;
			
				// build file path (absolute)
				filepath = MemPtrNew(StrLen(flst->currentDir) + StrLen(info.nameP) + 2);
				StrCopy(filepath, flst->currentDir);
				if (filepath[StrLen(filepath)-1] != '/') StrCat(filepath, "/");
				StrCat(filepath, info.nameP);
			
				// open file for getting infos
				if (VFSFileOpen(flst->volInfo.volRefNum, filepath, vfsModeRead, &fp2) == errNone) {
					// get file dates
					VFSFileGetDate(fp2, vfsFileDateCreated, &flst->fileList[flst->numFiles].crDate);
					VFSFileGetDate(fp2, vfsFileDateModified, &flst->fileList[flst->numFiles].moDate);
					VFSFileGetDate(fp2, vfsFileDateAccessed, &flst->fileList[flst->numFiles].acDate);
				
	/*				// use this in POSE
					flst->fileList[flst->numFiles].crDate += 2082844800;
					flst->fileList[flst->numFiles].moDate += 2082844800;
					flst->fileList[flst->numFiles].acDate += 2082844800;
	*/			
					if ((flst->fileList[flst->numFiles].attribs & vfsFileAttrDirectory) != vfsFileAttrDirectory) {
						// get file size
						VFSFileSize(fp2, &flst->fileList[flst->numFiles].size);
					}
					else {
						flst->fileList[flst->numFiles].size = 0;
					}
					// close file
					VFSFileClose(fp2);
				}
				else {	// could not open file ! - set FAKE values !
					flst->fileList[flst->numFiles].crDate = 0;
					flst->fileList[flst->numFiles].moDate = 0;
					flst->fileList[flst->numFiles].acDate = 0;
				
					flst->fileList[flst->numFiles].size = 0;
				}
			
				// free filepath
				if (filepath != NULL) MemPtrFree(filepath);
			
				// inc file counter
				flst->numFiles++;
			
				// show scan progress
				num2++;
				if (num2 % 5 == 0) {
					num = (num + 1) % 8;
					// show first image
					inProgress(num, 0, &backGround, glob->cfg.screenState, NULL);
					num2 = 0;
				}
			}
		}
		// close dir
		VFSFileClose(fp);
		
		// now change relative pointers to absolute pointers
		for (i = 0; i < flst->numFiles; i++) {
			flst->fileList[i].name = flst->fileNames + (UIntPtr)flst->fileList[i].name;
		}
		
	} // open directory !

	// restore bg
	inProgress(num, 2, &backGround, glob->cfg.screenState, NULL);
	
	return(1);
}

/*
 *	get database list for the current card
 */
UInt8 getDBList(dbListType *dblst)
{
	UInt32 i, counter;
	UInt16 num, num2;
	WinHandle backGround;


	// save bg
	inProgress(0, 1, &backGround, glob->cfg.screenState, "scanning...");
	
	num = 0;
	// show first image
	inProgress(num, 0, &backGround, glob->cfg.screenState, NULL);
	
	// get number of dbs on card
	dblst->numDBs = DmNumDatabases(dblst->cardInfo.cardNo);

	// cleat values
	dblst->numSelDBs = 0;
	dblst->sizeSelDBs = 0;
	
	counter = 0;
	
	// free memory
	if (dblst->dbList != NULL) {
		MemPtrFree(dblst->dbList);
		dblst->dbList = NULL;
	}

	// get memory for db list
	dblst->dbList = MemPtrNew(sizeof(dbInfoType) * dblst->numDBs);

	ErrFatalDisplayIf(dblst->dbList == NULL, "dblst->dbList == NULL");
		
	for (i = 0; i < dblst->numDBs; i++) {
				
		// get LocalID
		dblst->dbList[counter].lID = DmGetDatabase(dblst->cardInfo.cardNo, i);
		
		if ((dblst->cardInfo.typeId == cardTypeRom && MemLocalIDKind(dblst->dbList[counter].lID) == memIDPtr) ||
			(dblst->cardInfo.typeId == cardTypeRam && MemLocalIDKind(dblst->dbList[counter].lID) == memIDHandle)) {
		
			// save cardNo
			dblst->dbList[counter].cardNo = dblst->cardInfo.cardNo;
			// set database to not selected
			dblst->dbList[counter].selected = false;
			
			// get all the other values
			DmDatabaseInfo(dblst->dbList[counter].cardNo, dblst->dbList[counter].lID, dblst->dbList[counter].name,
				&dblst->dbList[counter].attribs, &dblst->dbList[counter].version, &dblst->dbList[counter].crDate,
				&dblst->dbList[counter].moDate, &dblst->dbList[counter].acDate, &dblst->dbList[counter].modNum,
				NULL, NULL, &dblst->dbList[counter].typeId, &dblst->dbList[counter].crid);
				
			DmDatabaseSize(dblst->dbList[counter].cardNo, dblst->dbList[counter].lID, &dblst->dbList[counter].numRecs, &dblst->dbList[counter].size, NULL);
			
			// inc database counter
			counter++;
			
			// show scan progress
			num2++;
			if (num2 % 5 == 0) {
				num = (num + 1) % 8;
				// show first image
				inProgress(num, 0, &backGround, glob->cfg.screenState, NULL);
				num2 = 0;
			}
		}
	}
	
	// this is the real number of databases
	dblst->numDBs = counter;
	
	// resize the memory block so we don't waste memory
	MemPtrResize(dblst->dbList, dblst->numDBs * sizeof(dbInfoType));
	
	// restore bg
	inProgress(num, 2, &backGround, glob->cfg.screenState, NULL);
			
	return(1);
}

/*
 *	make a list with all data volumes (vfs and memcard)
 */
UInt8 makeVolList(volListType *lst)
{
	UInt32 iterator;
	void *tmpPtr;
	UInt16 numCards, i;
	UInt16 volRefNum;
	UInt32 tmpRamSize;
	UInt32 version;
	volListItemType tmpItem;
	

	// free list memory
	if (lst->list != NULL) {
		MemPtrFree(lst->list);
		lst->list = NULL;
	}
	
	// set list count to 0
	lst->numVols = 0;

	// ---== memory cards (rom/ram) ==---
	
	// number of memory cards in system
	numCards = MemNumCards();
	
	for (i = 0; i < numCards; i++) {
		if (MemCardInfo(i, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == errNone) {
		/*
			// get new memory - add 2 new volumes (1 => rom / 2 => ram)
			tmpPtr = lst->list;
			lst->list = MemPtrNew(sizeof(volListItemType) * (lst->numVols+2));
			if (tmpPtr != NULL) {
				MemMove(lst->list, tmpPtr, sizeof(volListItemType) * lst->numVols);
				MemPtrFree(tmpPtr);
			}
		*/
			
			// clear
			MemSet(&tmpItem, sizeof(volListItemType), 0);
	
			// ---== ROM ==---
	/*		lst->list[lst->numVols].mode = brwModeCard;
			lst->list[lst->numVols].data.card.typeId = cardTypeRom;
			lst->list[lst->numVols].data.card.cardNo = i;	// cardNo
	*/
			tmpItem.mode = brwModeCard;
			tmpItem.data.card.typeId = cardTypeRom;
			tmpItem.data.card.cardNo = i;
			
		/*	MemCardInfo(lst->list[lst->numVols].data.card.cardNo, lst->list[lst->numVols].data.card.label, 
				lst->list[lst->numVols].data.card.manuf, &lst->list[lst->numVols].data.card.version,
				&lst->list[lst->numVols].data.card.crDate, &lst->list[lst->numVols].data.card.ramSize,
				&tmpRamSize, &lst->list[lst->numVols].data.card.ramUsed);
		*/
			
			MemCardInfo(tmpItem.data.card.cardNo, tmpItem.data.card.label, 
				tmpItem.data.card.manuf, &tmpItem.data.card.version,
				&tmpItem.data.card.crDate, &tmpItem.data.card.ramSize,
				&tmpRamSize, &tmpItem.data.card.ramUsed);
			
			// check if this is the MemPlug Springboard
			if (StrNCompare(tmpItem.data.card.label, MEMPLUG_LABEL_STR, MEMPLUG_LABEL_LENG) == 0) continue;
			
			// get new memory - add 2 new volumes (1 => rom / 2 => ram)
			tmpPtr = lst->list;
			lst->list = MemPtrNew(sizeof(volListItemType) * (lst->numVols+2));
			if (tmpPtr != NULL) {
				MemMove(lst->list, tmpPtr, sizeof(volListItemType) * lst->numVols);
				MemPtrFree(tmpPtr);
			}
			
			// copy tmpItem to new list item
			MemMove(&lst->list[lst->numVols], &tmpItem, sizeof(volListItemType));
			
			// ---== RAM ==---
			// copy to ram entry
			MemMove(&lst->list[lst->numVols+1], &lst->list[lst->numVols], sizeof(volListItemType));
			lst->list[lst->numVols+1].data.card.typeId = cardTypeRam;
			
			// this is the RAM part so copy the ram size
			lst->list[lst->numVols+1].data.card.ramSize = tmpRamSize;
			
			// MemCardInfo returns the free byte count to we have to re calc the "used bytes"
			lst->list[lst->numVols+1].data.card.ramUsed = lst->list[lst->numVols+1].data.card.ramSize - lst->list[lst->numVols+1].data.card.ramUsed;

			// ---== ROM ==---
			// MemCardInfo returns the free byte count so we have to re calc the "used bytes"
			// we expect: every bytes in rom is used !
			lst->list[lst->numVols].data.card.ramUsed = lst->list[lst->numVols].data.card.ramSize;
			
			// inc volume count
			lst->numVols += 2;
		}
	}
	
	// ---== VFS volumes ==---

	// check for VFS Manager !!!
	if (FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version) == 0) {
		
		iterator = vfsIteratorStart;
		while (iterator != vfsIteratorStop) {
			if (VFSVolumeEnumerate(&volRefNum, &iterator) == errNone) {

				// get new memory
				tmpPtr = lst->list;
				lst->list = MemPtrNew(sizeof(volListItemType) * (lst->numVols+1));
				MemSet(lst->list, sizeof(volListItemType) * (lst->numVols+1), 0);
				if (tmpPtr != NULL) {
					MemMove(lst->list, tmpPtr, sizeof(volListItemType) * lst->numVols);
					MemPtrFree(tmpPtr);
				}

				lst->list[lst->numVols].mode = brwModeVfs;
				lst->list[lst->numVols].data.vol.volRefNum = volRefNum;
				VFSVolumeInfo(volRefNum, &lst->list[lst->numVols].data.vol.volInfo);
				VFSVolumeSize(volRefNum, &lst->list[lst->numVols].data.vol.volUsed, &lst->list[lst->numVols].data.vol.volSize);
				VFSVolumeGetLabel(volRefNum, lst->list[lst->numVols].data.vol.label, 255);

				// inc volume count
				lst->numVols++;
			}
		}
		
	}
	
	return(1);
}


/*
 *	refresh the volume information
 * - stuff like: size (won't change) and space used (this is what we want!!!)
 */
void refreshVolumeInfo(volListType *lst)
{
	UInt16 i;
	UInt32 tmpRamSize;

		
	for (i = 0; i < lst->numVols; i++) {
		if (lst->list[i].mode == brwModeVfs) {
									
			// clear volinfo before writing to it (otherwise we get REALLY BAD ERRORS!)
			MemSet(&lst->list[i].data.vol.volInfo, sizeof(VolumeInfoType), 0);
			// volinfo
			VFSVolumeInfo(lst->list[i].data.vol.volRefNum, &lst->list[i].data.vol.volInfo);
			// get size and used space
			VFSVolumeSize(lst->list[i].data.vol.volRefNum, &lst->list[i].data.vol.volUsed, &lst->list[i].data.vol.volSize);
	
			// label
			VFSVolumeGetLabel(lst->list[i].data.vol.volRefNum, lst->list[i].data.vol.label, 255);
		}
		else if (lst->list[i].mode == brwModeCard && lst->list[i].data.card.typeId == cardTypeRom) {
			
			MemCardInfo(lst->list[i].data.card.cardNo, lst->list[i].data.card.label, 
				lst->list[i].data.card.manuf, &lst->list[i].data.card.version,
				&lst->list[i].data.card.crDate, &lst->list[i].data.card.ramSize,
				&tmpRamSize, &lst->list[i].data.card.ramUsed);
			
			// ---== RAM ==---
			// copy to ram entry
			MemMove(&lst->list[i+1], &lst->list[i], sizeof(volListItemType));
			// reset value (was destroyed from memmove)
			lst->list[i+1].data.card.typeId = cardTypeRam;
						
			// this is the RAM part so copy the ram size
			lst->list[i+1].data.card.ramSize = tmpRamSize;
			
			// MemCardInfo returns the free byte count so we have to re calc the "used bytes"
			lst->list[i+1].data.card.ramUsed = lst->list[i+1].data.card.ramSize - lst->list[i+1].data.card.ramUsed;

			// ---== ROM ==---
			// MemCardInfo returns the free byte count to we have to re calc the "used bytes"
			// we expect: every bytes in rom is used !
			lst->list[i].data.card.ramUsed = lst->list[i].data.card.ramSize;
		}
	}
}
