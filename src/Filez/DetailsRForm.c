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
 * Handles the funky details screen of the built-in card's files.
 *
 * Created on 5/25/01 by Tom Bulatewicz
 */

#include <PalmOS.h>						// all the system toolbox headers
#include <FeatureMgr.h>					// needed to get the ROM version
#include <ExgMgr.h>
#include <VFSMgr.h>
#include "resize.h"

#include "Main.h"
#include "Resource.h"					// application resource defines
#include "Stuph.h"
#include "UI.h"
#include "Main.h"
#include "DetailsForm.h"
#include "Menu.h"
#include "Tabs.h"

#include "TreeViewForm.h"
#include "Item.hpp"
#include "ItemFile.hpp"
#include "ItemFolder.hpp"
#include "ItemSet.hpp"

static DateTimeType		createdDateTime;
static DateTimeType		modifiedDateTime;
static DateTimeType		backedUpDateTime;

Boolean detailsPoppedUp;

static Char				formTitle[25];
static Boolean			originalResDB, originalOpen;
static GenericTabSet	*detailTabs;
static UInt8			curTab;

#define		genTab		0
#define		attrTab		1
#define 		datesTab		2

static Item  *item = 0;          // the currently selected item

/**
 * Save any changes made to the file details.
 *
 * @return True if ok, false if not
 */
