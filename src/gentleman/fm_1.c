/*
 *	file:		fm_1.c
 *	project:	GentleMan
 *	content:	main app functions
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

global_data_type *gglobals;

// -= prototypes =-
static Boolean about_Handler(EventPtr event);
static Boolean pluginMgr_Handler(EventPtr event);
static Boolean prefs_Handler(EventPtr event);
static Boolean main_Handler(EventPtr event);
static Boolean volDetails_Handler(EventPtr event);
static Boolean cardDetails_Handler(EventPtr event);
static Boolean mkdir_Handler(EventPtr event);
static Boolean copy_Handler(EventPtr event);
static void EventLoop(void);

static int StartApplication()
{	
	if (!checkBetaDate()) {
		FrmAlert(alertID_betaExpired);
		// exit app
		return(1);
	}
	
	if (checkPalmOSVersion()) {
		// init app
		init();
		// jump to the main form
		FrmGotoForm(formID_main);
	}
	else {		
		FrmAlert(alertID_palmOSVersion);
		// exit app
		return(1);
	}
		
	return(0);
}

static void StopApplication()
{
	deinit();
}

UInt32 PilotMain (UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
	Err error;

	
	if (cmd == sysAppLaunchCmdNormalLaunch) {
		error = StartApplication();
		if (error) return(error);

		EventLoop();

		StopApplication();
	}
	
	return(0);
}       

static void EventLoop(void)
{
	Err err;
	UInt16 formID;
	FormPtr form;
	EventType event;
	RectangleType rect;


	do {
		EvtGetEvent(&event, -1);

		if (SysHandleEvent(&event)) continue;
		if (MenuHandleEvent((void *)0, &event, &err)) continue;

		if (event.eType == frmLoadEvent) {
			formID = event.data.frmLoad.formID;
			form = FrmInitForm(formID);
			FrmSetActiveForm(form);

			switch (formID) {
			case	formID_about:
				FrmSetEventHandler(form, (FormEventHandlerPtr) about_Handler);
				break;
			case	formID_pluginMgr:
				FrmSetEventHandler(form, (FormEventHandlerPtr) pluginMgr_Handler);
				break;
			case	formID_prefs:
				FrmSetEventHandler(form, (FormEventHandlerPtr) prefs_Handler);
				break;
			case	formID_main:
				// size the window
				rect.topLeft.x = 0;
				rect.topLeft.y = 0;
				WinGetDisplayExtent(&rect.extent.x, &rect.extent.y);
				WinSetBounds(FrmGetWindowHandle(FrmGetActiveForm()), &rect);
				// set the event handler
				FrmSetEventHandler(form, (FormEventHandlerPtr) main_Handler);
				break;
			case	formID_volDetails:
				FrmSetEventHandler(form, (FormEventHandlerPtr) volDetails_Handler);
				break;
			case	formID_cardDetails:
				FrmSetEventHandler(form, (FormEventHandlerPtr) cardDetails_Handler);
				break;
			case	formID_mkdir:
				FrmSetEventHandler(form, (FormEventHandlerPtr) mkdir_Handler);
				break;
			case	formID_copy:
				FrmSetEventHandler(form, (FormEventHandlerPtr) copy_Handler);
				break;
			}
		}

		FrmDispatchEvent(&event);

	} while (event.eType != appStopEvent);
}

/*
 *	main form event handler
 */
