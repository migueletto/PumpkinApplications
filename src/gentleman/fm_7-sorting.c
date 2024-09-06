/*
 *	file:		fm_7-sorting.c
 *	project:	GentleMan
 *	content:	file / db list sorting
 *	updated:	Dec. 22. 2001
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
Int16 searchFileName(brwLstType *brw, char searchStr)
{
	if (brw->lst.mode == brwModeVfs) {
		
	}
	else if (brw->lst.mode == brwModeCard) {
		
	}
}

Int16 searchFNVfs(fileListType *lst, char searchStr)
{
	
}
*/

/*
 *	compare two dbInfoTyps by one given field!
 */
Int16 doCompareCard(dbInfoType *db1, dbInfoType *db2, cardSortType sortItem)
{
	Int16 retVal = 0;
	char *str1, *str2;
	
	
	switch (sortItem) {
		
	case	cardSortNameAscending:
		retVal = StrCompare(db1->name, db2->name);
		break;
	case	cardSortNameDescending:
		retVal = StrCompare(db1->name, db2->name);
		if (retVal > 0) retVal = -1;
		else if (retVal < 0) retVal = 1;
		break;
		
	case	cardSortSizeAscending:
		if (db1->size > db2->size) return(1);
		else if (db1->size < db2->size) return(-1);
		else return(0);
	case	cardSortSizeDescending:
		if (db1->size > db2->size) return(-1);
		else if (db1->size < db2->size) return(1);
		else return(0);
		break;
		
	case	cardSortTypeIdAscending:
		str1 = (char*) &db1->typeId;
		str2 = (char*) &db2->typeId;
		retVal = StrNCompare(str1, str2, 4);
		break;
	case	cardSortTypeIdDescending:
		str1 = (char*) &db1->typeId;
		str2 = (char*) &db2->typeId;
		retVal = StrNCompare(str1, str2, 4);
		if (retVal > 0) retVal = -1;
		else if (retVal < 0) retVal = 1;
		break;
		
	case	cardSortCridAscending:
		str1 = (char*) &db1->crid;
		str2 = (char*) &db2->crid;
		retVal = StrNCompare(str1, str2, 4);
		break;
	case	cardSortCridDescending:
		str1 = (char*) &db1->crid;
		str2 = (char*) &db2->crid;
		retVal = StrNCompare(str1, str2, 4);
		if (retVal > 0) retVal = -1;
		else if (retVal < 0) retVal = 1;
		break;
		
	case	cardSortcDateAscending:
		if (db1->crDate > db2->crDate) return(1);
		else if (db1->crDate < db2->crDate) return(-1);
		else return(0);
		break;
	case	cardSortcDateDescending:
		if (db1->crDate > db2->crDate) return(-1);
		else if (db1->crDate < db2->crDate) return(1);
		else return(0);
		break;
		
	case	cardSortmDateAscending:
		if (db1->moDate > db2->moDate) return(1);
		else if (db1->moDate < db2->moDate) return(-1);
		else return(0);
		break;
	case	cardSortmDateDescending:
		if (db1->moDate > db2->moDate) return(-1);
		else if (db1->moDate < db2->moDate) return(1);
		else return(0);
		break;
		
	case	cardSortbDateAscending:
		if (db1->acDate > db2->acDate) return(1);
		else if (db1->acDate < db2->acDate) return(-1);
		else return(0);
		break;
	case	cardSortbDateDescending:
		if (db1->acDate > db2->acDate) return(-1);
		else if (db1->acDate < db2->acDate) return(1);
		else return(0);
		break;
	}
	
	return(retVal);
}

/*
 *	compare two fileInfoTyps by one given field!
 */
