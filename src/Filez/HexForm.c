/****************************************************************************
 FileZ
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
 * Handles the hex editor form.
 *
 * Created on 12/21/01 by Tom Bulatewicz
 */
   
#include <PalmOS.h>				// all the system toolbox headers

#include "HexForm.h"
#include "Main.h"
#include "Resource.h"
#include "Stuph.h"
#include "UI.h"
#include "resize.h"

#include "Item.hpp"
#include "ItemFile.hpp"
#include "ItemFolder.hpp"

extern Item *editItem;

extern Int32			currentRecord;		// the current record id (defined in recordlist.c)
extern Boolean			isRecs;				// true - record db, false - resource db (defined in recordlist.c)
static Int32			top;					// the top of the scroll
static UInt32			offset;				// current offset
static RectangleType	rectHex;				// rectangle of currently selected hex char
static RectangleType	rectAsc;				// rectangle of currently selected ascii char
static Boolean			hexSelected;		// true if the hex side was selected, false if right side
static Boolean			isSelected;			// is something selected
static UInt32			recSize;				// the size of the current record
static Char				hexOne;				// first char when updating the hex side
static WinHandle		box;					// the secondary selection (the box around a char)
Boolean					readOnly;			// can the user change the file

#define	topMargin		35
#define	asciiMargin		108


static void SECT8 clearOffset()
   {
   FieldPtr    fld;
   MemHandle   memH;
   MemPtr      memP;

   memH = MemHandleNew( 1 );
	memP = (Char*)MemHandleLock( memH );
   StrCopy( (Char*)memP, "" );
	MemHandleUnlock( memH );
	
	fld = GetObjectPtr<FieldType>( HexEditorOffsetField );
	FldSetTextHandle( fld, memH );
	FldDrawField( fld );   
   }


/**
 * Determines if coords are in the hex area of the form (left side).
 *
 * @param the coordinates
 * @return true if in hex area, false if not
 */
static Boolean SECT8 HexEditorInHexArea( Int16 x, Int16 y )
	{
	if( x <= 96 && y >= topMargin && y <= topMargin + 100 )					// it's in the hex area
		return true;
	else
		return false;
	}


/**
 * Determines if coords are in the ascii area of the form (left side).
 *
 * @param the coordinates
 * @return true if in ascii area, false if not
 */
static Boolean SECT8 HexEditorInAsciiArea( Int16 x, Int16 y )
	{
	if( x >= asciiMargin && y >= topMargin && y <= topMargin + 100 )			// it's in the ascii area
		return true;
	else
		return false;
	}

 
/**
 * This routine draws or erases the list view scroll arrow buttons.
 *
 * @param frm             - pointer to the  list form
 * @param bottomRecord    - record index of the last visible record
 * @param lastItemClipped - true if the last item display is not fully visible
 */
static void SECT8 HexEditorUpdateScrollers()
	{
	UInt16 	upIndex, downIndex;
	Boolean	scrollableUp, scrollableDown;
	FormPtr	frm = FrmGetActiveForm();
	
	if( top )									// if not at the top of the screen
		scrollableUp = true;					//	then we can scroll up some
	else
		scrollableUp = false;

	if( recSize > 72 )
		{
		if( top >= (Int32)recSize - 80 )					// if we are at the bottom of the screen
			scrollableDown = false;
		else
			scrollableDown = true;
		}
	else
		scrollableDown = false;
		
	upIndex = FrmGetObjectIndex( frm, HexEditorScrollUpRepeating );
	downIndex = FrmGetObjectIndex( frm, HexEditorScrollDownRepeating );
	FrmUpdateScrollers( frm, upIndex, downIndex, scrollableUp, scrollableDown );
	}


/**
 * Clears the offset field.
 */
static void SECT8 HexEditorClearOffset()
	{
	MemHandle	memH;
	Char        *memP;
	//FieldPtr		fld;
	
	if( DEBUG )						// for some reason, this field will not deallocate properly, so this will
		return;						//	avoid a warning in the emulator
	
	/*fld =*/ GetObjectPtr<FieldType>( HexEditorOffsetField );
	memH = MemHandleNew( 1 );
	memP = (Char*)MemHandleLock( memH );
	StrCopy( memP, "" );
	MemHandleUnlock( memH );
	
	SetFieldTextFromHandle( HexEditorOffsetField, memH, true );
		
	hexOne = 0;
	isSelected = false;
	}


