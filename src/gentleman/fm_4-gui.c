/*
 *	file:		fm_4-gui.c
 *	project:	GentleMan
 *	content:	GUI support
 *	updated:	Aug. 22. 2001
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
 *	change between the two browsers (0 and 1) (1 and 2 on display)
 */
void doBrwSelect(global_data_type *glb, UInt16 id)
{
	glb->currentBrw = id - brw1_main_button;
	
	// setup the display positions
	setDisplayStuff(&glb->cfg, glb->brwLst[glb->currentBrw].brwMode);
						
	// volumes
	setUpVolList(&glb->vLst, glb->brwLst[glb->currentBrw].volIndex);
		
	// sort / show1 / show2
	setUpSortShow(&glb->brwLst[glb->currentBrw]);

	// scrollbar
	setUpFileBrowser(&glb->brwLst[glb->currentBrw]);
		
	// draw infos about selected files
	drawSelectedInfo(&glb->brwLst[glb->currentBrw]);
		
	// draw the file/database list
	drawFileList(&glb->brwLst[glb->currentBrw]);
}

/*
 *	popup the sort list and get the selection
 */
void doSortPopupList(brwLstType *brw)
{
	Int16 selection;
	
	
	if (brw->brwMode == brwModeVfs) {
			
		// popup list and wait for selection
		selection = LstPopupList(GetObjectPtr(sort1_main_lst));
	
		// make sure something was selected
		if (selection != noListSelection && brw->sort != selection) {
			brw->sort = selection;
				
			// set sort popuptrigger label
			CtlSetLabel(GetObjectPtr(sort_main_ptrg), LstGetSelectionText(GetObjectPtr(sort1_main_lst), brw->sort));
			
			// resort
			sort(brw);
		
			// draw infos about selected files
			drawSelectedInfo(&glob->brwLst[glob->currentBrw]);
			
			// redraw file list with new show option
			drawFileList(brw);
		}
	}
	else if (brw->brwMode == brwModeCard) {
		
		// popup list and wait for selection
		selection = LstPopupList(GetObjectPtr(sort2_main_lst));
	
		// make sure something was selected
		if (selection != noListSelection && brw->sort != selection) {
			brw->sort = selection;
	
			CtlSetLabel(GetObjectPtr(sort_main_ptrg), LstGetSelectionText(GetObjectPtr(sort2_main_lst), brw->sort));
			
			// resort
			sort(brw);
		
			// draw infos about selected files
			drawSelectedInfo(&glob->brwLst[glob->currentBrw]);
			
			// redraw file list with new show option
			drawFileList(brw);
		}
	}
}

/*
 *	setup the file browser stuff
 *	- set the scrollbar
 * - direct access to "glob" this should be ok
 */
void setUpFileBrowser(brwLstType *brw)
{
	ScrollBarType *sclB;
	FormPtr form;
	
		
	sclB = GetObjectPtr(file_main_slb);
	form = FrmGetActiveForm();
	
	if (brw->lst.mode == brwModeVfs) {
		// hide scrollbar
		FrmHideObject(form, FrmGetObjectIndex(form, file_main_slb));

		// draw directory info
		drawDirectoryInfo(brw);
						
		// resize scrollbar
		sclB->bounds.extent.y = glob->cfg.scrollBarLength;
		
		// init scrollbar
		SclSetScrollBar(sclB, brw->scrollPos, 0, ((brw->lst.data.vol.numFiles > glob->cfg.numDisplay) ? brw->lst.data.vol.numFiles - glob->cfg.numDisplay : 0), 1);
		
		// show - draw scrollbar
		FrmShowObject(form, FrmGetObjectIndex(form, file_main_slb));
		
		// set menu
		FrmSetMenu(form, vol_main_menu);	
	}
	else if (brw->lst.mode == brwModeCard) {
		
		FrmHideObject(form, FrmGetObjectIndex(form, file_main_slb));
		
		// draw directory info
		drawDirectoryInfo(brw);
		
		sclB->bounds.extent.y = glob->cfg.scrollBarLength;
		
		SclSetScrollBar(sclB, brw->scrollPos, 0, ((brw->lst.data.card.numDBs > glob->cfg.numDisplay) ? brw->lst.data.card.numDBs - glob->cfg.numDisplay : 0), 1);
		
		FrmShowObject(form, FrmGetObjectIndex(form, file_main_slb));
		
		FrmSetMenu(form, card_main_menu);
	}
}

/*
 *	set the volume selection list
 */