Int16 doCompareVol(fileInfoType *file1, fileInfoType *file2, vfsSortType sortItem)
{
	Int16 retVal = 0;
	
		
	switch (sortItem) {
		
	case	vfsSortNameAscending:
		retVal = StrCompare(file1->name, file2->name);
		break;
	case	vfsSortNameDescending:
		retVal = StrCompare(file1->name, file2->name);
		if (retVal > 0) retVal = -1;
		else if (retVal < 0) retVal = 1;
		break;
		
	case	vfsSortSizeAscending:
		if (file1->size > file2->size) return(1);
		else if (file1->size < file2->size) return(-1);
		else return(0);
	case	vfsSortSizeDescending:
		if (file1->size > file2->size) return(-1);
		else if (file1->size < file2->size) return(1);
		else return(0);
		break;
				
	case	vfsSortcDateAscending:
		if (file1->crDate > file2->crDate) return(1);
		else if (file1->crDate < file2->crDate) return(-1);
		else return(0);
		break;
	case	vfsSortcDateDescending:
		if (file1->crDate > file2->crDate) return(-1);
		else if (file1->crDate < file2->crDate) return(1);
		else return(0);
		break;
		
	case	vfsSortmDateAscending:
		if (file1->moDate > file2->moDate) return(1);
		else if (file1->moDate < file2->moDate) return(-1);
		else return(0);
		break;
	case	vfsSortmDateDescending:
		if (file1->moDate > file2->moDate) return(-1);
		else if (file1->moDate < file2->moDate) return(1);
		else return(0);
		break;
		
	case	vfsSortaDateAscending:
		if (file1->acDate > file2->acDate) return(1);
		else if (file1->acDate < file2->acDate) return(-1);
		else return(0);
		break;
	case	vfsSortaDateDescending:
		if (file1->acDate > file2->acDate) return(-1);
		else if (file1->acDate < file2->acDate) return(1);
		else return(0);
		break;
	}
	
	return(retVal);
}

/*
 *	sort a file/db list
 * - this uses selection sort
 */