/**
 * Draws a full row (hex and ascii) of a record.
 *
 * @param the record text
 * @param current position
 * @param total size
 * @param current row
 * @return true when no more rows, false when more to go
 */
static Boolean SECT8 HexEditorDrawRow( Char *text, UInt32 pos, UInt32 size, UInt32 row )
	{
	UInt32		i, j, max;
	Char			hex[8];
	RectangleType	r;
	
	r.topLeft.x = 0;		r.topLeft.y = topMargin + ( row * 10 );
	r.extent.x = 160;	r.extent.y = 11;
	WinEraseRectangle( &r, 0 );				// erase what's there now
		
	if( size - pos > 8 )						// if we have a full row
		max = 8;
	else									// if we only have a partial row
		max = size - pos;
	
	for( i=pos, j=0; i<max+pos; i++, j++ )
		{
		Char		*c = text+i;

      UInt32 tmp = (UInt8) c[0];
		DecToHex( tmp, hex, true );

		WinDrawChars( hex, 1, ( j * 12 ), ( row*10 )+topMargin );				// draws the hex text on the left
		WinDrawChars( hex+1, 1, ( j * 12 )+6, ( row*10 )+topMargin );			// draws the hex text on the left
		WinDrawChars( text+i, 1, ( j * 6 )+asciiMargin, ( row*10 )+topMargin );	// draws the ascii text on the right
		}
	
	if( pos + 8 <= size )						// more after this row
		return false;
	else									// no more
		return true;
	}


/**
 * Refresh the whole screen by redrawing all the rows.
 */
static void SECT8 redrawForm()
	{
	DmOpenRef	ref;
	MemHandle	recH;
	MemPtr		recP;
	UInt32		size;
	UInt32		i, row;
	DmResType	resType;
	DmResID		resID;
	//Err			err;

	ref = DmOpenDatabase( editItem->getVolumeNum(), editItem->getFile()->iAttr->id, dmModeReadOnly );
	
	if( isRecs )											// if this is a record database
		{
		recH = DmQueryRecord( ref, currentRecord );
		size = MemHandleSize( recH );
		recSize = size;
		recP = MemHandleLock( recH );
		}
	else													// if this is a resource database
		{
		/*err =*/ DmResourceInfo( ref, currentRecord, &resType, &resID, NULL );
		recH = DmGetResource( resType, resID );
		size = MemHandleSize( recH );
		recSize = size;
		recP = MemHandleLock( recH );
		}
	
	for( i=top, row=0; i<size; i++ )								// for each row
		{
		if( HexEditorDrawRow( (char*)recP, i, size, row ) )				// draw it
			break;
		
		i+= 7;
		row++;
		if( row >= 10 )
			break;
		}
		
	MemHandleUnlock( recH );
	DmCloseDatabase( ref );
	}


/**
 * Initialize the editor form.
 */
static void SECT8 handleFormOpen()
	{
	FormPtr		frm;
	
	frm = FrmGetActiveForm();
	
	if( readOnly )
		{
		FrmHideObject( frm, FrmGetObjectIndex( frm, HexEditorInsertButton ));
		FrmHideObject( frm, FrmGetObjectIndex( frm, HexEditorDeleteButton ));
		}
	
	FrmDrawForm( frm );
	
	top = 0;
	HexEditorClearOffset();
	
	redrawForm();
	HexEditorUpdateScrollers();
	}


/**
 * Highlight or unhighlight a character (both on hex side and ascii side).
 *
 * @param highlight when true, unhighlight when false
 */
