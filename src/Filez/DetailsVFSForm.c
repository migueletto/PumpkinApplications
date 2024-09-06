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
#include "Main.h"
#include "DetailsForm.h"
#include "Menu.h"
#include "Tabs.h"

#include "TreeViewForm.h"
#include "Item.hpp"
#include "ItemFile.hpp"
#include "ItemFolder.hpp"
#include "ItemSet.hpp"

#include "Stuph.h"
#include "UI.h"

static DateTimeType		createdDateTime;
static DateTimeType		modifiedDateTime;
static DateTimeType		accessedDateTime;

static Char             formTitle[25];
static GenericTabSet    *detailTabs;
static UInt8            curTab;

#define		genTab		0
#define		attrTab		1
#define 		datesTab		2

static Item  *item = 0;          // the currently selected item

/**
 * Save any changes made to the file details.
 *
 * @return True if ok, false if not
 */
static Boolean SECT7 DetailsVApply()
	{
	//Err         err=0;
	ControlPtr  ctl;
	FieldPtr    fld;
	Char        *oldName;

	// first save the general tab controls

   oldName = (Char*)MemPtrNew( StrLen( item->getName() ) + 1 );
   StrCopy( oldName, item->getName() );

	fld = GetObjectPtr<FieldType>( DetailsVNameField );			// get the name field

   if( checkForInvalidVFSCharacters( FldGetTextPtr( fld ), InvalidCharacterAlert, 0 ))
      return false;
   item->setName( FldGetTextPtr( fld ));

   // then save the attribute tab controls

   //UInt32 oldAttrs = item->getFile()->eAttr->attr;
   item->getFile()->eAttr->attr = 0;

   ctl = GetObjectPtr<ControlType>( DetailsVROCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->eAttr->attr |= vfsFileAttrReadOnly;
	else                          item->getFile()->eAttr->attr &= ~vfsFileAttrReadOnly;

   ctl = GetObjectPtr<ControlType>( DetailsVHiddenCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->eAttr->attr |= vfsFileAttrHidden;
	else                          item->getFile()->eAttr->attr &= ~vfsFileAttrHidden;

   ctl = GetObjectPtr<ControlType>( DetailsVSystemCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->eAttr->attr |= vfsFileAttrSystem;
	else                          item->getFile()->eAttr->attr &= ~vfsFileAttrSystem;

   ctl = GetObjectPtr<ControlType>( DetailsVArchivedCheckbox );
   if( CtlGetValue( ctl ) )      item->getFile()->eAttr->attr |= vfsFileAttrArchive;
	else                          item->getFile()->eAttr->attr &= ~vfsFileAttrArchive;

	// then save the date tab controls

   item->getFile()->eAttr->created = TimDateTimeToSeconds( &createdDateTime );
   item->getFile()->eAttr->modified = TimDateTimeToSeconds( &modifiedDateTime );
   item->getFile()->eAttr->accessed = TimDateTimeToSeconds( &accessedDateTime );

   item->updateFile( oldName );
   /*err =*/ MemPtrFree( oldName );
   oldName = 0;
	return true;
	}


/**
 * Convert a 4 byte int into a datetime struct.
 *
 * @param the date and the original integer
 */
/*
static void SECT7 DatesConvertDateTimeV( DateTimeType *fields, UInt32 base )
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
static void SECT7 DatesUpdateTimeV( Int16 id, DateTimeType *t )
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
static void SECT7 DatesSelectTimeV( Int16 id, DateTimeType *t )
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
static void SECT7 DatesSelectDateV( Int16 id, DateTimeType *t )
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
	
	lst = GetObjectPtr<ListType>( DetailsVDateChoicesList );
	switch( id )
		{
		case DetailsVCreatedDateSelTrigger:
			LstSetPosition( lst, 63, 47 );
			break;
		case DetailsVModifiedDateSelTrigger:
			LstSetPosition( lst, 63, 68 );
			break;
		case DetailsVAccessedDateSelTrigger:
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
		case DetailsVCreatedDateSelTrigger:
			DatesUpdateTimeV( DetailsVCreatedTimeSelTrigger, t );
			break;
		case DetailsVModifiedDateSelTrigger:
			DatesUpdateTimeV( DetailsVModifiedTimeSelTrigger, t );
			break;
		case DetailsVAccessedDateSelTrigger:
			DatesUpdateTimeV( DetailsVAccessedTimeSelTrigger, t );
			break;		
		}
	}


static void SECT7 DetailsVShowGeneral()
	{
	FormPtr		frm = FrmGetActiveForm();
	
	ShowObject( frm, DetailsVNameLabel );
	ShowObject( frm, DetailsVNameField );
	ShowObject( frm, DetailsVSizeLabel );
	ShowObject( frm, DetailsVSizeField );

   if( item->getFile()->iAttr )
      ShowObject( frm, DetailsVViewButton );
      
	if( detailTabs )
		detailTabs->DrawTabs( genTab );
	}

static void SECT7 DetailsVHideGeneral()
	{
	FormPtr		frm = FrmGetActiveForm();
	
	HideObject( frm, DetailsVNameLabel );
	HideObject( frm, DetailsVNameField );
	HideObject( frm, DetailsVSizeLabel );
	HideObject( frm, DetailsVSizeField );
	HideObject( frm, DetailsVViewButton );
	}

static void SECT7 DetailsVShowAttributes()
	{
	FormPtr		frm = FrmGetActiveForm();

   ShowObject( frm, DetailsVROCheckbox );
   ShowObject( frm, DetailsVHiddenCheckbox );
   ShowObject( frm, DetailsVSystemCheckbox );
   ShowObject( frm, DetailsVArchivedCheckbox );
   ShowObject( frm, DetailsVLinkCheckbox );

	if( detailTabs )
		detailTabs->DrawTabs( attrTab );
	}


//make another tab, called VFS that has the vfs file dates and attributes!!


	
static void SECT7 DetailsVHideAttributes()
	{
	FormPtr		frm = FrmGetActiveForm();
	
	HideObject( frm, DetailsVROCheckbox );
	HideObject( frm, DetailsVHiddenCheckbox );
	HideObject( frm, DetailsVSystemCheckbox );
	HideObject( frm, DetailsVArchivedCheckbox );
	HideObject( frm, DetailsVLinkCheckbox );
	}

	
static void SECT7 DetailsVShowDates()
	{
	FormPtr		frm = FrmGetActiveForm();

	ShowObject( frm, DetailsVCreatedLabel );
	ShowObject( frm, DetailsVCreatedDateSelTrigger );
	ShowObject( frm, DetailsVCreatedTimeSelTrigger );

	ShowObject( frm, DetailsVModifiedLabel );
	ShowObject( frm, DetailsVModifiedDateSelTrigger );
	ShowObject( frm, DetailsVModifiedTimeSelTrigger );

	ShowObject( frm, DetailsVAccessedLabel );
	ShowObject( frm, DetailsVAccessedDateSelTrigger );
	ShowObject( frm, DetailsVAccessedTimeSelTrigger );

	if( detailTabs )
		detailTabs->DrawTabs( datesTab );
	}
	
static void SECT7 DetailsVHideDates()
	{
	FormPtr		frm = FrmGetActiveForm();
	
	HideObject( frm, DetailsVCreatedLabel );
	HideObject( frm, DetailsVModifiedLabel );
	HideObject( frm, DetailsVAccessedLabel );

	HideObject( frm, DetailsVCreatedDateSelTrigger );
	HideObject( frm, DetailsVModifiedDateSelTrigger );
	HideObject( frm, DetailsVAccessedDateSelTrigger );

	HideObject( frm, DetailsVCreatedTimeSelTrigger );
	HideObject( frm, DetailsVModifiedTimeSelTrigger );
	HideObject( frm, DetailsVAccessedTimeSelTrigger );
	}

static Boolean SECT7 DetailsVShowCurrentTab()
	{
	switch( curTab )
		{
		case genTab:
			DetailsVHideAttributes();
			DetailsVHideDates();
			DetailsVShowGeneral();
			return true;
		
		case attrTab:
			DetailsVHideGeneral();
			DetailsVHideDates();
			DetailsVShowAttributes();
			return true;		
					
		case datesTab:
			DetailsVHideAttributes();
			DetailsVHideGeneral();
			DetailsVShowDates();
			return true;
		}
		
	return false;
	}

static void SECT7 DetailsVDrawBackground()
	{
	ColorSet( &bgColor, 0, 0, 0, 0 );
	RectangleType	r;
	r.topLeft.x = 0;		r.topLeft.y = 135;		r.extent.x = 159;	r.extent.y = 21;	
	WinDrawRectangle( &r, 0 );

	r.topLeft.x = 0;		r.topLeft.y = 12;		r.extent.x = 159;	r.extent.y = 5;	
	WinDrawRectangle( &r, 0 );

	ColorUnset();

	FormPtr	frm = FrmGetActiveForm();

	// this is to get around a bug in old PalmOS versions (like 3.0) where the
	//	whole control is not drawn by FrmShowObject like it should be.
	FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, DetailsVSaveButton ), &r );
	WinEraseRectangle( &r, 1 );
	FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, DetailsVCloseButton ), &r );
	WinEraseRectangle( &r, 1 );
	FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, DetailsVDeleteButton ), &r );
	WinEraseRectangle( &r, 1 );
	FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, DetailsVBeamButton ), &r );
	WinEraseRectangle( &r, 1 );

	ShowObject( frm, DetailsVSaveButton );
	ShowObject( frm, DetailsVCloseButton );
	ShowObject( frm, DetailsVDeleteButton );
	ShowObject( frm, DetailsVBeamButton );
	}

static void SECT7 DetailsVSetTitle( Char *str )
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


static void SECT7 DetailsVSetValues()
	{
	Char        *p;
	MemHandle	h;
	//FormPtr		frm;
	ControlPtr	ctl;
   
	//frm = FrmGetActiveForm();

	// first set the general tab controls
	
	h = MemHandleNew( StrLen( item->getName()) + 1 );	// set the name
	p = (Char*)MemHandleLock( h );
	StrCopy( p, item->getName() );
      
	DetailsVSetTitle( p );
	MemHandleUnlock( h );
	SetFieldTextFromHandle( DetailsVNameField, h, false );

   UInt32 size = item->getFile()->size;
      
   h = MemHandleNew( 64 );										// set the size/record count
   p = (Char*)MemHandleLock( h );
   StrPrintF( p, "%ld b", size );
   MemHandleUnlock( h );
   SetFieldTextFromHandle( DetailsVSizeField, h, false );


	// now set the attributes tab controls

   ctl = GetObjectPtr<ControlType>( DetailsVROCheckbox );			
   CtlSetValue( ctl, item->getFile()->eAttr->attr & vfsFileAttrReadOnly );
      
   ctl = GetObjectPtr<ControlType>( DetailsVHiddenCheckbox );		
   CtlSetValue( ctl, item->getFile()->eAttr->attr & vfsFileAttrHidden );

   ctl = GetObjectPtr<ControlType>( DetailsVSystemCheckbox );		
   CtlSetValue( ctl, item->getFile()->eAttr->attr & vfsFileAttrSystem );

   ctl = GetObjectPtr<ControlType>( DetailsVArchivedCheckbox );		
   CtlSetValue( ctl, item->getFile()->eAttr->attr & vfsFileAttrArchive );

   ctl = GetObjectPtr<ControlType>( DetailsVLinkCheckbox );		
   CtlSetValue( ctl, item->getFile()->eAttr->attr & vfsFileAttrLink );

   
	// and lastly set the dates tab controls ------------------------------------
	
	//Err                     err=0;
	Char                    buf[32];
	SystemPreferencesType	sysPrefs;
	DateFormatType          date;
	TimeFormatType          time;
	UInt32                  createdSEC, modifiedSEC, accessedSEC;

	PrefGetPreferences( &sysPrefs );
	date = sysPrefs.dateFormat;
	time = sysPrefs.timeFormat;

	CtlInit( DetailsVCreatedDateSelTrigger );				// initialize all the controls on the form
	CtlInit( DetailsVCreatedTimeSelTrigger );
	CtlInit( DetailsVModifiedDateSelTrigger );				// initialize all the controls on the form
	CtlInit( DetailsVModifiedTimeSelTrigger );
	CtlInit( DetailsVAccessedDateSelTrigger );				// initialize all the controls on the form
	CtlInit( DetailsVAccessedTimeSelTrigger );

	createdSEC = item->getFile()->eAttr->created;
	modifiedSEC = item->getFile()->eAttr->modified;
	accessedSEC = item->getFile()->eAttr->accessed;

	TimSecondsToDateTime( createdSEC, &createdDateTime );
	TimSecondsToDateTime( modifiedSEC, &modifiedDateTime );
	TimSecondsToDateTime( accessedSEC, &accessedDateTime );

	// set each one

	if( createdSEC )
		{
		DateToAscii( createdDateTime.month, createdDateTime.day, createdDateTime.year, date, buf );
		SetControlLabel( DetailsVCreatedDateSelTrigger, buf, 0 );
		TimeToAscii( createdDateTime.hour, createdDateTime.minute, time, buf );
		SetControlLabel( DetailsVCreatedTimeSelTrigger, buf, 0 );
		}
	else
		{
    StrCopy(buf, "Never");
		SetControlLabel( DetailsVCreatedDateSelTrigger, buf, 0 );
		SetControlLabel( DetailsVCreatedTimeSelTrigger, buf, 0 );		
		}

	if( modifiedSEC )
		{
		DateToAscii( modifiedDateTime.month, modifiedDateTime.day, modifiedDateTime.year, date, buf );
		SetControlLabel( DetailsVModifiedDateSelTrigger, buf, 0 );
		TimeToAscii( modifiedDateTime.hour, modifiedDateTime.minute, time, buf );
		SetControlLabel( DetailsVModifiedTimeSelTrigger, buf, 0 );
		}
	else
		{
    StrCopy(buf, "Never");
		SetControlLabel( DetailsVModifiedDateSelTrigger, buf, 0 );
		SetControlLabel( DetailsVModifiedTimeSelTrigger, buf, 0 );		
		}
 
	if( accessedSEC )
		{
		DateToAscii( accessedDateTime.month, accessedDateTime.day, accessedDateTime.year, date, buf );
		SetControlLabel( DetailsVAccessedDateSelTrigger, buf, 0 );
		TimeToAscii( accessedDateTime.hour, accessedDateTime.minute, time, buf );
		SetControlLabel( DetailsVAccessedTimeSelTrigger, buf, 0 );
		}
	else
		{
    StrCopy(buf, "Never");
		SetControlLabel( DetailsVAccessedDateSelTrigger, buf, 0 );
		SetControlLabel( DetailsVAccessedTimeSelTrigger, buf, 0 );		
		}
	}
	

/**
 * Sets up the file details form.
 */
static void SECT7 handleFormOpenEvent()
	{
   ItemSet *itemSet = tree->getSelectedItems();
   item = itemSet->getItem( 0 );
   delete itemSet;

	DetailsVHideGeneral();
	DetailsVHideAttributes();
	DetailsVHideDates();

	FrmDrawForm( FrmGetActiveForm() );
	StrCopy( formTitle, "" );
	FrmSetTitle( FrmGetActiveForm(), formTitle );	

	DetailsVDrawBackground();

 	detailTabs = new GenericTabSet( 156, 120 );

   ResString genStr( GeneralString );
   ResString atrStr( AttributesString );
   ResString datStr( DatesString );

	detailTabs->AddTab( genStr.GetString(), genTab );
	detailTabs->AddTab( atrStr.GetString(), attrTab );
	detailTabs->AddTab( datStr.GetString(), datesTab );

	detailTabs->FinalizeTabs();

   DetailsVSetValues();	
	DetailsVShowCurrentTab();
   }


static void SECT7 DetailsVFreeMemory()
	{
   if( detailTabs )
      {
      delete detailTabs;
      detailTabs = 0;
      }
	
	FldFreeMemory( GetObjectPtr<FieldType>( DetailsVNameField ));
	FldFreeMemory( GetObjectPtr<FieldType>( DetailsVSizeField ));
	
	CtlFreeMemory( DetailsVCreatedDateSelTrigger );
	CtlFreeMemory( DetailsVCreatedTimeSelTrigger );
	CtlFreeMemory( DetailsVModifiedDateSelTrigger );
	CtlFreeMemory( DetailsVModifiedTimeSelTrigger );
	CtlFreeMemory( DetailsVAccessedDateSelTrigger );
	CtlFreeMemory( DetailsVAccessedTimeSelTrigger );
	}
	

/**
 * Handle a menu selection.
 *
 * @param what menu command to do
 * @return whether it was handled or not
 */
static Boolean SECT7 DoCommandV( UInt16 command )
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
static Boolean SECT7 DetailsVHandleTap( Int16 x, Int16 y )
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
				DetailsVShowCurrentTab();
				}
			return true;
		
		case attrTab:
			if( curTab != attrTab )
				{
				curTab = attrTab;
				DetailsVShowCurrentTab();
				}
			return true;		
					
		case datesTab:
			if( curTab != datesTab )
				{
				curTab = datesTab;
				DetailsVShowCurrentTab();
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
Boolean DetailsVHandleEvent( EventPtr event )
	{
	Boolean		handled = false /*, gone*/;
	//ControlPtr	ctl;

   if ( ResizeHandleEvent( event ) )
      return true;

	switch( event->eType )
		{
		case ctlSelectEvent:
				
			switch( event->data.ctlEnter.controlID )
				{
				case DetailsVViewButton:
               // this one is ok to popup
               detailsPoppedUp = true;
               FrmPopupForm( DetailsRForm );
					handled = true;
					break;
					
				case DetailsVCloseButton:
               FrmGotoForm( TreeViewForm );
               FrmUpdateForm( TreeViewForm, frmRedrawTreeUpdateCode );
					handled = true;
					break;
					
				case DetailsVSaveButton:
					if( DetailsVApply() )
						{
                  FrmGotoForm( TreeViewForm );
                  FrmUpdateForm( TreeViewForm, frmReSortTreeUpdateCode );
                  }
					handled = true;
					break;

				case DetailsVDeleteButton:
               {
               Err err = 0;
               if( item->remove( true, err ))
                  {
                  FrmGotoForm( TreeViewForm );
                  FrmUpdateForm( TreeViewForm, frmReloadTreeUpdateCode );
                  }
               }
					handled = true;
					break;
				
				case DetailsVBeamButton:
					handled = true;
					break;
				
				// handle the date controls
				case DetailsVCreatedTimeSelTrigger:
					DatesSelectTimeV( DetailsVCreatedTimeSelTrigger, &createdDateTime );
					handled = true;
					break;
				case DetailsVModifiedTimeSelTrigger:
					DatesSelectTimeV( DetailsVModifiedTimeSelTrigger, &modifiedDateTime );
					handled = true;
					break;
				case DetailsVAccessedTimeSelTrigger:
					DatesSelectTimeV( DetailsVAccessedTimeSelTrigger, &accessedDateTime );
					handled = true;
					break;
					
				case DetailsVCreatedDateSelTrigger:
					DatesSelectDateV( DetailsVCreatedDateSelTrigger, &createdDateTime );
					handled = true;
					break;
					
				case DetailsVModifiedDateSelTrigger:
					DatesSelectDateV( DetailsVModifiedDateSelTrigger, &modifiedDateTime );
					handled = true;
					break;
					
				case DetailsVAccessedDateSelTrigger:
					DatesSelectDateV( DetailsVAccessedDateSelTrigger, &accessedDateTime );
					handled = true;
					break;
            }	
			break;

		case penDownEvent:											// if they did anything at all		
			handled = DetailsVHandleTap( event->screenX, event->screenY );
			break;

		case menuEvent:
			return DoCommandV( event->data.menu.itemID );

		case keyDownEvent:
			{
         UInt16	fldIndex = FrmGetFocus( FrmGetActiveForm());
			if( fldIndex == FrmGetObjectIndex( FrmGetActiveForm(), DetailsVNameField ))
				{
            // some devices have 'alt' keys that the user has to push first,
            //  followed by another key. so, here we have to make sure that
            //  we don't try to handle the 'alt' key being pushed, so we only
            //  update the title bar when we know it's a displayable character.
            if( event->data.keyDown.chr >= 32 && event->data.keyDown.chr <= 255 )
               {
               FieldPtr	fld= (FieldPtr)FrmGetObjectPtr( FrmGetActiveForm(), fldIndex );
               FldHandleEvent( fld, event );
               DetailsVSetTitle( FldGetTextPtr( fld ));
               
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
			handleFormOpenEvent();
			handled = true;
			break;

		case frmCloseEvent:
			DetailsVFreeMemory();

         // we need to put this here if we only redraw (and reload) the tree after this.
         if( item )
            item->setSelected( false );

         break;
    default:
         break;
		}
		
	return handled;
	}