void sort(brwLstType *brw)
{
	dbInfoType tmpDB;
	fileInfoType tmpFile;
	UInt16 i, i2, i3, insertPos, iP2;
	Int16 cmpRes;
	vfsSortType tmpSort;
	UInt32 num, num2;
	WinHandle backGround;

	
	if (brw->lst.mode == brwModeVfs) {
		
		
		// save bg
		inProgress(0, 1, &backGround, glob->cfg.screenState, "sorting...");
		num = 0;
		// show first image
		inProgress(num, 0, &backGround, glob->cfg.screenState, NULL);
		
		// move directory's up in list	
		if (brw->lst.data.vol.numFiles >= 2 && StrCompare(brw->lst.data.vol.fileList[1].name, "..") == 0) {
			// if "." and ".." are present start below them
			insertPos = 2;
			iP2 = insertPos;
			i3 = insertPos;
		}
		else {
			// only "." is present start below it
			insertPos = 1;
			iP2 = insertPos;
			i3 = insertPos;
		}
		
		// move all directory's to top of list (but below of "." BTW. "..")
		for (i = i3; i < brw->lst.data.vol.numFiles; i++) {
			if ((brw->lst.data.vol.fileList[i].attribs & vfsFileAttrDirectory) == vfsFileAttrDirectory) {
				if (i != insertPos) {
					MemMove(&tmpFile, &brw->lst.data.vol.fileList[insertPos], sizeof(fileInfoType));
					MemMove(&brw->lst.data.vol.fileList[insertPos], &brw->lst.data.vol.fileList[i], sizeof(fileInfoType));
					MemMove(&brw->lst.data.vol.fileList[i], &tmpFile, sizeof(fileInfoType));
					insertPos++;
					i--;
				}
				else {
					insertPos++;
				}
				
				
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
		
		// directory's all have size 0 (null) so if we sort be size, directory's will be sorted by name !
		tmpSort = brw->sort;
		if (brw->sort == vfsSortSizeAscending) {
			tmpSort = vfsSortNameAscending;
		}
		else if (brw->sort == vfsSortSizeDescending) {
			tmpSort = vfsSortNameDescending;
		}
		
		// sort directory's (below "." and "..")
		for (i = iP2; i < insertPos-1; i++) {
			i3 = i;
			for (i2 = i+1; i2 < insertPos; i2++) {
								
				// compare !
				cmpRes = doCompareVol(&brw->lst.data.vol.fileList[i3], &brw->lst.data.vol.fileList[i2], tmpSort);
				
				// remember "bigger" item
				if (cmpRes > 0) {
					i3 = i2;
				}
			}
			
			// swap items
			if (i3 != i) {
				MemMove(&tmpFile, &brw->lst.data.vol.fileList[i], sizeof(fileInfoType));
				MemMove(&brw->lst.data.vol.fileList[i], &brw->lst.data.vol.fileList[i3], sizeof(fileInfoType));
				MemMove(&brw->lst.data.vol.fileList[i3], &tmpFile, sizeof(fileInfoType));
			}
			
			// show scan progress
			num2++;
			if (num2 % 5 == 0) {
				num = (num + 1) % 8;
				// show first image
				inProgress(num, 0, &backGround, glob->cfg.screenState, NULL);
				num2 = 0;
			}
		}
		
		// sort the files
		for (i = insertPos; i < brw->lst.data.vol.numFiles-1; i++) {
			i3 = i;
			for (i2 = i+1; i2 < brw->lst.data.vol.numFiles; i2++) {
						
				// compare !
				cmpRes = doCompareVol(&brw->lst.data.vol.fileList[i3], &brw->lst.data.vol.fileList[i2], brw->sort);
				
				// remember "bigger" item
				if (cmpRes > 0) {
					i3 = i2;
				}
			}
			
			// swap items
			if (i3 != i) {
				MemMove(&tmpFile, &brw->lst.data.vol.fileList[i], sizeof(fileInfoType));
				MemMove(&brw->lst.data.vol.fileList[i], &brw->lst.data.vol.fileList[i3], sizeof(fileInfoType));
				MemMove(&brw->lst.data.vol.fileList[i3], &tmpFile, sizeof(fileInfoType));
			}
			
			// show scan progress
			num2++;
			if (num2 % 5 == 0) {
				num = (num + 1) % 8;
				// show first image
				inProgress(num, 0, &backGround, glob->cfg.screenState, NULL);
				num2 = 0;
			}
		}
		
		// restore bg
		inProgress(num, 2, &backGround, glob->cfg.screenState, NULL);
	}
	else if (brw->lst.mode == brwModeCard) {
		
		// save bg
		inProgress(0, 1, &backGround, glob->cfg.screenState, "sorting...");
	
		num = 0;
		// show first image
		inProgress(num, 0, &backGround, glob->cfg.screenState, NULL);
		
		// sort databases
		for (i = 0; i < brw->lst.data.card.numDBs-1; i++) {
			i3 = i;
			for (i2 = i+1; i2 < brw->lst.data.card.numDBs; i2++) {
							
				// compare !
				cmpRes = doCompareCard(&brw->lst.data.card.dbList[i3], &brw->lst.data.card.dbList[i2], brw->sort);
				
				// remember "bigger" item
				if (cmpRes > 0) {
					i3 = i2;
				}
			}
			
			// swap items
			if (i3 != i) {
				MemMove(&tmpDB, &brw->lst.data.card.dbList[i], sizeof(dbInfoType));
				MemMove(&brw->lst.data.card.dbList[i], &brw->lst.data.card.dbList[i3], sizeof(dbInfoType));
				MemMove(&brw->lst.data.card.dbList[i3], &tmpDB, sizeof(dbInfoType));
			}
			
			// show scan progress
			num2++;
			if (num2 % 5 == 0) {
				num = (num + 1) % 8;
				// show first image
				inProgress(num, 0, &backGround, glob->cfg.screenState, NULL);
				num2 = 0;
			}
		}
		
		// restore bg
		inProgress(num, 2, &backGround, glob->cfg.screenState, NULL);
	}
}
