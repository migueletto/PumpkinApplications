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
 * Handles the main menu screen.
 *
 * Created on 1/6/02 by Tom Bulatewicz
 */
  
#include <PalmOS.h>					// all the system toolbox headers
#include <DLServer.h>				// to get hotsync username
#include "resize.h"

#include "Main.h"
#include "MenuForm.h"
#include "Stuph.h"
#include "UI.h"
#include "Resource.h"				// application resource defines
#include "CardSet.hpp" 

#include "debug.h"

#define memx 75
#define memy 113
#define memw 70
#define memh 10

#define batx 75
#define baty 130
#define batw 70
#define bath 10


/**
 * Draw the percentage as text.
 *
 * @param x coord
 * @param y coord
 * @param width
 * @param height
 * @param the percent value
 */
static void SECT7 drawPercentText( UInt16 x, UInt16 y, UInt16 w, UInt16 h, double percent )
	{
	RectangleType	r;
	Char str[32];
	
	// erase what's there
	r.topLeft.x = x-1;
	r.topLeft.y = y-1;
	r.extent.y = h+2;	
	r.extent.x = w+2;

	WinEraseRectangle( &r, 0 );

	// now draw the alternate text
	StrFToA( str, percent, 2 );
	StrCat( str, " %" );
	WinDrawChars( str, StrLen(str), x, y );
	}


static void SECT7 drawDoublePercentText( UInt16 x, UInt16 y, UInt16 w, UInt16 h, double percent1, double percent2 )
	{
	RectangleType	r;
	Char percentStr1[16], percentStr2[16], str[32];

	// erase what's there
	r.topLeft.x = x-1;
	r.topLeft.y = y-1;
	r.extent.y = h+2;	
	r.extent.x = w+20;
	WinEraseRectangle( &r, 0 );

	// now draw the alternate text
	StrFToA( percentStr1, percent1, 0 );
	StrFToA( percentStr2, percent2, 0 );
   StrCopy( str, "In: " );
   StrCat( str, percentStr1 );
   StrCat( str, "%, Ex: " );
   StrCat( str, percentStr2 );
   StrCat( str, "%" );
	WinDrawChars( str, StrLen(str), x, y );
	}


/**
 * Draw the number as text.
 *
 * @param x coord
 * @param y coord
 * @param width
 * @param height
 * @param the number
 */
static void SECT7 drawSizeText( UInt16 x, UInt16 y, UInt16 w, UInt16 h, UInt32 number )
	{
	RectangleType	r;
	Char			str[32];
	
	// erase what's there
	r.topLeft.x = x-1;
	r.topLeft.y = y-1;
	r.extent.y = h+2;	
	r.extent.x = w+2;
	WinEraseRectangle( &r, 0 );

	SizeToString( number, str, 5 );
	WinDrawChars( str, StrLen(str), x, y );
	}


static void SECT7 drawVoltageText( UInt16 x, UInt16 y, UInt16 w, UInt16 h, double number )
	{
	RectangleType	r;
	Char			str[32];
	
	// erase what's there
	r.topLeft.x = x-1;
	r.topLeft.y = y-1;
	r.extent.y = h+2;	
	r.extent.x = w+2;
	WinEraseRectangle( &r, 0 );

	StrFToA( str, number, 2 );
   StrCat( str, " v" );
	WinDrawChars( str, StrLen(str), x, y );
	}


static void SECT7 drawDoubleSizeText( UInt16 x, UInt16 y, UInt16 w, UInt16 h, UInt32 number1, UInt32 number2 )
	{
	RectangleType	r;
	Char size1Str[16], size2Str[16], str[32];
	
	// erase what's there
	r.topLeft.x = x-1;
	r.topLeft.y = y-1;
	r.extent.y = h+2;	
	r.extent.x = w+20;
	WinEraseRectangle( &r, 0 );

	SizeToString( number1, size1Str, 5 );
   SizeToString( number2, size2Str, 5 );
   StrPrintF( str, "In: %s, Ex: %s", size1Str, size2Str );
	WinDrawChars( str, StrLen(str), x, y );
	}


/**
 * Draw the percentage as a progress bar.
 *
 * @param x coord
 * @param y coord
 * @param width
 * @param height
 * @param the percent value
 */