static Boolean main_Handler(EventPtr event)
{
	Boolean handled = false;
	FormPtr form;

	
	//CALLBACK_PROLOGUE

	switch (event->eType) {

	//case	displayExtentChangedEvent:
		// we have to handle this event other wise the event is repeated over and over
		//handled = true;
	case	frmUpdateEvent:		
	case	frmOpenEvent:
		form = FrmGetActiveForm();
		
		// check display size and change it if needed
		handleResizeDisplayEvent();
				
/*
		if (glob->cfg.screenState != screen160x160 && !glob->cfg.formModified) {
			// modify form for 240x240 / 240x300
			VgaFormModify(form, vgaFormModify160To240);
			// remember the modification
			glob->cfg.formModified = true;
		}
*/
				
		FrmDrawForm(form);
		
		// init display stuff
		setDisplayStuff(&glob->cfg, glob->brwLst[glob->currentBrw].brwMode);
		
		// scrollbar
		setUpFileBrowser(&glob->brwLst[glob->currentBrw]);
		
		// browser
		setUpBrwList(glob);
		
		// volumes
		setUpVolList(&glob->vLst, glob->brwLst[glob->currentBrw].volIndex);
		
		// sort / show1 / show2
		setUpSortShow(&glob->brwLst[glob->currentBrw]);
				
		// draw infos about selected files
		drawSelectedInfo(&glob->brwLst[glob->currentBrw]);
		
		// draw the file/database list
		drawFileList(&glob->brwLst[glob->currentBrw]);
		break;

	case	ctlSelectEvent:
		switch (event->data.ctlSelect.controlID) {
		case	brw1_main_button:
		case	brw2_main_button:
			doBrwSelect(glob, event->data.ctlSelect.controlID);
			handled = true;
			break;
		case	volst_main_button:
			// popup the volume list
			doVolPopupList(&glob->brwLst[glob->currentBrw], &glob->vLst);
			handled = true;
			break;
		case	sort_main_ptrg:
			// select a sort method
			doSortPopupList(&glob->brwLst[glob->currentBrw]);
			handled = true;
			break;
		case	show1_main_ptrg:
			// select a attribute to be displayed at pos 1
			doShowPopupList(&glob->brwLst[glob->currentBrw], show1_main_ptrg);
			handled = true;
			break;
		case	show2_main_ptrg:
			// select a attribute to be displayed at pos 2
			doShowPopupList(&glob->brwLst[glob->currentBrw], show2_main_ptrg);
			handled = true;
			break;
		}
		break;
		
	case	keyDownEvent:
		// hardbuttons !
		handled = handleKeyDown(&glob->brwLst[glob->currentBrw], glob, event);
		break;
	
	case	penDownEvent:
		// pen tapping
		handled = handlePenDown(&glob->brwLst[glob->currentBrw], glob, event->screenX, event->screenY);
		break;
	case	penMoveEvent:
		// pen tapping
		handled = handlePenMove(&glob->brwLst[glob->currentBrw], glob, event->screenX, event->screenY);
		break;
	case	penUpEvent:
		// pen tapping
		handled = handlePenUp(&glob->brwLst[glob->currentBrw], glob, event->screenX, event->screenY);
		break;
			
	case	sclExitEvent:
		glob->brwLst[glob->currentBrw].scrollPos = event->data.sclExit.newValue;
		drawFileList(&glob->brwLst[glob->currentBrw]);
		break;
	case	sclRepeatEvent:
		glob->brwLst[glob->currentBrw].scrollPos = event->data.sclExit.newValue;
		drawFileList(&glob->brwLst[glob->currentBrw]);
		break;

	case	menuEvent:
		switch (event->data.menu.itemID) {
		case	cmd_copy:
			handled = true;
			doCopy(glob);
			break;
		case	cmd_delete:
			handled = true;
			// all errors will be handled inside of doDelete(...)
			doDelete(&glob->brwLst[glob->currentBrw], &glob->prefs);
			// force a redraw of the main form 
			{
				EventType newEvent;
				newEvent.eType = frmUpdateEvent;
				EvtAddEventToQueue(&newEvent);
			}
			break;
		case	cmd_mkdir:
			handled = true;
			FrmPopupForm(formID_mkdir);
			break;
		case	cmd_fdetails:
			handled = true;
			glob->cfg.formModified = false;
			// all errors are handled inside of doDetails(...)
			doDetails(glob, 0, false);
			break;
		case	cmd_sellall:
			handled = true;
			// changed selection state
			doUnSelect(&glob->brwLst[glob->currentBrw], true);
			// force a redraw of the main form 
			{
				EventType newEvent;
				newEvent.eType = frmUpdateEvent;
				EvtAddEventToQueue(&newEvent);
			}
			break;
		case	cmd_unsellall:
			handled = true;
			// change selection state
			doUnSelect(&glob->brwLst[glob->currentBrw], false);
			// force a redraw of the main form 
			{
				EventType newEvent;
				newEvent.eType = frmUpdateEvent;
				EvtAddEventToQueue(&newEvent);
			}
			break;
		case	cmd_beam:
			handled = true;
			// handle error internly
			doBeam(&glob->brwLst[glob->currentBrw]);
			break;
		case	cmd_about:
			handled = true;
			glob->cfg.formModified = false;
			FrmGotoForm(formID_about);
			break;
		case	cmd_beamGentleMan:
			handled = true;
			beamGentleMan();
			break;
		case	cmd_prefs:
			handled = true;
			glob->cfg.formModified = false;
			FrmGotoForm(formID_prefs);
			break;
			
		case	cmd_pluginMgr:
			handled = true;
			glob->cfg.formModified = false;
			FrmGotoForm(formID_pluginMgr);
			break;
		case	cmd_pluginActivate:
			handled = true;
			// all errors are handled inside of doRunPlugin(...)
			doRunPlugin(&glob->brwLst[glob->currentBrw], &glob->pluginLst);
			// force a redraw of the main form 
			{
				EventType newEvent;
				newEvent.eType = frmUpdateEvent;
				EvtAddEventToQueue(&newEvent);
			}
			break;
		}
		break;
		
	case	nilEvent:
		handled = true;
		break;

	default:
		break;
	}

	//CALLBACK_EPILOGUE
	
	return(handled);
}