static void SECT8 HexEditorHighlight( Boolean on )
	{
	UInt16		err;
	RectangleType	r;
	
	if( on )										// we want to highlight it
		{
		if( hexSelected )							// if they selected the hex side
			{
			WinInvertRectangle( &rectHex, 0 );			// invert the selected char
			r.topLeft.x = rectAsc.topLeft.x - 1;	r.topLeft.y = rectAsc.topLeft.y - 1;
			r.extent.x = rectAsc.extent.x + 2;		r.extent.y = rectAsc.extent.y + 2;
			box = WinSaveBits( &r, &err );				// save the screen where we'll update
			WinDrawRectangleFrame( simpleFrame, &rectAsc );	// draw the box
			}
		else
			{
			WinInvertRectangle( &rectAsc, 0 );			// invert the selected char
			r.topLeft.x = rectHex.topLeft.x - 1;	r.topLeft.y = rectHex.topLeft.y - 1;
			r.extent.x = rectHex.extent.x + 2;		r.extent.y = rectHex.extent.y + 2;
			box = WinSaveBits( &r, &err );				// save the screen where we'll update
			WinDrawRectangleFrame( simpleFrame, &rectHex );	// draw the box
			}
		}
	else											// we want to unhighlight it
		{
		RectangleType	r;
		
		r.topLeft.x = 150;	r.extent.x = 20;
		r.topLeft.y = 18;		r.extent.y = 10;
		WinEraseRectangle( &r, 0 );					// erase the hint text in the upper-right
		
		if( !isSelected ) return;						// can't unhighlight if nothing selected
		
		if( hexSelected )							// if hex side was tapped
			{
			WinInvertRectangle( &rectHex, 0 );			// undo the previous highlight
			WinRestoreBits( box, rectAsc.topLeft.x-1, rectAsc.topLeft.y-1 );
			}
		else
			{
			WinInvertRectangle( &rectAsc, 0 );			// undo the previous highlight
			WinRestoreBits( box, rectHex.topLeft.x-1, rectHex.topLeft.y-1 );
			}
		}
	}


/**
 * Handle what happens when the user taps the screen.
 *
 * @param coordinates of the tap
 * @return if it was handled
 */
static Boolean SECT8 HexEditorHandleTap( Int16 x, Int16 y )
	{
	UInt8       row, col;
	MemHandle	memH;
	Char        *memP;
	FieldPtr		fld;

	if( y < topMargin || y > 146 )							// tapped above or below the main part
		return false;

	HexEditorHighlight( false );		

	if( HexEditorInHexArea( x, y ) )							// tapped in the hex area
		{
		hexSelected = true;
		HexEditorClearOffset();
		
		col = x / 12;									// figure out what column the tap was in
		row = (( y-topMargin ) / 10 );						// figure out what row it was in
		
		offset = ( row * 8 ) + col + top;						// calculate the new offset based on these
		if( offset >= recSize )
			return true;
		
		x = x - ( x % 12 );								// find the x pos of the upper-left corner of the char
		y = ( row * 10 ) + topMargin;						// same

		rectHex.topLeft.x = x;								// define the rectangle around the hex characters
		rectHex.topLeft.y = y;
		rectHex.extent.x = 12;
		rectHex.extent.y = 10;
		
		rectAsc.topLeft.x = asciiMargin + ( (offset%8) * 6 );		// define the rectangle around the ascii character
		rectAsc.topLeft.y = y;
		rectAsc.extent.x = 6;
		rectAsc.extent.y = 10;
		}
	else if( HexEditorInAsciiArea( x, y ) )						// tapped in the ascii area
		{
		hexSelected = false;

		HexEditorClearOffset();
		
		col = (x-asciiMargin) / 6;							// figure out what column the tap was in
		row = ( y-topMargin ) / 10;							// figure out what row it was in

		offset = ( row * 8 ) + col + top;						// calculate the new offset based on these
		if( offset >= recSize )
			return true;

		x = x - ( x % 6 );									// find the x pos of the upper-left corner of the char
		y = ( row * 10 ) + topMargin;						// same

		rectAsc.topLeft.x = x;								// define the rectangle around the ascii character
		rectAsc.topLeft.y = y;
		rectAsc.extent.x = 6;
		rectAsc.extent.y = 10;
		
		rectHex.topLeft.x = (offset % 8) * 12;					// define the rectangle around the hex characters
		rectHex.topLeft.y = y;
		rectHex.extent.x = 12;
		rectHex.extent.y = 10;
		}
	else												// if they didn't tap anywhere
		{
		isSelected = false;								// then clear everything
		HexEditorClearOffset();
		return true;
		}
	
	memH = MemHandleNew( 16 );						// update the value in the offset field
	memP = (char*)MemHandleLock( memH );
	StrIToA( memP, offset );
	MemHandleUnlock( memH );
	
	fld = GetObjectPtr<FieldType>( HexEditorOffsetField );
	FldSetTextHandle( fld, memH );
	FldDrawField( fld );
	
	HexEditorHighlight( true );							// highlight the selected character
	isSelected = true;
	FrmSetFocus( FrmGetActiveForm(), noFocus );
	return true;
	}


