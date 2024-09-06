/*
 *	file:		fm_3-init.c
 *	project:	GentleMan
 *	content:	init data structures (development only)
 *	updated:	Jul. 06. 2002
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
 *	get some data about the device the app runs on
 *	and do some basic init
 */
void initRunTimeData(runTimeDataType *cfg)
{
	//UInt32 version;
	UInt32 d;
	Boolean c;
	
	
	// set init to default 160x160 (normal palmos devcie screen)
	cfg->screenState = screen160x160;
	
/*
	if (_TRGVGAFeaturePresent(&version)) {
		
		// maximized silk
		cfg->screenState = screen240x240;
	
		// set to 1to1 (240x240) mode
		VgaSetScreenMode(screenMode1To1, rotateModeNone);
		
		if (!SilkWindowMaximized()) {
			// silk screen is minimized
			cfg->screenState = screen240x300;
		}
	}
*/

	// main form wasn't modified !
	cfg->formModified = false;
		
	// get current mode! (and save it)
	WinScreenMode(winScreenModeGet, NULL, NULL, &cfg->oldDepth, &cfg->oldColor);

	// get the maximum values
	WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &d, NULL);
	WinScreenMode(winScreenModeGetSupportsColor, NULL, NULL, NULL, &c);

	// set screen mode
	if ((d & 0x0080) == 0x0080) {	// 8 bit color
		d = 8;
	}
	else if ((d & 0x0008) == 0x0008) {	// 4 bit gray
		d = 4;
	}
	else {	// 2 bit gray (every device can do this)
		d = 2;
	}
	WinScreenMode(winScreenModeSet, NULL, NULL, &d, &c);
	
	// init some variables in "cfg" (brwModeCard could also be brwModeVfs this doesn't care here)
	setDisplayStuff(cfg, brwModeCard);
}

/*
 *	reset display depth, etc...
 */
void deInitRunTimeData(runTimeDataType *cfg)
{
	// reset display depth and color flag
	WinScreenMode(winScreenModeSet, NULL, NULL, &cfg->oldDepth, &cfg->oldColor);
}

/*
 *	initialize the browsers (scan rom and set rom as the default "card" for all browsers)
 */
void initBrowsers(global_data_type *glb)
{
	UInt8 i;
	

	// load prefs
	loadSavePrefs(&glb->prefs, false);
	
	// ask before deleting a single file
	glb->prefs.askDelete = true;
	// only ask once when deleting more then one file
	glb->prefs.askDeleteOnlyOnce = true;
	// dump user mode
	glb->prefs.superUserMode = false;
	
	// number of browsers	
	glb->numBrw = 2;
	
	// clear the browser list memory
	MemSet(glb->brwLst, sizeof(brwLstType) * 4, 0);
	
	glb->brwLst[0].brwMode = brwModeCard;
	glb->brwLst[0].sort = glb->prefs.defaultCardSort;
	glb->brwLst[0].show1 = glb->prefs.defaultCardShow1;
	glb->brwLst[0].show2 = glb->prefs.defaultCardShow2;
	
// 0 = ROM / 1 = RAM
	glb->brwLst[0].volIndex = 1;
	
	glb->brwLst[0].lst.mode = brwModeCard;
	glb->brwLst[0].lst.data.card.dbList = NULL;
	
	glb->brwLst[0].lst.data.card.numSelDBs = 0;
	glb->brwLst[0].lst.data.card.sizeSelDBs = 0;
	
	// copy the cardInfo to the list structure index 0 in the vList.list == volIndex
	MemMove(&glb->brwLst[0].lst.data.card.cardInfo, &glb->vLst.list[glb->brwLst[0].volIndex].data.card, sizeof(myCardInfoType));
	// get db list
	getDBList(&glb->brwLst[0].lst.data.card);
	
	// sort databases
	sort(&glb->brwLst[0]);
	
	// copy index 0 to index 1...numBrw-1 but get own dbList memory for each index	
	i = 1;
	do {
		MemMove(&glb->brwLst[i], &glb->brwLst[0], sizeof(brwLstType));
		glb->brwLst[i].lst.data.card.dbList = NULL;
		glb->brwLst[i].lst.data.card.dbList = MemPtrNew(glb->brwLst[i].lst.data.card.numDBs * sizeof(dbInfoType));
		MemMove(glb->brwLst[i].lst.data.card.dbList, glb->brwLst[0].lst.data.card.dbList, glb->brwLst[1].lst.data.card.numDBs * sizeof(dbInfoType));
		// next browser
		i++;
	} while (i < glb->numBrw);
	
	// set 0 as the current browser
	glb->currentBrw = 0;
}

