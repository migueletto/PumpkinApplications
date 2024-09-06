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


#include <PalmOS.h>
#include <PalmOSGlue.h>
#include "UI.h"
#include "Stuph.h"

#include "debug.h"

RGBColorType		oldrgbf, oldrgbb;

UInt8		colorMode;
UInt32	colorDepth;

// these should not be globals - make a function that returns a color instead
// and maybe make up a colorui object that has the mode and depth and all the
//  color-related methods, and then just have one global instance

RGBColorType		blue = 	{ 0, 51, 0, 153 };
RGBColorType		white = 	{ 0, 255, 255, 255 };
RGBColorType		black = 	{ 0, 0, 0, 0 };
RGBColorType		lightBlue = { 0, 0, 153, 255 };
RGBColorType		gray = 	{ 0, 51, 51, 51 };
RGBColorType		lightGray = { 0, 100, 100, 100 };
RGBColorType		red = 	{ 0, 255, 0, 0 };
RGBColorType		green	= 	{ 0, 0, 154, 0 };
RGBColorType		purple = { 0, 100, 0, 100 };
RGBColorType		olive	= 	{ 0, 101, 101, 0 };
RGBColorType		maroon = { 0, 136, 0, 0 };
RGBColorType		paleBlue = { 0, 222, 237, 255 };
RGBColorType		bgColor = { 0, 102, 153, 204 };


void DisableControl( FormPtr frm, UInt16 objectID )
   {
   ControlPtr		ctl;

	ctl = GetObjectPtr<ControlType>( objectID );	
   CtlSetEnabled( ctl, false );
   }
   

void EnableControl( FormPtr frm, UInt16 objectID )
   {
   ControlPtr		ctl;

	ctl = GetObjectPtr<ControlType>( objectID );	
   CtlSetEnabled( ctl, true);
   }
   

void DisableField( FormPtr frm, UInt16 objectID )
   {
   FieldPtr		fld;

	fld = GetObjectPtr<FieldType>( objectID );	
   FldSetUsable( fld, false );
   }
   

void EnableField( FormPtr frm, UInt16 objectID )
   {
   FieldPtr		fld;

	fld = GetObjectPtr<FieldType>( objectID );	
   FldSetUsable( fld, true );
   }
   

/**
 * Show an object (enable and draw).
 *
 * @param frm the object's form
 * @param objectID the object's id
 */
void ShowObject( FormPtr frm, UInt16 objectID )
   {
	FrmShowObject( frm, FrmGetObjectIndex( frm, objectID ));
   }
   

/**
 * Hide an object (disable and erase).
 *
 * @param frm the object's form
 * @param objectID the object's id
 */
void HideObject( FormPtr frm, UInt16 objectID )
   {
	FrmHideObject( frm, FrmGetObjectIndex( frm, objectID ));
   }


/**
 * Give the focus to an object.
 *
 * @param frm the object's form
 * @param objectID the object's id
 */
void SetFocus( FormPtr frm, UInt16 objectID )
	{
	FrmSetFocus( frm, FrmGetObjectIndex( frm, objectID ));
	}
	

/**
 * Draws a bitmap resource right to the screen.
 *
 * @param resID the bitmap's resource id
 * @param x the x coordinate
 * @param y the y coordinate
 */
void DrawBitmapSimple( Int16 resID, Int16 x, Int16 y )
	{
  	MemHandle	resH;
	BitmapPtr		resP;

	resH = DmGetResource( bitmapRsc, resID );
	ErrFatalDisplayIf( !resH, "Missing bitmap" );
	resP = (BitmapType*)MemHandleLock( resH );
	WinDrawBitmap( resP, x, y );
	MemPtrUnlock( resP );
	DmReleaseResource( resH );
   }


/**
 * Draws a bitmap resource to the screen, but does so using a mask.  If
 * you call this function, make sure that for whatever resID you use,
 * there needs to be a b/w mask for it that has resource id resID + 1;
 *
 * @param resID the bitmap's resource id
 * @param x the x coordinate
 * @param y the y coordinate
 */