/**
 * Update an individual character in the record (on screen and in database).
 *
 * @param the offset of the character to update
 * @param the new character
 */
static void SECT8 HexEditorUpdateChar( UInt32 off, Char ch )
	{
	Char			str[3], inHex[6];
	DmOpenRef	ref;
	MemHandle	recH;
	MemPtr		recP;
	Err			err;
	//Int16			x;
	
	WinEraseRectangle( &rectHex, 0 );									// erase both old characters
	WinEraseRectangle( &rectAsc, 0 );
	
	str[0] = ch;
	str[1] = 0;
	
	//x = ch;
	UInt32 tmp = (UInt8)ch;
   DecToHex( tmp, inHex, true );											// convert the decimal to hexadecimal
      
	WinDrawChars( inHex, 1, rectHex.topLeft.x, rectHex.topLeft.y );				// update the hex char 1
	WinDrawChars( inHex+1, 1, rectHex.topLeft.x+6, rectHex.topLeft.y );			// update the hex char 2
	
	WinDrawChars( str, 1, rectAsc.topLeft.x, rectAsc.topLeft.y );				// update the ascii char
	
	ref = DmOpenDatabase( editItem->getVolumeNum(), editItem->getFile()->iAttr->id, dmModeReadWrite );
	
	recH = DmQueryRecord( ref, currentRecord );							// get the record
	recP = MemHandleLock( recH );
	
	err = DmWrite( recP, off, &ch, 1 );									// write the new value
   checkError( err, "HexEditorUpdateChar()", 0 );
		
	MemHandleUnlock( recH );
	DmCloseDatabase( ref );
	}


/**
 * Inserts a character into a record.
 *
 * @param the offset to place the new character
 * @param the character to use
 */
static void SECT8 HexEditorInsertChar( UInt32 off, Char ch )
	{
	DmOpenRef	ref;
	MemHandle	recH;
	MemPtr		recP;
	Err			err;
	UInt32		origSize, i;
	
	HexEditorHighlight( false );						// unhighlight it
	isSelected = false;
		
	ref = DmOpenDatabase( editItem->getVolumeNum(), editItem->getFile()->iAttr->id, dmModeReadWrite );
	
	recH = DmQueryRecord( ref, currentRecord );	// get the record
	origSize = MemHandleSize( recH );
	MemHandleResize( recH, origSize + 1 );			// increase the size of rec by 1

	recP = MemHandleLock( recH );
	
	for( i=origSize; i>off; i-- )                // move all the bytes to the right by one
		{
		Char *h = (Char*)(recP);
		Char m = h[i-1];
		err = DmWrite( recP, i, &m, 1 );
      checkError( err, "HexEditorInsertChar()", 0 );
		}
		
	err = DmWrite( recP, off, &ch, 1 );				// write the new value
   checkError( err, "HexEditorInsertChar()", 0 );
	
	MemHandleUnlock( recH );
	DmCloseDatabase( ref );
	recSize = origSize + 1;								// update our global rec size guy

   clearOffset();
	redrawForm();									// redraw the whole record
	}


/**
 * Deletes a character from a record.
 *
 * @param the offset to remove the character from
 */
