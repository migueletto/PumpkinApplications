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
 * Handles the filtering of the internal memory card's files.
 *
 * Created on 5/25/01 by Tom Bulatewicz
 */
 
#include <PalmOS.h>				// all the system toolbox headers
#include <FeatureMgr.h>				// needed to get the ROM version
#include <ExgMgr.h>
#include <DLServer.h>				// to get hotsync username
#include <VFSMgr.h>
#include "resize.h"

#include "Resource.h"					// application resource defines
#include "Stuph.h"
#include "UI.h"
#include "Main.h"
#include "FilterForm.h"
#include "TreeViewForm.h"


/**
 * Do some initial setup of the filter dialog.
 */
static void SECT7 FilterPreInit()
	{
	FormPtr			frm;
	
	frm = FrmGetActiveForm();
	
	CtlInit( FilterHowPopTrigger );
	CtlInit( FilterCompTypePopTrigger );
	
	FrmHideObject( FrmGetActiveForm(), FrmGetObjectIndex( FrmGetActiveForm(), FilterStringField ) );

	HideObject( frm, FilterBackupCheckbox );
	HideObject( frm, FilterReadOnlyCheckbox );
	HideObject( frm, FilterCopyProtectCheckbox );
	HideObject( frm, FilterResourceDBCheckbox );
	HideObject( frm, FilterCompTypePopTrigger );
	}

	
/**
 * Initialize the filter selection dialog.
 */
static void SECT7 FilterInit()
	{
	Char        *label, *p;
	ListPtr		lst;
	MemHandle	h;
	//UInt32		size, roundDigit;
	//float       fsize, fsize2;
	FormPtr		frm;
	
	frm = FrmGetActiveForm();

   CtlSetValue( GetObjectPtr<ControlType>( FilterNotCheckbox ), prefs.filter.notbox );

	// set the how list (filename, creator, type, size ) - if something selected, use it, else not
/*
	if( selectedDB != -1 )
		{
		if( selectedCol == 0 )
			{
			prefs.filter.criteria = filterName;
			//StrCopy( prefs.filter.string, cache.getName( selectedDB ));
			}
		else
			{
			switch( prefs.list.column )
				{
				case colCreator:
					prefs.filter.criteria = filterCreator;
					//IntToStr( cache.getCreator( selectedDB ), prefs.filter.string );
					break;
				case colType:
					prefs.filter.criteria = filterType;
					//IntToStr( cache.getType( selectedDB ), prefs.filter.string );
					break;
				case colSize:
					prefs.filter.criteria = filterSize;
					//fsize = size = cache.getSize( selectedDB ) / 1024;
					fsize2 = fsize - (UInt32)fsize;
					roundDigit = UInt32(fsize2 * 10);
					if( roundDigit >=5 )
						size++;
					if( !size ) size = 1;
					StrIToA( prefs.filter.string, size );
					break;
				case colRec:
					prefs.filter.criteria = filterRec;
					//size = cache.getRecCount( selectedDB );
					StrIToA( prefs.filter.string, size );
					break;
				case colAttr:
					prefs.filter.criteria = filterAttr;
					if( cache.getAttr( selectedDB ) & dmHdrAttrReadOnly )		filterAttrRO = true;
					if( cache.getAttr( selectedDB ) & dmHdrAttrBackup )			filterAttrBU = true;
					if( cache.getAttr( selectedDB ) & dmHdrAttrCopyPrevention )	filterAttrCP = true;
					if( cache.getAttr( selectedDB ) & dmHdrAttrResDB )			filterAttrRD = true;					
					break;
				}
			}

		//cache.ResetSelected();
		}
*/	
	lst = GetObjectPtr<ListType>( FilterHowList );
	label = LstGetSelectionText( lst, prefs.filter.criteria );
	SetControlLabel( FilterHowPopTrigger, label, 0 );
	LstSetSelection( lst, prefs.filter.criteria );

	if( prefs.filter.criteria == filterAttr )
		{
		ControlPtr		ctl;
				
		HideObject( frm, FilterStringField );
		
		ctl = GetObjectPtr<ControlType>( FilterBackupCheckbox );
		//if( filterAttrBU )	CtlSetValue( ctl, true );
		CtlShowControl( ctl );
		
		ctl = GetObjectPtr<ControlType>( FilterReadOnlyCheckbox );
		//if( filterAttrRO )	CtlSetValue( ctl, true );
		CtlShowControl( ctl );
		
		ctl = GetObjectPtr<ControlType>( FilterCopyProtectCheckbox );
		//if( filterAttrCP )	CtlSetValue( ctl, true );
		CtlShowControl( ctl );

		ctl = GetObjectPtr<ControlType>( FilterResourceDBCheckbox );
		//if( filterAttrRD )	CtlSetValue( ctl, true );
		CtlShowControl( ctl );
				
		HideObject( frm, FilterCompTypePopTrigger );
		}
	else
		{
		// set the comparison type list (begins with, contains, ends with or <, =, > )

		if( prefs.filter.criteria == filterName || prefs.filter.criteria == filterCreator || prefs.filter.criteria == filterType )
			lst = GetObjectPtr<ListType>( FilterCompTypeSList );
		else
			lst = GetObjectPtr<ListType>( FilterCompTypeNList );
	
		label = LstGetSelectionText( lst, prefs.filter.comparator );
		SetControlLabel( FilterCompTypePopTrigger, label, 0 );
		LstSetSelection( lst, prefs.filter.comparator );
		
		// set the search string - if something selected use that, if not, use the last thing they used
		
		h = MemHandleNew( 33 );
		p = (Char*)MemHandleLock( h );
		StrCopyTry( p, prefs.filter.string, maxFilterString );
		MemHandleUnlock( h );
		SetFieldTextFromHandle( FilterStringField, h, false );
		
		HideObject( frm, FilterBackupCheckbox );
		HideObject( frm, FilterReadOnlyCheckbox );
		HideObject( frm, FilterCopyProtectCheckbox );
		HideObject( frm, FilterResourceDBCheckbox );
		
		ShowObject( frm, FilterCompTypePopTrigger );
		ShowObject( frm, FilterStringField );
		}		
	}


