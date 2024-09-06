/*
 *	file:		fm_8-details.c
 *	project:	GentleMan
 *	content:	details
 *	updated:	Oct. 30. 2001
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
 *	jump to the appropriate details form
 */
void doDetails(global_data_type *glb, UInt32 pos, Boolean usePos)
{
	UInt32 i;
	
		
	// get details global memory
	glb->details = MemPtrNew(sizeof(detailsType));
	// init stuff
	glb->details->mode = glb->brwLst[glb->currentBrw].brwMode;

	// get a mem handle for the "name text filed"
	glb->details->nameFldHand = MemHandleNew(256);
		
	if (glb->brwLst[glb->currentBrw].brwMode == brwModeVfs) {
		
		if (glb->brwLst[glb->currentBrw].lst.data.vol.numSelFiles == 0 && !usePos) {
			MemHandleFree(glb->details->nameFldHand);
			MemPtrFree(glb->details);
			
			// error no files selected
			FrmAlert(alertID_volNoSelFile);
		}
		else if (glb->brwLst[glb->currentBrw].lst.data.vol.numSelFiles == 1 || usePos) {
		
			if (usePos && pos >= 0 && pos < glb->brwLst[glb->currentBrw].lst.data.vol.numFiles) {
				// use POS as index to the file list
				i = pos;
			}
			else {
				// find first selected file
				i = 0;
				while (!glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].selected && i < glb->brwLst[glb->currentBrw].lst.data.vol.numFiles) {
					i++;
				}
			}
			
			if (i < glb->brwLst[glb->currentBrw].lst.data.vol.numFiles) {
				MemMove(&glb->details->data.vol[0], &glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i], sizeof(fileInfoType));
				MemMove(&glb->details->data.vol[1], &glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i], sizeof(fileInfoType));
		
				// copy names
				StrCopy(glb->details->name[0], glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name);
				StrCopy(glb->details->name[1], glb->brwLst[glb->currentBrw].lst.data.vol.fileList[i].name);
				// change pointers
				glb->details->data.vol[0].name = &glb->details->name[0][0];
				glb->details->data.vol[1].name = &glb->details->name[1][0];
			
				// all ways single file mode
				glb->details->single = true;
				
			/*	// single file mode
				if (glb->brwLst[glb->currentBrw].lst.data.vol.numSelFiles == 1) glb->details->single = true;
				// multi file mode
				else {
					glb->details->single = false;
					
					// clear all attribs !
					glb->details->data.vol[0].attribs = 0;
					glb->details->data.vol[1].attribs = 0;
				}
			*/
					
				// jump to the "vfs file Details" form
				FrmGotoForm(formID_volDetails);
			}
			else {
				// error can't find selected file !
				FrmAlert(alertID_volNoSelFile);
			}
		}
		else {
			MemHandleFree(glb->details->nameFldHand);
			MemPtrFree(glb->details);
			
			// alert ... this is a single file operation
			FrmAlert(alertID_volSingleFileOnly);
		}
	}
	else if (glb->brwLst[glb->currentBrw].brwMode == brwModeCard) {
		
		if (glb->brwLst[glb->currentBrw].lst.data.card.numSelDBs == 0 && !usePos) {
			MemHandleFree(glb->details->nameFldHand);
			MemPtrFree(glb->details);
			
			// error no selected databases found
			FrmAlert(alertID_cardNoSelDB);
		}
		else if (glb->brwLst[glb->currentBrw].lst.data.card.numSelDBs == 1 || usePos) {
			
			if (usePos && pos >= 0 && pos < glb->brwLst[glb->currentBrw].lst.data.card.numDBs) {
				// use POS as index to the db list
				i = pos;
			}
			else {
				// find selected file
				i = 0;
				while (!glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].selected && i < glb->brwLst[glb->currentBrw].lst.data.card.numDBs) {
					i++;
				}
			}
		
			if (i < glb->brwLst[glb->currentBrw].lst.data.card.numDBs) {
				MemMove(&glb->details->data.card[0], &glb->brwLst[glb->currentBrw].lst.data.card.dbList[i], sizeof(dbInfoType));
				MemMove(&glb->details->data.card[1], &glb->brwLst[glb->currentBrw].lst.data.card.dbList[i], sizeof(dbInfoType));
			
				// get a mem handle for the "type text filed"
				glb->details->typeFldHand = MemHandleNew(5);
				// get a mem handle for the "creator text filed"
				glb->details->cridFldHand = MemHandleNew(5);
			
				// all ways single db mode
				glb->details->single = true;
				
			/*	// single db mode
				if (glb->brwLst[glb->currentBrw].lst.data.card.numSelDBs == 1) glb->details->single = true;
				// multiple db mode
				else {
					glb->details->single = false;
					
					// clear all attribs !
					glb->details->data.card[0].attribs = 0;
					glb->details->data.card[1].attribs = 0;
				}
			*/
				
				if (usePos) {
					if (glb->prefs.dTapCard == doubleTapDetailsGeneral) {
						glb->details->general = true;
					}
					else if (glb->prefs.dTapCard == doubleTapDetailsAttributes) {
						glb->details->general = false;
					}
				}
				else {
					// when activated thru the MENU use GENERAL as start!
					glb->details->general = true;
				}
					
				// jump to the card database details form
				FrmGotoForm(formID_cardDetails);
			}
			else {
				// error can't find selected file !
				// error no selected databases found
				FrmAlert(alertID_cardNoSelDB);
			}
		}
		else {
			MemHandleFree(glb->details->nameFldHand);
			MemPtrFree(glb->details);
			
			// error no selected databases found
			FrmAlert(alertID_cardSingleDBOnly);
		}
	}
}

/*
 *	free the details memory
 */
void freeDetails(global_data_type *glb, Boolean card)
{	
	//MemHandleFree(glb->details->nameFldHand);

	if (card) {
		if (glb->details->typeFldHand != NULL) MemHandleFree(glb->details->typeFldHand);
		if (glb->details->cridFldHand != NULL) MemHandleFree(glb->details->cridFldHand);
	}
	
	MemPtrFree(glb->details);
}