static void SECT7 drawProgressBar( UInt16 x, UInt16 y, UInt16 w, UInt16 h, double percent )
	{
	RectangleType	r;
	
	// draw the outside black frame
	r.topLeft.x = x;
	r.topLeft.y = y;
	r.extent.y = h;	
	r.extent.x = w;
	WinEraseRectangle( &r, 0 );
	WinDrawRectangleFrame( simpleFrame, &r );

	// now set the rect to the bar's size
	r.topLeft.x += 1;
	r.topLeft.y += 1;
	r.extent.x -= 2;
	r.extent.y -= 2;

	// now adjust the width of the bar	
	r.extent.x = (Coord)((double)r.extent.x*(double)percent);
	
	// now draw the inside bar
	if( colorDepth >= 8 )
		{
		WinPushDrawState();
		WinSetForeColor( WinRGBToIndex( &blue ) );
		WinDrawRectangle( &r, 0 );
		WinPopDrawState();
		}
	else
		WinDrawRectangle( &r, 0 );
	}


static void SECT7 drawDoubleProgressBar( UInt16 x, UInt16 y, UInt16 w, UInt16 h, double percent1, double percent2 )
	{
	RectangleType	frame, bar1, bar2;
	
	// set the rect to be the size of the frame
	frame.topLeft.x = x;
	frame.topLeft.y = y;
	frame.extent.y = h;	
	frame.extent.x = w+20;

   // draw the frame
	WinEraseRectangle( &frame, 0 );
   frame.extent.x -= 20;
	WinDrawRectangleFrame( simpleFrame, &frame );

   // set the rects to be just smaller than the frame
	bar1.topLeft.x = bar2.topLeft.x = frame.topLeft.x + 1;
	bar1.topLeft.y = bar2.topLeft.y = frame.topLeft.y + 1;
	bar1.extent.x = bar2.extent.x = frame.extent.x - 2;
	bar1.extent.y = bar2.extent.y = frame.extent.y - 2;

   // adjust the height and vertical placement of the bars
   bar1.extent.y = bar2.extent.y = (Coord)(((double)h / 2.0) - 1);
   bar2.topLeft.y += bar1.extent.y;

	// now adjust the width of the bars
	bar1.extent.x = (Coord)((double)bar1.extent.x * percent1 );
	bar2.extent.x = (Coord)((double)bar2.extent.x * percent2 );
	
	// now draw the inside bar
	if( colorDepth >= 8 )
		{
		WinPushDrawState();
		WinSetForeColor( WinRGBToIndex( &blue ) );
		WinDrawRectangle( &bar1, 0 );
		WinDrawRectangle( &bar2, 0 );
		WinPopDrawState();
		}
	else
      {
		WinDrawRectangle( &bar1, 0 );
		WinDrawRectangle( &bar2, 0 );
      }
	}


/**
 * Draw the whole main view.
 */
static void SECT7 redrawForm()
	{
	Char     osversion[32];
	UInt32	intFreeMemory, intTotalMemory;
	UInt32	extFreeMemory, extTotalMemory;
	UInt8    percent;
   double   intPercent, extPercent;

   cardSet.getInternalMemory( 0, &intFreeMemory, &intTotalMemory );
   intPercent = (double)intFreeMemory/(double)intTotalMemory;
   cardSet.getExternalMemory( &extFreeMemory, &extTotalMemory );
   extPercent = (double)extFreeMemory/(double)extTotalMemory;

	switch( prefs.memoryView )
		{
		case viewBar:
         if( cardSet.size() > 1 )
            drawDoubleProgressBar( memx, memy, memw, memh, intPercent, extPercent );
			else
            drawProgressBar( memx, memy, memw, memh, intPercent );
         break;
		case viewPercent:
         if( cardSet.size() > 1 )
            drawDoublePercentText( memx, memy, memw, memh, intPercent*100.0, extPercent*100.0 );
			else
            drawPercentText( memx, memy, memw, memh, intPercent*100.0 );
         break;
		case viewNum:
         if( cardSet.size() > 1 )
            drawDoubleSizeText( memx, memy, memw, memh, intFreeMemory, extFreeMemory );
         else
            drawSizeText( memx, memy, memw, memh, intFreeMemory );
			break;
		}
	
	UInt16 centiVolts = SysBatteryInfo( false, 0, 0, 0, 0, 0, &percent );
	switch( prefs.batteryView )
		{
		case viewBar:
			drawProgressBar( batx, baty, batw, bath, (double)percent/100.0 );						// draw battery level
			break;	
		case viewPercent:
			drawPercentText( batx, baty, batw, bath, percent );
			break;
      case viewNum:
         drawVoltageText( batx, baty, batw, bath, (double)centiVolts/100.0 );
         break;
		}
      
	getOSVersion( osversion );
	WinDrawChars( osversion, StrLen( osversion ), 75, 146 );				// draw os version string

   WinDrawLine( 5, 102, 155, 102 );
	}


