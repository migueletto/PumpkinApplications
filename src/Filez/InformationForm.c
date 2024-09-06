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
 * Handles the tabbed information form.
 *
 * Created on 12/20/02 by Tom Bulatewicz
 */
    
#include <PalmOS.h>						// all the system toolbox headers
#include <FeatureMgr.h>					// needed to get the ROM version
#include <ExgMgr.h>
#include <VFSMgr.h>
#include <DLServer.h>					// to get hotsync username
#include "resize.h"

#include "Resource.h"					// application resource defines
#include "Stuph.h"
#include "UI.h"
#include "InformationForm.h"
#include "Tabs.h"
#include "Main.h"
#include "CardSet.hpp"

#include "debug.h"

static GenericTabSet	*infoTabs;
static UInt8			curTab;

#define memoryTab	0
#define hotsyncTab	1
#define batteryTab	2

#define top			32
#define col1		3
#define col2		70
#define height		13


/**
 * Draw the memory details for the internal card.
 */
static void SECT6 MemDrawInternal()
	{
	Char			str[64];
	UInt16			y=top;
	//Err				err;
	Char			cardName[64], manufName[32];
	UInt16			version;
	UInt32			crDate, romSize, ramSize, freeBytes, totalBytes;
	UInt16			cardNo;
   	DateTimeType			d;
	DateFormatType			date;
 	SystemPreferencesType	sysPrefs;

	PrefGetPreferences( &sysPrefs );
	date = sysPrefs.dateFormat;
	
	cardNo = cardSet.getCurrentCardVolumeNum();
  /* err =*/ MemCardInfo( cardNo, cardName, manufName, &version, &crDate, &romSize, &ramSize, 0 );
	cardSet.getCurrentFreeMemory( &freeBytes, &totalBytes );
		
	StrPrintF( str, "Card Number:" );		WinDrawChars( str, StrLen( str ), col1, y );
	StrPrintF( str, "%d", cardNo );			WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Card Name:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrPrintF( str, "%s", cardName );		WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Manuf. Name:" );		WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrPrintF( str, "%s", manufName );		WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Version:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrPrintF( str, "%d", version );		WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Created:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	TimSecondsToDateTime( crDate, &d );
	DateToAscii( d.month, d.day, d.year, date, str );
	WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "ROM Size:" );				WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrPrintF( str, "%ld bytes", romSize );		WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "RAM Size:" );				WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrPrintF( str, "%ld bytes", ramSize );		WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Free RAM:" );					WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrPrintF( str, "%ld bytes", freeBytes );	WinDrawChars( str, StrLen( str ), col2, y );		
	}

	
/**
 * Draw the memory details for a VFS card.
 */