/*
 *	format a number in US format
 */
void preFormatNumberStr(char *in)
{
	Int8 i, t, c, numSeps;
		

	numSeps = (((StrLen(in) % 3) > 0) ? (StrLen(in) / 3) : (StrLen(in) / 3) - 1);
	
	i = StrLen(in) - 1;
	t = i + numSeps;
	in[t+1] = 0;
	c = 0;
	while (i >= 0) {
		in[t] = in[i];
		i--;
		t--;
		c++;
		if (c == 3) {
			in[t] = ',';
			c = 0;
			t--;
		}
	}
}

// ----=== CARD DETAILS ===----

/*
 *	show / hide the "Save / Cancel" and "Done" buttons
 */
void showHideCardDetailsButton(Boolean show)
{
	FormPtr form = FrmGetActiveForm();
	
		
	if (show) {
		FrmHideObject(form, FrmGetObjectIndex(form, done_cardDetails_button));
		FrmShowObject(form, FrmGetObjectIndex(form, save_cardDetails_button));
		FrmShowObject(form, FrmGetObjectIndex(form, cancel_cardDetails_button));
	}
	else {
		FrmHideObject(form, FrmGetObjectIndex(form, save_cardDetails_button));
		FrmHideObject(form, FrmGetObjectIndex(form, cancel_cardDetails_button));
		FrmShowObject(form, FrmGetObjectIndex(form, done_cardDetails_button));
	}
}

/*
 *	draw / update the date time in the card details form
 */
void drawCardDetailsDateTime(detailsType *details, screenModeType screenState)
{
	const RectangleType rect160x160[6] = {{{60,98}, {42,12}}, {{110,98}, {42,12}}, {{60,113}, {42,12}}, {{110,113}, {42,12}}, {{60,128}, {42,12}}, {{110,128}, {42,12}}};
	const RectangleType rect240x240[6] = {{{BIG(60),BIG(98)+1}, {BIG(42),BIG(12)}}, {{BIG(110),BIG(98)+1}, {BIG(42),BIG(12)}}, {{BIG(60),BIG(113)+2}, {BIG(42),BIG(12)-1}}, {{BIG(110),BIG(113)+2}, {BIG(42),BIG(12)-1}}, {{BIG(60),BIG(128)+1}, {BIG(42),BIG(12)}}, {{BIG(110),BIG(128)+1}, {BIG(42),BIG(12)}}};
	RectangleType rect[6];
	UInt8 i;
	char drawStr[30];
	
		
	// get screen config
	if (screenState == screen160x160) {
		MemMove(&rect, &rect160x160, sizeof(rect160x160));
	}
	else {
		MemMove(&rect, &rect240x240, sizeof(rect240x240));
	}
	
	// clear button text area
	for (i = 0; i < 6; i++) {
		WinEraseRectangle(&rect[i], 0);
	}
	
	// date / time
	MemSet(drawStr, 30, 0);
	makeDisplayDate(details->data.card[1].crDate, drawStr);
	WinDrawChars(drawStr, StrLen(drawStr), rect[0].topLeft.x + 1, rect[0].topLeft.y);
	MemSet(drawStr, 30, 0);
	makeDisplayTime(details->data.card[1].crDate, drawStr);
	WinDrawChars(drawStr, StrLen(drawStr), rect[1].topLeft.x + 1, rect[1].topLeft.y);
	
	MemSet(drawStr, 30, 0);
	makeDisplayDate(details->data.card[1].moDate, drawStr);
	WinDrawChars(drawStr, StrLen(drawStr), rect[2].topLeft.x + 1, rect[2].topLeft.y);
	MemSet(drawStr, 30, 0);
	makeDisplayTime(details->data.card[1].moDate, drawStr);
	WinDrawChars(drawStr, StrLen(drawStr), rect[3].topLeft.x + 1, rect[3].topLeft.y);
	
	MemSet(drawStr, 30, 0);
	makeDisplayDate(details->data.card[1].acDate, drawStr);
	WinDrawChars(drawStr, StrLen(drawStr), rect[4].topLeft.x + 1, rect[4].topLeft.y);
	MemSet(drawStr, 30, 0);
	makeDisplayTime(details->data.card[1].acDate, drawStr);
	WinDrawChars(drawStr, StrLen(drawStr), rect[5].topLeft.x + 1, rect[5].topLeft.y);
}

/*
 *	hide and show the appropriate GUI objects
 */
