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
 * Handles the record list view.
 *
 * Created on 12/21/01 by Tom Bulatewicz
 */
  
#include <PalmOS.h>							// all the system toolbox headers

#include "RecListForm.h"
#include "Stuph.h"
#include "UI.h"
#include "Resource.h"
#include "Main.h"
#include "TreeViewForm.h"
#include "resize.h"

#include "Item.hpp"
#include "ItemFile.hpp"
#include "ItemFolder.hpp"
#include "ItemSet.hpp"

#define leftMargin	118

Int32					currentRecord;
extern Boolean 	readOnly;				// defined in hex.c, true if user can change the file
Boolean				isRecs;					// true = record db, false = resource db

Item *editItem;

/**
 * Draw the record's rec number as it's desscription in the list.
 *
 * @param the item
 * @param it's bounds
 * @param it's text (which is null)
 */
static void SECT8 RecordListDrawRecord( Int16 itemNum, RectangleType *bounds, Char **itemsText )
	{
	Char			text[32], idstr[6];
	//Err			err;
	DmResType	resType;
	DmResID		resID;
	DmOpenRef	ref;
	
	if( isRecs )
		{
		StrIToA( text, itemNum );
		WinDrawChars( text, StrLen( text ), bounds->topLeft.x, bounds->topLeft.y );
		}
	else
		{
		ref = DmOpenDatabase( editItem->getVolumeNum(), editItem->getFile()->iAttr->id, dmModeReadOnly );
		/*err =*/ DmResourceInfo( ref, itemNum, &resType, &resID, NULL );
		DmCloseDatabase( ref );
		IntToStr( resType, text );
		StrIToA( idstr, resID );
		StrCat( text, " #" );
		StrCat( text, idstr );
		WinDrawChars( text, StrLen( text ), bounds->topLeft.x, bounds->topLeft.y );
		}
	}


/**
 * Determine if the record has been deleted or not (via a call 
 * to DmDeleteRecord())..
 *
 * @param The record index to check
 * @return True if it was deleted, false if not
 */
static Boolean SECT8 RecordListIsRecDeleted( UInt16 index )
	{
	Err			err;
	DmOpenRef	ref;
	UInt16		attr;
	UInt32		uniqueID;
	MemHandle	recH;
	DmResType	resType;
	DmResID		resID;
		
	ref = DmOpenDatabase( editItem->getVolumeNum(), editItem->getFile()->iAttr->id, dmModeReadOnly );
	if( !ref )
		{
		FrmCustomAlert( GeneralErrorAlert, "Unable to open database.", NULL, NULL );
		return true;	// well, it's not exactly deleted, but we still can't get to it
		}
	
	if( isRecs )
		err = DmRecordInfo( ref, index, &attr, &uniqueID, NULL );
	else
		err = DmResourceInfo( ref, index, &resType, &resID, NULL );
		
	if( err != errNone )
		{
		FrmCustomAlert( GeneralErrorAlert, "Unable to get record/resource info.", NULL, NULL );
		DmCloseDatabase( ref );
		return true;	// same as previous comment
		}
		
	if( isRecs )
		recH = DmQueryRecord( ref, index );
	else
		recH = DmGetResource( resType, resID );
	
	// an null handle means that the record was deleted, but the reference to it is still there 	
	if( !recH )
		{
		DmCloseDatabase( ref );
		return true;
		}

	DmCloseDatabase( ref );
	return false;	
	}


/**
 * Draw the record's statistics on the right of the screen.
 *
 * @param which record's stats to draw
 */
