/*
 *	file:		fm_6-anim.c
 *	project:	GentleMan
 *	content:	progress animation
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
 *	draw in Prograss graphics
 * - accesses globals
 */
void inProgress(UInt16 num, UInt8 doWhat, WinHandle *bg, screenModeType screenState, char *title)
{
	MemHandle resHand;
	BitmapPtr bmpPtr;
	Err error;
	const RectangleType rect160x160 = {{0, 0}, {160, 160}};
	//const RectangleType rect240x240 = {{0, 0}, {240, 240}};
	RectangleType area;
	Int16 pos, w, h;
	
	
#if 0
	if (screenState != screen160x160) {
		if (doWhat == 0) {	// animate
			// get bitmap
			resHand = DmGetResource('Tbmp', bmpID_ani_big_1 + num);
			bmpPtr = MemHandleLock(resHand);
			// draw bitmap
			WinGetWindowExtent(&w, &h);
			// HandEra 330 bug !
			if (w > 240 || h > 304) {
				w = ((double)w / (double)1.5);
				h = ((double)h / (double)1.5);
			}
			WinDrawBitmap(bmpPtr, (w/2)-38, (h/2)-30);
			// release bitmap
			MemHandleUnlock(resHand);
			DmReleaseResource(resHand);
		}
		else if (doWhat == 1) {	// save background AND draw frame
			// save bg
			*(bg) = WinSaveBits(&rect240x240, &error);
			
			// draw frame
			FntSetFont(VgaBaseToVgaFont(1));
			
			WinGetWindowExtent(&w, &h);
			// HandEra 330 bug !
			if (w > 240 || h > 304) {
				w = ((double)w / (double)1.5);
				h = ((double)h / (double)1.5);
			}
			
			area.extent.x = 140;
			area.topLeft.x = (w-area.extent.x)/2;
			area.extent.y = 76 + FntCharHeight() + 2;
			area.topLeft.y = (h/2) - (area.extent.y / 2);
			
			// clear area
			WinEraseRectangle(&area, 0);
			// draw frame
			WinDrawRectangleFrame(dialogFrame, &area);
			
			area.extent.y = FntCharHeight();
			// draw title bar
			WinDrawRectangle(&area, 0);
			// draw title text
			pos = FntLineWidth(title, StrLen(title)) / 2;
			WinEraseChars(title, StrLen(title), (w/2)-pos, area.topLeft.y-2);
			
			FntSetFont(glob->cfg.font0);
		}
		else if (doWhat == 2) {	// restore background
			WinRestoreBits(*(bg), rect240x240.topLeft.x, rect240x240.topLeft.y);
		}
	}
	else {
#endif
		if (doWhat == 0) {	// animate
			// get bitmap
			resHand = DmGetResource('Tbmp', bmpID_ani_1 + num);
			bmpPtr = MemHandleLock(resHand);
			// draw bitmap
			WinGetWindowExtent(&w, &h);
			WinDrawBitmap(bmpPtr, (w/2)-24, (h/2)-20);
			// release bitmap
			MemHandleUnlock(resHand);
			DmReleaseResource(resHand);
		}
		else if (doWhat == 1) {	// save background AND draw frame
			// save bg
			*(bg) = WinSaveBits(&rect160x160, &error);
			
			FntSetFont(1);
			
			WinGetWindowExtent(&w, &h);
			
			area.extent.x = 100;
			area.topLeft.x = (w-area.extent.x)/2;
			area.extent.y = 50 + FntCharHeight() + 2;
			area.topLeft.y = (h/2) - (area.extent.y / 2);
			
			// clear area
			WinEraseRectangle(&area, 0);
			// draw frame
			WinDrawRectangleFrame(dialogFrame, &area);
			
			area.extent.y = FntCharHeight();
			// draw title bar
			WinDrawRectangle(&area, 0);
			// draw title text
			pos = FntLineWidth(title, StrLen(title)) / 2;
			WinEraseChars(title, StrLen(title), (w/2)-pos, area.topLeft.y-2);
			
			FntSetFont(glob->cfg.font0);
		}
		else if (doWhat == 2) {	// restore background
			WinRestoreBits(*(bg), rect160x160.topLeft.x, rect160x160.topLeft.y);
		}
	//}
}