void DrawBitmap( Int16 resID, Int16 x, Int16 y )
	{
	MemHandle	dataH, maskH;
	BitmapPtr	data, mask;
   WinHandle   oldWinH, winH;

	dataH = DmGetResource( bitmapRsc, resID );
	maskH = DmGetResource( bitmapRsc, resID+1 );
	ErrFatalDisplayIf( !dataH || !maskH, "Missing bitmap" );
	data = (BitmapPtr)MemHandleLock( dataH );
	mask = (BitmapPtr)MemHandleLock( maskH );

   Coord width, height;
   UInt16 err=errNone;
   
   BmpGlueGetDimensions( data, &width, &height, NULL );
   
   winH = WinCreateOffscreenWindow( width, height, nativeFormat, &err );
   ErrFatalDisplayIf( err != errNone, "Unable to create offscreen window" );

   oldWinH = WinSetDrawWindow( winH );
   WinDrawBitmap( mask, 0, 0 );

   WinSetDrawWindow( oldWinH );
   
   RectangleType srcRect;
   srcRect.topLeft.x = srcRect.topLeft.y = 0;
   srcRect.extent.x = width;
   srcRect.extent.y = height;
   
   WinCopyRectangle( winH, oldWinH, &srcRect, x, y, winMask );

   oldWinH = WinSetDrawWindow( winH );

   WinDrawBitmap( data, 0, 0 );
   WinSetDrawWindow( oldWinH );
   
   WinCopyRectangle( winH, oldWinH, &srcRect, x, y, winOverlay );
   WinSetDrawWindow( oldWinH );
   WinDeleteWindow( winH, false );

	MemPtrUnlock( data );
	MemPtrUnlock( mask );
	DmReleaseResource( dataH );
	DmReleaseResource( maskH );
   }


/**
 * Deallocates the memory used by a control.
 *
 * @param id the control's id
 */
void CtlFreeMemory( UInt16 id )
	{
	ControlPtr		ctl;
   Err err = 0;
   
   ctl = GetObjectPtr<ControlType>( id );
	const Char *label = CtlGetLabel( ctl );
	if( label )
      {
      err = MemPtrFree( (void*)label );
      checkError( err, "CtlFreeMemory", 0 );
      }
	}


/**
 * Allocates a small (1 byte) of memory to a control.
 *
 * @param id the control's id
 */
void CtlInit( UInt16 id )
	{
	ControlPtr		ctl;
	Char 		*label;

	ctl = GetObjectPtr<ControlType>( id );
	label = (char*)MemPtrNew( 1 );
	label[0] = 0;
	CtlSetLabel( ctl, label );
	}
	

/**
 * Returns a pointer to the field that has the focus, or NULL if no field
 * has the focus.
 *
 * @return a pointer to the field with the focus
 */
FieldPtr GetFocusObjectPtr()
	{
	FormPtr           frm;
	UInt16            focusIndex;    // the index, not the id of the object
	FormObjectKind 	objType;

	frm = FrmGetActiveForm();
	focusIndex = FrmGetFocus( frm );
	if( focusIndex == noFocus )
		return NULL;

	objType = FrmGetObjectType( frm, focusIndex );

   switch( objType )
      {
      case frmFieldObj:
         return GetObjectPtr<FieldType>( FrmGetObjectId( frm, focusIndex ));
	
      case frmTableObj:
         return TblGetCurrentField( GetObjectPtr<TableType>( FrmGetObjectId( frm, focusIndex )));
	
      default:
         return NULL;
      }
	}


/**
 * Sets the label of a control, and frees any memory already used by it.
 *
 * @param ctlID control id to set
 * @param str the string to set it to
 * @param allowWidth the max width in pixels that the label can be
 * @return a pointer to the control
 */
ControlPtr SetControlLabel( UInt16 ctlID, Char *str, Int16 allowWidth )
	{
	ControlPtr		ctl;
	const Char		*oldStr;
	Char				*p;
	Int16				drawLen;
	Boolean			fits;
	FontID			curFont = FntSetFont( stdFont );						// save current font
   Err            err = 0;
	
	ctl = GetObjectPtr<ControlType>( ctlID );										// make sure we can get the control
	ErrNonFatalDisplayIf( !ctl, "Incorrect Control Specified." );				// some debugging stuff
	ErrNonFatalDisplayIf( !str, "No string passed to SetControlLabel." );

	drawLen = StrLen( str );											// how many to draw
	FntCharsInWidth( str, &allowWidth, &drawLen, &fits );					// see how much fits

	if( allowWidth != 0 )												// if we only have so much room
		{
		p = (char*)MemPtrNew( drawLen + 1 );				// allocate new memory for the label
		StrNCopy( p, str, drawLen );							// copy in the new string
		p[drawLen] = 0;											// have to terminate it!!
		}
	else															// if we've got lots of room for it
		{
		p = (char*)MemPtrNew( StrLen( str ) + 1 );							// allocate the full amount
		ErrFatalDisplayIf( !p, "Unable to allocate memory" );
      StrCopy( p, str );											// copy in the new string
		}

  ctl = GetObjectPtr<ControlType>( ctlID );
	oldStr = CtlGetLabel( ctl );							// save the old stuff
	CtlSetLabel( ctl, p );												// set the new value
	if( oldStr )
      {
      err = MemPtrFree( (void*)oldStr );								// free the old stuff
      checkError( err, "SetControlLabel()", 0 );
      oldStr = 0;
      }
   
	FntSetFont( curFont );											// restore previous font

	return ctl;														// return a pointer to the control
	}


/**
 * Sets a field's text to a string, and frees any memory previously used
 * by it. It also draws the field, so it must be called after the form has
 * been drawn.
 *
 * @param fldID field id to set
 * @param txtH the string to set it to
 * @return a pointer to the field
 */
