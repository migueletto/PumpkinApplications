/****************************************************************************
 Common Code
 Copyright (C) 2005  Tom Bulatewicz, nosleep software

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
***************************************************************************/

/** @file
 * This just has some really useful functions for all kinds of stuph.
 *
 * Created on 9/18/2000 by Tom Bulatewicz
 */


#ifndef _ui_h_
#define _ui_h_

#include "SectionsCmn.hpp"

#define colorOldAPI		1
#define colorNewAPI		2

extern UInt8				colorMode;
extern UInt32				colorDepth;

extern RGBColorType		blue;
extern RGBColorType		white;
extern RGBColorType		black;
extern RGBColorType		gray;
extern RGBColorType		lightGray;
extern RGBColorType		lightBlue;
extern RGBColorType		red;
extern RGBColorType		green;
extern RGBColorType		purple;
extern RGBColorType		olive;
extern RGBColorType		maroon;
extern RGBColorType		paleBlue;
extern RGBColorType		bgColor;

void DisableControl( FormPtr frm, UInt16 objectID ) SECT_CMN;
void EnableControl( FormPtr frm, UInt16 objectID ) SECT_CMN;
void DisableField( FormPtr frm, UInt16 objectID ) SECT_CMN;
void EnableField( FormPtr frm, UInt16 objectID ) SECT_CMN;
void ShowObject( FormPtr frm, UInt16 objectID ) SECT_CMN;
void HideObject( FormPtr frm, UInt16 objectID ) SECT_CMN;
void SetFocus( FormPtr frm, UInt16 objectID ) SECT_CMN;
void DrawBitmapSimple( Int16 resID, Int16 x, Int16 y ) SECT_CMN;
void DrawBitmap( Int16 resID, Int16 x, Int16 y ) SECT_CMN;
void CtlFreeMemory( UInt16 id ) SECT_CMN;
void CtlInit( UInt16 id ) SECT_CMN;
FieldPtr GetFocusObjectPtr() SECT_CMN;
ControlPtr SetControlLabel( UInt16 ctlID, Char *str, Int16 allowWidth ) SECT_CMN;
FieldPtr SetFieldTextFromHandle( UInt16 fldID, MemHandle txtH, Boolean draw ) SECT_CMN;
UInt16 calcTableRows( TablePtr table, Boolean isChooser ) SECT_CMN;
void SetupColorSupport() SECT_CMN;
void ColorSet( RGBColorType *fore, RGBColorType *back, RGBColorType *text, RGBColorType *foreBW, RGBColorType *backBW ) SECT_CMN;
void ColorUnset() SECT_CMN;

template <class T> T* GetObjectPtr( UInt16 id ) SECT_CMN;

template <class T> T* GetObjectPtr( UInt16 id )
   {
	FormType * frmP;

	frmP = FrmGetActiveForm();
	return static_cast<T *>(
		FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, id)));
   }



#endif