/*
 *	call all the init functions
 * - get global memory
 * - load font
 */
void init(void)
{
	UInt16 cardNo;
	LocalID lId;

		
	glob = MemPtrNew(sizeof(global_data_type));
	
	ErrFatalDisplayIf(glob == NULL, "glob == NULL");

	initRunTimeData(&glob->cfg);
		
	// load special font
	glob->font128 = MemHandleLock(DmGetResource('NFNT', 1000));
	FntDefineFont(128, glob->font128);
	
	// init structure !
	MemSet(&glob->vLst, sizeof(volListType), 0x00);
	// make volume list
	makeVolList(&glob->vLst);
	
	// init variable
	glob->dirTapLast = -1;
	
	// init the browsers
	initBrowsers(glob);

	// --= plugins =--
	glob->pluginLst.list = NULL;
	makePluginList(&glob->pluginLst);
		
	// --= register with notification manager ! =--
	
	// get cardno and localid
	SysCurAppDatabase(&cardNo, &lId);
	// set handlers
	SysNotifyRegister(cardNo, lId, sysNotifyVolumeUnmountedEvent, myNotifyRemoveHandler, sysNotifyNormalPriority, NULL);
	SysNotifyRegister(cardNo, lId, sysNotifyVolumeMountedEvent, myNotifyInsertHandler, sysNotifyNormalPriority, NULL);
}

/*
 *	free memory and other stuff
 * - unregister notification functions
 */
void deinit()
{
	UInt16 cardNo;
	LocalID lId;
	
	
	deInitRunTimeData(&glob->cfg);	
	
	// --= unregister with notification manager ! =--
	
	// get cardno and localid
	SysCurAppDatabase(&cardNo, &lId);
	// set handlers
	SysNotifyUnregister(cardNo, lId, sysNotifyVolumeUnmountedEvent, sysNotifyNormalPriority);
	SysNotifyUnregister(cardNo, lId, sysNotifyVolumeMountedEvent, sysNotifyNormalPriority);	
	
	
	// free global memory	
	if (glob != NULL) MemPtrFree(glob);
	glob = NULL;
}


/*
 *	handle the insertion of a VFS volume (e.g.: sd,cf,mmc,ms,...)
 * - accesses globals
 */
Err myNotifyInsertHandler(SysNotifyParamType *notifyParamsP)
{
	volListItemType *tmpVLst;
	VFSAnyMountParamType *param;
	
	
	//CALLBACK_PROLOGUE
	
	param = (VFSAnyMountParamType*) notifyParamsP->notifyDetailsP;
			
	// get memory for the new volume list
	tmpVLst = MemPtrNew(sizeof(volListItemType) * (glob->vLst.numVols + 1));
	// copy old data
	MemMove(tmpVLst, glob->vLst.list, sizeof(volListItemType) * glob->vLst.numVols);
	// free old memory
	MemPtrFree(glob->vLst.list);
	// set pointer to new memory
	glob->vLst.list = tmpVLst;
	
	glob->vLst.list[glob->vLst.numVols].mode = brwModeVfs;
	glob->vLst.list[glob->vLst.numVols].data.vol.volRefNum = param->volRefNum;
	
	// get volume info
	VFSVolumeInfo(glob->vLst.list[glob->vLst.numVols].data.vol.volRefNum, &glob->vLst.list[glob->vLst.numVols].data.vol.volInfo);
	VFSVolumeSize(glob->vLst.list[glob->vLst.numVols].data.vol.volRefNum, &glob->vLst.list[glob->vLst.numVols].data.vol.volUsed, &glob->vLst.list[glob->vLst.numVols].data.vol.volSize);
	VFSVolumeGetLabel(glob->vLst.list[glob->vLst.numVols].data.vol.volRefNum, glob->vLst.list[glob->vLst.numVols].data.vol.label, 255);

	// inc volume count
	glob->vLst.numVols++;

	// we handle this so nobody after us gets this notification
	notifyParamsP->handled = true;

	// refresh the main form if it is the current form !
	if (FrmGetFormId(FrmGetActiveForm()) == formID_main) {
		// force a redraw of the main form 
		{
			EventType newEvent;
			newEvent.eType = frmUpdateEvent;
			EvtAddEventToQueue(&newEvent);
		}	
	}
						
	//CALLBACK_EPILOGUE
	return 0;
}