static void SECT8 HexEditorDeleteChar( UInt32 off )
	{
	DmOpenRef	ref;
	MemHandle	recH;
	MemPtr		recP;
	Err			err;
	UInt32		origSize, i;

	HexEditorHighlight( false );										// unhighlight it
	isSelected = false;
		
	ref = DmOpenDatabase( editItem->getVolumeNum(), editItem->getFile()->iAttr->id, dmModeReadWrite );
	
	recH = DmQueryRecord( ref, currentRecord );							// get the record
	origSize = MemHandleSize( recH );
	recP = MemHandleLock( recH );
	
	for( i=off; i<origSize; i++ )											// move each byte to the left by one
		{
		Char *h = (Char*)(recP);
		Char m = h[i+1];
		err = DmWrite( recP, i, &m, 1 );
      checkError( err, "HexEditorDeleteChar()", 0 );
		}
	
	MemHandleUnlock( recH );
	MemHandleResize( recH, origSize - 1 );								// decrease the size of rec by 1
	DmCloseDatabase( ref );
	recSize = origSize - 1;
	
   clearOffset();
	redrawForm();												// redraw the whole record
	}


/**
 * Handle what happens when the user makes a key stroke.
 *
 * @param the character the user typed/graffitied
 * @return if it was handled
 */
static Boolean SECT8 HexEditorHandleKey( Char ch )
   {
   Char		str[4];

	if( readOnly )													// readonly can't make changes
		return false;

	if( hexSelected )
		{														// check for two numbers
		if( ( (UInt8)ch >= 'a' && (UInt8)ch <= 'f' ) || 
			( (UInt8)ch >= 'A' && (UInt8)ch <= 'F' ) ||
			( (UInt8)ch >= '0' && (UInt8)ch <= '9' ) )
			{
			if( !hexOne )											// haven't typed anything yet
				{
				hexOne = ch;
				str[0] = ch;
				str[1] = '?';
				str[2] = 0;
				WinDrawChars( str, 2, 150, 18 );						// draw the hint
				return true;
				}
			else													// already have one, so do it
				{
				RectangleType		r;
				Int16             x;
				
				HexEditorHighlight( false );
				
				r.topLeft.x = 149;
				r.topLeft.y = 18;
				r.extent.x = 10;
				r.extent.y = 10;
				WinEraseRectangle( &r, 0 );							// erase the preview
				
				str[0] = hexOne;
				str[1] = ch;
				str[2] = 0;
				x = HexToDec( str );
				
				HexEditorUpdateChar( offset, x );						// update to the new number
				
            // reset the state
            isSelected = false;
            hexOne = 0;
            }
			
			isSelected = false;
			}
		}
	else
		{														// check for alphanumeric char
		HexEditorHighlight( false );
      HexEditorUpdateChar( offset, ch );							// update to the new character
		HexEditorClearOffset();
		return false;
		}
		
	return true;
	}


/**
 * Jump to a specific character.
 */
static void SECT8 HexEditorGo()
	{
	FieldPtr		fld;
	UInt16		x, y, col, row;
   UInt32      off;
	MemHandle	memH;
	Char        *memP;
	
	HexEditorHighlight( false );
	isSelected = false;
	
	fld = GetObjectPtr<FieldType>( HexEditorOffsetField );
	memH = FldGetTextHandle( fld );
	memP = (Char*)MemHandleLock( memH );
	off = StrAToI( memP );
	MemHandleUnlock( memH );
	
   // first we check to see if they entered an offset that is bigger than the
   //  size of the file. if it is, then we tell the user that the offset is
   //  too big.
   if( off > recSize )
      {
      Char msg[256];
      StrPrintF( msg, "You entered an offset of %ld, but the length of the file is only %ld", off, recSize );
      showMessage( msg );
      return;
      }
   
	// if offset not visible, then need to scroll there	
	offset = off;
	if( off >= 80 || (Int32)off < top )		// <= top ?			// if the target byte is not visible,
		{
		top = offset - 80;						// calculate the new top global value (scroll value)
		if( top < 0 )
			top = 0;
		else
			{
			top += 8;
			top -= ( top % 8 );
			}
		}
	
	offset -= top;								// remove it from the offset value
	
	redrawForm();							// redraw the whole record
	HexEditorUpdateScrollers();
	
	col = offset % 8;
	row = offset / 8;
	
	x = col * 12;								// get some fake coords
	y = ( row * 10 ) + topMargin;
	HexEditorHandleTap( x, y );					// fake out the handler to think the user tapped it
	}