void showCardDetails(Boolean general, Boolean single, screenModeType screenState, Boolean hide)
{
	FormPtr form;
	UInt16 i;
	const RectangleType rect160x160 = {{0, 17}, {160, 120}};
	//const RectangleType rect240x240 = {{BIG(0), BIG(17)}, {BIG(160), BIG(120)}};
	RectangleType *rect;
	
		
	form = FrmGetActiveForm();

	//	get screen config
	if (screenState == screen160x160) {
		rect = (RectangleType *)&rect160x160;
		// MemMove(&rect, &rect160x160, sizeof(RectangleType));
	}
/*
	else {
		rect = &rect240x240;
		// MemMove(&rect, &rect240x240, sizeof(RectangleType));
	}
*/
			
	if (single) {
		if (general) {
			if (hide) {
				for (i = 0; i <= (ds6_cardDetails_button - backup_cardDetails_cbox); i++) {
					FrmHideObject(form, FrmGetObjectIndex(form, backup_cardDetails_cbox + i));
				}
			}
			else {
			
				WinEraseRectangle(rect, 0);
			
				// show
				for (i = 0; i <= (sort_cardDetails_label - name_cardDetails_fld); i++) {
					FrmShowObject(form, FrmGetObjectIndex(form, name_cardDetails_fld + i));
				}
			}
		}	
		else {
		
			if (hide) {
				FldSetTextHandle(GetObjectPtr(name_cardDetails_fld), NULL);
				FldSetTextHandle(GetObjectPtr(type_cardDetails_fld), NULL);
				FldSetTextHandle(GetObjectPtr(crid_cardDetails_fld), NULL);
		
		
				for (i = 0; i <= (sort_cardDetails_label - name_cardDetails_fld); i++) {
					FrmHideObject(form, FrmGetObjectIndex(form, name_cardDetails_fld + i));
				}
			}
			else {
				
				WinEraseRectangle(rect, 0);
			
				// show
				for (i = 0; i <= (ds6_cardDetails_button - backup_cardDetails_cbox); i++) {
					FrmShowObject(form, FrmGetObjectIndex(form, backup_cardDetails_cbox + i));
				}
			}
		}
	}

// only single db mode !!!
	
/*	else {
		// if we edit multiple dbs don't allow to view general stuff
		FrmHideObject(form, FrmGetObjectIndex(form, gen_cardDetails_button));
		FrmHideObject(form, FrmGetObjectIndex(form, attr_cardDetails_button));
		
		FldSetTextHandle(GetObjectPtr(name_cardDetails_fld), NULL);
		FldSetTextHandle(GetObjectPtr(type_cardDetails_fld), NULL);
		FldSetTextHandle(GetObjectPtr(crid_cardDetails_fld), NULL);
		
		// hide
		for (i = 0; i <= (sort_cardDetails_label - name_cardDetails_fld); i++) {
			FrmHideObject(form, FrmGetObjectIndex(form, name_cardDetails_fld + i));
		}
		WinEraseRectangle(&rect, 0);
			
		// show
		for (i = 0; i <= (ds6_cardDetails_button - backup_cardDetails_cbox); i++) {
			FrmShowObject(form, FrmGetObjectIndex(form, backup_cardDetails_cbox + i));
		}
	}
*/
}

/*
 *	set up the details form
 */