static void SECT8 RecordListShowStats( Int16 index )
	{
	Char			text[32];
	Err			err;
	DmOpenRef	ref;
	UInt16		attr;
	UInt32		uniqueID;
	MemHandle	recH;
	UInt32		size;
	RectangleType	r;
	DmResType	resType;
	DmResID		resID;

	Boolean updateCtrls = false;
	if (currentRecord != index) {
	  updateCtrls = true;
	}
	
	currentRecord = index;
	
	ref = DmOpenDatabase( editItem->getVolumeNum(), editItem->getFile()->iAttr->id, dmModeReadOnly );
	if( !ref )
		{
		FrmCustomAlert( GeneralErrorAlert, "Unable to open database.", NULL, NULL );
		return;
		}
	
	if( isRecs )
		err = DmRecordInfo( ref, index, &attr, &uniqueID, NULL );
	else
		err = DmResourceInfo( ref, index, &resType, &resID, NULL );
		
	if( err != errNone )
		{
		FrmCustomAlert( GeneralErrorAlert, "Unable to get record/resource info.", NULL, NULL );
		DmCloseDatabase( ref );
		return;
		}
	
	r.topLeft.x = leftMargin;
	r.topLeft.y = 18;
	r.extent.x = 160 - leftMargin;
	r.extent.y = 78;
	
	WinEraseRectangle( &r, 0 );
	
	if( isRecs )
		recH = DmQueryRecord( ref, index );
	else
		recH = DmGetResource( resType, resID );
		
	if( !recH )
		{
		StrCopy( text, "(deleted)" );
		WinDrawChars( text, StrLen( text ), leftMargin, 19 );
		DmCloseDatabase( ref );
		return;
		}
	
	size = MemHandleSize( recH );
	
	DmCloseDatabase( ref );
		
	StrIToA( text, size );
	WinDrawChars( text, StrLen( text ), leftMargin, 19 );
	
	if( !isRecs )
		return;
	
	StrIToA( text, uniqueID );
	WinDrawChars( text, StrLen( text ), leftMargin, 31 );
	
	StrIToA( text, attr & dmRecAttrCategoryMask );
	WinDrawChars( text, StrLen( text ), leftMargin, 44 );
	
	if( attr & dmRecAttrBusy )
		StrCopy( text, "Yes" );
	else
		StrCopy( text, "No" );
	WinDrawChars( text, StrLen( text ), leftMargin, 57 );
	
//	if( attr & dmRecAttrDirty )
	ControlPtr ctl = GetObjectPtr<ControlType>( RecordListDirtyCheckbox );	
	if( attr & dmRecAttrDirty ) {
		StrCopy( text, "Yes" );
//	else
		if (updateCtrls) CtlSetValue( ctl, true );
	} else {
		StrCopy( text, "No" );
		if (updateCtrls) CtlSetValue( ctl, false );
	}
	WinDrawChars( text, StrLen( text ), leftMargin, 70 );
	
//	if( attr & dmRecAttrSecret )
	ctl = GetObjectPtr<ControlType>( RecordListSecretCheckbox );		
	if( attr & dmRecAttrSecret ) {
		StrCopy( text, "Yes" );
//	else
		CtlSetValue( ctl, true );
	} else {
		StrCopy( text, "No" );
		CtlSetValue( ctl, false );
	}
	WinDrawChars( text, StrLen( text ), leftMargin, 83 );
	}


/**
 * Initialize the record list form.
 */
static Boolean SECT8 handleFormOpen( EventPtr event )
	{
	ListPtr		lst;
	UInt16		itemCount;
	//Err			err;
	UInt16		attr;
	FormPtr		frm;
	DmOpenRef	ref;
  Char buf[32];
	
	frm = FrmGetActiveForm();

   ItemSet *itemSet = tree->getSelectedItems();
   editItem = itemSet->getItem( 0 );
   delete itemSet;

	/*err =*/ DmDatabaseInfo( editItem->getVolumeNum(), editItem->getFile()->iAttr->id, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL );
	ref = DmOpenDatabase( editItem->getVolumeNum(), editItem->getFile()->iAttr->id, dmModeReadOnly );
	if( !ref )
      {
      showMessage( "Unable to open database in handleFormOpen() %s", editItem->getName()); 
      return true;
      }
		
	if( attr & dmHdrAttrResDB )						// if it's a resource db instead of a record db
		{
		itemCount = DmNumResources( ref );
    StrCopy(buf, "Resource List");
		FrmSetTitle( FrmGetActiveForm(), buf );
		isRecs = false;
		FrmHideObject( frm, FrmGetObjectIndex( frm, RecordListEditButton ));
		FrmHideObject( frm, FrmGetObjectIndex( frm, RecordListDeleteButton ));
		FrmHideObject( frm, FrmGetObjectIndex( frm, RecordListSetAttrButton ));
		}
	else
		{
		itemCount = DmNumRecords( ref );
    StrCopy(buf, "Record List");
		FrmSetTitle( FrmGetActiveForm(), 	buf );
		isRecs = true;
		}
	
	DmCloseDatabase( ref );
	
	if( !isRecs )
		{
		FrmHideObject( frm, FrmGetObjectIndex( frm, RecordListIDLabel ));
		FrmHideObject( frm, FrmGetObjectIndex( frm, RecordListCategoryLabel ));
		FrmHideObject( frm, FrmGetObjectIndex( frm, RecordListBusyLabel ));
//		FrmHideObject( frm, FrmGetObjectIndex( frm, RecordListDirtyLabel ));
//		FrmHideObject( frm, FrmGetObjectIndex( frm, RecordListSecretLabel ));
		FrmHideObject( frm, FrmGetObjectIndex( frm, RecordListDirtyCheckbox ));
		FrmHideObject( frm, FrmGetObjectIndex( frm, RecordListSecretCheckbox ));
		}	

	FrmDrawForm( FrmGetActiveForm() );
	
	lst = GetObjectPtr<ListType>( RecordListRecordList );
	LstSetDrawFunction( lst, RecordListDrawRecord );
	LstSetListChoices( lst, NULL, itemCount );
	LstDrawList( lst );
	
	if( itemCount )
		{
		LstSetSelection( lst, 0 );
//		currentRecord = 0;
		currentRecord = -1;			// will be reset
		RecordListShowStats( 0 );
		}
	else
		currentRecord = -1;
      
   return true;
	}