/*
 *	volume file details form event handler
 */
static Boolean volDetails_Handler(EventPtr event)
{
	Boolean handled = false;
	FormPtr form;
	Err error;
	
	
	//CALLBACK_PROLOGUE

	switch (event->eType) {

	case    frmOpenEvent:
		form = FrmGetActiveForm();
		
/*
		if (glob->cfg.screenState != screen160x160) {
			// modify form for 240x240 / 240x300
			VgaFormModify(form, vgaFormModify160To240);
		}
*/
				
		FrmDrawForm(form);
		// draw everything
		setUpVolDetails(glob->details, glob->cfg.screenState);
		drawVolDetailsDateTime(glob->details, glob->cfg.screenState);
		break;
	
/*
	case	GMCloseFormEvent:
		handled = true;
		FldSetTextHandle(GetObjectPtr(name_volDetails_fld), NULL);
		freeDetails(glob, false);
		FrmGotoForm(formID_main);
		break;
*/

	case	ctlSelectEvent:
		switch (event->data.ctlSelect.controlID) {
		case	done_volDetails_button:
			handled = true;
			FldSetTextHandle(GetObjectPtr(name_volDetails_fld), NULL);
			freeDetails(glob, false);
			FrmGotoForm(formID_main);
			break;
		case	cancel_volDetails_button:
			handled = true;
			FldSetTextHandle(GetObjectPtr(name_volDetails_fld), NULL);
			freeDetails(glob, false);
			FrmGotoForm(formID_main);
			break;
		case	save_volDetails_button:
			handled = true;
			getVolDetailsAttrName(glob->details);
			if ((error = doChangeVolDetails(glob)) != errNone) {
				switch (error) {
				case	1:
					FrmAlert(alertID_volReadOnly);
					break;
				case	vfsErrFileAlreadyExists:
				case	vfsErrBadName:
					FrmAlert(alertID_volFileName);
					break;
				case	vfsErrFilePermissionDenied:
					FrmAlert(alertID_volPerms);
					break;
				case	vfsErrVolumeFull:
					FrmAlert(alertID_volFull);
					break;
				case	vfsErrFileGeneric:
				default:
					FrmAlert(alertID_volUnknownError);
					break;
				}
			}
			else {
				FldSetTextHandle(GetObjectPtr(name_volDetails_fld), NULL);
				freeDetails(glob, false);
				
				// scan dir
				getFilesInDir(&glob->brwLst[glob->currentBrw].lst.data.vol);
				// sort the files
				sort(&glob->brwLst[glob->currentBrw]);
				
				FrmGotoForm(formID_main);
			}
			break;
		
		case	ds1_volDetails_button:
		case	ds2_volDetails_button:
		case	ds3_volDetails_button:
			// get new date settings
			if (getNewDateVolDetails(glob->details, event->data.ctlSelect.controlID)) {
				// show the SAVE button if something has changed
				showHideVolDetailsButton(true);
				// redraw stuff
				drawVolDetailsDateTime(glob->details, glob->cfg.screenState);
			}
			handled = true;
			break;
			
		case	ds4_volDetails_button:
		case	ds5_volDetails_button:
		case	ds6_volDetails_button:
			// get new date settings
			if (getNewTimeVolDetails(glob->details, event->data.ctlSelect.controlID)) {
				// show the SAVE button if something has changed
				showHideVolDetailsButton(true);
				// redraw stuff
				drawVolDetailsDateTime(glob->details, glob->cfg.screenState);
			}
			handled = true;
			break;
			
		case	archive_volDetails_cbox:
		case	system_volDetails_cbox:
		case	hidden_volDetails_cbox:
		case	link_volDetails_cbox:
		case	readonly_volDetails_cbox:
			// check if an option has changed
			showHideVolDetailsButton(true);
			break;
			
		}
		break;

	case	fldEnterEvent:
		if (event->data.fldEnter.fieldID == name_volDetails_fld) {
			showHideVolDetailsButton(true);
		}
		break;
			
	case	menuEvent:
		switch (event->data.menu.itemID) {
		}
		break;

	case	nilEvent:
		handled = true;
		break;

	default:
		break;
	}

	//CALLBACK_EPILOGUE
	
	return(handled);
}

