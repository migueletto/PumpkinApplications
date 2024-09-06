/*
 *	file:		fm_5-filelist.c
 *	project:	GentleMan
 *	content:	file list events/drawing/etc...
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
 *	handle hardkey events
 */
Boolean handleKeyDown(brwLstType *brw, global_data_type *glb, EventPtr event)
{
	if (event->data.keyDown.chr == vchrPageDown /*|| event->data.keyDown.chr == vchrJogDown*/) {
		if (brw->brwMode == brwModeVfs) {
			if (brw->scrollPos + (glb->cfg.numDisplay-1) + glb->cfg.numDisplay <= brw->lst.data.vol.numFiles) {
				brw->scrollPos += (glb->cfg.numDisplay-1);
					
				setUpFileBrowser(&glb->brwLst[glb->currentBrw]);
				drawFileList(&glb->brwLst[glb->currentBrw]);
			}		
			else if (brw->scrollPos + glb->cfg.numDisplay < brw->lst.data.vol.numFiles) {
					
				brw->scrollPos = brw->lst.data.vol.numFiles - glb->cfg.numDisplay;
					
				setUpFileBrowser(&glb->brwLst[glb->currentBrw]);
				drawFileList(&glb->brwLst[glb->currentBrw]);
			}
		}
		else if (brw->brwMode == brwModeCard) {
			if (brw->scrollPos + (glb->cfg.numDisplay-1) + glb->cfg.numDisplay <= brw->lst.data.card.numDBs) {
				brw->scrollPos += (glb->cfg.numDisplay-1);
					
				setUpFileBrowser(&glb->brwLst[glb->currentBrw]);
				drawFileList(&glb->brwLst[glb->currentBrw]);
			}
			else if (brw->scrollPos + glb->cfg.numDisplay < brw->lst.data.card.numDBs) {
				
				brw->scrollPos = brw->lst.data.card.numDBs - glb->cfg.numDisplay;
					
				setUpFileBrowser(&glb->brwLst[glb->currentBrw]);
				drawFileList(&glb->brwLst[glb->currentBrw]);
			}
		}
			
		return(true);
	}
	else if (event->data.keyDown.chr == vchrPageUp /*|| event->data.keyDown.chr == vchrJogUp*/) {
		if (brw->brwMode == brwModeVfs) {
			if ((Int16)(brw->scrollPos - (glb->cfg.numDisplay-1)) >= (Int16)0) {								
				brw->scrollPos -= (glb->cfg.numDisplay-1);
					
				setUpFileBrowser(&glb->brwLst[glb->currentBrw]);
				drawFileList(&glb->brwLst[glb->currentBrw]);
			}
			else if (brw->scrollPos > 0) {
				brw->scrollPos = 0;
					
				setUpFileBrowser(&glb->brwLst[glb->currentBrw]);
				drawFileList(&glb->brwLst[glb->currentBrw]);
			}
		}
		else if (brw->brwMode == brwModeCard) {
			if ((Int16)(brw->scrollPos - (glb->cfg.numDisplay-1)) >= (Int16)0) {								
				brw->scrollPos -= (glb->cfg.numDisplay-1);
					
				setUpFileBrowser(&glb->brwLst[glb->currentBrw]);
				drawFileList(&glb->brwLst[glb->currentBrw]);
			}
			else if (brw->scrollPos > 0) {
				brw->scrollPos = 0;
					
				setUpFileBrowser(&glb->brwLst[glb->currentBrw]);
				drawFileList(&glb->brwLst[glb->currentBrw]);
			}
		}
		
		return(true);
	}
	
	return(false);
}

/*
 *	handle the penDown events within the filelist
 *	- returns true if the event was handled by this !
 */