/**
 * Delete the currently selected item.
 */
static void SECT8 RecordListDelete()
	{
	Err			err;
	DmOpenRef	ref;
	
	if( FrmAlert( DeleteRecordAlert ) != DeleteRecordDelete )
		return;
	
	ref = DmOpenDatabase( editItem->getVolumeNum(), editItem->getFile()->iAttr->id, dmModeReadWrite );
	if( !ref )
		{
		showMessage( "Unable to open database in RecordListDelete()", editItem->getName());
		return;
		}
	
	err = DmRemoveRecord( ref, currentRecord );
	if( err != errNone )
		showMessage( "Unable to remove record in RecordListDelete()", editItem->getName());
		
	DmCloseDatabase( ref );
	handleFormOpen( 0 );
	}


/**
 * Update the form.
 */
static void SECT8 RecordListUpdate()
	{
	ListPtr	lst;
	Int16		sel;

	lst = GetObjectPtr<ListType>( RecordListRecordList );
	sel = LstGetSelection( lst );
	
	//LstDrawList( lst );							// fixes some bad refresh thing in 3.x when returning from the hex view
	
	if( sel != -1 )
		RecordListShowStats( sel );
	}


static void SECT8 RecordListScroll( WinDirectionType direction )
	{
	ListPtr	lst = GetObjectPtr<ListType>( RecordListRecordList );
	LstScrollList( lst, direction, LstGetVisibleItems( lst ));
	}


static Boolean SECT8 handleSetAttrButton( EventPtr event )
	{
   if( currentRecord != -1 )
      {
      if( RecordListIsRecDeleted( currentRecord ))
         {
         FrmCustomAlert( GeneralErrorAlert, "This operation cannot be performed on a deleted record.", NULL, NULL );
         return true;
         }
						
      // do it here...
        Char buf[128];
	Err			err;
	DmOpenRef	ref;
	UInt16		attr;
	UInt32		uniqueID;

	
	int index = currentRecord;
	
	if (!isRecs) {
	        FrmCustomAlert( GeneralErrorAlert, "Can't set attributes for resource database.", NULL, NULL );
		return true;
	}
	ref = DmOpenDatabase( editItem->getVolumeNum(), editItem->getFile()->iAttr->id, dmModeReadWrite );
	if( !ref )
		{
		FrmCustomAlert( GeneralErrorAlert, "Unable to open database.", NULL, NULL );
		return true;
		}
	
	err = DmRecordInfo( ref, index, &attr, &uniqueID, NULL );
		
	if( err != errNone )
		{
		FrmCustomAlert( GeneralErrorAlert, "Unable to get record info.", NULL, NULL );
		DmCloseDatabase( ref );
		return true;
		}
		
	attr = attr | dmRecAttrDirty | dmRecAttrSecret;

	ControlPtr ctl = GetObjectPtr<ControlType>( RecordListDirtyCheckbox );	
	if (!CtlGetValue( ctl))
	    attr -= dmRecAttrDirty;
	ctl = GetObjectPtr<ControlType>( RecordListSecretCheckbox );		
	if (!CtlGetValue( ctl))
	    attr -= dmRecAttrSecret;

	err = DmSetRecordInfo( ref, index, &attr, &uniqueID );
	if (err != errNone) {
	  StrPrintF(buf, "Unable to change attributes: %d", err);
	  FrmCustomAlert( GeneralErrorAlert, buf, NULL, NULL );
	} else {
	  //FrmCustomAlert( GeneralErrorAlert, "Successful change attributes.", NULL, NULL );
	  RecordListShowStats(index);
	}

	DmCloseDatabase( ref );
	return true;
      }
   else
      FrmAlert( SelectRecordAlert );

   return true;
   }