/*
 *	card database details form event handler
 */
static Boolean cardDetails_Handler(EventPtr event)
{
	Boolean handled = false;
	FormPtr form;
	Err error;
	
	
	//CALLBACK_PROLOGUE

	form = FrmGetActiveForm();
	
	switch (event->eType) {

	case    frmOpenEvent:
		
/*
		if (glob->cfg.screenState != screen160x160) {
			// modify form for 240x240 / 240x300
			VgaFormModify(form, vgaFormModify160To240);
		}
*/
					
		if (glob->details->single) {
			if (glob->details->general) {
				FrmSetControlGroupSelection(form, (UInt8)cd_group, gen_cardDetails_button);
				// show the general stuff
				//showCardDetails(glob->details->general, glob->details->single, glob->cfg.screenState, true);
			}
			else {
				FrmSetControlGroupSelection(form, (UInt8)cd_group, attr_cardDetails_button);
				// show the attributes stuff
				//showCardDetails(glob->details->general, glob->details->single, glob->cfg.screenState, true);
			}
		}

// only single mode !!!
/*		else {
			glob->details->general = false;
			// in multi db mode show the attribs
			showCardDetails(glob->details->general, glob->details->single, glob->cfg.screenState);
		}
*/
	
		// show the general/attribute stuff
		showCardDetails(glob->details->general, glob->details->single, glob->cfg.screenState, true);
		FrmDrawForm(form);
		showCardDetails(glob->details->general, glob->details->single, glob->cfg.screenState, false);
		setUpCardDetails(glob->details, glob->cfg.screenState);
		break;
			
	case	ctlSelectEvent:
		switch (event->data.ctlSelect.controlID) {
		case	done_cardDetails_button:
			if (glob->details->general) {
				FldSetTextHandle(GetObjectPtr(name_cardDetails_fld), NULL);
				FldSetTextHandle(GetObjectPtr(type_cardDetails_fld), NULL);
				FldSetTextHandle(GetObjectPtr(crid_cardDetails_fld), NULL);
			}
			freeDetails(glob, true);
			FrmGotoForm(formID_main);
			handled = true;
			break;
		case	cancel_cardDetails_button:
			if (glob->details->general) {
				FldSetTextHandle(GetObjectPtr(name_cardDetails_fld), NULL);
				FldSetTextHandle(GetObjectPtr(type_cardDetails_fld), NULL);
				FldSetTextHandle(GetObjectPtr(crid_cardDetails_fld), NULL);
			}
			freeDetails(glob, true);
			FrmGotoForm(formID_main);
			handled = true;
			break;
		case	save_cardDetails_button:
			getCardDetailsAttrName(glob->details, glob->details->general);
			if ((error = doChangeCardDetails(glob)) != errNone) {
				switch (error) {
				case	dmErrAlreadyExists:
					FrmAlert(alertID_cardDetName);
					break;
				case	1:
					FrmAlert(alertID_cardDBRomBased);
					break;
				default:
					FrmAlert(alertID_volUnknownError);
					// is this alert ok ??!?!
					break;
				}
			}
			else {
				if (glob->details->general) {
					FldSetTextHandle(GetObjectPtr(name_cardDetails_fld), NULL);
					FldSetTextHandle(GetObjectPtr(type_cardDetails_fld), NULL);
					FldSetTextHandle(GetObjectPtr(crid_cardDetails_fld), NULL);
				}
				freeDetails(glob, true);
				
				// get all db's
				getDBList(&glob->brwLst[glob->currentBrw].lst.data.card);
				// sort the dbs
				sort(&glob->brwLst[glob->currentBrw]);
				
				FrmGotoForm(formID_main);
			}
			handled = true;
			break;
			
		case	attr_cardDetails_button:
			getCardDetailsAttrName(glob->details, glob->details->general);
			glob->details->general = false;
			showCardDetails(glob->details->general, glob->details->single, glob->cfg.screenState, true);
			FrmDrawForm(form);
			showCardDetails(glob->details->general, glob->details->single, glob->cfg.screenState, false);
			setUpCardDetails(glob->details, glob->cfg.screenState);
			handled = true;
			break;
		case	gen_cardDetails_button:
			getCardDetailsAttrName(glob->details, glob->details->general);
			glob->details->general = true;
			showCardDetails(glob->details->general, glob->details->single, glob->cfg.screenState, true);
			FrmDrawForm(form);
			showCardDetails(glob->details->general, glob->details->single, glob->cfg.screenState, false);
			setUpCardDetails(glob->details, glob->cfg.screenState);
			handled = true;
			break;
			
		case	open_cardDetails_cbox:
			CtlSetValue(GetObjectPtr(open_cardDetails_cbox), ((glob->details->data.card[1].attribs & dmHdrAttrOpen) == dmHdrAttrOpen));
			FrmAlert(alertID_cardCantChangeAttr);
			break;
		case	res_cardDetails_cbox:
			CtlSetValue(GetObjectPtr(res_cardDetails_cbox), ((glob->details->data.card[1].attribs & dmHdrAttrResDB) == dmHdrAttrResDB));
			FrmAlert(alertID_cardCantChangeAttr);
			break;
		case	backup_cardDetails_cbox:
		case	bundled_cardDetails_cbox:
		case	copy_cardDetails_cbox:
		case	dirty_cardDetails_cbox:
		case	hidden_cardDetails_cbox:
		case	launch_cardDetails_cbox:
		case	install_cardDetails_cbox:
		case	ro_cardDetails_cbox:
		case	reset_cardDetails_cbox:
		case	stream_cardDetails_cbox:
			showHideCardDetailsButton(true);
			break;
		case	recycle_cardDetails_cbox:
			showHideCardDetailsButton(true);
			if (!glob->prefs.superUserMode) {
				FrmAlert(alertID_cardRecycleWarning);
			}
			break;

		case	ds1_cardDetails_button:
		case	ds2_cardDetails_button:
		case	ds3_cardDetails_button:
			// get new date settings
			if (getNewDateCardDetails(glob->details, event->data.ctlSelect.controlID)) {
				// show the SAVE button if something has changed
				showHideCardDetailsButton(true);
				// redraw stuff
				drawCardDetailsDateTime(glob->details, glob->cfg.screenState);
			}
			handled = true;
			break;
			
		case	ds4_cardDetails_button:
		case	ds5_cardDetails_button:
		case	ds6_cardDetails_button:
			// get new date settings
			if (getNewTimeCardDetails(glob->details, event->data.ctlSelect.controlID)) {
				// show the SAVE button if something has changed
				showHideCardDetailsButton(true);
				// redraw stuff
				drawCardDetailsDateTime(glob->details, glob->cfg.screenState);
			}
			handled = true;
			break;
		}
		break;
				
	case	fldEnterEvent:
		// if a field gets activated
		showHideCardDetailsButton(true);
		break;
			
	case	menuEvent:
		switch (event->data.menu.itemID) {
		}
		break;

	case	nilEvent:
		handled = true;
		break;

	default:
		break;
	}

	//CALLBACK_EPILOGUE
	
	return(handled);
}

