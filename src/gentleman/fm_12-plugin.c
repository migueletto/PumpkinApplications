/*
 *	file:		fm_12-plugin.c
 *	project:	GentleMan
 *	content:	plugin
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
 *	callback for list draw function
 */
void myPluginListDraw(Int16 item, RectangleType *rect, char **text)
{
	UInt16 lineLeng;
	
	
	//CALLBACK_PROLOGUE
	
	lineLeng = StrLen(glob->pluginLst.list[item].name);
			
	if (FntLineWidth(glob->pluginLst.list[item].name, lineLeng) > 150) {
		
		do {
			lineLeng--;
		} while (FntLineWidth(glob->pluginLst.list[item].name, lineLeng) > 150);
	}	
	
	WinDrawChars(glob->pluginLst.list[item].name, lineLeng, rect->topLeft.x, rect->topLeft.y);
	
	//CALLBACK_EPILOGUE
}

/*
 * init stuff for plugin Manager form
 */
void initPluginMgr(global_data_type *glb)
{
	ListPtr lstPtr;
	
	
	// rescan for plugins
	makePluginList(&glb->pluginLst);
	
	lstPtr = GetObjectPtr(plug_plugin_lst);
	
	// set callback function
	LstSetDrawFunction(lstPtr, myPluginListDraw);
	
	// set number of list entry's
	LstSetListChoices(lstPtr, NULL, glb->pluginLst.numPlugins);

	if (glb->pluginLst.numPlugins > 0) {
		LstSetSelection(lstPtr, 0);
	}
	else {
		LstSetSelection(lstPtr, noListSelection);
	}
	
	FrmDrawForm(FrmGetActiveForm());
	
	if (glb->pluginLst.numPlugins > 0) {
		drawPluginInfo(&glb->pluginLst, 0, glb->cfg.screenState);
	}
}

/*
 *	free memory and stuff before exiting the plugin manager form
 */
void deInitPluginMgr(global_data_type *glb)
{

}

/*
 * draw information about a plugin
 */
void drawPluginInfo(pluginListType *pluginLst, Int16 index, screenModeType screenState)
{
	RectangleType rect160x160 = {{0, 109}, {160, 12}};
	//RectangleType rect240x240 = {{0, BIG(109)}, {240, BIG(12)}};
	
	
	if (screenState == screen160x160) {
		FntSetFont(0);
		
		WinEraseRectangle(&rect160x160, 0);	
		WinDrawChars(pluginLst->list[index].name, StrLen(pluginLst->list[index].name), 1, 109);
		
		rect160x160.topLeft.y = 131;
		WinEraseRectangle(&rect160x160, 0);
		WinDrawChars(pluginLst->list[index].extensions, StrLen(pluginLst->list[index].extensions), 1, 131);
	}
/*
	else {
		FntSetFont(VgaBaseToVgaFont(0));
		
		WinEraseRectangle(&rect240x240, 0);
		WinDrawChars(pluginLst->list[index].name, StrLen(pluginLst->list[index].name), BIG(1), BIG(109));
		
		rect240x240.topLeft.y = BIG(131);
		WinEraseRectangle(&rect240x240, 0);
		WinDrawChars(pluginLst->list[index].extensions, StrLen(pluginLst->list[index].extensions), BIG(1), BIG(131));
	}
*/
}
			
/*
 *	run plugin with "ShowAbout" command
 */
void runAboutPlugin(pluginListType *pluginLst, UInt16 index)
{
	WinHandle saveScreen;
	RectangleType display;
	UInt16 error;
	UInt32 result;
	//UInt32 version;
	//VgaScreenModeType screenMode;
	//VgaRotateModeType rotMode;
	
	
	// save old display values
	display.topLeft.x = 0;
	display.topLeft.y = 0;
	WinGetDisplayExtent(&display.extent.x, &display.extent.y);			
	saveScreen = WinSaveBits(&display, &error);
	WinPushDrawState();
/*
	if (_TRGVGAFeaturePresent(&version)) {
		VgaGetScreenMode(&screenMode, &rotMode);
	}
*/
	
	// launch plugin
	if (SysAppLaunch(pluginLst->list[index].cardNo, pluginLst->list[index].lID, 0, GMPluginAppCmdShowAbout, NULL, &result) != 0) {
	}
	
	// restore old display values
/*
	if (_TRGVGAFeaturePresent(&version)) {
		VgaSetScreenMode(screenMode, rotMode);
	}
*/
	WinPopDrawState();
	WinRestoreBits(saveScreen, 0, 0);
}

/*
 *	find / and run plugin
 */