void setUpCardDetails(detailsType *details, screenModeType screenState)
{
	MemHandle oldHand;
	MemPtr tmp;
	char *namePtr;
	NumberFormatType numberFormat;
	LocalID app, sort;
	//UInt32 size;
	char dS, tS;
	char drawStr[30];

	PointType appInfoPos;
	PointType sortInfoPos;
	PointType mNumPos;
	PointType verPos;
	PointType recsPos;
	PointType sizePos;
	
	const PointType appInfoPos160x160 = {50, 113};
	const PointType sortInfoPos160x160 = {50, 128};
	const PointType mNumPos160x160 = {95, 98};
	const PointType verPos160x160 = {50, 83};
	const PointType recsPos160x160 = {50, 68};
	const PointType sizePos160x160 = {35, 53};
	
	const PointType appInfoPos240x240 = {BIG(50), BIG(113)};
	const PointType sortInfoPos240x240 = {BIG(50), BIG(128)};
	const PointType mNumPos240x240 = {BIG(95), BIG(98)};
	const PointType verPos240x240 = {BIG(50), BIG(83)};
	const PointType recsPos240x240 = {BIG(50), BIG(68)};
	const PointType sizePos240x240 = {BIG(35), BIG(53)};
	
	
	// get screen config
	if (screenState == screen160x160) {
		MemMove(&appInfoPos, &appInfoPos160x160, sizeof(PointType));
		MemMove(&sortInfoPos, &sortInfoPos160x160, sizeof(PointType));
		MemMove(&mNumPos, &mNumPos160x160, sizeof(PointType));
		MemMove(&verPos, &verPos160x160, sizeof(PointType));
		MemMove(&recsPos, &recsPos160x160, sizeof(PointType));
		MemMove(&sizePos, &sizePos160x160, sizeof(PointType));
	}
	else {
		MemMove(&appInfoPos, &appInfoPos240x240, sizeof(PointType));
		MemMove(&sortInfoPos, &sortInfoPos240x240, sizeof(PointType));
		MemMove(&mNumPos, &mNumPos240x240, sizeof(PointType));
		MemMove(&verPos, &verPos240x240, sizeof(PointType));
		MemMove(&recsPos, &recsPos240x240, sizeof(PointType));
		MemMove(&sizePos, &sizePos240x240, sizeof(PointType));
	}
	
	if (details->single) {
		if (details->general) {
				
			// init localization
			numberFormat = PrefGetPreference(prefNumberFormat);
			LocGetNumberSeparators(numberFormat, &tS, &dS);
		
			// app info and sort info
			DmDatabaseInfo(details->data.card[1].cardNo, details->data.card[1].lID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &app, &sort, NULL, NULL);
		
			if (app != 0) {
			
				if (MemLocalIDKind(app) == memIDPtr) {
					//size = MemPtrSize(MemLocalIDToPtr(app, details->data.card[1].cardNo));
				}
				else {
					tmp = MemLocalIDToLockedPtr(app, details->data.card[1].cardNo);
					//size = MemPtrSize(tmp);
					MemHandleUnlock(MemPtrRecoverHandle(tmp));
				}
			
				MemSet(drawStr, 30, 0);
				StrIToA(drawStr, details->data.card[1].modNum);
	
				preFormatNumberStr(drawStr);
	
				StrLocalizeNumber(drawStr, tS, dS);
				StrCat(drawStr, " Bytes");
				WinDrawChars(drawStr, StrLen(drawStr), appInfoPos.x, appInfoPos.y);
			}
			else {
				WinDrawChars("No AppInfo block", 16, appInfoPos.x, appInfoPos.y);
			}
		
			if (sort != 0) {
			
				if (MemLocalIDKind(sort) == memIDPtr) {			
					//size = MemPtrSize(MemLocalIDToPtr(sort, details->data.card[1].cardNo));
				}
				else {
					tmp = MemLocalIDToLockedPtr(sort, details->data.card[1].cardNo);
					//size = MemPtrSize(tmp);
					MemHandleUnlock(MemPtrRecoverHandle(tmp));
				}
			
				MemSet(drawStr, 30, 0);
				StrIToA(drawStr, details->data.card[1].modNum);
	
				preFormatNumberStr(drawStr);
	
				StrLocalizeNumber(drawStr, tS, dS);
				StrCat(drawStr, " Bytes");
				WinDrawChars(drawStr, StrLen(drawStr), sortInfoPos.x, sortInfoPos.y);
			}
			else {
				WinDrawChars("No SortInfo block", 17, sortInfoPos.x, sortInfoPos.y);
			}
		
			// modification number
			MemSet(drawStr, 30, 0);
			StrIToA(drawStr, details->data.card[1].modNum);
			preFormatNumberStr(drawStr);
			StrLocalizeNumber(drawStr, tS, dS);
			WinDrawChars(drawStr, StrLen(drawStr), mNumPos.x, mNumPos.y);
		
			// version
			MemSet(drawStr, 30, 0);
			StrIToA(drawStr, details->data.card[1].version);
			preFormatNumberStr(drawStr);
			StrLocalizeNumber(drawStr, tS, dS);
			WinDrawChars(drawStr, StrLen(drawStr), verPos.x, verPos.y);
		
			// records
			MemSet(drawStr, 30, 0);
			StrIToA(drawStr, details->data.card[1].numRecs);
			preFormatNumberStr(drawStr);
			StrLocalizeNumber(drawStr, tS, dS);
			WinDrawChars(drawStr, StrLen(drawStr), recsPos.x, recsPos.y);
		
			// size
			MemSet(drawStr, 30, 0);
			StrIToA(drawStr, details->data.card[1].size);
			preFormatNumberStr(drawStr);
			StrLocalizeNumber(drawStr, tS, dS);
			StrCat(drawStr, " Bytes");
			WinDrawChars(drawStr, StrLen(drawStr), sizePos.x, sizePos.y);
	
			// type
			// get the old text memhandle
			oldHand = FldGetTextHandle(GetObjectPtr(type_cardDetails_fld));
			namePtr = MemHandleLock(details->typeFldHand);
			// copy 4 bytes
			MemMove(namePtr, &details->data.card[1].typeId, 4);
			namePtr[4] = 0;
			MemHandleUnlock(details->typeFldHand);
			FldSetTextHandle(GetObjectPtr(type_cardDetails_fld), details->typeFldHand);
			// free the old text memhandle
			if (oldHand != NULL) MemHandleFree(oldHand);
			FldDrawField(GetObjectPtr(type_cardDetails_fld));
	
			// creator
			// get the old text memhandle
			oldHand = FldGetTextHandle(GetObjectPtr(crid_cardDetails_fld));
			namePtr = MemHandleLock(details->cridFldHand);
			// copy 4 bytes
			MemMove(namePtr, &details->data.card[1].crid, 4);
			namePtr[4] = 0;
			MemHandleUnlock(details->cridFldHand);
			FldSetTextHandle(GetObjectPtr(crid_cardDetails_fld), details->cridFldHand);
			// free the old text memhandle
			if (oldHand != NULL) MemHandleFree(oldHand);
			FldDrawField(GetObjectPtr(crid_cardDetails_fld));

			// name
			// get the old text memhandle
			oldHand = FldGetTextHandle(GetObjectPtr(name_cardDetails_fld));
			namePtr = MemHandleLock(details->nameFldHand);
			StrCopy(namePtr, details->data.card[1].name);
			MemHandleUnlock(details->nameFldHand);
			FldSetTextHandle(GetObjectPtr(name_cardDetails_fld), details->nameFldHand);
			// free the old text memhandle
			if (oldHand != NULL) MemHandleFree(oldHand);
			FldDrawField(GetObjectPtr(name_cardDetails_fld));
		}
		else {	// attributes + dates
			// dates
			drawCardDetailsDateTime(details, screenState);
		
			// attributes
			CtlSetValue(GetObjectPtr(backup_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrBackup) == dmHdrAttrBackup));
			CtlSetValue(GetObjectPtr(bundled_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrBundle) == dmHdrAttrBundle));
			CtlSetValue(GetObjectPtr(copy_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrCopyPrevention) == dmHdrAttrCopyPrevention));
			CtlSetValue(GetObjectPtr(dirty_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrAppInfoDirty) == dmHdrAttrAppInfoDirty));		
			CtlSetValue(GetObjectPtr(hidden_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrHidden) == dmHdrAttrHidden));
			CtlSetValue(GetObjectPtr(launch_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrLaunchableData) == dmHdrAttrLaunchableData));
			CtlSetValue(GetObjectPtr(install_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrOKToInstallNewer) == dmHdrAttrOKToInstallNewer));
			CtlSetValue(GetObjectPtr(open_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrOpen) == dmHdrAttrOpen));
			CtlSetValue(GetObjectPtr(ro_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrReadOnly) == dmHdrAttrReadOnly));
			CtlSetValue(GetObjectPtr(recycle_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrRecyclable) == dmHdrAttrRecyclable));
			CtlSetValue(GetObjectPtr(reset_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrResetAfterInstall) == dmHdrAttrResetAfterInstall));
			CtlSetValue(GetObjectPtr(res_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrResDB) == dmHdrAttrResDB));
			CtlSetValue(GetObjectPtr(stream_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrStream) == dmHdrAttrStream));
		}
	}

// only single db mode !!!
	
/*	else {
		// dates
		drawCardDetailsDateTime(details, screenState);
		
		// attributes
		CtlSetValue(GetObjectPtr(backup_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrBackup) == dmHdrAttrBackup));
		CtlSetValue(GetObjectPtr(bundled_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrBundle) == dmHdrAttrBundle));
		CtlSetValue(GetObjectPtr(copy_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrCopyPrevention) == dmHdrAttrCopyPrevention));
		CtlSetValue(GetObjectPtr(dirty_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrAppInfoDirty) == dmHdrAttrAppInfoDirty));		
		CtlSetValue(GetObjectPtr(hidden_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrHidden) == dmHdrAttrHidden));
		CtlSetValue(GetObjectPtr(launch_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrLaunchableData) == dmHdrAttrLaunchableData));
		CtlSetValue(GetObjectPtr(install_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrOKToInstallNewer) == dmHdrAttrOKToInstallNewer));
		CtlSetValue(GetObjectPtr(open_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrOpen) == dmHdrAttrOpen));
		CtlSetValue(GetObjectPtr(ro_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrReadOnly) == dmHdrAttrReadOnly));
		CtlSetValue(GetObjectPtr(recycle_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrRecyclable) == dmHdrAttrRecyclable));
		CtlSetValue(GetObjectPtr(reset_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrResetAfterInstall) == dmHdrAttrResetAfterInstall));
		CtlSetValue(GetObjectPtr(res_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrResDB) == dmHdrAttrResDB));
		CtlSetValue(GetObjectPtr(stream_cardDetails_cbox), ((details->data.card[1].attribs & dmHdrAttrStream) == dmHdrAttrStream));
	}
*/
}

