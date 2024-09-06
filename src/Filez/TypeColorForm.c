/****************************************************************************
 FileZ
 Copyright (C) 2003  nosleep software

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
***************************************************************************/

/** @file
 * The file type color selection form.
 *
 * Created on 7/14/03 by Tom Bulatewicz
 */
 
#include <PalmOS.h>						// all the system toolbox headers

#include "Main.h"
#include "TypeColorForm.h"
#include "Stuph.h"
#include "Resource.h"

#define colorTopLeftx	61				// The boundaries of the color square
#define colorTopLefty	113
#define colorExtentx		23
#define colorExtenty 	8

static RGBColorType	theColor;			// The currently selected color


/**
 * Draw the type as a string, in its color.
 *
 * @param The item
 * @param it's bounds
 * @param it's text (which is null)
 * @return if it was handled
 */
static void SECT2 TypeColorDrawRecord( Int16 itemNum, RectangleType *bounds, Char **itemsText )
	{
	Char		text[5];
	
	IntToStr( prefs.list.typeColors.types[itemNum], text );
	ColorSet( NULL, NULL, &prefs.list.typeColors.colors[itemNum], NULL, NULL );
	WinDrawChars( text, StrLen( text ), bounds->topLeft.x, bounds->topLeft.y );
	ColorUnset();
	}


/**
 * Draw the type list.
 */
static void SECT2 TypeColorDrawList()
	{
	ListPtr		lst;
	lst = GetObjectPtr<ListType>( TypeColorTypeList );
	LstSetDrawFunction( lst, TypeColorDrawRecord );
	LstSetListChoices( lst, NULL, prefs.list.typeColors.count );
	LstDrawList( lst );
	}


/**
 * Hides the small details area.
 */
static void SECT2 TypeColorHideDetails()
	{
	RectangleType	rect;
	
	FormPtr		frm = FrmGetActiveForm();
	
	HideObject( frm, TypeColorTypeLabel );
	HideObject( frm, TypeColorTypeField );
	HideObject( frm, TypeColorOKButton );
	HideObject( frm, TypeColorColorLabel );
	
	rect.topLeft.x = colorTopLeftx-1;
	rect.topLeft.y = colorTopLefty-1;
	rect.extent.x = colorExtentx+2;
	rect.extent.y = colorExtenty+2;
	
	WinEraseRectangle( &rect, 0 );
	}


/**
 * Draws the current color in the details area.
 */
static void SECT2 TypeColorDrawColor()
	{
	RectangleType	rect;

	rect.topLeft.x = colorTopLeftx;
	rect.topLeft.y = colorTopLefty;
	rect.extent.x = colorExtentx;
	rect.extent.y = colorExtenty;
	
	ColorSet( &black, NULL, NULL, NULL, NULL );
	WinDrawRectangleFrame( simpleFrame, &rect );
	ColorUnset();
	
	ColorSet( &theColor, NULL, NULL, NULL, NULL );
	WinDrawRectangle( &rect, 0 );	
	ColorUnset();
	}


/**
 * Draws the frame around the details area.
 */
static void SECT2 TypeColorDrawFrame()
	{
	RectangleType	rect;
	
	rect.topLeft.x = 20;
	rect.topLeft.y = 91;
	rect.extent.x = 114;
	rect.extent.y = 35;

	ColorSet( &black, NULL, NULL, NULL, NULL );
	WinDrawRectangleFrame( simpleFrame, &rect );
	ColorUnset();
	}


/**
 * Draws the details area.
 */
static void SECT2 TypeColorShowDetails()
	{
	FormPtr		frm = FrmGetActiveForm();
		
	ShowObject( frm, TypeColorTypeLabel );
	ShowObject( frm, TypeColorTypeField );
	ShowObject( frm, TypeColorOKButton );
	ShowObject( frm, TypeColorColorLabel );

	TypeColorDrawColor();	
	}


/**
 * Handle what happens when the OK button is tapped.
 */
static void SECT2 TypeColorHandleOKButton()
	{
	Char 		textid[5];
	FieldPtr 	fld;
	UInt32		numid;
	
	TypeColorHideDetails();

	fld = GetObjectPtr<FieldType>( TypeColorTypeField );
   if( FldGetTextPtr( fld ))
      {
      StrCopy( textid, FldGetTextPtr( fld ));
      numid = StrToInt( textid );
      prefs.list.typeColors.types[prefs.list.typeColors.count] = numid;
      prefs.list.typeColors.colors[prefs.list.typeColors.count] = theColor;
      prefs.list.typeColors.count++;
      }
      
	TypeColorDrawList();
	}
	