void setUpVolList(volListType *lst, Int16 index)
{
	ListPtr lstPtr;
	char displayStr[200];
		
	
	// all ways the same list object
	lstPtr = GetObjectPtr(volst_main_lst);
		
	// set number of items
	LstSetListChoices(lstPtr, NULL, lst->numVols);
	// set draw function
	LstSetDrawFunction(lstPtr, myVolListDraw);
	// set the selection
	LstSetSelection(lstPtr, index);
	
	// clear string
	MemSet(displayStr, 200, 0);
	
	// make string
	makeVolInfo(&lst->list[index], displayStr, glob->cfg.volListStrLeng);
	
	// clear area before drawing to it
	WinEraseRectangle(&glob->cfg.volListArea, 0);
	
	// draw it
	WinDrawChars(displayStr, StrLen(displayStr), glob->cfg.volListStrX, glob->cfg.volListStrY);
}

/*
 * make a displayable "label"
 */
void makeVolInfo(volListItemType *vInfo, char *drawStr, UInt8 length)
{
	char sizeStr[30];
	char label[256];
	const char strReadOnly[5] = {" [R]\0"};
	UInt16 leng, extraLeng;
	
		
	if (vInfo->mode == brwModeVfs) {
		switch (vInfo->data.vol.volInfo.mediaType) {
		case	'cfsh':
			StrCat(drawStr, "CF: ");
			break;
		case	'mstk':
			StrCat(drawStr, "MS: ");
			break;
		case	'mmcd':
			StrCat(drawStr, "MMC: ");
			break;
		case	'sdig':
			StrCat(drawStr, "SD: ");
			break;
		case	'pose':
			StrCat(drawStr, "POSE: ");
			break;
		case	'PSim':
			StrCat(drawStr, "PSIM: ");
			break;
		case	'smed':
			StrCat(drawStr, "SM: ");
			break;
		}
			
		extraLeng = FntLineWidth(drawStr, StrLen(drawStr));
			
		// check if volume is readonly
		if ((vInfo->data.vol.volInfo.attributes & vfsVolumeAttrReadOnly) == vfsVolumeAttrReadOnly) {
			extraLeng += FntLineWidth(strReadOnly, StrLen(strReadOnly));
		}
		
		// clear string
		MemSet(sizeStr, 30, 0);
		
		// shows like this: free_space / size
		
		// free size
		makeDisplaySize((vInfo->data.vol.volSize - vInfo->data.vol.volUsed), sizeStr);
		
		// size
		// makeDisplaySize(vInfo->data.vol.volSize, sizeStr);
		
		// delimiter
		StrCat(sizeStr, "/");

		// free size
		// makeDisplaySize((vInfo->data.vol.volSize - vInfo->data.vol.volUsed), sizeStr+StrLen(sizeStr));
		
		// size
		makeDisplaySize(vInfo->data.vol.volSize, sizeStr+StrLen(sizeStr));
			
		// add length of size String
		extraLeng += FntLineWidth(sizeStr, StrLen(sizeStr));
		
		// add length of a space 0x20
		extraLeng += FntLineWidth(" ", 1);
			
		// clear string
		MemSet(label, 256, 0);
		// no Label set!
		if (StrLen(vInfo->data.vol.label) == 0) {
			StrCopy(label, "(no label)");
		}
		else {
			StrCopy(label, vInfo->data.vol.label);
		}
			
		if ((extraLeng + FntLineWidth(label, StrLen(label))) <= length) {
			StrCat(drawStr, label);
		}
		else {
			leng = StrLen(vInfo->data.vol.label);
			do {
				leng--;
			} while ((extraLeng + FntLineWidth(label, leng)) <= length);
			StrNCopy(drawStr+StrLen(drawStr), label, leng);
		}
		
		StrCat(drawStr, " ");
		StrCat(drawStr, sizeStr);
		if ((vInfo->data.vol.volInfo.attributes & vfsVolumeAttrReadOnly) == vfsVolumeAttrReadOnly) {
			StrCat(drawStr, strReadOnly);
		}
	}
	else if (vInfo->mode == brwModeCard) {
		// media type
		if (vInfo->data.card.typeId == cardTypeRom) {
			StrCopy(drawStr, "ROM: ");
		}
		else if (vInfo->data.card.typeId == cardTypeRam) {
			StrCopy(drawStr, "RAM: ");
		}
		// label
		StrCat(drawStr, vInfo->data.card.label);
		
		// space between label and size
		StrCat(drawStr, " ");
		
		// free size
		makeDisplaySize((vInfo->data.card.ramSize - vInfo->data.card.ramUsed), drawStr+StrLen(drawStr));
		
		// size
		//makeDisplaySize(vInfo->data.card.ramSize, drawStr+StrLen(drawStr));
		
		// delimiter
		StrCat(drawStr, "/");
		
		// free size
		//makeDisplaySize((vInfo->data.card.ramSize - vInfo->data.card.ramUsed), drawStr+StrLen(drawStr));
				
		// size
		makeDisplaySize(vInfo->data.card.ramSize, drawStr+StrLen(drawStr));
		
		// add the readonly mark to the id string
		if (vInfo->data.card.typeId == cardTypeRom) {
			StrCat(drawStr, strReadOnly);
		}		
	}
}