static Boolean SECT8 handleViewEditButton( EventPtr event )
	{
   if( currentRecord != -1 )
      {
      if( RecordListIsRecDeleted( currentRecord ))
         {
         FrmCustomAlert( GeneralErrorAlert, "This operation cannot be performed on a deleted record.", NULL, NULL );
         return true;
         }
						
      if( event->data.ctlSelect.controlID == RecordListViewButton )
         readOnly = true;
      else
         readOnly = false;
      // this one is ok to pop up
      FrmPopupForm( HexEditorForm );
      }
   else
      FrmAlert( SelectRecordAlert );

   return true;
   }


static Boolean SECT8 handleDeleteButton( EventPtr event )
	{
   if( currentRecord != -1 )
      {
      if( RecordListIsRecDeleted( currentRecord ))
         {
         FrmCustomAlert( GeneralErrorAlert, "This operation cannot be performed on a deleted record.", NULL, NULL );
         return true;
         }
      
      RecordListDelete();
      }
   else
      FrmAlert( SelectRecordAlert );

   return true;
   }


static Boolean SECT8 handleCloseButton( EventPtr event )
	{
   FrmGotoForm( TreeViewForm );	
   FrmUpdateForm( TreeViewForm, frmRedrawTreeUpdateCode );
   return true;
   }


static Boolean SECT8 handleUpdateEvent()
   {
   FrmDrawForm( FrmGetActiveForm());
   RecordListUpdate();
   
   return true;
   }
   

static Boolean SECT8 handleSelectEvent( EventPtr event )
	{
   Boolean handled = false;
   
   switch( event->data.ctlSelect.controlID )
      {
      case RecordListViewButton:
      case RecordListEditButton:
         handled = handleViewEditButton( event );
         break;

		case RecordListSetAttrButton:
			handled = handleSetAttrButton( event );
			break;

      case RecordListDeleteButton:
         handled = handleDeleteButton( event );
         break;

      case RecordListCloseButton:
         handled = handleCloseButton( event );
         break;
      }
   
   return handled;
   }


static Boolean SECT8 handleKeyDownEvent( EventPtr event )
	{
   Boolean handled = false;
	
	if( EvtKeydownIsVirtual( event ))
		{
		switch( event->data.keyDown.chr )
			{
			case vchrPageUp:
				RecordListScroll( winUp );
				handled = true;
				break;
			case vchrPageDown:
				RecordListScroll( winDown );
				handled = true;
				break;
			}
		}

	return handled;
	}
   
   
static Boolean SECT8 handleListSelectEvent( EventPtr event )
	{
   RecordListShowStats( event->data.lstSelect.selection );
   return true;
   }


/**
 * Handle all the events.
 *
 * @param an event
 * @return if it was handled
 */
 Boolean RecordListHandleEvent( EventPtr event )
	{
	Boolean handled = false;

   if ( ResizeHandleEvent( event ) )
      return true;

	switch( event->eType )
		{
		case frmOpenEvent:
			handled = handleFormOpen( event );
			break;

		case ctlSelectEvent:
         handled = handleSelectEvent( event );
         break;

      case keyDownEvent:
         handled = handleKeyDownEvent( event );
         break;

      case lstSelectEvent:
         handled = handleListSelectEvent( event );
         break;
		
      case frmUpdateEvent:
         handled = handleUpdateEvent();
         break;
      default:
         break;
      }
	
	return handled;
	}