/*
 *	make directory form handler
 */
static Boolean mkdir_Handler(EventPtr event)
{
	Boolean handled = false;
	FormPtr form;
	Err error;
	
	
	//CALLBACK_PROLOGUE

	switch (event->eType) {

	case	frmOpenEvent:
		form = FrmGetActiveForm();
				
/*
		if (glob->cfg.screenState != screen160x160) {
			// modify form for 240x240 / 240x300
			VgaFormModify(form, vgaFormModify160To240);
		}
*/
				
		FrmDrawForm(form);
		FrmSetFocus(form, FrmGetObjectIndex(form, name_mkdir_fld));
		break;

/*
	case	GMCloseFormEvent:
		handled = true;
		FrmReturnToForm(formID_main);
		// force a redraw of the main form 
		{
			EventType newEvent;
			newEvent.eType = frmUpdateEvent;
			EvtAddEventToQueue(&newEvent);
		}
		break;
*/
			
	case	ctlSelectEvent:
		switch (event->data.ctlSelect.controlID) {
		case	ok_mkdir_button:
			handled = true;
			if ((error = doMkDir(glob)) != errNone) {
				switch (error) {
				case	1:
					FrmAlert(alertID_volReadOnly);
					break;
				case	2:
				case	vfsErrBadName:
				case	vfsErrFileAlreadyExists:
					FrmAlert(alertID_volDirName);
				case	vfsErrVolumeFull:
					FrmAlert(alertID_volFull);
					break;
				default:
					FrmAlert(alertID_volUnknownError);
					break;
				}
			}
			else {
				FrmReturnToForm(formID_main);
				// force a redraw of the main form 
				{
					EventType newEvent;
					newEvent.eType = frmUpdateEvent;
					EvtAddEventToQueue(&newEvent);
				}
			}
			break;
		case	cancel_mkdir_button:
			handled = true;
			FrmReturnToForm(formID_main);
			break;
		}
		break;
			
	case	menuEvent:
		switch (event->data.menu.itemID) {
		}
		break;

	case	nilEvent:
		handled = true;
		break;

	default:
		break;
	}

	//CALLBACK_EPILOGUE
	
	return(handled);
}