/*
 *	popup the volume list and get the selection
 * accesses "glob"
 */
void doVolPopupList(brwLstType *brw, volListType *lst)
{
	Int16 selection;
	char displayStr[200];
	

	// popup list and wait for selection
	selection = LstPopupList(GetObjectPtr(volst_main_lst));
	
	if (selection != noListSelection && brw->volIndex != selection) {
		// set selected volume for the browser
		brw->volIndex = selection;
		
		// free memory
		if (brw->lst.mode == brwModeVfs && brw->lst.data.vol.fileList != NULL) {
			MemPtrFree(brw->lst.data.vol.fileList);
			brw->lst.data.vol.fileList =  NULL;
		}
		else if (brw->lst.mode == brwModeCard && brw->lst.data.card.dbList != NULL) {
			MemPtrFree(brw->lst.data.card.dbList);
			brw->lst.data.card.dbList =  NULL;
		}
		
		// clear list info
		MemSet(&brw->lst, sizeof(listInfoType), 0);
		
		// set the button text
		if (lst->list[selection].mode == brwModeVfs) {
			
			// set listmode to vfs volume
			brw->lst.mode = brwModeVfs;
			
			// copy the volume info to the browsers list structure
			MemMove(&brw->lst.data.vol.volInfo, &lst->list[selection].data.vol, sizeof(myVolumeInfoType));
			
			// set browser mode to vfs volume
			brw->brwMode = brwModeVfs;
			
			// all ways set this to the beginning
			brw->scrollPos = 0;
			
			// set with defaults from prefs
			brw->sort = glob->prefs.defaultVfsSort;
			brw->show1 = glob->prefs.defaultVfsShow1;
			brw->show2 = glob->prefs.defaultVfsShow2;
						
			// set root directory as current directory !
			StrCopy(brw->lst.data.vol.currentDir, "/");
			
			// scan dir
			getFilesInDir(&brw->lst.data.vol);	
		}
		else if (lst->list[selection].mode == brwModeCard) {
						
			// set listmode to card
			brw->lst.mode = brwModeCard;
			
			// copy the volume info to the browsers list structure
			MemMove(&brw->lst.data.card.cardInfo, &lst->list[selection].data.card, sizeof(myCardInfoType));
			
			// set browser mode to card
			brw->brwMode = brwModeCard;
			
			// all ways set this to the beginning
			brw->scrollPos = 0;
			
			brw->sort = glob->prefs.defaultCardSort;
			brw->show1 = glob->prefs.defaultCardShow1;
			brw->show2 = glob->prefs.defaultCardShow2;
						
			// get all db's
			getDBList(&brw->lst.data.card);
		}
		
		// clear string
		MemSet(displayStr, 200, 0);
	
		// make string
		makeVolInfo(&lst->list[selection], displayStr, glob->cfg.volListStrLeng);
		
		// clear the button area before drawing to it
		WinEraseRectangle(&glob->cfg.volListArea, 0);
		
		// draw it
		WinDrawChars(displayStr, StrLen(displayStr), glob->cfg.volListStrX, glob->cfg.volListStrY);

		// setup display stuff
		setDisplayStuff(&glob->cfg, lst->list[selection].mode);
		
		// scroll bar
		setUpFileBrowser(brw);
		
		// sort the dbs/files
		sort(brw);
			
		// redraw the file list / popups / etc...
		setUpSortShow(brw);

		// draw info about selected files		
		drawSelectedInfo(brw);
		
		// draw file list
		drawFileList(brw);
	}
}

/*
 *	volume List draw callback
 *	- this directly reads globals ! but this is really ok !
 */