/*
 *	popUp the list and set the date to current date/time or let the user choose a date (no time)
 */
Boolean getNewDateCardDetails(detailsType *details, UInt16 controlID)
{
	const char title[16] = {"Select New Date\0"};
	Int16 selection;
	DateTimeType curDate;
	
	
	selection = LstPopupList(GetObjectPtr(d3_cardDetails_lst - (ds3_cardDetails_button - controlID)));
		
	if (selection != noListSelection) {
		if (selection == 0) {	// NOW
			switch (ds3_cardDetails_button - controlID) {
			case	2:
				details->data.card[1].crDate = TimGetSeconds();
				break;
			case	1:
				details->data.card[1].moDate = TimGetSeconds();
				break;
			case	0:
				details->data.card[1].acDate = TimGetSeconds();
				break;
			}
			
			return(true);
		}
		else if (selection == 1) {	// CHOOSE
			switch (ds3_cardDetails_button - controlID) {
			case	2:
				TimSecondsToDateTime(details->data.card[1].crDate, &curDate);
				if (SelectDay(selectDayByDay, &curDate.month, &curDate.day, &curDate.year, title)) {
					details->data.card[1].crDate = TimDateTimeToSeconds(&curDate);
					
					return(true);
				}
				break;
			case	1:
				TimSecondsToDateTime(details->data.card[1].moDate, &curDate);
				if (SelectDay(selectDayByDay, &curDate.month, &curDate.day, &curDate.year, title)) {
					details->data.card[1].moDate = TimDateTimeToSeconds(&curDate);
					
					return(true);
				}
				break;
			case	0:
				TimSecondsToDateTime(details->data.card[1].acDate, &curDate);
				if (SelectDay(selectDayByDay, &curDate.month, &curDate.day, &curDate.year, title)) {
					details->data.card[1].acDate = TimDateTimeToSeconds(&curDate);
					
					return(true);
				}
				break;
			}
		}
	}
	
	return(false);
}

/*
 *	get a new time from the user
 */
Boolean getNewTimeCardDetails(detailsType *details, UInt16 controlID)
{
	const char title[16] = {"Select New Time\0"};
	DateTimeType curDate;
	
	
	switch (ds6_cardDetails_button - controlID) {
	case	2:
		TimSecondsToDateTime(details->data.card[1].crDate, &curDate);
		if (SelectOneTime(&curDate.hour, &curDate.minute, title)) {
			details->data.card[1].crDate = TimDateTimeToSeconds(&curDate);
			
			return(true);
		}
		break;
	case	1:
		TimSecondsToDateTime(details->data.card[1].moDate, &curDate);
		if (SelectOneTime(&curDate.hour, &curDate.minute, title)) {
			details->data.card[1].moDate = TimDateTimeToSeconds(&curDate);
			
			return(true);
		}
		break;
	case	0:
		TimSecondsToDateTime(details->data.card[1].acDate, &curDate);
		if (SelectOneTime(&curDate.hour, &curDate.minute, title)) {
			details->data.card[1].acDate = TimDateTimeToSeconds(&curDate);
			
			return(true);
		}
		break;
	}
	
	return(false);
}

/*
 *	before saving the changes we need to get them
 */