Boolean handlePenDown(brwLstType *brw, global_data_type *glb, UInt16 x, UInt16 y)
{
	RectangleType rect;
	
		
	if (x >= glb->cfg.xS && x < glb->cfg.xE && y >= glb->cfg.yS && y < glb->cfg.yE) {
				
		glb->pos = ((((y - glb->cfg.yS) % glb->cfg.font0Height > 0) ? ((y - glb->cfg.yS) / glb->cfg.font0Height) + 1 : (y - glb->cfg.yS) / glb->cfg.font0Height) - 1);
		
		if (glb->pos >= glb->cfg.numDisplay) {
			glb->pos = -1;
			return(false);
		}
		
		if (brw->lst.mode == brwModeVfs) {
			if (glb->pos < brw->lst.data.vol.numFiles) {
				
				rect.topLeft.x = glb->cfg.xS;
				rect.topLeft.y = glb->cfg.yS + (glb->pos * glb->cfg.font0Height);
				rect.extent.x = glb->cfg.xE - 1;
				rect.extent.y = glb->cfg.font0Height;
				
				WinInvertRectangle(&rect, 0);
		
				glb->isInverted = true;
		
				return(true);
			}
			else {
				glb->pos = -1;
			}
		}
		else if (brw->lst.mode == brwModeCard) {
			if (glb->pos < brw->lst.data.card.numDBs) {
		
				rect.topLeft.x = glb->cfg.xS;
				rect.topLeft.y = glb->cfg.yS + (glb->pos * glb->cfg.font0Height);
				rect.extent.x = glb->cfg.xE - 1;
				rect.extent.y = glb->cfg.font0Height;
				
				WinInvertRectangle(&rect, 0);
		
				glb->isInverted = true;
		
				return(true);
			}
			else {
				glb->pos = -1;
			}
		}
	}
	else {
		glb->pos = -1;
	}
	
	return(false);
}

/*
 *	handle the penUp events within the filelist
 *	- returns true if the event was handled by this !
 */
Boolean handlePenUp(brwLstType *brw, global_data_type *glb, UInt16 x, UInt16 y)
{
	RectangleType rect;
	Boolean doubleTap = false;
	
	
	if (x >= glb->cfg.xS && x < glb->cfg.xE && y >= glb->cfg.yS && y < glb->cfg.yE && glb->pos != -1) {
		
		// same field as penDown ?
		if (glb->pos == ((((y - glb->cfg.yS) % glb->cfg.font0Height > 0) ? ((y - glb->cfg.yS) / glb->cfg.font0Height) + 1 : (y - glb->cfg.yS) / glb->cfg.font0Height) - 1)) {

			if (glb->dirTapLast == glb->pos) {
				if (glb->dirTapTicks + (SysTicksPerSecond() / 2) >= TimGetTicks()) {	// double-tap
					// THIS IS A DOUBLE-TAP
					doubleTap = true;
				}
			}
			
			if (brw->lst.mode == brwModeVfs) {
				if (doubleTap) {
					// handle double-tap
					if (handleDoubelTapVfs(brw, glb)) {
						// file/directory is now selected
						if (!brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].selected) {
							brw->lst.data.vol.numSelFiles++;
							brw->lst.data.vol.sizeSelFiles += brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].size;
						}
						else {	// file/directory just was unselected
							brw->lst.data.vol.numSelFiles--;
							brw->lst.data.vol.sizeSelFiles -= brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].size;
						}
														
						// invert selection state
						brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].selected = !brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].selected;
					
						// draw infos about selected files
						drawSelectedInfo(brw);
					}
				}
				else {
					// file/directory is now selected
					if (!brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].selected) {
						brw->lst.data.vol.numSelFiles++;
						brw->lst.data.vol.sizeSelFiles += brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].size;
					}
					else {	// file/directory just was unselected
						brw->lst.data.vol.numSelFiles--;
						brw->lst.data.vol.sizeSelFiles -= brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].size;
					}
														
					// invert selection state
					brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].selected = !brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].selected;
					
					// draw infos about selected files
					drawSelectedInfo(brw);
				}
			}
			else if (brw->lst.mode == brwModeCard) {
				if (doubleTap) {
					// handle double-tap
					handleDoubelTapCard(brw, glb);
				}
				
				// database is now selected
				if (!brw->lst.data.card.dbList[brw->scrollPos + glb->pos].selected) {
					brw->lst.data.card.numSelDBs++;
					brw->lst.data.card.sizeSelDBs += brw->lst.data.card.dbList[brw->scrollPos + glb->pos].size;
				}
				else {	// file just was unselected
					brw->lst.data.card.numSelDBs--;
					brw->lst.data.card.sizeSelDBs -= brw->lst.data.card.dbList[brw->scrollPos + glb->pos].size;
				}		
							
				// invert selection state
				brw->lst.data.card.dbList[brw->scrollPos + glb->pos].selected = !brw->lst.data.card.dbList[brw->scrollPos + glb->pos].selected;
				
				// draw infos about selected files
				drawSelectedInfo(brw);	
			}
			
			// save ticks - for double-tap checking
			glb->dirTapTicks = TimGetTicks();
			glb->dirTapLast = glb->pos;
			glb->isInverted = false;
		}
		else {
			if (glb->isInverted) {
				rect.topLeft.x = glb->cfg.xS;
				rect.topLeft.y = glb->cfg.yS + (glb->pos * glb->cfg.font0Height);
				rect.extent.x = glb->cfg.xE - 1;
				rect.extent.y = glb->cfg.font0Height;
				
				WinInvertRectangle(&rect, 0);
				
				glb->isInverted = false;
			}
		}
		
		return(true);
		
	}
	
	return(false);
}