/*
 *	copy file/database form handler
 */
static Boolean copy_Handler(EventPtr event)
{
	Boolean handled = false;
	//FormPtr form;
	
	
	//CALLBACK_PROLOGUE

	switch (event->eType) {

	case	frmOpenEvent:
/*
		form = FrmGetActiveForm();
			
		if (glob->cfg.screenState != screen160x160) {
			// modify form for 240x240 / 240x300
			VgaFormModify(form, vgaFormModify160To240);
		}
*/
				
		setUpCopyForm(glob);
		break;
		
/*
	case	GMCloseFormEvent:
		handled = true;
		freeCopy(glob);
		FrmReturnToForm(formID_main);
		// force a redraw of the main form 
		{
			EventType newEvent;
			newEvent.eType = frmUpdateEvent;
			EvtAddEventToQueue(&newEvent);
		}
		break;
*/

	case	ctlSelectEvent:
		switch (event->data.ctlSelect.controlID) {
		case	cancel_copy_button:
			handled = true;
			freeCopy(glob);
			FrmReturnToForm(formID_main);
			break;
		case	copy_copy_button:
			handled = true;
			if (doFileCopy(glob)) {
				freeCopy(glob);				
				FrmReturnToForm(formID_main);
			}
			break;
		}
		break;
				
	case	menuEvent:
		switch (event->data.menu.itemID) {
		}
		break;

	case	nilEvent:
		handled = true;
		break;

	default:
		break;
	}

	//CALLBACK_EPILOGUE
	
	return(handled);
}

/*
 *	about form handler
 */