static Boolean SECT7 DetailsApply()
	{
	//Err         err=0;
	ControlPtr  ctl;
	FieldPtr    fld;
	char        typeStr[5], creatorStr[5];
	char        versionIDStr[12], oldName[dmDBNameLength];
   
   if( item->getFile()->eAttr )
      {
      showMessage( "Saving a VFS file in the database file details in DetailsApply(): %s", item->getName());
      return false;
      }
   
	// first save the general tab controls ------------------------------------------

   StrCopy( oldName, item->getName() );
	
	fld = GetObjectPtr<FieldType>( DetailsRNameField );			// get the name field
   item->setName( FldGetTextPtr( fld ));

   fld = GetObjectPtr<FieldType>( DetailsRTypeField );			// get the type field
   StrCopy( typeStr, FldGetTextPtr( fld ));
   item->getFile()->iAttr->type = StrToInt( typeStr );

   fld = GetObjectPtr<FieldType>( DetailsRCreatorField );		// get the creator field
   StrCopy( creatorStr, FldGetTextPtr( fld ));
   item->getFile()->iAttr->creator = StrToInt( creatorStr );

   fld = GetObjectPtr<FieldType>( DetailsRVersionIDField );	// get the version id field
   StrCopy( versionIDStr, FldGetTextPtr( fld ));
   item->getFile()->iAttr->version = StrAToI( versionIDStr );

   // then save the attribute tab controls ------------------------------------------

   //item->getFile()->iAttr->attr = 0;

   ctl = GetObjectPtr<ControlType>( DetailsRROCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrReadOnly;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrReadOnly;

   ctl = GetObjectPtr<ControlType>( DetailsRInstallNewCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrOKToInstallNewer;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrOKToInstallNewer;
	
   ctl = GetObjectPtr<ControlType>( DetailsRBackupCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrBackup;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrBackup;

   ctl = GetObjectPtr<ControlType>( DetailsRResetCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrResetAfterInstall;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrResetAfterInstall;

   ctl = GetObjectPtr<ControlType>( DetailsRStreamCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrStream;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrStream;

   ctl = GetObjectPtr<ControlType>( DetailsRProtectCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrCopyPrevention;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrCopyPrevention;
		
   ctl = GetObjectPtr<ControlType>( DetailsRHiddenCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrHidden;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrHidden;

   ctl = GetObjectPtr<ControlType>( DetailsRAppDirtyCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrAppInfoDirty;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrAppInfoDirty;

   ctl = GetObjectPtr<ControlType>( DetailsRLaunchableCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrLaunchableData;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrLaunchableData;

   ctl = GetObjectPtr<ControlType>( DetailsROpenCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrOpen;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrOpen;

   ctl = GetObjectPtr<ControlType>( DetailsRResDBCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrResDB;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrResDB;
   
   ctl = GetObjectPtr<ControlType>( DetailsRBundleCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrBundle;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrBundle;
   
   ctl = GetObjectPtr<ControlType>( DetailsRRecyclableCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->iAttr->attr |= dmHdrAttrRecyclable;
	else                          item->getFile()->iAttr->attr &= ~dmHdrAttrRecyclable;

	// then save the date tab controls ------------------------------------------

   item->getFile()->iAttr->created = TimDateTimeToSeconds( &createdDateTime );
   item->getFile()->iAttr->modified = TimDateTimeToSeconds( &modifiedDateTime );
   item->getFile()->iAttr->backedUp = TimDateTimeToSeconds( &backedUpDateTime );

   item->updateFile( oldName );

	return true;
	}


/**
 * Convert a 4 byte int into a datetime struct.
 *
 * @param the date and the original integer
 */
/*
static void SECT7 DatesConvertDateTime( DateTimeType *fields, UInt32 base )
	{
	if( base == 0)
		MemSet( &fields, sizeof(DateTimeType), 0 );
	else
		TimSecondsToDateTime( base, fields);
	}
*/


/**
 * Update the display with the new time.
 *
 * @param the id of the control to update, the new datetime
 */
static void SECT7 DatesUpdateTime( Int16 id, DateTimeType *t )
	{
	//Int16			hours, minutes, seconds;
	Char			label[20];
	
	//hours = t->hour;
	//minutes = t->minute;
	//seconds = t->second;
	
	if( !TimDateTimeToSeconds( t ) )
		StrCopy( label, "Never" );	
	else
		TimeToAscii( t->hour, t->minute, tfColonAMPM, label );
	SetControlLabel( id, label, 0 );
	}


/**
 * Handle when the user taps a time.
 *
 * @param the tapped id, the new datetime
 */
static void SECT7 DatesSelectTime( Int16 id, DateTimeType *t )
	{
	Char					*titleP;
	MemHandle				titleH;
	Int16					hours, minutes;
	Char					label[20];
	SystemPreferencesType	sysPrefs;
	TimeFormatType			timeFormat;

	PrefGetPreferences( &sysPrefs );
	timeFormat = sysPrefs.timeFormat;
	
	if( !TimDateTimeToSeconds( t ))
		{ FrmAlert( SelectDateAlert ); return; }
		
	titleH = DmGetResource( strRsc, SelectTimeString );
	titleP = (Char*)MemHandleLock( titleH );
	
	hours = t->hour;
	minutes = t->minute;
		
	if( SelectATime( &hours, &minutes, titleP ) )
		{
		t->hour = hours;
		t->minute = minutes;
		t->second = 0;

		TimeToAscii( t->hour, t->minute, timeFormat, label );
		SetControlLabel( id, label, 0 );
		}

	MemHandleUnlock( titleH );
	}


/**
 * Handle when the user taps a date.
 *
 * @param the tapped id, the new datetime
 */
static void SECT7 DatesSelectDate( Int16 id, DateTimeType *t )
	{
	Char					*titleP;
	MemHandle				titleH;
	Int16					day, month, year, selected;
	Char					label[20];
	ListPtr					lst;
	UInt32					secs;
	SystemPreferencesType	sysPrefs;
	DateFormatType			dateFormat;

	PrefGetPreferences( &sysPrefs );
	dateFormat = sysPrefs.dateFormat;
	
	// first show the dropdown list of choices and see what they say...
	
	lst = GetObjectPtr<ListType>( DetailsRDateChoicesList );
	switch( id )
		{
		case DetailsRCreatedDateSelTrigger:
			LstSetPosition( lst, 63, 47 );
			break;
		case DetailsRModifiedDateSelTrigger:
			LstSetPosition( lst, 63, 68 );
			break;
		case DetailsRBackedUpDateSelTrigger:
			LstSetPosition( lst, 63, 90 );
			break;		
		}
			
	selected = LstPopupList( lst );
	
	if( selected == -1 )											// no choice
		{
		return;
		}
	else if( selected == 0 )										// now choice
		{
		secs = TimGetSeconds();
		TimSecondsToDateTime( secs, t );
		DateToAscii( t->month, t->day, t->year, dateFormat, label );
		}
	else if( selected == 1 )										// never choice
		{
		t->hour = t->second = t->minute = t->weekDay = 0;
		t->year = 1904;
		t->day = t->month = 1;
		 
		StrCopy( label, "Never" );
		}
	if( selected == 2 )											// they want to choose themselves
		{
		titleH = DmGetResource( strRsc, SelectDateString );
		titleP = (Char*)MemHandleLock( titleH );
	
		day = t->day;
		month = t->month;
		year = t->year;
		
		DateToAscii( t->month, t->day, t->year, dateFormat, label );
		
		if( SelectDay( selectDayByDay, &month, &day, &year, titleP ) )
			{
			t->day = day;
			t->month = month;
			t->year = year;
			
			DateToAscii( t->month, t->day, t->year, dateFormat, label );
			}

		MemHandleUnlock( titleH );
		}
		
	SetControlLabel( id, label, 0 );
	
	switch( id )
		{
		case DetailsRCreatedDateSelTrigger:
			DatesUpdateTime( DetailsRCreatedTimeSelTrigger, t );
			break;
		case DetailsRModifiedDateSelTrigger:
			DatesUpdateTime( DetailsRModifiedTimeSelTrigger, t );
			break;
		case DetailsRBackedUpDateSelTrigger:
			DatesUpdateTime( DetailsRBackedUpTimeSelTrigger, t );
			break;		
		}
	}


static void SECT7 DetailsShowGeneral()
	{
	FormPtr		frm = FrmGetActiveForm();
	
	ShowObject( frm, DetailsRNameLabel );
	ShowObject( frm, DetailsRNameField );
	ShowObject( frm, DetailsRSizeLabel );
	ShowObject( frm, DetailsRSizeField );
   ShowObject( frm, DetailsRTypeLabel );
   ShowObject( frm, DetailsRTypeField );
   ShowObject( frm, DetailsRCreatorLabel );
   ShowObject( frm, DetailsRCreatorField );
   ShowObject( frm, DetailsRCreatorSizeField );
   ShowObject( frm, DetailsRUniqueIDLabel );
   ShowObject( frm, DetailsRUniqueIDField );
   ShowObject( frm, DetailsRVersionIDLabel );
   ShowObject( frm, DetailsRVersionIDField );

   if( item->getFile()->eAttr )
      {
      DisableField( frm, DetailsRNameField );
      DisableField( frm, DetailsRSizeField );
      DisableField( frm, DetailsRTypeField );
      DisableField( frm, DetailsRCreatorField );
      DisableField( frm, DetailsRCreatorSizeField );
      DisableField( frm, DetailsRUniqueIDField );
      DisableField( frm, DetailsRVersionIDField );
      }

	if( detailTabs )
		detailTabs->DrawTabs( genTab );
	}

static void SECT7 DetailsHideGeneral()
	{
	FormPtr		frm = FrmGetActiveForm();

   // have to re-enable them so that they will erase :(
   if( item->getFile()->eAttr )
      {
      EnableField( frm, DetailsRNameField );
      EnableField( frm, DetailsRSizeField );
      EnableField( frm, DetailsRTypeField );
      EnableField( frm, DetailsRCreatorField );
      EnableField( frm, DetailsRCreatorSizeField );
      EnableField( frm, DetailsRUniqueIDField );
      EnableField( frm, DetailsRVersionIDField );
      }
	
	HideObject( frm, DetailsRNameLabel );
	HideObject( frm, DetailsRNameField );
	HideObject( frm, DetailsRTypeLabel );
	HideObject( frm, DetailsRTypeField );
	HideObject( frm, DetailsRCreatorLabel );
	HideObject( frm, DetailsRCreatorField );
	HideObject( frm, DetailsRCreatorSizeField );
	HideObject( frm, DetailsRSizeLabel );
	HideObject( frm, DetailsRSizeField );
	HideObject( frm, DetailsRUniqueIDLabel );
	HideObject( frm, DetailsRUniqueIDField );
	HideObject( frm, DetailsRVersionIDLabel );
	HideObject( frm, DetailsRVersionIDField );
	}

static void SECT7 DetailsShowAttributes()
	{
	FormPtr		frm = FrmGetActiveForm();

   ShowObject( frm, DetailsRROCheckbox );
   ShowObject( frm, DetailsRHiddenCheckbox );
   ShowObject( frm, DetailsRBackupCheckbox );
   ShowObject( frm, DetailsRStreamCheckbox );
   ShowObject( frm, DetailsRInstallNewCheckbox );
   ShowObject( frm, DetailsRResetCheckbox );
   ShowObject( frm, DetailsRProtectCheckbox );
   ShowObject( frm, DetailsRAppDirtyCheckbox );
   ShowObject( frm, DetailsRLaunchableCheckbox );
   ShowObject( frm, DetailsROpenCheckbox );
   ShowObject( frm, DetailsRResDBCheckbox );
   ShowObject( frm, DetailsRBundleCheckbox );
   ShowObject( frm, DetailsRRecyclableCheckbox );

   if( item->getFile()->eAttr )
      {
      DisableControl( frm, DetailsRROCheckbox );
      DisableControl( frm, DetailsRHiddenCheckbox );
      DisableControl( frm, DetailsRBackupCheckbox );
      DisableControl( frm, DetailsRStreamCheckbox );
      DisableControl( frm, DetailsRInstallNewCheckbox );
      DisableControl( frm, DetailsRResetCheckbox );
      DisableControl( frm, DetailsRProtectCheckbox );
      DisableControl( frm, DetailsRAppDirtyCheckbox );
      DisableControl( frm, DetailsRLaunchableCheckbox );
      DisableControl( frm, DetailsROpenCheckbox );
      DisableControl( frm, DetailsRResDBCheckbox );
      DisableControl( frm, DetailsRBundleCheckbox );
      DisableControl( frm, DetailsRRecyclableCheckbox );
      }
      
	if( detailTabs )
		detailTabs->DrawTabs( attrTab );
	}


static void SECT7 DetailsHideAttributes()
	{
	FormPtr		frm = FrmGetActiveForm();
	
	HideObject( frm, DetailsRROCheckbox );
	HideObject( frm, DetailsRBackupCheckbox );
	HideObject( frm, DetailsRStreamCheckbox );
	HideObject( frm, DetailsRHiddenCheckbox );
	HideObject( frm, DetailsRInstallNewCheckbox );
	HideObject( frm, DetailsRResetCheckbox );
	HideObject( frm, DetailsRProtectCheckbox );
	HideObject( frm, DetailsRAppDirtyCheckbox );
	HideObject( frm, DetailsRLaunchableCheckbox );
	HideObject( frm, DetailsROpenCheckbox );
	HideObject( frm, DetailsRResDBCheckbox );
	HideObject( frm, DetailsRBundleCheckbox );
	HideObject( frm, DetailsRRecyclableCheckbox );
	}

	
static void SECT7 DetailsShowDates()
	{
	FormPtr		frm = FrmGetActiveForm();
	
	ShowObject( frm, DetailsRCreatedLabel );
	ShowObject( frm, DetailsRCreatedDateSelTrigger );
	ShowObject( frm, DetailsRCreatedTimeSelTrigger );

	ShowObject( frm, DetailsRModifiedLabel );
	ShowObject( frm, DetailsRModifiedDateSelTrigger );
	ShowObject( frm, DetailsRModifiedTimeSelTrigger );

   ShowObject( frm, DetailsRBackedUpLabel );
   ShowObject( frm, DetailsRBackedUpDateSelTrigger );
   ShowObject( frm, DetailsRBackedUpTimeSelTrigger );

   if( item->getFile()->eAttr )
      {
      DisableControl( frm, DetailsRCreatedDateSelTrigger );
      DisableControl( frm, DetailsRCreatedTimeSelTrigger );
      DisableControl( frm, DetailsRModifiedDateSelTrigger );
      DisableControl( frm, DetailsRModifiedTimeSelTrigger );
      DisableControl( frm, DetailsRBackedUpDateSelTrigger );
      DisableControl( frm, DetailsRBackedUpTimeSelTrigger );
      }
	
	if( detailTabs )
		detailTabs->DrawTabs( datesTab );
	}
	
static void SECT7 DetailsHideDates()
	{
	FormPtr		frm = FrmGetActiveForm();
	
	HideObject( frm, DetailsRCreatedLabel );
	HideObject( frm, DetailsRModifiedLabel );
	HideObject( frm, DetailsRBackedUpLabel );
	HideObject( frm, DetailsRCreatedDateSelTrigger );
	HideObject( frm, DetailsRModifiedDateSelTrigger );
	HideObject( frm, DetailsRBackedUpDateSelTrigger );
	HideObject( frm, DetailsRCreatedTimeSelTrigger );
	HideObject( frm, DetailsRModifiedTimeSelTrigger );
	HideObject( frm, DetailsRBackedUpTimeSelTrigger );
	}

static Boolean SECT7 DetailsShowCurrentTab()
	{
	switch( curTab )
		{
		case genTab:
			DetailsHideAttributes();
			DetailsHideDates();
			DetailsShowGeneral();
			return true;
		
		case attrTab:
			DetailsHideGeneral();
			DetailsHideDates();
			DetailsShowAttributes();
			return true;		
					
		case datesTab:
			DetailsHideAttributes();
			DetailsHideGeneral();
			DetailsShowDates();
			return true;
		}
		
	return false;
	}

static void SECT7 DetailsDrawBackground()
	{
	FormPtr	frm = FrmGetActiveForm();
      
   if( item->getFile()->eAttr )
      {
      DisableControl( frm, DetailsRSaveButton );
      DisableControl( frm, DetailsRDeleteButton );
      DisableControl( frm, DetailsRBeamButton );

      HideObject( frm, DetailsRSaveButton );
      HideObject( frm, DetailsRDeleteButton );
      HideObject( frm, DetailsRBeamButton );
      }

	ColorSet( &bgColor, 0, 0, 0, 0 );
	RectangleType	r;
	r.topLeft.x = 0;		r.topLeft.y = 135;		r.extent.x = 159;	r.extent.y = 21;	
	WinDrawRectangle( &r, 0 );

	r.topLeft.x = 0;		r.topLeft.y = 12;		r.extent.x = 159;	r.extent.y = 5;	
	WinDrawRectangle( &r, 0 );

	ColorUnset();

	// this is to get around a bug in old PalmOS versions (like 3.0) where the
	//	whole control is not drawn by FrmShowObject like it should be.
   if( !item->getFile()->eAttr )
      {
      FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, DetailsRSaveButton ), &r );
      WinEraseRectangle( &r, 1 );
      FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, DetailsRDeleteButton ), &r );
      WinEraseRectangle( &r, 1 );
      FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, DetailsRBeamButton ), &r );
      WinEraseRectangle( &r, 1 );
      }
   FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, DetailsRCloseButton ), &r );
	WinEraseRectangle( &r, 1 );

   ShowObject( frm, DetailsRCloseButton );

   if( !item->getFile()->eAttr )
      {
      ShowObject( frm, DetailsRSaveButton );
      ShowObject( frm, DetailsRDeleteButton );
      ShowObject( frm, DetailsRBeamButton );
      }
	}

static void SECT7 DetailsSetTitle( Char *str )
	{
	if( StrLen( str ) < 22 )
		{
		StrCopy( formTitle, str );
		for( int i=0; i<StrLen( formTitle ); i++ )
			if( formTitle[i] == '\n' )
				formTitle[i] = 0;
		}
	else
		{
		StrNCopy( formTitle, str, 23 );
		formTitle[21] = '.';
		formTitle[22] = '.';
		formTitle[23] = '.';
		formTitle[24] = 0;

		for( int i=0; i<StrLen( formTitle ); i++ )
			if( formTitle[i] == '\n' )
				{
				formTitle[i] = '.';
				formTitle[i+1] = '.';
				formTitle[i+2] = '.';
				formTitle[i+3] = 0;
				}

		}
	FrmSetTitle( FrmGetActiveForm(), formTitle );
	}

static void SECT7 DetailsSetValues()
	{
	char		*p;
	MemHandle	h;
	//FormPtr		frm;
	ControlPtr	ctl;

	//frm = FrmGetActiveForm();

	// first set the general tab controls ------------------------------------
	
	h = MemHandleNew( dmDBNameLength );						// set the name
	p = (Char*)MemHandleLock( h );
	StrCopy( p, item->getName() );
      
	DetailsSetTitle( p );
	MemHandleUnlock( h );
	SetFieldTextFromHandle( DetailsRNameField, h, false );

   h = MemHandleNew( 5 );										// set the type
   p = (Char*)MemHandleLock( h );
   IntToStr( item->getFile()->iAttr->type, p );
   MemHandleUnlock( h );
   SetFieldTextFromHandle( DetailsRTypeField, h, false );

   h = MemHandleNew( 5 );										// set the creator
   p = (Char*)MemHandleLock( h );
   IntToStr( item->getFile()->iAttr->creator, p );
   MemHandleUnlock( h );
   SetFieldTextFromHandle( DetailsRCreatorField, h, false );
	
   UInt32 recCount = item->getFile()->iAttr->recCount;
   UInt32 size = item->getFile()->size;

   h = MemHandleNew( 64 );										// set the size/record count
   p = (Char*)MemHandleLock( h );
   StrPrintF( p, "%ld b, %ld recs", size, recCount );
   MemHandleUnlock( h );
   SetFieldTextFromHandle( DetailsRSizeField, h, false );

   h = MemHandleNew( 64 );										// set the unique id
   p = (Char*)MemHandleLock( h );
   StrPrintF( p, "%ld, card %d", item->getFile()->iAttr->id, item->getVolumeNum());
   MemHandleUnlock( h );
   SetFieldTextFromHandle( DetailsRUniqueIDField, h, false );

   h = MemHandleNew( 12 );										// set the version id
   p = (Char*)MemHandleLock( h );
   StrIToA( p, item->getFile()->iAttr->version );
   MemHandleUnlock( h );
   SetFieldTextFromHandle( DetailsRVersionIDField, h, false );

	// now set the attributes tab controls ------------------------------------

   ctl = GetObjectPtr<ControlType>( DetailsRROCheckbox );			
   CtlSetValue( ctl, item->getFile()->iAttr->attr & dmHdrAttrReadOnly );
      
   ctl = GetObjectPtr<ControlType>( DetailsRInstallNewCheckbox );	
   CtlSetValue( ctl, item->getFile()->iAttr->attr & dmHdrAttrOKToInstallNewer );

   ctl = GetObjectPtr<ControlType>( DetailsRBackupCheckbox );		
   CtlSetValue( ctl, item->getFile()->iAttr->attr & dmHdrAttrBackup );
      
   ctl = GetObjectPtr<ControlType>( DetailsRResetCheckbox );		
   CtlSetValue( ctl, item->getFile()->iAttr->attr & dmHdrAttrResetAfterInstall );

   ctl = GetObjectPtr<ControlType>( DetailsRStreamCheckbox );		
   CtlSetValue( ctl, item->getFile()->iAttr->attr & dmHdrAttrStream );
	
   ctl = GetObjectPtr<ControlType>( DetailsRProtectCheckbox );		
   CtlSetValue( ctl, item->getFile()->iAttr->attr & dmHdrAttrCopyPrevention );

   ctl = GetObjectPtr<ControlType>( DetailsRHiddenCheckbox );		
   CtlSetValue( ctl, item->getFile()->iAttr->attr & dmHdrAttrHidden );
      
   ctl = GetObjectPtr<ControlType>( DetailsRAppDirtyCheckbox );	
   CtlSetValue( ctl, item->getFile()->iAttr->attr & dmHdrAttrAppInfoDirty );

   ctl = GetObjectPtr<ControlType>( DetailsRLaunchableCheckbox );	
   CtlSetValue( ctl, item->getFile()->iAttr->attr & dmHdrAttrLaunchableData );

   ctl = GetObjectPtr<ControlType>( DetailsROpenCheckbox );		
   CtlSetValue( ctl, item->getFile()->iAttr->attr & dmHdrAttrOpen );
      
   ctl = GetObjectPtr<ControlType>( DetailsRResDBCheckbox );		
   CtlSetValue( ctl, item->getFile()->iAttr->attr & dmHdrAttrResDB );

   originalOpen = item->getFile()->iAttr->attr & dmHdrAttrOpen;
   originalResDB = item->getFile()->iAttr->attr & dmHdrAttrResDB;
   
	// and lastly set the dates tab controls ------------------------------------
	
	//Err						err=0;
	Char                 buf[32];
	SystemPreferencesType	sysPrefs;
	DateFormatType			date;
	TimeFormatType			time;
	UInt32					createdSEC, modifiedSEC, backedUpSEC;

	PrefGetPreferences( &sysPrefs );
	date = sysPrefs.dateFormat;
	time = sysPrefs.timeFormat;

	CtlInit( DetailsRCreatedDateSelTrigger );				// initialize all the controls on the form
	CtlInit( DetailsRCreatedTimeSelTrigger );
	CtlInit( DetailsRModifiedDateSelTrigger );				// initialize all the controls on the form
	CtlInit( DetailsRModifiedTimeSelTrigger );
	CtlInit( DetailsRBackedUpDateSelTrigger );				// initialize all the controls on the form
	CtlInit( DetailsRBackedUpTimeSelTrigger );

	createdSEC = item->getFile()->iAttr->created;
	modifiedSEC = item->getFile()->iAttr->modified;
   backedUpSEC = item->getFile()->iAttr->backedUp;

	TimSecondsToDateTime( createdSEC, &createdDateTime );
	TimSecondsToDateTime( modifiedSEC, &modifiedDateTime );
	TimSecondsToDateTime( backedUpSEC, &backedUpDateTime );

	// set each one

	if( createdSEC )
		{
		DateToAscii( createdDateTime.month, createdDateTime.day, createdDateTime.year, date, buf );
		SetControlLabel( DetailsRCreatedDateSelTrigger, buf, 0 );
		TimeToAscii( createdDateTime.hour, createdDateTime.minute, time, buf );
		SetControlLabel( DetailsRCreatedTimeSelTrigger, buf, 0 );
		}
	else
		{
    StrCopy(buf, "Never");
		SetControlLabel( DetailsRCreatedDateSelTrigger, buf, 0 );
		SetControlLabel( DetailsRCreatedTimeSelTrigger, buf, 0 );		
		}

	if( modifiedSEC )
		{
		DateToAscii( modifiedDateTime.month, modifiedDateTime.day, modifiedDateTime.year, date, buf );
		SetControlLabel( DetailsRModifiedDateSelTrigger, buf, 0 );
		TimeToAscii( modifiedDateTime.hour, modifiedDateTime.minute, time, buf );
		SetControlLabel( DetailsRModifiedTimeSelTrigger, buf, 0 );
		}
	else
		{
    StrCopy(buf, "Never");
		SetControlLabel( DetailsRModifiedDateSelTrigger, buf, 0 );
		SetControlLabel( DetailsRModifiedTimeSelTrigger, buf, 0 );		
		}
 
	if( backedUpSEC )
		{
		DateToAscii( backedUpDateTime.month, backedUpDateTime.day, backedUpDateTime.year, date, buf );
		SetControlLabel( DetailsRBackedUpDateSelTrigger, buf, 0 );
		TimeToAscii( backedUpDateTime.hour, backedUpDateTime.minute, time, buf );
		SetControlLabel( DetailsRBackedUpTimeSelTrigger, buf, 0 );
		}
	else
		{
    StrCopy(buf, "Never");
		SetControlLabel( DetailsRBackedUpDateSelTrigger, buf, 0 );
		SetControlLabel( DetailsRBackedUpTimeSelTrigger, buf, 0 );		
		}
	}
	

/**
 * Sets up the file details form.
 */
static void SECT7 DetailsRInit()
	{
   ItemSet *itemSet = tree->getSelectedItems();
   item = itemSet->getItem( 0 );
   delete itemSet;

	DetailsHideGeneral();
	DetailsHideAttributes();
	DetailsHideDates();

	FrmDrawForm( FrmGetActiveForm() );
	StrCopy( formTitle, "" );
	FrmSetTitle( FrmGetActiveForm(), formTitle );	

	DetailsDrawBackground();

 	detailTabs = new GenericTabSet( 156, 120 );

   ResString genStr( GeneralString );
   ResString atrStr( AttributesString );
   ResString datStr( DatesString );

	detailTabs->AddTab( genStr.GetString(), genTab );
	detailTabs->AddTab( atrStr.GetString(), attrTab );
	detailTabs->AddTab( datStr.GetString(), datesTab );

	detailTabs->FinalizeTabs();

   DetailsSetValues();	
	DetailsShowCurrentTab();
   }

static void SECT7 DetailsFreeMemory()
	{
   if( detailTabs )
      {
      delete detailTabs;
      detailTabs = 0;
      }
	
	FldFreeMemory( GetObjectPtr<FieldType>( DetailsRNameField ));
	FldFreeMemory( GetObjectPtr<FieldType>( DetailsRTypeField ));
	FldFreeMemory( GetObjectPtr<FieldType>( DetailsRCreatorField ));
	FldFreeMemory( GetObjectPtr<FieldType>( DetailsRSizeField ));
	FldFreeMemory( GetObjectPtr<FieldType>( DetailsRVersionIDField ));
	FldFreeMemory( GetObjectPtr<FieldType>( DetailsRUniqueIDField ));
	
	CtlFreeMemory( DetailsRCreatedDateSelTrigger );
	CtlFreeMemory( DetailsRCreatedTimeSelTrigger );
	CtlFreeMemory( DetailsRModifiedDateSelTrigger );
	CtlFreeMemory( DetailsRModifiedTimeSelTrigger );
	CtlFreeMemory( DetailsRBackedUpDateSelTrigger );
	CtlFreeMemory( DetailsRBackedUpTimeSelTrigger );
	}
	

/**
 * Handle a menu selection.
 *
 * @param what menu command to do
 * @return whether it was handled or not
 */
static Boolean SECT7 DoCommand( UInt16 command )
	{
	Boolean	handled = false;
	return handled;
	}


/**
 * Given a set of coordinates, determine if they tapped on a tab.
 *
 * @param Some coordinates
 * @return True if handled
 */
static Boolean SECT7 DetailsHandleTap( Int16 x, Int16 y )
	{
	int	whichTab = detailTabs->CheckForTap( x, y );
	
	switch( whichTab )
		{
		case -1:
			return false;

		case genTab:
			if( curTab != genTab )
				{
				curTab = genTab;
				DetailsShowCurrentTab();
				}
			return true;
		
		case attrTab:
			if( curTab != attrTab )
				{
				curTab = attrTab;
				DetailsShowCurrentTab();
				}
			return true;		
					
		case datesTab:
			if( curTab != datesTab )
				{
				curTab = datesTab;
				DetailsShowCurrentTab();
				}
			return true;
		}

	return false;
	}


/**
 * Handle all the events for the file details form.
 *
 * @param event - the most recent event
 * @return True if the event is handled, false otherwise
 */
Boolean DetailsRHandleEvent( EventPtr event )
	{
	Boolean		handled = false /*, gone*/;
	ControlPtr	ctl;

   if ( ResizeHandleEvent( event ) )
      return true;

	switch( event->eType )
		{
		case ctlSelectEvent:
			if( event->data.ctlEnter.controlID == DetailsRResDBCheckbox )
				{
				ctl = GetObjectPtr<ControlType>( DetailsRResDBCheckbox );
				if( CtlGetValue( ctl ) != originalResDB)
					{
					if( FrmAlert( SeriouslyAlert ) == SeriouslyNo )
						CtlSetValue( ctl, originalResDB );
					}
				}
				
			if( event->data.ctlEnter.controlID == DetailsROpenCheckbox )
				{
				ctl = GetObjectPtr<ControlType>( DetailsROpenCheckbox );
				if( CtlGetValue( ctl ) != originalOpen)
					{
					if( FrmAlert( SeriouslyAlert ) == SeriouslyNo )
						CtlSetValue( ctl, originalOpen );
					}
				}
				
			switch( event->data.ctlEnter.controlID )
				{
				case DetailsRCloseButton:
               if( detailsPoppedUp )
                  {
                  DetailsFreeMemory();
                  FrmReturnToForm( 0 );
                  }
               else
                  {
                  FrmGotoForm( TreeViewForm );
                  FrmUpdateForm( TreeViewForm, frmRedrawTreeUpdateCode );
                  }
               handled = true;
					break;
					
				case DetailsRSaveButton:
					if( DetailsApply() )
						{
                  FrmGotoForm( TreeViewForm );
                  FrmUpdateForm( TreeViewForm, frmReSortTreeUpdateCode );
                  }
					handled = true;
					break;
				
				case DetailsRDeleteButton:
               {
               Err err = 0;
               
               if( item->remove( true, err ) )
                  {
                  FrmGotoForm( TreeViewForm );
                  FrmUpdateForm( TreeViewForm, frmReloadTreeUpdateCode );
                  }
               }
					handled = true;
					break;
				
				case DetailsRBeamButton:
               tree->sendSelectedItems();
					handled = true;
					break;
				
				// handle the date controls
				
				case DetailsRCreatedTimeSelTrigger:
					DatesSelectTime( DetailsRCreatedTimeSelTrigger, &createdDateTime );
					handled = true;
					break;
				case DetailsRModifiedTimeSelTrigger:
					DatesSelectTime( DetailsRModifiedTimeSelTrigger, &modifiedDateTime );
					handled = true;
					break;
				case DetailsRBackedUpTimeSelTrigger:
					DatesSelectTime( DetailsRBackedUpTimeSelTrigger, &backedUpDateTime );
					handled = true;
					break;
					
				case DetailsRCreatedDateSelTrigger:
					DatesSelectDate( DetailsRCreatedDateSelTrigger, &createdDateTime );
					handled = true;
					break;
					
				case DetailsRModifiedDateSelTrigger:
					DatesSelectDate( DetailsRModifiedDateSelTrigger, &modifiedDateTime );
					handled = true;
					break;
					
				case DetailsRBackedUpDateSelTrigger:
					DatesSelectDate( DetailsRBackedUpDateSelTrigger, &backedUpDateTime );
					handled = true;
					break;					
				}	
			break;

		case penDownEvent:											// if they did anything at all		
			handled = DetailsHandleTap( event->screenX, event->screenY );
			break;

		case menuEvent:
			return DoCommand( event->data.menu.itemID );

		case keyDownEvent:
			{
         UInt16	fldIndex = FrmGetFocus( FrmGetActiveForm());
			if( fldIndex == FrmGetObjectIndex( FrmGetActiveForm(), DetailsRNameField ))
				{
            // some devices have 'alt' keys that the user has to push first,
            //  followed by another key. so, here we have to make sure that
            //  we don't try to handle the 'alt' key being pushed, so we only
            //  update the title bar when we know it's a displayable character.
            if( event->data.keyDown.chr >= 32 && event->data.keyDown.chr <= 255 )
               {
               FieldPtr	fld= (FieldPtr)FrmGetObjectPtr( FrmGetActiveForm(), fldIndex );
               FldHandleEvent( fld, event );
               DetailsSetTitle( FldGetTextPtr( fld ));
               
               //Char msg[256];
               //StrPrintF( msg, "key: %d, mod: %d", event->data.keyDown.chr, event->data.keyDown.modifiers );
               //showMessage( msg );
               
               handled = true;
               }
            }
			}
         break;

		case frmOpenEvent:
			curTab = genTab;
			DetailsRInit();
			handled = true;
			break;

		case frmCloseEvent:
			DetailsFreeMemory();

         // we need to put this here if we only redraw (and reload) the tree after this.
         if( item )
            item->setSelected( false );

			break;
			
		case frmUpdateEvent:
			break;
		default:
			break;
		}
		
	return handled;
	}