/*
 *	handle the penMove events within the filelist
 *	- returns true if the event was handled by this !
 */
Boolean handlePenMove(brwLstType *brw, global_data_type *glb, UInt16 x, UInt16 y)
{
	RectangleType rect;
	
	
	if (x >= glb->cfg.xS && x < glb->cfg.xE && y >= glb->cfg.yS && y < glb->cfg.yE && glb->pos != -1) {
			
		// same field as penDown ?
		if (glb->pos == ((((y - glb->cfg.yS) % glb->cfg.font0Height > 0) ? ((y - glb->cfg.yS) / glb->cfg.font0Height) + 1 : (y - glb->cfg.yS) / glb->cfg.font0Height) - 1)) {
			if (!glb->isInverted) {
				rect.topLeft.x = glb->cfg.xS;
				rect.topLeft.y = glb->cfg.yS + (glb->pos * glb->cfg.font0Height);
				rect.extent.x = glb->cfg.xE - 1;
				rect.extent.y = glb->cfg.font0Height;
			
				WinInvertRectangle(&rect, 0);
				
				glb->isInverted = true;
			}
		}
		else {
			if (glb->isInverted) {
				rect.topLeft.x = glb->cfg.xS;
				rect.topLeft.y = glb->cfg.yS + (glb->pos * glb->cfg.font0Height);
				rect.extent.x = glb->cfg.xE - 1;
				rect.extent.y = glb->cfg.font0Height;
			
				WinInvertRectangle(&rect, 0);
				
				glb->isInverted = false;
			}
		}
		
		return(true);
	}
	else {
		if (glb->isInverted) {
			rect.topLeft.x = glb->cfg.xS;
			rect.topLeft.y = glb->cfg.yS + (glb->pos * glb->cfg.font0Height);
			rect.extent.x = glb->cfg.xE - 1;
			rect.extent.y = glb->cfg.font0Height;
			
			WinInvertRectangle(&rect, 0);
				
			glb->isInverted = false;
		}
		
		return(false);
	}
}

/*
 *	handle a double-tap in a VOL mode browser (e.g. change directory)
 * - returns FALSE when tap was on a directory
 */