void getCardDetailsAttrName(detailsType *details, Boolean general)
{
	char *textPtr;
	
	
	if (!general) {
	
		// clear all attribs
		details->data.card[1].attribs = 0;
	
		// get attributes
		if (CtlGetValue(GetObjectPtr(backup_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrBackup;
		if (CtlGetValue(GetObjectPtr(bundled_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrBundle;
		if (CtlGetValue(GetObjectPtr(copy_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrCopyPrevention;
		if (CtlGetValue(GetObjectPtr(dirty_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrAppInfoDirty;
		if (CtlGetValue(GetObjectPtr(hidden_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrHidden;
		if (CtlGetValue(GetObjectPtr(launch_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrLaunchableData;
		if (CtlGetValue(GetObjectPtr(install_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrOKToInstallNewer;
		if (CtlGetValue(GetObjectPtr(ro_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrReadOnly;
		if (CtlGetValue(GetObjectPtr(recycle_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrRecyclable;
		if (CtlGetValue(GetObjectPtr(reset_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrResetAfterInstall;
		if (CtlGetValue(GetObjectPtr(stream_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrStream;
		
		if (CtlGetValue(GetObjectPtr(res_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrResDB;
		if (CtlGetValue(GetObjectPtr(open_cardDetails_cbox))) details->data.card[1].attribs |= dmHdrAttrOpen;
	}
	else {
		textPtr = FldGetTextPtr(GetObjectPtr(name_cardDetails_fld));
		if (textPtr != NULL) {
			StrCopy(details->data.card[1].name, textPtr);
		}
		
		textPtr = FldGetTextPtr(GetObjectPtr(type_cardDetails_fld));
		if (textPtr != NULL) {
			MemMove(&details->data.card[1].typeId, textPtr, 4);
		}
		
		textPtr = FldGetTextPtr(GetObjectPtr(crid_cardDetails_fld));
		if (textPtr != NULL) {
			MemMove(&details->data.card[1].crid, textPtr, 4);
		}
	}
}

/*
 *	save the changed details of a database
 */
Err doChangeCardDetails(global_data_type *glb)
{
	Err retVal = errNone;
	char *namePtr;
//	UInt32 i, i2;
	
	
	if (glb->brwLst[glb->currentBrw].lst.data.card.cardInfo.typeId == cardTypeRam) {
		
		if (glb->details->single) {
		
			namePtr = NULL;
			// check if the name was changed
			if (StrCompare(glb->details->data.card[1].name, glb->details->data.card[0].name) != 0) {
				namePtr = glb->details->data.card[1].name;
			}
		
			// set the new values
			retVal = DmSetDatabaseInfo(glb->details->data.card[1].cardNo, glb->details->data.card[1].lID, namePtr, &glb->details->data.card[1].attribs,
				&glb->details->data.card[1].version, &glb->details->data.card[1].crDate, &glb->details->data.card[1].moDate, &glb->details->data.card[1].acDate,
				&glb->details->data.card[1].modNum, NULL, NULL, &glb->details->data.card[1].typeId, &glb->details->data.card[1].crid);
		}
		
// only single db mode at the moment !!!
		
/*		else {
			// find selected dbs
			i = 0;
			i2 = 0;
			while (i < glb->brwLst[glb->currentBrw].lst.data.card.numDBs && i2 < glb->brwLst[glb->currentBrw].lst.data.card.numSelDBs) {	
				if (glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].selected) {
					// set the new values
					// only attribs and dates !!!
					retVal = DmSetDatabaseInfo(glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].cardNo, glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].lID, NULL, &glb->details->data.card[1].attribs,
						NULL, &glb->details->data.card[1].crDate, &glb->details->data.card[1].moDate, &glb->details->data.card[1].acDate,
						NULL, NULL, NULL, NULL, NULL);
					
					if (retVal != errNone) {
						// display an error message if the change failed !
						if (FrmCustomAlert(alertID_cardDetailsUnknownError, glb->brwLst[glb->currentBrw].lst.data.card.dbList[i].name, NULL, NULL) != 0) {
							break;
						}
					}
					
					// inc num of selected modified files
					i2++;
				}
				// goto next file in list
				i++;
			}
			
			// multiple dbs all ways return No Error
			retVal = errNone;
		}
*/
	}
	else if (glb->brwLst[glb->currentBrw].lst.data.card.cardInfo.typeId == cardTypeRom) {
		// can't modify databases in rom !
		retVal = 1;
	}
	
	return(retVal);
}

// ----=== VOL DETAILS ===---

/*
 *	before saving the changes we need to get them
 */
void getVolDetailsAttrName(detailsType *details)
{
	char *textPtr;
	
	
	// clear all attribs
	details->data.vol[1].attribs = 0;
	
	if (CtlGetValue(GetObjectPtr(archive_volDetails_cbox))) details->data.vol[1].attribs |= vfsFileAttrArchive;
	if (CtlGetValue(GetObjectPtr(hidden_volDetails_cbox))) details->data.vol[1].attribs |= vfsFileAttrHidden;
	if (CtlGetValue(GetObjectPtr(system_volDetails_cbox))) details->data.vol[1].attribs |= vfsFileAttrSystem;
	if (CtlGetValue(GetObjectPtr(link_volDetails_cbox))) details->data.vol[1].attribs |= vfsFileAttrLink;
	if (CtlGetValue(GetObjectPtr(readonly_volDetails_cbox))) details->data.vol[1].attribs |= vfsFileAttrReadOnly;
	
	if (details->single) {
		textPtr = FldGetTextPtr(GetObjectPtr(name_volDetails_fld));
		if (textPtr != NULL) {
			StrCopy(details->data.vol[1].name, textPtr);
		}
	}
}

/*
 *	draw / update the date time in the vol details form
 */
void drawVolDetailsDateTime(detailsType *details, screenModeType screenState)
{
	const RectangleType rect160x160[6] = {{{60,90}, {42,12}}, {{110,90}, {42,12}}, {{60,105}, {42,12}}, {{110,105}, {42,12}}, {{60,120}, {42,12}}, {{110,120}, {42,12}}};
	const RectangleType rect240x240[6] = {{{BIG(60),BIG(90)+1}, {BIG(42),BIG(12)}}, {{BIG(110),BIG(90)+1}, {BIG(42),BIG(12)}}, {{BIG(60),BIG(105)+2}, {BIG(42),BIG(12)-1}}, {{BIG(110),BIG(105)+2}, {BIG(42),BIG(12)-1}}, {{BIG(60),BIG(120)+1}, {BIG(42),BIG(12)}}, {{BIG(110),BIG(120)+1}, {BIG(42),BIG(12)}}};
	RectangleType rect[6];
	UInt8 i;
	char drawStr[30];
	

	// get screen config
	if (screenState == screen160x160) {
		MemMove(&rect, &rect160x160, sizeof(rect160x160));
	}
	else {
		MemMove(&rect, &rect240x240, sizeof(rect240x240));
	}
	
	for (i = 0; i < 6; i++) {
		WinEraseRectangle(&rect[i], 0);
	}
	
	// date / time
	MemSet(drawStr, 30, 0);
	makeDisplayDate(details->data.vol[1].crDate, drawStr);
	WinDrawChars(drawStr, StrLen(drawStr), rect[0].topLeft.x + 1, rect[0].topLeft.y);
	MemSet(drawStr, 30, 0);
	makeDisplayTime(details->data.vol[1].crDate, drawStr);
	WinDrawChars(drawStr, StrLen(drawStr), rect[1].topLeft.x + 1, rect[1].topLeft.y);
	
	MemSet(drawStr, 30, 0);
	makeDisplayDate(details->data.vol[1].moDate, drawStr);
	WinDrawChars(drawStr, StrLen(drawStr), rect[2].topLeft.x + 1, rect[2].topLeft.y);
	MemSet(drawStr, 30, 0);
	makeDisplayTime(details->data.vol[1].moDate, drawStr);
	WinDrawChars(drawStr, StrLen(drawStr), rect[3].topLeft.x + 1, rect[3].topLeft.y);
	
	MemSet(drawStr, 30, 0);
	makeDisplayDate(details->data.vol[1].acDate, drawStr);
	WinDrawChars(drawStr, StrLen(drawStr),rect[4].topLeft.x + 1, rect[4].topLeft.y);
	MemSet(drawStr, 30, 0);
	makeDisplayTime(details->data.vol[1].acDate, drawStr);
	WinDrawChars(drawStr, StrLen(drawStr), rect[5].topLeft.x + 1, rect[5].topLeft.y);
}

/*
 *	set up the details form
 * - accesses globals
 */
void setUpVolDetails(detailsType *details, screenModeType screenState)
{
	const char dirStr[12] = {"<Directory>\0"};
	char drawStr[30];
	char tS, dS;
	char *namePtr;
	MemHandle oldHand;
	FormPtr form;
	const PointType sizePos160x160 = {35, 38};
	const PointType sizePos240x240 = {BIG(35), BIG(38)};
	PointType sizePos;
		
	
	if (screenState == screen160x160) {
		MemMove(&sizePos, &sizePos160x160, sizeof(PointType));
	}
	else {
		MemMove(&sizePos, &sizePos240x240, sizeof(PointType));
	}
	
	// attributes
	CtlSetValue(GetObjectPtr(archive_volDetails_cbox), ((details->data.vol[1].attribs & vfsFileAttrArchive) == vfsFileAttrArchive));
	CtlSetValue(GetObjectPtr(hidden_volDetails_cbox), ((details->data.vol[1].attribs & vfsFileAttrHidden) == vfsFileAttrHidden));
	CtlSetValue(GetObjectPtr(link_volDetails_cbox), ((details->data.vol[1].attribs & vfsFileAttrLink) == vfsFileAttrLink));
	CtlSetValue(GetObjectPtr(system_volDetails_cbox), ((details->data.vol[1].attribs & vfsFileAttrSystem) == vfsFileAttrSystem));
	CtlSetValue(GetObjectPtr(readonly_volDetails_cbox), ((details->data.vol[1].attribs & vfsFileAttrReadOnly) == vfsFileAttrReadOnly));
	
	if (details->single) {
	
		// this is a dir
		if ((details->data.vol[1].attribs & vfsFileAttrDirectory) == vfsFileAttrDirectory) {
			form = FrmGetActiveForm();
			FrmHideObject(form, FrmGetObjectIndex(form, size_volDetails_label));
			
			if (screenState == screen160x160) {
				FntSetFont(1);
				
				WinDrawChars(dirStr, 11, (160/2) - (FntLineWidth(dirStr, 11) / 2), 38);
			}
/*
			else {
				FntSetFont(VgaBaseToVgaFont(1));
					
				WinDrawChars(dirStr, 11, (240/2) - (FntLineWidth(dirStr, 11) / 2), BIG(38));
			}
*/
			
			FntSetFont(glob->cfg.font0);
		}	
		else {	// this is a file
			// size
			MemSet(drawStr, 30, 0);
			LocGetNumberSeparators(PrefGetPreference(prefNumberFormat), &tS, &dS);
			StrIToA(drawStr, details->data.vol[1].size);
	
			preFormatNumberStr(drawStr);
						
			StrLocalizeNumber(drawStr, tS, dS);
			StrCat(drawStr, " Bytes");
			WinDrawChars(drawStr, StrLen(drawStr), sizePos.x, sizePos.y);
		}

		// name
		// get the old text memhandle
		oldHand = FldGetTextHandle(GetObjectPtr(name_volDetails_fld));
		namePtr = MemHandleLock(details->nameFldHand);
		StrCopy(namePtr, details->data.vol[1].name);
		MemHandleUnlock(details->nameFldHand);
		FldSetTextHandle(GetObjectPtr(name_volDetails_fld), details->nameFldHand);
		// free the old text memhandle
		if (oldHand != NULL) MemHandleFree(oldHand);
		FldDrawField(GetObjectPtr(name_volDetails_fld));
	}
}

/*
 *	show / hide the "Save / Cancel" and "Done" buttons
 */
void showHideVolDetailsButton(Boolean show)
{
	FormPtr form = FrmGetActiveForm();
	
	
	if (show) {
		FrmHideObject(form, FrmGetObjectIndex(form, done_volDetails_button));
		FrmShowObject(form, FrmGetObjectIndex(form, save_volDetails_button));
		FrmShowObject(form, FrmGetObjectIndex(form, cancel_volDetails_button));
	}
	else {
		FrmHideObject(form, FrmGetObjectIndex(form, save_volDetails_button));
		FrmHideObject(form, FrmGetObjectIndex(form, cancel_volDetails_button));
		FrmShowObject(form, FrmGetObjectIndex(form, done_volDetails_button));
	}
}

/*
 *	get a new time from the user
 */
Boolean getNewTimeVolDetails(detailsType *details, UInt16 controlID)
{
	const char title[16] = {"Select New Time\0"};
	DateTimeType curDate;
	
	
	switch (ds6_volDetails_button - controlID) {
	case	2:
		TimSecondsToDateTime(details->data.vol[1].crDate, &curDate);
		if (SelectOneTime(&curDate.hour, &curDate.minute, title)) {
			details->data.vol[1].crDate = TimDateTimeToSeconds(&curDate);
			
			return(true);
		}
		break;
	case	1:
		TimSecondsToDateTime(details->data.vol[1].moDate, &curDate);
		if (SelectOneTime(&curDate.hour, &curDate.minute, title)) {
			details->data.vol[1].moDate = TimDateTimeToSeconds(&curDate);
			
			return(true);
		}
		break;
	case	0:
		TimSecondsToDateTime(details->data.vol[1].acDate, &curDate);
		if (SelectOneTime(&curDate.hour, &curDate.minute, title)) {
			details->data.vol[1].acDate = TimDateTimeToSeconds(&curDate);
			
			return(true);
		}
		break;
	}
	
	return(false);
}

/*
 *	popUp the list and set the date to current date/time or let the user choose a date (no time)
 */
Boolean getNewDateVolDetails(detailsType *details, UInt16 controlID)
{
	const char title[16] = {"Select New Date\0"};
	Int16 selection;
	DateTimeType curDate;
	
	
	selection = LstPopupList(GetObjectPtr(d3_volDetails_lst - (ds3_volDetails_button - controlID)));
		
	if (selection != noListSelection) {
		if (selection == 0) {	// NOW
			switch (ds3_volDetails_button - controlID) {
			case	2:
				details->data.vol[1].crDate = TimGetSeconds();
				break;
			case	1:
				details->data.vol[1].moDate = TimGetSeconds();
				break;
			case	0:
				details->data.vol[1].acDate = TimGetSeconds();
				break;
			}
			
			return(true);
		}
		else if (selection == 1) {	// CHOOSE
			switch (ds3_volDetails_button - controlID) {
			case	2:
				TimSecondsToDateTime(details->data.vol[1].crDate, &curDate);
				if (SelectDay(selectDayByDay, &curDate.month, &curDate.day, &curDate.year, title)) {
					details->data.vol[1].crDate = TimDateTimeToSeconds(&curDate);
					
					return(true);
				}
				break;
			case	1:
				TimSecondsToDateTime(details->data.vol[1].moDate, &curDate);
				if (SelectDay(selectDayByDay, &curDate.month, &curDate.day, &curDate.year, title)) {
					details->data.vol[1].moDate = TimDateTimeToSeconds(&curDate);
					
					return(true);
				}
				break;
			case	0:
				TimSecondsToDateTime(details->data.vol[1].acDate, &curDate);
				if (SelectDay(selectDayByDay, &curDate.month, &curDate.day, &curDate.year, title)) {
					details->data.vol[1].acDate = TimDateTimeToSeconds(&curDate);
					
					return(true);
				}
				break;
			}
		}
	}
	
	return(false);
}

/*
 *	save the changed details of a file or directory !
 */
Err doChangeVolDetails(global_data_type *glb)
{
	char *fullfilepath;
	FileRef fp;
	Err retVal;
	
	
	// volume is readonly !
	if ((glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volInfo.attributes & vfsVolumeAttrReadOnly) == vfsVolumeAttrReadOnly) {
		return(1);
	}
	
	// get mem
	fullfilepath = MemPtrNew(StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.currentDir) + StrLen(glb->details->data.vol[0].name) + 2);
	// make fullpath
	StrCopy(fullfilepath, glb->brwLst[glb->currentBrw].lst.data.vol.currentDir);
	if (fullfilepath[StrLen(fullfilepath)-1] != '/') StrCat(fullfilepath, "/");
	StrCat(fullfilepath, glb->details->data.vol[0].name);
	
	if ((retVal = VFSFileOpen(glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, fullfilepath, vfsModeReadWrite, &fp)) != errNone) {
		
		// free filepath memory
		MemPtrFree(fullfilepath);
		
		return(retVal);
	}
	else {
		// clear readOnly bit if present !
		if ((glb->details->data.vol[0].attribs & vfsFileAttrReadOnly) == vfsFileAttrReadOnly) {
			if ((retVal = VFSFileSetAttributes(fp, ((glb->details->data.vol[0].attribs - vfsFileAttrReadOnly) - (((glb->details->data.vol[0].attribs & vfsFileAttrDirectory) == vfsFileAttrDirectory) ? vfsFileAttrDirectory : 0)))) != errNone) {
				
				// THIS LOOKS LIKE A SPECIAL FILE (E.G. MEMSTICK.IND) WICH IS PROTECTED BY THE "FS LIBRARY"
				
				// close file
				VFSFileClose(fp);
				
				// free filepath memory
				MemPtrFree(fullfilepath);
				
				return(retVal);
			}
		}
		
		// set time date
		if (glb->details->data.vol[0].crDate != glb->details->data.vol[1].crDate) {
			VFSFileSetDate(fp, vfsFileDateCreated, glb->details->data.vol[1].crDate);
		}
		if (glb->details->data.vol[0].moDate != glb->details->data.vol[1].moDate) {
			VFSFileSetDate(fp, vfsFileDateModified, glb->details->data.vol[1].moDate);
		}
		if (glb->details->data.vol[0].acDate != glb->details->data.vol[1].acDate) {
			VFSFileSetDate(fp, vfsFileDateAccessed, glb->details->data.vol[1].acDate);
		}
				
		// rename
		if (StrCompare(glb->details->data.vol[0].name, glb->details->data.vol[1].name) != 0) {
			
			// close file
			VFSFileClose(fp);		
			
			if ((retVal = VFSFileRename(glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, fullfilepath, glb->details->data.vol[1].name)) != errNone) {
			}
			else {
				MemPtrFree(fullfilepath);
				// get mem
				fullfilepath = MemPtrNew(StrLen(glb->brwLst[glb->currentBrw].lst.data.vol.currentDir) + StrLen(glb->details->data.vol[1].name) + 2);
				// make fullpath
				StrCopy(fullfilepath, glb->brwLst[glb->currentBrw].lst.data.vol.currentDir);
				if (fullfilepath[StrLen(fullfilepath)-1] != '/') StrCat(fullfilepath, "/");
				StrCat(fullfilepath, glb->details->data.vol[1].name);
			}
			
			// we can be sure that this works!
			VFSFileOpen(glb->brwLst[glb->currentBrw].lst.data.vol.volInfo.volRefNum, fullfilepath, vfsModeReadWrite, &fp);
			
		}
		
		// set the attributes
		if (glb->details->data.vol[0].attribs != glb->details->data.vol[1].attribs || (glb->details->data.vol[0].attribs & vfsFileAttrReadOnly) == vfsFileAttrReadOnly) {
			VFSFileSetAttributes(fp, glb->details->data.vol[1].attribs);
		}
		// close file
		VFSFileClose(fp);
		
		// free filepath memory
		MemPtrFree(fullfilepath);
		
		return(retVal);
	}
}