/**
 * Scroll the record up or down by a row.
 *
 * @param direction
 */
static void SECT8 HexEditorScroll( Int8 amount )
	{
	if( recSize <= 80 )					// if the record is less than a page long
		return;								//	then we don't do anything
	
	top += ( 8*amount );					// 10 rows of 8 characters per screen

	if( top < 0 )							// if scrolled off the top,
		top = 0;								// 	then set it to the topmost possible
		
	if( top > (Int32)recSize - 80 )			// if the top is past the last page
		{
      if( !( recSize % 8 ))			// if a multiple of 8 (the row width)
			{
         top = recSize - 80;
			}
      else									// if not a multiple of 8
			{
         top = recSize - 72 - ( recSize % 8 );
			}
		}
	
	redrawForm();							// redraw the whole screen
	HexEditorUpdateScrollers();		// update the arrows on screen
   clearOffset();
	}


static Boolean SECT8 handleKeyDown( EventPtr event )
	{
	Boolean handled = false;
	
	if( isSelected || hexOne )
		{
		if( FrmGetFocus( FrmGetActiveForm() ) != FrmGetObjectIndex( FrmGetActiveForm(), HexEditorOffsetField ))
			handled = HexEditorHandleKey( event->data.keyDown.chr );
		else
			{
			HexEditorHighlight( false );
			isSelected = false;
			}
		}
	
	if( EvtKeydownIsVirtual( event ))
		{
		switch( event->data.keyDown.chr )
			{
			case vchrPageUp:
				HexEditorHighlight( false );
				isSelected = false;
				HexEditorScroll( -9 );
				handled = true;
				break;
			case vchrPageDown:
				HexEditorHighlight( false );
				isSelected = false;
				HexEditorScroll( 9 );
				handled = true;
				break;
			}
		}

	return handled;
	}


/**
 * Handle the events of the form.
 *
 * @param an event
 * @return if it was handled
 */
Boolean HexEditorHandleEvent( EventPtr event )
	{
	Boolean handled = false;

   if ( ResizeHandleEvent( event ) )
      return true;

	switch( event->eType )
		{
		case keyDownEvent:
			handled = handleKeyDown( event );
			break;

		case ctlSelectEvent:
			switch( event->data.ctlSelect.controlID )
				{
				case HexEditorDoneButton:
					FldFreeMemory( GetObjectPtr<FieldType>( HexEditorOffsetField ) );
					FrmUpdateForm( RecordListForm, frmRedrawUpdateCode );
					FrmReturnToForm( RecordListForm );
					handled = true;
					break;
				
				case HexEditorInsertButton:
					if( !isSelected )	
						FrmAlert( SelectByteAlert );
					else
						HexEditorInsertChar( offset, 'X' );
					handled = true;
					break;
				
				case HexEditorDeleteButton:
					if( !isSelected )
						FrmAlert( SelectByteAlert );
					else
						HexEditorDeleteChar( offset );
					handled = true;
					break;
					
				case HexEditorGoButton:
					HexEditorGo();
					handled = true;
					break;
				}
			break;

		case penDownEvent:
			HexEditorHandleTap( event->screenX, event->screenY );
			break;

		case ctlRepeatEvent:
			HexEditorHighlight( false );
			isSelected = false;

			switch( event->data.ctlRepeat.controlID )
				{
				case HexEditorScrollUpRepeating:
					HexEditorScroll( -1 );
					break;
				
				case HexEditorScrollDownRepeating:
					HexEditorScroll( 1 );
					break;
				}
			break;
			
		case frmOpenEvent:
			handleFormOpen();
			handled = true;
			break;
			
		case frmCloseEvent:
			FldFreeMemory( GetObjectPtr<FieldType>( HexEditorOffsetField ) );
			break;		

      case frmUpdateEvent:
         FrmDrawForm( FrmGetActiveForm());
         redrawForm();
         break;
    default:
      break;
		}

	return handled;
	}