/*
 *	handle the removal of a VFS volume (e.g.: sd,cf,mmc,ms,...)
 */
Err myNotifyRemoveHandler(SysNotifyParamType *notifyParamsP)
{
	UInt16 i, pos;
	UInt16 formID;
	Boolean removedCurrent = false;
	
	
	//CALLBACK_PROLOGUE

	// set all browsers that have the removed card as current card to the first item in the volumeList
	// this should all ways be PalmCard (all ways available)
	for (i = 0; i < glob->numBrw; i++) {
		if (glob->vLst.list[glob->brwLst[i].volIndex].mode == brwModeVfs) {
			if (glob->vLst.list[glob->brwLst[i].volIndex].data.vol.volRefNum == (UIntPtr) notifyParamsP->notifyDetailsP) {
				
				// the card of the current browser was removed !
				if (i == glob->currentBrw) removedCurrent = true;
				
				if (glob->brwLst[i].lst.data.vol.fileList != NULL) {
					MemPtrFree(glob->brwLst[i].lst.data.vol.fileList);
					glob->brwLst[i].lst.data.vol.fileList = NULL;
					
					// clear list info
					MemSet(&glob->brwLst[i].lst, sizeof(listInfoType), 0);
					
					// set this to the defaults
					glob->brwLst[i].sort = glob->prefs.defaultCardSort;
					glob->brwLst[i].show1 = glob->prefs.defaultCardShow1;
					glob->brwLst[i].show2 = glob->prefs.defaultCardShow2;
					
					// index 1 in vol list (this will be the RAM view!)
					glob->brwLst[i].volIndex = 1;
					
					// see top of list
					glob->brwLst[i].scrollPos = 0;
					
					glob->brwLst[i].lst.mode = brwModeCard;
					glob->brwLst[i].brwMode = brwModeCard;
					glob->brwLst[i].lst.data.card.dbList = NULL;
					// copy the cardInfo to the list structure index 0 in the vList.list == volIndex
					MemMove(&glob->brwLst[i].lst.data.card.cardInfo, &glob->vLst.list[0].data.card, sizeof(myCardInfoType));
			
					// get db list
					getDBList(&glob->brwLst[i].lst.data.card);
					
					// sort db list
					sort(&glob->brwLst[i]);
				}
			}
		}
	}
		
	// get position of removed card in volume list
	for (i = 0; i < glob->vLst.numVols; i++) {
		if (glob->vLst.list[i].mode == brwModeVfs) {
			if (glob->vLst.list[i].data.vol.volRefNum == (UIntPtr) notifyParamsP->notifyDetailsP) {
				pos = i;
			}
		}
	}
	
	// remove "removed" card from volume list
	if (pos < glob->vLst.numVols - 1) {
		MemMove(&glob->vLst.list[pos], &glob->vLst.list[pos+1], sizeof(volListItemType) * ((glob->vLst.numVols - pos) - 1));
	}
	
	// resize memory block !
	MemPtrResize(glob->vLst.list, (glob->vLst.numVols-1) * sizeof(volListItemType));
		
	// dec number of volumes
	glob->vLst.numVols--;
	
	// we handle this so nobody after us gets this notification
	notifyParamsP->handled = true;

	// ---= do UI stuff =---
	formID = FrmGetFormId(FrmGetActiveForm());
	
	if (removedCurrent) {
		switch (formID) {
		case	formID_main:
			// update form
			{
				EventType newEvent;
				newEvent.eType = frmUpdateEvent;
				EvtAddEventToQueue(&newEvent);
			}
			break;
		case	formID_copy:
		case	formID_mkdir:
		case	formID_volDetails:
			// close form
			{
				EventType newEvent;
				newEvent.eType = GMCloseFormEvent;
				EvtAddEventToQueue(&newEvent);
			}
			break;
		}
	}
	else {
		switch (formID) {
		case	formID_main:
			// update form
			{
				EventType newEvent;
				newEvent.eType = frmUpdateEvent;
				EvtAddEventToQueue(&newEvent);
			}
			break;
		case	formID_copy:
			// close
			{
				EventType newEvent;
				newEvent.eType = GMCloseFormEvent;
				EvtAddEventToQueue(&newEvent);
			}
			break;
		}
	}
		
	//CALLBACK_EPILOGUE
	return 0;
}