static Boolean SECT7 handleUpdateEvent()
   {
   FrmDrawForm( FrmGetActiveForm());
   redrawForm();
   
   return true;
   }


/**
 * Setup the main view.
 */
static void SECT7 handleFormOpenEvent()
	{
   FormPtr	frm = FrmGetActiveForm();
   
   // we reload the cardset here because the user may have deleted or moved
   //  files, resulting in different amounts of free memory than when FileZ
   //  was first run.
   cardSet.init();
	prefs.lastForm = MainViewForm;
	FrmDrawForm( frm );
	redrawForm();
	}


/**
 * This routine performs the menu command specified.
 *
 * @param What menu command to do
 * @return Whether it was handled or not
 */
static Boolean SECT7 handleMenuEvent( UInt16 command )
	{
	Boolean	handled = false;
	FormPtr	frmP;
	//Boolean	move = false;

	switch( command )
		{		
		case MenuOptionsAbout:
			MenuEraseStatus( 0 );
			frmP = FrmInitForm( AboutBoxForm );
			FrmDoDialog( frmP );
	 		FrmDeleteForm( frmP );
			handled = true;
			break;		
		}

	return handled;
	}


/**
 * Handle what happens when the user taps a progress bar.
 *
 * @param x coordinate
 * @param y coordinate
 * @return if it was handled
 */
static Boolean SECT7 handleTap( Int16 x, Int16 y )
	{
	RectangleType		r;
	Boolean				handled=false;

	// check for tapping in the memory bar area
	r.topLeft.x = memx;
	r.topLeft.y = memy;
	r.extent.y = memh;	
	r.extent.x = memw;
			
	if( RctPtInRectangle( x, y, &r ) )
		{
		prefs.memoryView++;
		if( prefs.memoryView > viewNum )
			prefs.memoryView = viewBar;
		redrawForm();
		handled =true;
		}

	// check for tapping in the battery bar area
	r.topLeft.x = batx;
	r.topLeft.y = baty;
	r.extent.y = bath;	
	r.extent.x = batw;
			
	if( RctPtInRectangle( x, y, &r ) )
		{
		prefs.batteryView++;
		if( prefs.batteryView > viewNum )
			prefs.batteryView = viewBar;
		redrawForm();
		handled =true;
		}
		
	return handled;
	}
	

/**
 * Handle the events of the system info form.
 *
 * @param an event
 * @return if it was handled
 */
Boolean MainViewHandleEvent( EventPtr event )
	{
	Boolean handled = false;

   if ( ResizeHandleEvent( event ) )
      return true;

	switch( event->eType )
		{
		case ctlSelectEvent:
			switch( event->data.ctlSelect.controlID )
				{
				case MainViewFilesButton:
					FrmGotoForm( TreeViewForm );
					handled = true;
					break;
					
				case MainViewInformationButton:
					FrmGotoForm( InformationForm );
					handled = true;
					break;
					
				case MainViewPreferencesButton:
					FrmGotoForm( PrefListForm );
					handled = true;
					break;
				}
			break;

		case penDownEvent:
			handled = handleTap( event->screenX, event->screenY );
			break;

		case frmOpenEvent:
			handleFormOpenEvent();
			handled = true;
			break;

		case frmUpdateEvent:
         handled = handleUpdateEvent();
			break;

		case menuEvent:
			handled = handleMenuEvent( event->data.menu.itemID );
    default:
      break;
		}

	return handled;
	}