Boolean handleDoubelTapVfs(brwLstType *brw, global_data_type *glb)
{
	UInt16 strPos;
	char *fullpath;
	Err error;
	
	
	if ((brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].attribs & vfsFileAttrDirectory) == vfsFileAttrDirectory) {	
		if (StrCompare(brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].name, "..") == 0) {		// one level up
			strPos = StrLen(brw->lst.data.vol.currentDir) - 1;
									
			while (brw->lst.data.vol.currentDir[strPos] != '/') {
				brw->lst.data.vol.currentDir[strPos] = 0;
				strPos--;
			}
			if (StrLen(brw->lst.data.vol.currentDir) > 1) {
				brw->lst.data.vol.currentDir[strPos] = 0;
			}
						
			// reset scroll pos
			brw->scrollPos = 0;
		}
		else if (StrCompare(brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].name, ".") == 0) {	// rescann current dir (maybe comes in handy)
			// reset scroll pos
			brw->scrollPos = 0;
		}
		else {	// change current directory
			if (brw->lst.data.vol.currentDir[StrLen(brw->lst.data.vol.currentDir)-1] != '/') {
				StrCat(brw->lst.data.vol.currentDir, "/");
			}
			StrCat(brw->lst.data.vol.currentDir, brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].name);
								
			// reset scroll pos
			brw->scrollPos = 0;
		}
							
		// scan the directory
		getFilesInDir(&brw->lst.data.vol);
							
		// resort files
		sort(brw);
							
		// draw info about selected files, etc...
		drawSelectedInfo(brw);
							
		// setup the file browser
		setUpFileBrowser(brw);
							
		// redraw everything
		drawFileList(brw);
		
		return(false);
	}
	else {
		if (glb->prefs.dTapVfs == doubleTapRunPlugin) {
		
			fullpath = MemPtrNew(StrLen(brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].name) + StrLen(brw->lst.data.vol.currentDir) + 2);
			StrCopy(fullpath, brw->lst.data.vol.currentDir);
			if (fullpath[StrLen(fullpath)-1] != '/') StrCat(fullpath, "/");
			StrCat(fullpath, brw->lst.data.vol.fileList[brw->scrollPos + glb->pos].name);
			
			// find and run plugin
			error = runPlugin(&glb->pluginLst, brw->lst.data.vol.volInfo.volRefNum, fullpath);
			
			MemPtrFree(fullpath);
			
			// force a redraw of the main form 
			{
				EventType newEvent;
				newEvent.eType = frmUpdateEvent;
				EvtAddEventToQueue(&newEvent);
			}
			
			// if no plugin was found for this file - alert
			if (error == 1) {
				FrmAlert(alertID_pluginNotFound);
			}
		}
		else if (glb->prefs.dTapVfs == doubleTapDetails) {
			
			glb->cfg.formModified = false;
			doDetails(glb, brw->scrollPos + glb->pos, true);
		}
	}
	
	return(true);
}

/*
 *	run plugin (from menu)
 */
void doRunPlugin(brwLstType *brw, pluginListType *pluginLst)
{
	UInt32 i;
	char *fullpath;
	Err error;
	
	
	if (brw->brwMode == brwModeVfs) {
		
		if (brw->lst.data.vol.numSelFiles == 1) {
			// find first selected file
			i = 0;
			while (i < brw->lst.data.vol.numFiles && !brw->lst.data.vol.fileList[i].selected) {
				i++;
			}
			
			if (i < brw->lst.data.vol.numFiles) {
				
				fullpath = MemPtrNew(StrLen(brw->lst.data.vol.fileList[i].name) + StrLen(brw->lst.data.vol.currentDir) + 2);
				StrCopy(fullpath, brw->lst.data.vol.currentDir);
				if (fullpath[StrLen(fullpath)-1] != '/') StrCat(fullpath, "/");
				StrCat(fullpath, brw->lst.data.vol.fileList[i].name);

				// find and run plugin
				error = runPlugin(pluginLst, brw->lst.data.vol.volInfo.volRefNum, fullpath);
			
				MemPtrFree(fullpath);
			}
		}
		else if (brw->lst.data.vol.numSelFiles == 0) {
			error = 2;
		}
		else if (brw->lst.data.vol.numSelFiles > 1) {
			error = 3;
		}
		
		switch (error) {
		case	1:
			FrmAlert(alertID_pluginNotFound);
			break;
		case	2:
			FrmAlert(alertID_volNoSelFile);
			break;
		case	3:
			FrmAlert(alertID_volSingleFileOnly);
			break;
		}
	}
}

/*
 *	handle a double-tap in a CARD mode browser
 */
void handleDoubelTapCard(brwLstType *brw, global_data_type *glb)
{
	glb->cfg.formModified = false;
	doDetails(glb, brw->scrollPos + glb->pos, true);
}

/*
 *	draw the current directory
 * - accesses "glob"
 */
void drawDirectoryInfo(brwLstType *brw)
{
	char *dP;
	

	if (brw->brwMode == brwModeVfs) {
		
		dP = brw->lst.data.vol.currentDir;
	
		if (FntLineWidth(dP, StrLen(dP)) > glob->cfg.dirInfoArea.extent.x-2) {
			do {
				dP++;
			} while (FntLineWidth(dP, StrLen(dP)) > glob->cfg.dirInfoArea.extent.x - 2);
		}
	
		// clear area before drawing to it
		WinDrawRectangle(&glob->cfg.dirInfoArea, 0);
		
		// draw current directory
		WinEraseChars(dP, StrLen(dP), glob->cfg.dirInfoArea.topLeft.x + 1, glob->cfg.dirInfoArea.topLeft.y);
	}
	else if (brw->brwMode == brwModeCard) {
			
		// clear area before drawing to it
		WinEraseRectangle(&glob->cfg.dirInfoArea, 0);
	}
}