/*
 *	set the display stuff (main screen - file list)
 */
void setDisplayStuff(runTimeDataType *cfg, brwModeType mode)
{
	const RectangleType volListArea160x160 = {{1,17},{158,11}};
	const RectangleType selInfoArea160x160 = {{0,30},{160,8}};
	const RectangleType dirInfoArea160x160 = {{0,149},{160,12}};
	
	//const RectangleType volListArea240x240 = {{BIG(1),BIG(17)+1},{BIG(158),BIG(11)}};
	//const RectangleType selInfoArea240x240 = {{0,BIG(30)},{BIG(160),BIG(8)}};
	//const RectangleType dirInfoArea240x240 = {{0,225},{240,15}};
	
	//const RectangleType dirInfoArea240x300 = {{0,304-15},{240,15}};
	
	
	if (mode == brwModeCard) {
		if (cfg->screenState == screen160x160) {
	
			// init font stuff
			cfg->font0 = 0;
			FntSetFont(cfg->font0);
			cfg->font0Height = FntCharHeight();
			
			cfg->dateTimeFont = MINI_DATE_4x11_FONT;
			
			cfg->numDisplay = 10;
	
			cfg->xS = 0;
			cfg->yS = 48;
			cfg->xE = 153;
			cfg->yE = 158;
		
			cfg->sX1 = 81;
			cfg->sX2 = 117;
		
			cfg->nameLengthShort = 80;
			cfg->nameLengthLong = 115;
				
			cfg->volListStrLeng = 150;	// pixel
			cfg->volListStrX = 2;
			cfg->volListStrY = 17;
			
			cfg->selInfoStrX = 1;
			cfg->selInfoStrY = 28;
			
			cfg->scrollBarLength = 110;
			
			MemMove(&cfg->volListArea, &volListArea160x160, sizeof(RectangleType));
			MemMove(&cfg->selInfoArea, &selInfoArea160x160, sizeof(RectangleType));
			
			MemMove(&cfg->dirInfoArea, &dirInfoArea160x160, sizeof(RectangleType));
		}
#if 0
		else if (cfg->screenState == screen240x240) {
			
			// init the font stuff
			cfg->font0 = VgaBaseToVgaFont(0);
			FntSetFont(cfg->font0);
			cfg->font0Height = FntCharHeight();
			
			cfg->dateTimeFont = 2;
			
			cfg->numDisplay = 10;
	
			cfg->xS = 0;
			cfg->yS = BIG(48);
			cfg->xE = BIG(153);
			cfg->yE = 240;
		
			cfg->sX1 = BIG(81);
			cfg->sX2 = BIG(117);
			
			cfg->volListStrLeng = BIG(150);	// pixel
			cfg->volListStrX = BIG(2);
			cfg->volListStrY = BIG(17) + 2;
			
			cfg->selInfoStrX = BIG(1);
			cfg->selInfoStrY = BIG(28) + 2;
			
			cfg->scrollBarLength = 159;
			
			cfg->nameLengthShort = 118;
			cfg->nameLengthLong = 173;
			
			MemMove(&cfg->volListArea, &volListArea240x240, sizeof(RectangleType));
			MemMove(&cfg->selInfoArea, &selInfoArea240x240, sizeof(RectangleType));
			
			MemMove(&cfg->dirInfoArea, &dirInfoArea240x240, sizeof(RectangleType));
			cfg->dirInfoArea.topLeft.y = (240 - cfg->font0Height);
			cfg->dirInfoArea.extent.y = cfg->font0Height;
		}
		else if (cfg->screenState == screen240x300) {
			
			// init the font stuff
			cfg->font0 = VgaBaseToVgaFont(0);
			FntSetFont(cfg->font0);
			cfg->font0Height = FntCharHeight();

			cfg->dateTimeFont = 2;
						
			cfg->numDisplay = 14;
	
			cfg->xS = 0;
			cfg->yS = BIG(48);
			cfg->xE = BIG(153);
			cfg->yE = 304;
		
			cfg->sX1 = BIG(81);
			cfg->sX2 = BIG(117);
			
			cfg->volListStrLeng = BIG(150);	// pixel
			cfg->volListStrX = BIG(2);
			cfg->volListStrY = BIG(17) + 2;
			
			cfg->selInfoStrX = BIG(1);
			cfg->selInfoStrY = BIG(28) + 2;
			
			cfg->scrollBarLength = 223;
			
			cfg->nameLengthShort = 118;
			cfg->nameLengthLong = 173;
			
			MemMove(&cfg->volListArea, &volListArea240x240, sizeof(RectangleType));
			MemMove(&cfg->selInfoArea, &selInfoArea240x240, sizeof(RectangleType));
		}
#endif
	}
	else if (mode == brwModeVfs) {
		if (cfg->screenState == screen160x160) {
			
			// init font stuff
			FntSetFont(0);
			cfg->font0Height = FntCharHeight();
			
			cfg->dateTimeFont = MINI_DATE_4x11_FONT;
			
			cfg->numDisplay = 9;
			
			cfg->xS = 0;
			cfg->yS = 48;
			cfg->xE = 153;
			cfg->yE = 147;
		
			cfg->sX1 = 81;
			cfg->sX2 = 117;
			
			cfg->volListStrLeng = 150;	// pixel
			cfg->volListStrX = 2;
			cfg->volListStrY = 17;
			
			cfg->selInfoStrX = 1;
			cfg->selInfoStrY = 28;

			cfg->scrollBarLength = 99;
			
			cfg->nameLengthShort = 80;
			cfg->nameLengthLong = 115;
						
			MemMove(&cfg->volListArea, &volListArea160x160, sizeof(RectangleType));
			MemMove(&cfg->selInfoArea, &selInfoArea160x160, sizeof(RectangleType));
			
			MemMove(&cfg->dirInfoArea, &dirInfoArea160x160, sizeof(RectangleType));
		}
#if 0
		else if (cfg->screenState == screen240x240) {
			
			// init the font stuff
			cfg->font0 = VgaBaseToVgaFont(0);
			FntSetFont(cfg->font0);
			cfg->font0Height = FntCharHeight();

			cfg->dateTimeFont = 2;
								
			cfg->numDisplay = 9;
	
			cfg->xS = 0;
			cfg->yS = BIG(48);
			cfg->xE = BIG(153);
			cfg->yE = BIG(147)+3;
		
			cfg->sX1 = BIG(81);
			cfg->sX2 = BIG(117);
			
			cfg->volListStrLeng = BIG(150);	// pixel
			cfg->volListStrX = BIG(2);
			cfg->volListStrY = BIG(17) + 2;
			
			cfg->selInfoStrX = BIG(1);
			cfg->selInfoStrY = BIG(28) + 2;
			
			cfg->scrollBarLength = 142;
			
			cfg->nameLengthShort = 118;
			cfg->nameLengthLong = 173;
			
			MemMove(&cfg->volListArea, &volListArea240x240, sizeof(RectangleType));
			MemMove(&cfg->selInfoArea, &selInfoArea240x240, sizeof(RectangleType));
			
			MemMove(&cfg->dirInfoArea, &dirInfoArea240x240, sizeof(RectangleType));
			cfg->dirInfoArea.topLeft.y = (240 - cfg->font0Height);
			cfg->dirInfoArea.extent.y = cfg->font0Height;
		}
		else if (cfg->screenState == screen240x300) {
			
			// init the font stuff
			cfg->font0 = VgaBaseToVgaFont(0);
			FntSetFont(cfg->font0);
			cfg->font0Height = FntCharHeight();
			
			cfg->dateTimeFont = 2;
					
			cfg->numDisplay = 13;
	
			cfg->xS = 0;
			cfg->yS = BIG(48);
			cfg->xE = BIG(153);
			cfg->yE = (304 - cfg->font0Height) - 1;
		
			cfg->sX1 = BIG(81);
			cfg->sX2 = BIG(117);
			
			cfg->volListStrLeng = BIG(150);	// pixel
			cfg->volListStrX = BIG(2);
			cfg->volListStrY = BIG(17) + 2;
			
			cfg->selInfoStrX = BIG(1);
			cfg->selInfoStrY = BIG(28) + 2;
			
			cfg->scrollBarLength = 207;
			
			cfg->nameLengthShort = 118;
			cfg->nameLengthLong = 173;
			
			MemMove(&cfg->volListArea, &volListArea240x240, sizeof(RectangleType));
			MemMove(&cfg->selInfoArea, &selInfoArea240x240, sizeof(RectangleType));
			
			MemMove(&cfg->dirInfoArea, &dirInfoArea240x300, sizeof(RectangleType));
			cfg->dirInfoArea.topLeft.y = (304 - cfg->font0Height);
			cfg->dirInfoArea.extent.y = cfg->font0Height;
		}
#endif
	}
}