void myVolListDraw(Int16 item, RectangleType *rect, char **text)
{
	char displayStr[200];
	
	
	//CALLBACK_PROLOGUE

			
	MemSet(displayStr, 200, 0);
	
	// make string
	makeVolInfo(&glob->vLst.list[item], displayStr, glob->cfg.volListStrLeng);
	// draw it
	WinDrawChars(displayStr, StrLen(displayStr), rect->topLeft.x, rect->topLeft.y);

				
	//CALLBACK_EPILOGUE
}

/*
 *	set up the browser selection list
 */
void setUpBrwList(global_data_type *glb)
{
	FrmSetControlGroupSelection(FrmGetActiveForm(), (UInt8)brw_group, (glb->currentBrw + brw1_main_button));
}

/*
 *	setup the sort and the 2 show popup triggers and lists
 */
void setUpSortShow(brwLstType *brw)
{
	if (brw->brwMode == brwModeVfs) {
		// set list selection
		LstSetSelection(GetObjectPtr(show11_main_lst), brw->show1);
		// set popuptrigger text
		CtlSetLabel(GetObjectPtr(show1_main_ptrg), LstGetSelectionText(GetObjectPtr(show11_main_lst), brw->show1));
		
		// set list selection
		LstSetSelection(GetObjectPtr(show21_main_lst), brw->show2);
		// set popuptrigger text
		CtlSetLabel(GetObjectPtr(show2_main_ptrg), LstGetSelectionText(GetObjectPtr(show21_main_lst), brw->show2));
		
		// set list selection
		LstSetSelection(GetObjectPtr(sort1_main_lst), brw->sort);
		// set popuptrigger text
		CtlSetLabel(GetObjectPtr(sort_main_ptrg), LstGetSelectionText(GetObjectPtr(sort1_main_lst), brw->sort));
	}
	else if (brw->brwMode == brwModeCard) {
		// set list selection
		LstSetSelection(GetObjectPtr(show12_main_lst), brw->show1);
		// set popuptrigger text
		CtlSetLabel(GetObjectPtr(show1_main_ptrg), LstGetSelectionText(GetObjectPtr(show12_main_lst), brw->show1));
				
		// set list selection
		LstSetSelection(GetObjectPtr(show22_main_lst), brw->show2);
		// set popuptrigger text
		CtlSetLabel(GetObjectPtr(show2_main_ptrg), LstGetSelectionText(GetObjectPtr(show22_main_lst), brw->show2));
		
		// set list selection
		LstSetSelection(GetObjectPtr(sort2_main_lst), brw->sort);
		// set popuptrigger text
		CtlSetLabel(GetObjectPtr(sort_main_ptrg), LstGetSelectionText(GetObjectPtr(sort2_main_lst), brw->sort));
	}
}

/*
 *	popup the show lists and get the value and update the popuptrigger
 *
 *	- somehow we have to notify the browser of the changes in the displaying options !!!
 */
void doShowPopupList(brwLstType *brw, UInt16 id)
{
	Int16 selection;
	
	
	if (id == show1_main_ptrg) {
		// show list and wait for choice
		selection = LstPopupList(GetObjectPtr(show11_main_lst+(brw->brwMode-1)));
		if (selection != noListSelection && brw->show1 != selection) {
			// set selection
			brw->show1 = selection;
			
			// set new label
			CtlSetLabel(GetObjectPtr(show1_main_ptrg), LstGetSelectionText(GetObjectPtr(show11_main_lst+brw->brwMode-1), brw->show1));
			
			// draw infos about selected files
			drawSelectedInfo(&glob->brwLst[glob->currentBrw]);
			
			// redraw file list with new show option
			drawFileList(brw);
		}
	}
	else if (id == show2_main_ptrg) {
		// show list and wait for choice
		selection = LstPopupList(GetObjectPtr(show21_main_lst+(brw->brwMode-1)));
		if (selection != noListSelection && brw->show2 != selection) {
			// set selection
			brw->show2 = selection;
			
			// set new label
			CtlSetLabel(GetObjectPtr(show2_main_ptrg), LstGetSelectionText(GetObjectPtr(show21_main_lst+brw->brwMode-1), brw->show2));
			
			// draw infos about selected files
			drawSelectedInfo(&glob->brwLst[glob->currentBrw]);
			
			// redraw file list with new show option
			drawFileList(brw);
		}
	}
}

/*
 *	return the object Ptr for the given object id
 */
void *GetObjectPtr(UInt16 id)
{
	FormPtr form;
	
	
	form = FrmGetActiveForm();
	return(FrmGetObjectPtr(form, FrmGetObjectIndex(form, id)));
}