/*
 *	draws stuff like number of files/databases on card, number of selected files and the size of all selected files/dbs
 * - direct access to globals
 */
void drawSelectedInfo(brwLstType *brw)
{
	char drawStr[100];
	WinDrawOperation oldMode;
	
	
	// clear string
	MemSet(drawStr, 100, 0);
	
	if (brw->brwMode == brwModeVfs) {
		
		StrCopy(drawStr, "Files: ");
		StrIToA(drawStr+StrLen(drawStr), brw->lst.data.vol.numFiles);
		StrCat(drawStr, " Marked: ");
		StrIToA(drawStr+StrLen(drawStr), brw->lst.data.vol.numSelFiles);
		StrCat(drawStr, " Size: ");
		
		// make displayable size and add it to the end of the string
		makeDisplaySize(brw->lst.data.vol.sizeSelFiles, drawStr+StrLen(drawStr));
		
		// clear draw area
		WinEraseRectangle(&glob->cfg.selInfoArea, 0);
		
		// select font
/*
		if (glob->cfg.screenState != screen160x160) {
			FntSetFont(VgaBaseToVgaFont(1));
		}
		else {
*/
			FntSetFont(1);
		//}
		
		oldMode = WinSetDrawMode(winOverlay);
		WinPaintChars(drawStr, StrLen(drawStr), glob->cfg.selInfoStrX, glob->cfg.selInfoStrY);
		WinSetDrawMode(oldMode);
		
		FntSetFont(glob->cfg.font0);
	}
	else if (brw->brwMode == brwModeCard) {
		
		StrCopy(drawStr, "DBs: ");
		StrIToA(drawStr+StrLen(drawStr), brw->lst.data.card.numDBs);
		StrCat(drawStr, " Marked: ");
		StrIToA(drawStr+StrLen(drawStr), brw->lst.data.card.numSelDBs);
		StrCat(drawStr, " Size: ");
		
		// make displayable size and add it to the end of the string
		makeDisplaySize(brw->lst.data.card.sizeSelDBs, drawStr+StrLen(drawStr));
		
		// clear draw area
		WinEraseRectangle(&glob->cfg.selInfoArea, 0);
		
		// select font
/*
		if (glob->cfg.screenState != screen160x160) {
			FntSetFont(VgaBaseToVgaFont(1));
		}
		else {
*/
			FntSetFont(1);
		//}
		
		oldMode = WinSetDrawMode(winOverlay);
		WinPaintChars(drawStr, StrLen(drawStr), glob->cfg.selInfoStrX, glob->cfg.selInfoStrY);
		WinSetDrawMode(oldMode);
		
		FntSetFont(glob->cfg.font0);
	}
}

/*
 *	draw the current file list view
 *	- accesses "glob"
 */