Err runPlugin(pluginListType *pluginLst, UInt16 volRefNum, char *fullpath)
{
	unsigned char *data;
	GMPluginDoActionType actionData;
	UInt32 result; //, version;
	UInt16 i, i2;
	char ext[20];
//	WinHandle saveScreen;
//	RectangleType display;
//	UInt16 error;
	//VgaScreenModeType screenMode;
	//VgaRotateModeType rotMode;
	Err retVal = 1;	// plugin not found

	
	// clear ext
	MemSet(ext, 20, 0);
	
	// get file extension
	for (i2 = StrLen(fullpath); i2 > 0; i2--) {
		if (fullpath[i2] == '.') {
			i2++;
			StrNCopy(ext, &fullpath[i2], StrLen(fullpath) - i2);
			break;
		}
	}	
	
	// extension to lower case for "StrStr"
	StrToLower(ext, ext);
	
	// search for extension		
	for (i = 0; i < pluginLst->numPlugins; i++) {
		
		// extension found - run plugin
		if (StrStr(pluginLst->list[i].extensions, ext) != NULL) {
			
			actionData.volRefNum = volRefNum;
			actionData.pathLength = StrLen(fullpath);
			data = MemPtrNew(sizeof(GMPluginDoActionType) + actionData.pathLength + 1);
			MemMove(data, &actionData, sizeof(GMPluginDoActionType));
			MemMove(data+sizeof(GMPluginDoActionType), fullpath, actionData.pathLength + 1);
			MemPtrSetOwner(data, 0);
			
			// save old display values
		/*	display.topLeft.x = 0;
			display.topLeft.y = 0;
			WinGetDisplayExtent(&display.extent.x, &display.extent.y);			
			saveScreen = WinSaveBits(&display, &error);
		*/
			WinPushDrawState();
/*
			if (_TRGVGAFeaturePresent(&version)) {
				VgaGetScreenMode(&screenMode, &rotMode);
			}
*/
			
			// launch plugin
			if (SysAppLaunch(pluginLst->list[i].cardNo, pluginLst->list[i].lID, 0, GMPluginAppCmdDoAction, data, &result) != 0) {
			}
					
			// restore old display values
/*
			if (_TRGVGAFeaturePresent(&version)) {
				VgaSetScreenMode(screenMode, rotMode);
			}
*/
			// restore old display values
			WinPopDrawState();
		//	WinRestoreBits(saveScreen, 0, 0);
	
			if (data != NULL) MemPtrFree(data);
			
			// plugin returned now we exit
			retVal = errNone;
			break;
		}
	}
	
	return(retVal);	
}

/*
 *	make a list with all found plugins
 */
void makePluginList(pluginListType *lst)
{
	Boolean newSearch = true;
	DmSearchStateType searchState;
	UInt16 cNo;
	LocalID lId;
	pluginInfoType pluginInfo;
	DmOpenRef db_ref;
	MemHandle resHand;
	void *resPtr;
	void *tmp;
	
		
	// free list memory
	if (lst->list != NULL) MemPtrFree(lst->list);
	lst->list = NULL;
	lst->numPlugins = 0;
	
	while (DmGetNextDatabaseByTypeCreator(newSearch, &searchState, TYPEID, CREATORID, false, &cNo, &lId) == errNone) {
		// we only need to do this once but ...
		newSearch = false;
		
		db_ref = DmOpenDatabase(cNo, lId, dmModeReadOnly);
		
		// clear temp info field
		MemSet(&pluginInfo, sizeof(pluginInfoType), 0);
		
		resHand = DmGetResource('tSTR', GM_PLUGIN_NAME_STR_ID);
		if (resHand != NULL) {
			resPtr = MemHandleLock(resHand);
			StrNCopy(pluginInfo.name, resPtr, 100);
			MemHandleUnlock(resHand);
			
			resHand = DmGetResource('tSTR', GM_PLUGIN_EXTENSIONS_STR_ID);
			if (resHand != NULL) {
				resPtr = MemHandleLock(resHand);
				
				StrNCopy(pluginInfo.extensions, resPtr, 50);
				StrToLower(pluginInfo.extensions, pluginInfo.extensions);
				
				MemHandleUnlock(resHand);
				
				// copy database info
				pluginInfo.cardNo = cNo;
				pluginInfo.lID = lId;
				
				if (lst->list == NULL) {
					lst->list = MemPtrNew(sizeof(pluginInfoType));
				}
				else {
					if (MemPtrResize(lst->list, sizeof(pluginInfoType) * (lst->numPlugins + 1)) != 0) {
						tmp = lst->list;
						lst->list = MemPtrNew(sizeof(pluginInfoType) * (lst->numPlugins + 1));
						MemMove(lst->list, tmp, sizeof(pluginInfoType) * lst->numPlugins);
						MemPtrFree(tmp);
					}
				}
				
				MemMove(&lst->list[lst->numPlugins], &pluginInfo, sizeof(pluginInfoType));
				lst->numPlugins++;
			}
			else {
			}
		}
		else {
		}
		
		DmCloseDatabase(db_ref);
	}
}