static Boolean about_Handler(EventPtr event)
{
	Boolean handled = false;
	FormPtr form;
	MemHandle bmpHand;
	BitmapPtr bmpPtr;
	
	
	//CALLBACK_PROLOGUE

	switch (event->eType) {

	case	frmOpenEvent:
		form = FrmGetActiveForm();
			
/*
		if (glob->cfg.screenState != screen160x160) {
			// modify form for 240x240 / 240x300
			VgaFormModify(form, vgaFormModify160To240);
			
			// get the hander style logo
			bmpHand = DmGetResource('tAIB', 2000);
		}
		else {
*/
			// normal style
			bmpHand = DmGetResource('tAIB', 1000);
		//}
				
		FrmDrawForm(form);
		
		// draw the logo
		bmpPtr = MemHandleLock(bmpHand);
		if (glob->cfg.screenState != screen160x160) {
			WinDrawBitmap(bmpPtr, BIG(10), BIG(42));
		}
		else {
			WinDrawBitmap(bmpPtr, 10, 42);
		}
		MemHandleUnlock(bmpHand);
		DmReleaseResource(bmpHand);
		
		break;

	case	ctlSelectEvent:
		switch (event->data.ctlSelect.controlID) {
		case	done_about_button:
			handled = true;
			FrmGotoForm(formID_main);
			break;
		}
		break;
				
	case	menuEvent:
		switch (event->data.menu.itemID) {
		}
		break;

	case	nilEvent:
		handled = true;
		break;

	default:
		break;
	}

	//CALLBACK_EPILOGUE
	
	return(handled);
}

/*
 *	prefs form handler
 */
static Boolean prefs_Handler(EventPtr event)
{
	Boolean handled = false;
	FormPtr form;
	
	
	//CALLBACK_PROLOGUE

	switch (event->eType) {

	case	frmOpenEvent:
		form = FrmGetActiveForm();
			
/*
		if (glob->cfg.screenState != screen160x160) {
			// modify form for 240x240 / 240x300
			VgaFormModify(form, vgaFormModify160To240);
		}
*/
		
		setupPrefsLists(glob);
		
		FrmDrawForm(form);		
		break;

	case	ctlSelectEvent:
		switch (event->data.ctlSelect.controlID) {
		case	ok_prefs_button:
			handled = true;
			getPrefsLists(glob);
			// save prefs
			loadSavePrefs(&glob->prefs, true);
			FrmGotoForm(formID_main);
			break;
		case	cancel_prefs_button:
			handled = true;
			FrmGotoForm(formID_main);
			break;
		}
		break;
				
	case	menuEvent:
		switch (event->data.menu.itemID) {
		}
		break;

	case	nilEvent:
		handled = true;
		break;

	default:
		break;
	}

	//CALLBACK_EPILOGUE
	
	return(handled);
}

/*
 *	plugin manager form handler
 */
static Boolean pluginMgr_Handler(EventPtr event)
{
	Boolean handled = false;
	//FormPtr form;
	
	
	//CALLBACK_PROLOGUE

	switch (event->eType) {

	case	frmOpenEvent:
/*
		form = FrmGetActiveForm();
			
		if (glob->cfg.screenState != screen160x160) {
			// modify form for 240x240 / 240x300
			VgaFormModify(form, vgaFormModify160To240);
		}
*/
	
		initPluginMgr(glob);
		break;

	case	ctlSelectEvent:
		switch (event->data.ctlSelect.controlID) {
		case	done_plugin_button:
			handled = true;
			FrmGotoForm(formID_main);
			deInitPluginMgr(glob);
			break;
		}
		break;
		
	case	lstSelectEvent:
		drawPluginInfo(&glob->pluginLst, event->data.lstSelect.selection, glob->cfg.screenState);
		break;

	case	menuEvent:
		switch (event->data.menu.itemID) {
		case	cmd_pluginAbout:
			handled = true;
			{
				Int16 selection = LstGetSelection(GetObjectPtr(plug_plugin_lst));
				if (selection != noListSelection) {
					runAboutPlugin(&glob->pluginLst, selection);
				}
			}
			break;
		}
		break;
		
	case	nilEvent:
		handled = true;
		break;

	default:
		break;
	}

	//CALLBACK_EPILOGUE
	
	return(handled);
}