UInt8 drawFileList(brwLstType *brw)
{
	Int8 viewNumFiles;
	UInt8 i, i2;
	RectangleType rect = {{glob->cfg.xS, glob->cfg.yS},{glob->cfg.xE - glob->cfg.xS, (glob->cfg.yE - glob->cfg.yS) + 1}};
	RectangleType invRect;
	char displayStr[70];
	vfsShowType volShowTest;
	cardShowType cardShowTest;
	UInt16 xP;
	
		
	// this is a vfs volume
	if (brw->brwMode == brwModeVfs) {
	
		if (brw->lst.data.vol.numFiles < glob->cfg.numDisplay) {
			viewNumFiles = brw->lst.data.vol.numFiles;
		}
		else {	
			viewNumFiles = glob->cfg.numDisplay;
		}

		// erase file display area before drawing !
		WinEraseRectangle(&rect, 0);
		
		for (i = 0; i < viewNumFiles; i++) {
			// clear the displayStr
			MemSet(displayStr, 70, 0);

			// draw filename
			makeDisplayName(brw->lst.data.vol.fileList[brw->scrollPos + i].name, displayStr, ((brw->show1 == vfsShowNone) ? glob->cfg.nameLengthLong : glob->cfg.nameLengthShort), brw->lst.data.vol.fileList[brw->scrollPos + i].attribs);
			WinDrawChars(displayStr, StrLen(displayStr), glob->cfg.xS, glob->cfg.yS + (i * glob->cfg.font0Height));
					
			// draw attributes (show1 and show2)

			volShowTest = brw->show1;
			xP = glob->cfg.sX1;
			
			for (i2 = 0; i2 < 2; i2++) {
						
				// clear the displayStr
				MemSet(displayStr, 70, 0);
			
				switch (volShowTest) {
				case	vfsShowSize:
					if ((brw->lst.data.vol.fileList[brw->scrollPos + i].attribs & vfsFileAttrDirectory) != vfsFileAttrDirectory) {
						// format the size
						makeDisplaySize(brw->lst.data.vol.fileList[brw->scrollPos + i].size, displayStr);
						// draw
						WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					}
					break;
				case	vfsShowAttr:	// attr
					makeDisplayAttr(brw->lst.data.vol.fileList[brw->scrollPos + i].attribs, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					break;
				case	vfsShowCrDate:
					FntSetFont(glob->cfg.dateTimeFont);
					makeDisplayDate(brw->lst.data.vol.fileList[brw->scrollPos + i].crDate, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					FntSetFont(glob->cfg.font0);
					break;
				case	vfsShowMoDate:
					FntSetFont(glob->cfg.dateTimeFont);
					makeDisplayDate(brw->lst.data.vol.fileList[brw->scrollPos + i].moDate, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					FntSetFont(glob->cfg.font0);
					break;
				case	vfsShowAcDate:
					FntSetFont(glob->cfg.dateTimeFont);
					makeDisplayDate(brw->lst.data.vol.fileList[brw->scrollPos + i].acDate, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					FntSetFont(glob->cfg.font0);
					break;
				case	vfsShowCrTime:
					FntSetFont(glob->cfg.dateTimeFont);
					makeDisplayTime(brw->lst.data.vol.fileList[brw->scrollPos + i].crDate, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					FntSetFont(glob->cfg.font0);
					break;
				case	vfsShowMoTime:
					FntSetFont(glob->cfg.dateTimeFont);
					makeDisplayTime(brw->lst.data.vol.fileList[brw->scrollPos + i].moDate, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					FntSetFont(glob->cfg.font0);
					break;
				case	vfsShowAcTime:
					FntSetFont(glob->cfg.dateTimeFont);
					makeDisplayTime(brw->lst.data.vol.fileList[brw->scrollPos + i].acDate, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					FntSetFont(glob->cfg.font0);
					break;
				default:
					break;
				}
				
				// set everything to show2
				xP = glob->cfg.sX2;
				volShowTest = brw->show2;
			}

			// show selected (invert field)
			if (brw->lst.data.vol.fileList[brw->scrollPos + i].selected) {
				invRect.topLeft.x = glob->cfg.xS;
				invRect.topLeft.y = glob->cfg.yS + (i * glob->cfg.font0Height);
				invRect.extent.x = glob->cfg.xE - 1;
				invRect.extent.y = glob->cfg.font0Height;
				
				WinInvertRectangle(&invRect, 0);
			}
			
		}
	}
	// this is a memory card
	else if (brw->brwMode == brwModeCard) {
		
		if (brw->lst.data.card.numDBs < glob->cfg.numDisplay) {
			viewNumFiles = brw->lst.data.card.numDBs;
		}
		else {	
			viewNumFiles = glob->cfg.numDisplay;
		}
			
		WinEraseRectangle(&rect, 0);
		
		for (i = 0; i < viewNumFiles; i++) {

			// clear the displayStr
			MemSet(displayStr, 70, 0);
			
			// draw filename
			makeDisplayName(brw->lst.data.card.dbList[brw->scrollPos + i].name, displayStr, ((brw->show1 == cardShowNone) ? glob->cfg.nameLengthLong : glob->cfg.nameLengthShort), 0);
			WinDrawChars(displayStr, StrLen(displayStr), glob->cfg.xS, glob->cfg.yS + (i * glob->cfg.font0Height));
			
			cardShowTest = brw->show1;
			xP = glob->cfg.sX1;
			
			for (i2 = 0; i2 < 2; i2++) {
			
				// clear the displayStr
				MemSet(displayStr, 70, 0);
			
				switch (cardShowTest) {
				case	cardShowTypeID:
					{
						char *strPtr = (char *) &brw->lst.data.card.dbList[brw->scrollPos + i].typeId;
						Int16 j, pos_x = xP;
					
						for (j = 0; j < 4; j++) {
							if (*(strPtr+j) != 0) {
								WinDrawChars(strPtr+j, 1, pos_x, glob->cfg.yS + (i * glob->cfg.font0Height));
								pos_x += FntCharWidth(*(strPtr+j));
							}
						}
					}
					break;
				case	cardShowCrid:
					{
						char *strPtr = (char *) &brw->lst.data.card.dbList[brw->scrollPos + i].crid;
						Int16 j, pos_x = xP;
					
						for (j = 0; j < 4; j++) {
							if (*(strPtr+j) != 0) {
								WinDrawChars(strPtr+j, 1, pos_x, glob->cfg.yS + (i * glob->cfg.font0Height));
								pos_x += FntCharWidth(*(strPtr+j));
							}
						}
					}
					break;
				case	cardShowRecs:
					StrIToA(displayStr, brw->lst.data.card.dbList[brw->scrollPos + i].numRecs);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					break;
				case	cardShowSize:
					makeDisplaySize(brw->lst.data.card.dbList[brw->scrollPos + i].size, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					break;
				case	cardShowRW:
					if ((brw->lst.data.card.dbList[brw->scrollPos + i].attribs & dmHdrAttrReadOnly) == dmHdrAttrReadOnly) {
						WinDrawChars("RO", 2, xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					}
					else {
						WinDrawChars("RW", 2, xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					}
					break;
				case	cardShowVer:
					StrIToA(displayStr, brw->lst.data.card.dbList[brw->scrollPos + i].version);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					break;
				case	cardShowMNum:
					StrIToA(displayStr, brw->lst.data.card.dbList[brw->scrollPos + i].modNum);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i *  glob->cfg.font0Height));
					break;
				case	cardShowCrDate:
					FntSetFont(glob->cfg.dateTimeFont);
					makeDisplayDate(brw->lst.data.card.dbList[brw->scrollPos + i].crDate, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					FntSetFont(glob->cfg.font0);
					break;
				case	cardShowMoDate:
					FntSetFont(glob->cfg.dateTimeFont);
					makeDisplayDate(brw->lst.data.card.dbList[brw->scrollPos + i].moDate, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					FntSetFont(glob->cfg.font0);
					break;
				case	cardShowBkDate:
					FntSetFont(glob->cfg.dateTimeFont);
					makeDisplayDate(brw->lst.data.card.dbList[brw->scrollPos + i].acDate, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					FntSetFont(glob->cfg.font0);
					break;
				case	cardShowCrTime:
					FntSetFont(glob->cfg.dateTimeFont);
					makeDisplayTime(brw->lst.data.card.dbList[brw->scrollPos + i].crDate, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					FntSetFont(glob->cfg.font0);
					break;
				case	cardShowMoTime:
					FntSetFont(glob->cfg.dateTimeFont);
					makeDisplayTime(brw->lst.data.card.dbList[brw->scrollPos + i].moDate, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					FntSetFont(glob->cfg.font0);
					break;
				case	cardShowBkTime:
					FntSetFont(glob->cfg.dateTimeFont);
					makeDisplayTime(brw->lst.data.card.dbList[brw->scrollPos + i].acDate, displayStr);
					WinDrawChars(displayStr, StrLen(displayStr), xP, glob->cfg.yS + (i * glob->cfg.font0Height));
					FntSetFont(glob->cfg.font0);
					break;
				default:
					break;
				}
			
				xP = glob->cfg.sX2;
				cardShowTest = brw->show2;
			
			}
		
			// show selected (invert field)
			if (brw->lst.data.card.dbList[brw->scrollPos + i].selected) {
				invRect.topLeft.x = glob->cfg.xS;
				invRect.topLeft.y = glob->cfg.yS + (i * glob->cfg.font0Height);
				invRect.extent.x = glob->cfg.xE - 1;
				invRect.extent.y = glob->cfg.font0Height;
				
				WinInvertRectangle(&invRect, 0);
			}
		}
	}
	
	return(1);
}

/*
 *	make the displayable filename
 *	e.g. if the filename is too long cut it and add "..." to the end of it
 */
void makeDisplayName(char *in, char *out, UInt8 length, UInt32 attribs)
{
	const char dots[4] = {"...\0"};
	const char bracks[3] = {"<>\0"};
	UInt8 dotsLength, brackLength;
	UInt16 inLength;
	
	
	dotsLength = FntLineWidth(dots, StrLen(dots));
	
	if ((attribs & vfsFileAttrDirectory) == vfsFileAttrDirectory) {	// directory
		
		brackLength = FntLineWidth(bracks, StrLen(bracks));
		
		if ((FntLineWidth(in, StrLen(in)) + brackLength) <= length) {
			StrCopy(out, "<");
			StrCat(out, in);
			StrCat(out, ">");
		}
		else {
			inLength = StrLen(in);
			do {
				inLength--;
			} while ((FntLineWidth(in, inLength) + dotsLength + brackLength) > length);
			StrCopy(out, "<");
			StrNCopy(out+StrLen(out), in, inLength);
			out[StrLen(out)] = 0;
			StrCat(out, dots);
			StrCat(out, ">");
		}	
	}
	else {	// normal file name
		// filename length is ok
		if (FntLineWidth(in, StrLen(in)) <= length) {
			StrCopy(out, in);
		}
		else {	// filename is too long ... cut it
			inLength = StrLen(in);
			do {
				inLength--;
			} while ((FntLineWidth(in, inLength) + dotsLength) > length);
			StrNCopy(out, in, inLength);
			out[inLength] = 0;
			StrCat(out, dots);
		}
	}
}

/*
 *	format the file size
 */
void makeDisplaySize(UInt32 size, char *out)
{
	char size_str[20];
	char size_str_2[20];
	
	
	MemSet(size_str, 20, 0);
	MemSet(size_str_2, 20, 0);
	
	if (size < 1024) {
		StrIToA(size_str, size);
	}
	else if (size < ((UInt32)1024 * 1024)) {
		StrIToA(size_str, size/1024);
		StrCat(size_str, "k");
	}
	else {
		StrIToA(size_str, size / ((UInt32)1024*1024));
		size = size % ((UInt32)1024*1024);
		if (size < ((UInt32)100*1024)) {
			StrCopy(size_str_2, "0");
		}
		else {
			StrIToA(size_str_2, size);
		}
		StrCat(size_str, ".");
		StrNCopy(size_str+StrLen(size_str), size_str_2, 1);
		StrCat(size_str, "M");
	}
	
	StrCopy(out, size_str);
}

/*
 *	format the file attributes
 */
void makeDisplayAttr(UInt32 attribs, char *out)
{	
	if ((attribs & vfsFileAttrReadOnly) == vfsFileAttrReadOnly) {
		StrCat(out, "R");
	}
	if ((attribs & vfsFileAttrHidden) == vfsFileAttrHidden) {
		StrCat(out, "H");
	}
	if ((attribs & vfsFileAttrSystem) == vfsFileAttrSystem) {
		StrCat(out, "S");
	}
	if ((attribs & vfsFileAttrLink) == vfsFileAttrLink) {
		StrCat(out, "L");
	}
	if ((attribs & vfsFileAttrArchive) == vfsFileAttrArchive) {
		StrCat(out, "A");
	}
}

/*
 *	format the date
 *	- take format from system prefs
 */
void makeDisplayDate(UInt32 date, char *out)
{
	DateTimeType cur_datetime;
	
	
	TimSecondsToDateTime(date, &cur_datetime);
	DateToAscii(cur_datetime.month, cur_datetime.day, cur_datetime.year, PrefGetPreference(prefDateFormat), out);
}

/*
 *	format the time
 *	- take format from system prefs
 */
void makeDisplayTime(UInt32 date, char *out)
{
	DateTimeType cur_datetime;
	
	
	TimSecondsToDateTime(date, &cur_datetime);
	TimeToAscii(cur_datetime.hour, cur_datetime.minute, PrefGetPreference(prefTimeFormat), out);
}