static void SECT6 MemDrawVFS()
	{
	Char			str[64];
	UInt16			y=top;
	//Err				err;
	Char			cardName[64];
	VolumeInfoType	vi;
	UInt32 			freeMemory, totalMemory;
	UInt16 			cardNo;
	
	cardNo = cardSet.getCurrentCardVolumeNum();
	/*err =*/ VFSVolumeInfo( cardNo, &vi );
	cardSet.getCurrentFreeMemory( &freeMemory, &totalMemory );
	StrCopy( cardName, cardSet.getCurrentName());
	
	StrPrintF( str, "Volume Label:" );		WinDrawChars( str, StrLen( str ), col1, y );
	WinDrawChars( cardName, StrLen( cardName ), col2, y );
		
	StrPrintF( str, "Media Type:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	IntToStr( vi.mediaType, str );
	WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Filesystem:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	IntToStr( vi.fsType, str );
	WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Driver Creator:" );		WinDrawChars( str, StrLen( str ), col1, y+=height );
	IntToStr( vi.fsCreator, str );
	WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Mount Class:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	IntToStr( vi.mountClass, str );
	WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Total Size:" );		WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrPrintF( str, "%ld bytes", totalMemory );			WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Free:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrPrintF( str, "%ld bytes", freeMemory );		WinDrawChars( str, StrLen( str ), col2, y );		
	}

	
/**
 * Draw the memory details for a MemPlug card.
 */
 /*
static void SECT3 MemDrawMemPlug()
	{
	UInt16 cardNo;
	Char		str[64];
	UInt32 freeMemory, totalMemory;
	sVOL_INFO	info;
	SfsErr	err;
	Char		cardName[64];
	UInt16	y=top;
	Int32		version;
	
	cardNo = cardSet.getCurrentCardVolumeNum();
	StrCopy( cardName, cardSet.getCurrentName());
	cardMgr.getMemPlugMemory( &freeMemory, &totalMemory );
	err = SfsGetVolInfo( cardNo, "dev0:", &info );
	SfsLibGetLibAPIVersion( cardNo, &version );

	StrPrintF( str, "Volume Label:" );		WinDrawChars( str, StrLen( str ), col1, y );
	WinDrawChars( cardName, StrLen( cardName ), col2, y );
		
	StrPrintF( str, "Bad Clusters:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrIToA( str, info.bad_cluster_num );
	WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Sectors/Cluster:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrIToA( str, info.sector_per_cluster );
	WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Sector Size:" );		WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrIToA( str, info.sector_size );
	WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "API Version:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrIToA( str, version );
	WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Total Size:" );		WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrPrintF( str, "%ld bytes", totalMemory );			WinDrawChars( str, StrLen( str ), col2, y );
		
	StrPrintF( str, "Free:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrPrintF( str, "%ld bytes", freeMemory );		WinDrawChars( str, StrLen( str ), col2, y );
	
   }
*/

/**
 * Draw the serial number of the device.
 *
 * @param x coord
 * @param y coord
 * @param text to display if there is no serial number
 */
static void SECT6 DrawSerialNumOrMessage(Int16 x, Int16 y, Char* noNumberMessage)
	{
	Char* bufP;
	UInt16 bufLen;
	Err retval;
	Int16 count;
	UInt8 checkSum;
	Char checksumStr[2];

	// holds the dash and the checksum digit
	retval = SysGetROMToken (0, sysROMTokenSnum, (UInt8**) &bufP, &bufLen);
	if ((!retval) && (bufP) && ((UInt8) *bufP != 0xFF)) {
	// there's a valid serial number!
	// Calculate the checksum: Start with zero, add each digit,
	// then rotate the result one bit to the left and repeat.
	checkSum = 0;
	for (count=0; count<bufLen; count++) {
	checkSum += bufP[count];
	checkSum = (checkSum<<1) | ((checkSum & 0x80) >> 7);
	}
	
	// Add the two hex digits (nibbles) together, +2
	// (range: 2 - 31 ==> 2-9, A-W)
	// By adding 2 to the result before converting to ascii,
	// we eliminate the numbers 0 and 1, which can be
	// difficult to distinguish from the letters O and I.
	checkSum = ((checkSum>>4) & 0x0F) + (checkSum & 0x0F) + 2;
	// draw the serial number and find out how wide it was
	WinDrawChars(bufP, bufLen, x, y);
	x += FntCharsWidth(bufP, bufLen);
	// draw the dash and the checksum digit right after it
	checksumStr[0] = '-';
	checksumStr[1] =
	((checkSum < 10) ? (checkSum +'0'):(checkSum -10 +'A'));
	WinDrawChars (checksumStr, 2, x, y);
	}
	else // there's no serial number
	// draw a status message if the caller provided one
	if (noNumberMessage)
	WinDrawChars(noNumberMessage, StrLen(noNumberMessage),x, y);
	}


/**
 * Erase the contents of the tabs.
 */
static void SECT6 InfoHideTabs()
	{
	RectangleType	r;
	r.topLeft.x = 2;		r.topLeft.y = top;		r.extent.x = 156;	r.extent.y = 109;
	WinEraseRectangle( &r, 0 );
	}


/**
 * Draw the background of the tabs.
 */
static void SECT6 InfoDrawBackground()
	{
	ColorSet( &bgColor, 0, 0, 0, 0 );
	RectangleType	r;

   r.topLeft.x = 0;		r.topLeft.y = 140;		r.extent.x = 160;	r.extent.y = 30;	
	WinDrawRectangle( &r, 0 );

	r.topLeft.x = 0;		r.topLeft.y = 13;		r.extent.x = 160;	r.extent.y = 8;	
	WinDrawRectangle( &r, 0 );
  
	ColorUnset();

	FormPtr	frm = FrmGetActiveForm();

	// this is to get around a bug in old PalmOS versions (like 3.0) where the
	//	whole control is not drawn by FrmShowObject like it should be.
	FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, InformationDoneButton ), &r );
	WinEraseRectangle( &r, 1 );

	ShowObject( frm, InformationDoneButton );
	}


/**
 * Draw the hotsync tab.
 */