FieldPtr SetFieldTextFromHandle( UInt16 fldID, MemHandle txtH, Boolean draw )
	{
	MemHandle	oldH;
	FieldPtr		fld;
	
	fld = GetObjectPtr<FieldType>( fldID );
	ErrNonFatalDisplayIf( !fld, "Incorrect Field Specified." );
	ErrNonFatalDisplayIf( !txtH, "No string passed to SetFieldTextFromHandle." );
	
	oldH = FldGetTextHandle( fld );
	FldSetTextHandle( fld, txtH );
	if( oldH )	MemHandleFree( oldH );

   if( draw )
      FldDrawField( fld );
   
	return fld;
	}


// table rows are always either 10 or 16, depending on the bounds of the table
UInt16 calcTableRows( TablePtr table, Boolean isChooser )
   {	
   if( isChooser )
      return 9;
   
   RectangleType  bounds;
   TblGetBounds( table, &bounds );
   
   UInt16 rows = bounds.extent.y / 10;
   if( rows > 16 )
      rows = 16;

   if( rows < 16 )
      rows = 10;

   return rows;
   }


/**
 * Set the color depth to the highest possible.  Do not put this in any
 * segment (it might be called without the application actually running).
 */
void SetupColorSupport()
	{
	UInt32		desired=8, actual;
	UInt32		romVersion;

/*
   this commented code will not force 8bit, but we have to force it until
   i figure out how to draw transparent bitmaps in 16bit
   
   // see what the color depth is
	WinScreenMode( winScreenModeGet, 0, 0, &actual, 0 );
   
   // now, if the depth is less than 8, then we want to try to set it to 8bit,
   //  or at least as high as possible.
   if( actual < 8 )
      {
      again:

      WinScreenMode( winScreenModeSet, 0, 0, &desired, 0 );    // try a specific mode
      WinScreenMode( winScreenModeGet, 0, 0, &actual, 0 );		// see if it worked or not

      if( desired != actual )											// if the depth was too high,
         {
         switch( desired )										// try a lower depth
            {
            case 8:	desired = 4;	break;
            case 4:	desired = 1;	break;
            //case 2:	desired = 1;	break;
            case 1:	ErrNonFatalDisplay( "Invalid color depth." );		// should never get here
                  colorDepth = 1;
                  return;
            }
			
         goto again;
         }
      }
*/   

   again:

   WinScreenMode( winScreenModeSet, 0, 0, &desired, 0 );    // try a specific mode
   WinScreenMode( winScreenModeGet, 0, 0, &actual, 0 );		// see if it worked or not

   if( desired != actual )											// if the depth was too high,
      {
      switch( desired )										// try a lower depth
         {
         case 8:	desired = 4;	break;
         case 4:	desired = 1;	break;
         //case 2:	desired = 1;	break;
         case 1:	ErrNonFatalDisplay( "Invalid color depth." );		// should never get here
               colorDepth = 1;
               return;
         }
			
      goto again;
      }

   // save the depth so we can check it later
   colorDepth = actual;

	// then see what the os version supports for the color api
	FtrGet( sysFtrCreator, sysFtrNumROMVersion, &romVersion );
   if ( romVersion < sysMakeROMVersion( 3, 5, 0, sysROMStageRelease, 0 ))	// if we have to use the old api calls
		colorMode = colorOldAPI;
	else										// we can use the nice new api calls
		colorMode = colorNewAPI;
	}
	

/**
 * Set the color of the user interface.
 *
 * @param fore the foregound color (or 0 to not change)
 * @param back the backgound color (or 0 to not change)
 * @param text the text color (or 0 to not change)
 * @param foreBW the foregound color if in b/w mode (or 0 to not change)
 * @param bacKBW the backgound color if in b/w mode (or 0 to not change)
 */
void ColorSet( RGBColorType *fore, RGBColorType *back, RGBColorType *text, RGBColorType *foreBW, RGBColorType *backBW )
	{
	if( colorDepth == 1 ) return;									// don't do anything for 1-bit color
	
	 if( colorMode == colorOldAPI )								// if we have to use the old api calls
		{
		WinSetColors( foreBW, &oldrgbf, backBW, &oldrgbb );		
		}
	else																	// we can use the nice new api calls
		{
		WinPushDrawState();
		if( fore )
			WinSetForeColor( WinRGBToIndex( fore ) );
		if( back )
			WinSetBackColor( WinRGBToIndex( back ) );
		if( text )
			WinSetTextColor( WinRGBToIndex( text ) );
		}
	}
	
   
/**
 * Unset the color of the user interface.
 */
void ColorUnset()
	{
	if( colorDepth == 1 ) return;								// don't do anything for 1-bit color

	 if ( colorMode == colorOldAPI )							// if we have to use the old api calls
		WinSetColors( &oldrgbf, 0, &oldrgbb, 0 );	
	else																// we can use the nice new api calls
		WinPopDrawState();
	}