/**
 * Set the filter parameters selected by the user.
 */
static void SECT7 FilterSet()
	{
	FieldPtr    fld;
	Char        *str;
	ControlPtr  ctl;
	
   prefs.filter.notbox = CtlGetValue( GetObjectPtr<ControlType>( FilterNotCheckbox ));
   
	if( prefs.filter.criteria == filterAttr )
		{
      prefs.filter.attr = 0;
      
		ctl = GetObjectPtr<ControlType>( FilterReadOnlyCheckbox );
      Boolean ro = CtlGetValue( ctl );		
      ctl = GetObjectPtr<ControlType>( FilterBackupCheckbox );
      Boolean bu = CtlGetValue( ctl );
		ctl = GetObjectPtr<ControlType>( FilterCopyProtectCheckbox );
      Boolean cp = CtlGetValue( ctl );
		ctl = GetObjectPtr<ControlType>( FilterResourceDBCheckbox );
      Boolean rd = CtlGetValue( ctl );
		
      if( ro ) prefs.filter.attr |= dmHdrAttrReadOnly;
      if( bu ) prefs.filter.attr |= dmHdrAttrBackup;
      if( cp ) prefs.filter.attr |= dmHdrAttrCopyPrevention;
      if( rd ) prefs.filter.attr |= dmHdrAttrResDB;
      }
	else
		{
		fld = GetObjectPtr<FieldType>( FilterStringField );
		if( FldDirty( fld ))
			{
			str = FldGetTextPtr( fld );
			StrCopyTry( prefs.filter.string, str, maxFilterString );
			}
	
		prefs.filter.criteria = LstGetSelection( GetObjectPtr<ListType>( FilterHowList ));
	
		if( prefs.filter.criteria == filterName || prefs.filter.criteria == filterCreator || prefs.filter.criteria == filterType )
			prefs.filter.comparator = LstGetSelection( GetObjectPtr<ListType>( FilterCompTypeSList ));
		else
			prefs.filter.comparator = LstGetSelection( GetObjectPtr<ListType>( FilterCompTypeNList ));
		}
	}


/**
 * Handle what happens when the user taps the filter method.
 */
static void SECT7 FilterSelectHow()
	{
	ListPtr	lst;
	Int16		selected;
	
	lst = GetObjectPtr<ListType>( FilterHowList );
	
	selected = LstPopupList( lst );	
	if( selected == -1 ) return;
	
	prefs.filter.criteria = selected;
	FilterInit();
	}


/**
 * Handle when the user taps the filter comparison type.
 */
static void SECT7 FilterSelectCompType()
	{
	ListPtr	lst;
	Int16		selected;
	Char		*label;
	
	if( prefs.filter.criteria == filterName || prefs.filter.criteria == filterCreator || prefs.filter.criteria == filterType )
		lst = GetObjectPtr<ListType>( FilterCompTypeSList );
	else
		lst = GetObjectPtr<ListType>( FilterCompTypeNList );
	
	selected = LstPopupList( lst );
	
	if( selected == -1 ) return;
	
	label = LstGetSelectionText( lst, selected );
	SetControlLabel( FilterCompTypePopTrigger, label, 0 );
	prefs.filter.comparator = selected;
	}

static void SECT7 FilterFreeMemory()
	{
	CtlFreeMemory( FilterHowPopTrigger );
	CtlFreeMemory( FilterCompTypePopTrigger );
	FldFreeMemory( GetObjectPtr<FieldType>( FilterStringField ) );
	}


/**
 * Handle the events in the filter dialog.
 *
 * @param an event
 * @return if it was handled
 */
Boolean FilterHandleEvent (EventPtr event)
	{
	Boolean handled = false;
	FormPtr frm;

   if ( ResizeHandleEvent( event ) )
      return true;

	if (event->eType == ctlSelectEvent)
		{
		switch (event->data.ctlSelect.controlID)
			{
			case FilterOKButton:
				FilterSet();
				FilterFreeMemory();

            FrmReturnToForm( TreeViewForm );
            FrmUpdateForm( TreeViewForm, frmFilterTreeUpdateCode );
            handled = true;
				break;

			case FilterCancelButton:
				FilterFreeMemory();
            FrmReturnToForm( TreeViewForm );
            handled = true;
				break;
			
			case FilterHowPopTrigger:
				FilterSelectHow();
				handled = true;
				break;

			case FilterCompTypePopTrigger:
				FilterSelectCompType();
				handled = true;
				break;
			}
		}

	else if (event->eType == frmOpenEvent)
		{
		frm = FrmGetActiveForm ();
		FilterPreInit();
		FrmDrawForm( frm );
		FilterInit();
		handled = true;
		}

	else if( event->eType == frmCloseEvent )
		{
		}

	return handled;
	}