/**
 * Handle what happens when the Add button is tapped.
 */
static void SECT2 TypeColorHandleAddButton()
	{
   if( prefs.list.typeColors.count >= maxTypeColors )
      FrmAlert( 1201 );
   
	FieldPtr	fld;
	fld = GetObjectPtr<FieldType>( TypeColorTypeField );
	FldDelete( fld, 0, FldGetTextLength( fld ));
	theColor = black;
	TypeColorShowDetails();
	}
		

/**
 * Handle what happens when the Remove button is tapped.
 */
static void SECT2 TypeColorHandleRemoveButton()
	{
	Int16    index;
	ListPtr	lst;
	
	lst = GetObjectPtr<ListType>( TypeColorTypeList );
	index = LstGetSelection( lst );
	
   // can't delete anything if there's nothing in the list.
   if( index < 0 )
      return;
   
	for( int i=index; i<prefs.list.typeColors.count-1; i++ )
		{
		prefs.list.typeColors.types[i] = prefs.list.typeColors.types[i+1];
		prefs.list.typeColors.colors[i] = prefs.list.typeColors.colors[i+1];
		}

	prefs.list.typeColors.count--;
	LstSetListChoices( lst, NULL, prefs.list.typeColors.count );
	TypeColorDrawList();
	}


/**
 * Sets up the form.
 */
static void SECT2 TypeColorInit()
	{
	ListPtr		lst;
	FormPtr		frm;
	
	frm = FrmGetActiveForm();

	FrmDrawForm( FrmGetActiveForm() );

	lst = GetObjectPtr<ListType>( TypeColorTypeList );
	LstSetSelection( lst, -1 );		
	TypeColorHideDetails();	
	TypeColorDrawFrame();
	TypeColorDrawList();
	}


/**
 * Sets up the form.
 *
 * @param The direction to scroll
 */
static void SECT2 TypeColorScroll( WinDirectionType direction )
	{
	ListPtr	lst = GetObjectPtr<ListType>( TypeColorTypeList );
	LstScrollList( lst, direction, LstGetVisibleItems( lst ));
	}


/**
 * This decides whether to show the color form or tell the
 *	user that they can't set the colors.  might want to come 
 *	up with an actual alternate menu so that the option is.
 * not visible on the menu for non-color devices.
 */
void HandleTypeColorMenu()
	{
	if( colorDepth >= 8 )
		FrmGotoForm( TypeColorForm );
	else
		FrmAlert( ColorOnlyAlert );
	}


/**
 * Handle when the user taps on the form.
 *
 * @param x coordinate of the tap
 * @param y coordinate of the tap
 * @return if it was handled
 */
static Boolean SECT2 TypeColorHandleTap( Int16 x, Int16 y )
	{
	if( x > colorTopLeftx && x < colorTopLeftx+colorExtentx && y > colorTopLefty && y < colorTopLefty+colorExtenty )
		{
		UIPickColor( NULL, &theColor, NULL, NULL, NULL );		
		TypeColorDrawColor();
		return true;
		}
	return false;
	}


/**
 * Handle all form events.
 *
 * @param an event
 * @return if it was handled
 */
Boolean TypeColorHandleEvent( EventPtr event )
	{
	Boolean handled = false;

	switch( event->eType )
		{
		case penDownEvent:											// if they did anything at all		
			handled = TypeColorHandleTap( event->screenX, event->screenY );
			break;

		
		case keyDownEvent:
			if (event->data.keyDown.chr == pageUpChr)					// Scroll up key presed?
				{
				TypeColorScroll( winUp );
				handled = true;
				}
				else if (event->data.keyDown.chr == pageDownChr)			// Scroll down key presed?
				{
				TypeColorScroll( winDown );
				handled = true;
				}			
			break;


		case ctlSelectEvent:
			switch( event->data.ctlSelect.controlID )
				{
				case TypeColorAddButton:
					TypeColorHandleAddButton();
					handled=true;
					break;

				case TypeColorRemoveButton:
					TypeColorHandleRemoveButton();
					handled=true;
					break;

				case TypeColorOKButton:
					TypeColorHandleOKButton();
					handled=true;
					break;

				case TypeColorCloseButton:
					FrmReturnToForm( 0 );	
					handled=true;
					break;
				}
			break;

		case frmOpenEvent:
			TypeColorInit();
			handled = true;
			break;
		}
	
	return handled;
	}