static void SECT6 InfoShowHotSyncTab()
	{
	Char					*s;
	//Err						err;
	UInt32					succSyncDate;
 	SystemPreferencesType	sysPrefs;
	DateFormatType			date;
   	DateTimeType			d;
	TimeFormatType			time;
   	Char					str[32], dat[32], tim[32];
	UInt8					y=top;

	StrPrintF( str, "Username:" );				WinDrawChars( str, StrLen( str ), col1, y );

	s = (char*)MemPtrNew( dlkMaxUserNameLength + 1 );		// allocate memory for hotsync user name
 	DlkGetSyncInfo( NULL, NULL, NULL, s, NULL, NULL );		// get the user's hotsync name
	if( !StrLen( s ) )		StrCopy( s, "(none)" );
	WinDrawChars( s, StrLen( s ), col2, y );		
	if( s )		MemPtrFree( s );							// deallocate memory

	StrPrintF( str, "Serial:" );				WinDrawChars( str, StrLen( str ), col1, y+=height );
  StrCopy(str, "(none)");
	DrawSerialNumOrMessage( col2, y, str );
	
	PrefGetPreferences( &sysPrefs );
	date = sysPrefs.dateFormat;
	time = sysPrefs.timeFormat;

	/*err =*/ DlkGetSyncInfo( &succSyncDate, 0, 0, 0, 0, 0 );
	TimSecondsToDateTime( succSyncDate, &d );

	StrPrintF( str, "Last Synced:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	DateToAscii( d.month, d.day, d.year, date, dat );
	//WinDrawChars( str, StrLen( str ), col2, y );	
	TimeToAscii( d.hour, d.minute, time, tim );
	//WinDrawChars( str, StrLen( str ), col2, y );
	StrPrintF( str, "%s @ %s", dat, tim );
	WinDrawChars( str, StrLen( str ), col2, y );	
	
	if( infoTabs )
		infoTabs->DrawTabs( hotsyncTab );	
	}


/**
 * Draw the memory tab.
 */
static void SECT6 InfoShowMemoryTab()
	{
	switch( cardSet.getCurrentKind())
		{
		case volInternal:	MemDrawInternal();	break;
		case volExternal:		MemDrawVFS();		break;
		//case cardSFS:		MemDrawMemPlug();	break;
		}
	
	if( infoTabs )
		infoTabs->DrawTabs( memoryTab );		
	}


/**
 * Draw the battery tab.
 */
static void SECT6 InfoShowBatteryTab()
	{
	UInt16			warning, critical;
	SysBatteryKind	kind;
	Boolean			pluggedIn;
	UInt8			percent;
	Char			str[24];
	UInt8			y=top;
   UInt16      centiVolts;

	centiVolts = SysBatteryInfo( false, &warning, &critical, 0, &kind, &pluggedIn, &percent );

	StrPrintF( str, "Percent:" );				WinDrawChars( str, StrLen( str ), col1, y );
	StrIToA( str, percent );					StrCat( str, " %" );
	WinDrawChars( str, StrLen( str ), col2, y );		
	
	StrPrintF( str, "Current:" );				WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrFToA( str, ((float)centiVolts)/100.0, 2 );		StrCat( str, "v" );
	WinDrawChars( str, StrLen( str ), col2, y );		
	
	StrPrintF( str, "Warning:" );				WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrFToA( str, ((float)warning)/100.0, 2 );		StrCat( str, "v" );
	WinDrawChars( str, StrLen( str ), col2, y );		
	
	StrPrintF( str, "Critical:" );				WinDrawChars( str, StrLen( str ), col1, y+=height );
	StrFToA( str, ((float)critical)/100.0, 2 );	StrCat( str, "v" );
	WinDrawChars( str, StrLen( str ), col2, y );		
	
	StrPrintF( str, "Type:" );					WinDrawChars( str, StrLen( str ), col1, y+=height );
	switch( kind )
		{
		case sysBatteryKindAlkaline:	StrCopy( str, "Alkaline" );		break;
		case sysBatteryKindNiCad:		StrCopy( str, "NiCad" );		break;
		case sysBatteryKindLiIon:		StrCopy( str, "LiIon" );		break;
		case sysBatteryKindRechAlk:	StrCopy( str, "RechAlk" );		break;
		case sysBatteryKindNiMH:		StrCopy( str, "NiMH" );			break;
		case sysBatteryKindLiIon1400:	StrCopy( str, "LiIon1400" );	break;
		default:                      StrCopy( str, "Unknown" );		break;
		}
	WinDrawChars( str, StrLen( str ), col2, y );

	StrPrintF( str, "Plugged In:" );			WinDrawChars( str, StrLen( str ), col1, y+=height );
	if( pluggedIn == true )
		StrCopy( str, "Yes" );
	else
		StrCopy( str, "No" );		
	WinDrawChars( str, StrLen( str ), col2, y ); 
		
	if( infoTabs )
		infoTabs->DrawTabs( batteryTab );	
	}


/**
 * Tell the current tab to draw itself.
 */
static Boolean SECT6 InfoShowCurrentTab()
	{
	switch( curTab )
		{
		case hotsyncTab:
			InfoHideTabs();
			InfoShowHotSyncTab();
			return true;
		
		case memoryTab:
			InfoHideTabs();
			InfoShowMemoryTab();
			return true;		
					
		case batteryTab:
			InfoHideTabs();
			InfoShowBatteryTab();
			return true;
		}
		
	return false;
	}


/**
 * Handle what happens when the user taps the card trigger.
 */
static void SECT6 InfoDoCardPopTrigger()
	{
	ListPtr		lst;
	//ControlPtr		ctl;
	Int16			sel;
	
	if( cardSet.size() == 1 ) return;
	
	lst = GetObjectPtr<ListType>( InformationCardList );
	//ctl = GetObjectPtr<ControlType>( InformationCardPopTrigger );

	LstSetListChoices( lst, cardSet.getMenu(), cardSet.size() );
	LstSetHeight( lst, cardSet.size() );
	LstSetSelection( lst, cardSet.getCurrentCardIndex());

	sel = LstPopupList( lst );

	if( sel != -1 )
		{
		cardSet.setCurrentCard( sel );
		SetControlLabel( InformationCardPopTrigger, cardSet.getCurrentName(), 0 );
		InfoShowCurrentTab();
		}
   }


/**
 * Setup the info form.
 */
static void SECT6 InfoFreeMemory()
	{
	delete infoTabs;

	CtlFreeMemory( InformationCardPopTrigger );
	}
	

/**
 * Setup the info form.
 */
static void SECT6 InfoInit()
	{
   curTab = memoryTab;
   FormPtr frm = FrmGetActiveForm();
   Char buf[32];
   
	prefs.lastForm = InformationForm;
	
	CtlInit( InformationCardPopTrigger );
	SetControlLabel( InformationCardPopTrigger, cardSet.getCurrentName(), 0 );

   if( cardSet.size() == 1 )
      HideObject( frm, InformationCardPopTrigger );

	FrmDrawForm( frm );

	InfoHideTabs();	
	InfoDrawBackground();

 	infoTabs = new GenericTabSet( 160, 125 );
	
  StrCopy(buf, "Memory");
	infoTabs->AddTab( buf, memoryTab );
  StrCopy(buf, "Hotsync");
	infoTabs->AddTab( buf, hotsyncTab );
  StrCopy(buf, "Battery");
	infoTabs->AddTab( buf, batteryTab );
	
	infoTabs->FinalizeTabs();
	
	InfoShowCurrentTab();
   }


static Boolean handleUpdateEvent()
   {
   FrmDrawForm( FrmGetActiveForm());
   InfoShowCurrentTab();
   
   return true;
   }


/**
 * Given a set of coordinates, determine if they tapped on a tab.
 *
 * @param x coordinate
 * @param y coordingate
 * @return if it was handled
 */
static Boolean SECT6 InfoHandleTap( Int16 x, Int16 y )
	{
	int	whichTab = infoTabs->CheckForTap( x, y );
	
	switch( whichTab )
		{
		case -1:
			return false;

		case hotsyncTab:
			if( curTab != hotsyncTab )
				{
				curTab = hotsyncTab;
				InfoShowCurrentTab();
				}
			return true;
		
		case memoryTab:
			if( curTab != memoryTab )
				{
				curTab = memoryTab;
				InfoShowCurrentTab();
				}
			return true;		
					
		case batteryTab:
			if( curTab != batteryTab )
				{
				curTab = batteryTab;
				InfoShowCurrentTab();
				}
			return true;
		}

	return false;
	}


/**
 * Handle all the events for the file details form.
 *
 * @param an event
 * @return if it was handled
 */
Boolean InfoHandleEvent( EventPtr event )
	{
	Boolean		handled = false;

   if ( ResizeHandleEvent( event ) )
      return true;

	switch( event->eType )
		{
		case ctlSelectEvent:
				
			switch( event->data.ctlEnter.controlID )
				{
				case InformationDoneButton:
					FrmGotoForm( MainViewForm );
					handled = true;
					break;

				case InformationCardPopTrigger:
					InfoDoCardPopTrigger();
					handled = true;
					break;					
				}
			break;

		case penDownEvent:							// if they did anything at all		
			handled = InfoHandleTap( event->screenX, event->screenY );
			break;

		case  frmOpenEvent:
			InfoInit();
			handled = true;
			break;

		case frmCloseEvent:
			InfoFreeMemory();
			break;
			
		case frmUpdateEvent:
         handled = handleUpdateEvent();
			break;
    default:
			break;
		}
		
	return handled;
	}