/*
 *	handle a resize display event event
 * - happens when silk gets minimized / maximized !
 * - access to global data
 */
void handleResizeDisplayEvent()
{
#if 0
	RectangleType rect;
	UInt32 version;
	
	
	rect.topLeft.x = 0;
	rect.topLeft.y = 0;

	// only check this if the device has a soft silk area (HE330)
	if (_TRGSilkFeaturePresent(&version)) {
		
		WinGetDisplayExtent(&rect.extent.x, &rect.extent.y);	
		WinSetBounds(FrmGetWindowHandle(FrmGetActiveForm()), &rect);
	
		glob->cfg.screenState = screen240x240;
	
		if (!SilkWindowMaximized()) {
			// silk screen is minimized
			glob->cfg.screenState = screen240x300;
			
			// clear area before the new display data applay's
			// this clears the directory info line (block box at bottom of a VFS screen)
			// this could lead to errors !
			WinEraseRectangle(&glob->cfg.dirInfoArea, 0);
		
			// react to screenState change !
			setDisplayStuff(&glob->cfg,glob->brwLst[glob->currentBrw].brwMode);
		
			if (glob->brwLst[glob->currentBrw].brwMode == brwModeCard) {
				if ((glob->brwLst[glob->currentBrw].scrollPos + glob->cfg.numDisplay) >= glob->brwLst[glob->currentBrw].lst.data.card.numDBs) {
				
					if (glob->cfg.numDisplay > glob->brwLst[glob->currentBrw].lst.data.card.numDBs) {
						glob->brwLst[glob->currentBrw].scrollPos = 0;
					}
					else {
						// re calc scroll position
						glob->brwLst[glob->currentBrw].scrollPos = glob->brwLst[glob->currentBrw].lst.data.card.numDBs - glob->cfg.numDisplay;
					}
				}
			}
			else if (glob->brwLst[glob->currentBrw].brwMode == brwModeVfs) {	
				if ((glob->brwLst[glob->currentBrw].scrollPos + glob->cfg.numDisplay) >= glob->brwLst[glob->currentBrw].lst.data.vol.numFiles) {
				
					if (glob->cfg.numDisplay > glob->brwLst[glob->currentBrw].lst.data.vol.numFiles) {
						glob->brwLst[glob->currentBrw].scrollPos = 0;
					}
					else {
						// re calc scroll position
						glob->brwLst[glob->currentBrw].scrollPos = glob->brwLst[glob->currentBrw].lst.data.vol.numFiles - glob->cfg.numDisplay;
					}
				}
			}	
		}
	}
#endif
}

/*
 *	check the version of the system rom !
 * returns false if < 3.5
 */
Boolean checkPalmOSVersion()
{
	UInt32 romVer;


	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVer);

	if (romVer < 0x3500000) return(false);
	else return(true);
}

/*
 *	beam GentleMan
 */
void beamGentleMan()
{
	DmSearchStateType search;
	UInt16 cNo;
	LocalID lId;
	
	
	DmGetNextDatabaseByTypeCreator(true, &search, 'appl', CRID, true, &cNo, &lId);
	beamDB(cNo, lId, "GentleMan.prc", "GentleMan");
}

/*
 *	check the date
 *	returns true if app may run !!!
 */
Boolean checkBetaDate()
{
	DateTimeType date;

	// never ever expire!!!! (Jul. 06. 2002)
	return(true);	
	
	TimSecondsToDateTime(TimGetSeconds(), &date);

	if (date.year == 2002 && date.month < 7) {
		return(true);
	}
	
	return(false);
}
