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

#include "TreeViewForm.h"
#include "TreeView.hpp"
#include "SelectionChecker.hpp"
#include "Item.hpp"
#include "ItemFile.hpp"
#include "ItemFolder.hpp"
#include "ItemSet.hpp"
#include "Chooser.h"
#include "DetailsForm.h"
#include "resize.h"

// palmOne SDK
//#include "PalmChars.h"

// sony SDK
//#include "SonyCLIE.h"

#include "Stuph.h"
#include "UI.h"
#include "Resource.h"
#include "Main.h"

#include "debug.h"

TreeView *tree = 0;

static void handleResize()
   {
   FormPtr frm = FrmGetActiveForm();
	
   Int16          index;
   RectangleType  bounds, tableBounds;
   
   index = FrmGetObjectIndex( frm, TreeViewTable );
   TablePtr table = (TablePtr)FrmGetObjectPtr( frm, index );
   TblGetBounds( table, &tableBounds );
   UInt16 tableRows = calcTableRows( table, false );
      
   FontID   oldFont = FntSetFont( stdFont );
   Int16    lineHeight = FntLineHeight();
   FntSetFont( oldFont );

   HideObject( frm, TreeViewScrollBar );

   lineHeight = TblGetRowHeight( table, 0 );

   index = FrmGetObjectIndex( frm, TreeViewScrollBar );
   FrmGetObjectBounds( frm, index, &bounds );
   bounds.extent.y = ( lineHeight * (tableRows) ) + 2;
   FrmSetObjectBounds( frm, index, &bounds );
   ScrollBarPtr scrollBar = (ScrollBarPtr)FrmGetObjectPtr( frm, index );

   ShowObject( frm, TreeViewScrollBar );

   if( tree )
      tree->updateTable( table, scrollBar, false );
   }


/**
 * Call the record drawing method of the tree object.
 */
void SECT6 drawRec( void *tableP, Int16 row, Int16 column, RectanglePtr bounds )
   {
   TablePtr table = *((TablePtr*)&tableP);
   if( tree )
      tree->drawRecord( row, column, bounds, table );
   }


/**
 * Set the label of the column.
 */
static void SECT6 setColumnLabel()
	{
	ControlPtr		ctl;

	ctl = GetObjectPtr<ControlType>( TreeViewColumnHeaderButton );
	switch( prefs.list.column )
		{
		case colCreator:
			CtlSetLabel( ctl, "Creator" );
			break;
		case colType:
			CtlSetLabel( ctl, "Type" );
			break;
		case colSize:
			CtlSetLabel( ctl, "Size" );
			break;
		case colRec:
			CtlSetLabel( ctl, "Records" );
			break;
		case colAttr:
			CtlSetLabel( ctl, "Attrib" );
			break;
		case colCreate:
			CtlSetLabel( ctl, "Created" );
			break;
		case colMod:
			CtlSetLabel( ctl, "Modified" );
			break;
		case colBackup:
			CtlSetLabel( ctl, "Backup" );
			break;
		case colNone:
			CtlSetLabel( ctl, "" );
			break;
		}

   if( prefs.list.column == colNone )
      HideObject( FrmGetActiveForm(), TreeViewColumnHeaderButton );
   else
      ShowObject( FrmGetActiveForm(), TreeViewColumnHeaderButton );
	}


static void SECT6 drawLine()
   {
   Coord width, height;
   WinGetDisplayExtent( &width, &height );

   ColorSet( &gray, NULL, NULL, NULL, NULL );
	WinDrawLine( 0, 29, width, 29 );
   ColorUnset();
   }
   

static void SECT6 drawBackground()
   {
   FormPtr  frm = FrmGetActiveForm();

   Coord width, height;
   WinGetDisplayExtent( &width, &height );

   //ColorSet( &gray, NULL, NULL, NULL, NULL );
	//WinDrawLine( 0, 29, width, 29 );
   //ColorUnset();

   drawLine();

   ColorSet( &paleBlue, NULL, NULL, NULL, NULL );
   RectangleType r;
   r.topLeft.x = 0;  r.topLeft.y = height-15; r.extent.x = width; r.extent.y = 50;
   WinDrawRectangle( &r, 3 );
   ColorUnset();

   FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, TreeViewCloseButton ), &r );
   WinEraseRectangle( &r, 1 );
   FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, TreeViewDetailsButton ), &r );
   WinEraseRectangle( &r, 1 );
   FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, TreeViewFilterButton ), &r );
   WinEraseRectangle( &r, 1 );
   FrmGetObjectBounds( frm, FrmGetObjectIndex( frm, TreeViewSendButton ), &r );
   WinEraseRectangle( &r, 1 );

   ShowObject( frm, TreeViewCloseButton );
   ShowObject( frm, TreeViewDetailsButton );
   ShowObject( frm, TreeViewSendButton );

   if( tree )
      {
      if( tree->getFilterOn())
         ShowObject( frm, TreeViewUnFilterButton );
      else
         ShowObject( frm, TreeViewFilterButton );
      }
   else
      ShowObject( frm, TreeViewFilterButton );
   }


static Boolean SECT6 handleFormOpen( EventPtr event )
   {
   FormPtr     frm = FrmGetActiveForm();
   ControlPtr  ctl;

   HideObject( frm, TreeViewUnFilterButton );
	ShowObject( frm, TreeViewFilterButton );
   setColumnLabel();

   HideObject( frm, TreeViewCloseButton );
   HideObject( frm, TreeViewDetailsButton );
   HideObject( frm, TreeViewFilterButton );
   HideObject( frm, TreeViewSendButton );

	FrmDrawForm( frm );
   drawBackground();

   prefs.lastForm = TreeViewForm;

	if( prefs.list.hideROM )
		{
		ctl = GetObjectPtr<ControlType>( TreeViewHideROMCheckbox );
		CtlSetValue( ctl, true );
		}

   ScrollBarPtr scrollBar = GetObjectPtr<ScrollBarType>( TreeViewScrollBar );
	TablePtr table = GetObjectPtr<TableType>( TreeViewTable );

   Boolean needToDraw = false;

   if( !tree )
      {
      tree = (TreeView*)MemPtrNew( sizeof( TreeView ));
      if( !tree ) showMessage( "Unable to allocate tree" );
      tree->init();
      needToDraw = true;
      }
   
   TblSetCustomDrawProcedure( table, 0, drawRec );
   tree->setup( table, false );
  
   if( needToDraw )
      tree->updateTable( table, scrollBar, true );
  
   return true;
   }
   

// just a place holder, the deallocation takes place when the app exits.
static Boolean SECT6 handleFormClose( EventPtr event )
   {
   return false;
   }


void freeTree()
   {
   Err err = 0;
   
   if( tree )
      {
      tree->free();
      err = MemPtrFree( tree );
      checkError( err, "freeTree() a", 0 );
      tree = 0;
      }
   }


static Boolean SECT6 handleColumnButton( EventPtr event )
   {
	ListPtr		lst;
	Int16			selected;
	//FormPtr		frm;
	//TablePtr		table;
	
	lst = GetObjectPtr<ListType>( TreeViewColumnList );
	LstSetSelection( lst, prefs.list.column );
	
	selected = LstPopupList( lst );
	
	if( selected == -1 ) return true;								// return if nothing selected
		
	switch( selected )
		{
		case 0:										// change column to show creator id
			prefs.list.column = colCreator;
			break;
		case 1:										// change column to show type id
			prefs.list.column = colType;
			break;
		case 2:										// change column to show size
			prefs.list.column = colSize;
			break;
		case 3:
			prefs.list.column = colRec;
			break;
		case 4:
			prefs.list.column = colAttr;
			break;
		case 5:
			prefs.list.column = colCreate;
			break;
		case 6:
			prefs.list.column = colMod;
			break;
		case 7:
			prefs.list.column = colBackup;
			break;
		case 8:
			prefs.list.column = colNone;
			break;
		}

   setColumnLabel();
   FrmUpdateForm( TreeViewForm, frmRedrawTreeUpdateCode );

   return true;
   }
      

static Boolean SECT6 handleDetailsButton( EventPtr event )
   {
   if( !SelectionChecker::check( selectionOne, selectionIntExt, selectionFileFold, tree, false, selectionVolOK ))
      return true;
   
   ItemSet *itemSet = tree->getSelectedItems();
   Item *item = itemSet->getItem( 0 );
   delete itemSet;

   if( item->getFolder())
      FrmPopupForm( FolderDetailsForm );
   
   if( item->getFile())
      {
      if( item->getFile()->eAttr )
         FrmGotoForm( DetailsVForm );
      else
         {
         detailsPoppedUp = false;
         FrmGotoForm( DetailsRForm );
         }
      }
      
   return true;
   }


static Boolean SECT6 handleDeleteEvent( EventPtr event )
   {
   Err      err = 0;
   ItemSet *itemSet = tree->getSelectedItems();

   if( !SelectionChecker::check( selectionMultiple, selectionIntExt, selectionFileFold, tree, false, selectionVolNo ))
      return true;

/*
   Char msg[512];
   StrCopy( msg, "Files: " );
   for( int i=0; i<itemSet->size(); i++ )
      {
      Item *item = itemSet->getItem( i );
      StrCat( msg, item->getName());
      StrCat( msg, ", " );
      }
   showMessage( msg );
*/

   // if any items are a folder, make sure they know that
   Char countStr[16];
   StrPrintF( countStr, "%d", itemSet->size() );
   if( FrmCustomAlert( DeleteItemsAlert, countStr, 0, 0 ) == DeleteItemsCancel )				// if cancelled, don't do anything
      return false;

   for( int i=0; i<itemSet->size(); i++ )
      {
      Item *item = itemSet->getItem( i );
      item->remove( false, err );
      if( err != errNone )
         {
         showMessage( "There was an error while deleting." );
         break;
         }
      }

   delete itemSet;   
   FrmUpdateForm( TreeViewForm, frmReloadTreeUpdateCode );
   
   return true;
   }


// op==true, then copy, if false, then move
static Boolean SECT6 handleCopyEvent( Boolean op, EventPtr event )
   {
   if( !SelectionChecker::check( selectionMultiple, selectionIntExt, selectionFiles, tree, true, selectionVolNo ))
      return true;
   chooserOp = op;
   FrmGotoForm( ChooserForm );   
   return true;
   }
   

static Boolean SECT6 handleEditEvent( EventPtr event )
   {
   if( !SelectionChecker::check( selectionOne, selectionInternal, selectionFiles, tree, false, selectionVolNo ))
      return true;

   FrmGotoForm( RecordListForm );
   
   return true;
   }
   

static Boolean SECT6 handleFolderEvent( EventPtr event )
   {
   chooserOp = createFolder;
   FrmGotoForm( ChooserForm );
   return true;
   }


static Boolean SECT6 handleSendEvent( EventPtr event )
   {
   if( !SelectionChecker::check( selectionMultiple, selectionIntExt, selectionFiles, tree, false, selectionVolNo ))
      return true;

   tree->sendSelectedItems();
   return true;
   }


static Boolean SECT6 handleSetAttributesEvent( EventPtr event )
   {
   if( !SelectionChecker::check( selectionMultiple, selectionInternal, selectionFiles, tree, false, selectionVolNo ))
      return true;

   FrmGotoForm( SetAttributesForm );
   return true;
   }
   

static Boolean SECT6 handleSetTypeColorsEvent( EventPtr event )
   {
   FrmGotoForm( PreferencesForm );
   return true;
   }
   

static Boolean SECT6 handleFilterEvent( EventPtr event )
   {
   if( tree->getFilterOn())
      tree->filterTree();
   else
      FrmPopupForm( FilterForm );

   return true;
   }
   

static Boolean SECT6 handleMenuEvent( EventPtr event )
   {
   Boolean handled = false;

   switch( event->data.menu.itemID )
      {
      case TreeItemDetails:
         handled = handleDetailsButton( event );
         break;
      case TreeItemSend:
         handled = handleSendEvent( event );
         break;
      case TreeItemDelete:
         handled = handleDeleteEvent( event );
         break;
      case TreeItemEdit:
         handled = handleEditEvent( event );
         break;
      case TreeItemCopy:
         handled = handleCopyEvent( copyFile, event );
         break;
      case TreeItemMove:
         handled = handleCopyEvent( moveFile, event );
         break;
      case TreeItemCreateFolder:
         handled = handleFolderEvent( event );
         break;

      case TreeSelectSelectAll:
         tree->selectAll( true );
         FrmUpdateForm( MainViewForm, frmRedrawTreeUpdateCode );
         handled = true;
         break;
      case TreeSelectUnselectAll:
         tree->selectAll( false );
         FrmUpdateForm( MainViewForm, frmRedrawTreeUpdateCode );
         handled = true;
         break;
      case TreeSelectSetAttributes:
         handled = handleSetAttributesEvent( event );
         break;

      case TreeTreeCollapse:
         tree->collapseAll( false );
         FrmUpdateForm( MainViewForm, frmReloadTreeUpdateCode );
         handled = true;
         break;
      case TreeTreeExpand:
         tree->collapseAll( true );
         FrmUpdateForm( MainViewForm, frmRedrawTreeUpdateCode );
         handled = true;
         break;
      case TreeTreeGoto:
			FrmPopupForm( GotoForm );
         handled = true;
         break;

      case TreeOptionsFilter:
         handled = handleFilterEvent( event );
         FrmUpdateForm( MainViewForm, frmRedrawTreeUpdateCode );
         break;
      case TreeOptionsSendList:
         tree->sendToMemo( GetObjectPtr<TableType>( TreeViewTable ));
         handled = true;
         break;
      case TreeOptionsSetTypeColors:
         handled = handleSetTypeColorsEvent( event );
         break;
      }
      
   return handled;
   }


static Boolean SECT6 handleCloseButton( EventPtr event )
	{
	freeTree();
	FrmGotoForm( MainViewForm );
	return true;
	}
	

static Boolean SECT6 handleSelectEvent( EventPtr event )
   {
   Boolean handled = false;
   
   switch( event->data.ctlEnter.controlID )
      {
      case TreeViewCloseButton:
         handled = handleCloseButton( event );
         break;
      case TreeViewDetailsButton:
         handled = handleDetailsButton( event );
         break;
      case TreeViewFilterButton:
      case TreeViewUnFilterButton:
         handled = handleFilterEvent( event );
         FrmUpdateForm( MainViewForm, frmRedrawTreeUpdateCode );
         break;
      case TreeViewSendButton:
         handleSendEvent( event );
         handled = true;
         break;
      case TreeViewFilenameButton:
         tree->reSortTree( true );
         FrmUpdateForm( MainViewForm, frmRedrawTreeUpdateCode );
         handled = true;
         break;
      case TreeViewColumnHeaderButton:
         tree->reSortTree( false );
         FrmUpdateForm( MainViewForm, frmRedrawTreeUpdateCode );
         handled = true;
         break;
      case TreeViewColumnButton:
         handled = handleColumnButton( event );
         break;
      case TreeViewHideROMCheckbox:
         prefs.list.hideROM = !prefs.list.hideROM;
         FrmUpdateForm( MainViewForm, frmReloadTreeUpdateCode );
         handled = true;
         break;
      }

   return handled;
   }


static Boolean SECT6 handleFormUpdate( EventPtr event )
   {
   Boolean handled = false;

   ScrollBarPtr scrollBar = GetObjectPtr<ScrollBarType>( TreeViewScrollBar );
	TablePtr table = GetObjectPtr<TableType>( TreeViewTable );
   
   switch( event->data.frmUpdate.updateCode )
      {
      case frmRedrawTreeUpdateCode:
         if( tree ) tree->updateTable( table, scrollBar, true );
         handled = true;
         break;
      case frmReloadTreeUpdateCode:
         if( tree ) tree->reloadTree( table, scrollBar );
         handled = true;
         break;
      case frmReSortTreeUpdateCode:
         if( tree )
            {
            tree->reSortTree();
            tree->updateTable( table, scrollBar, true );
            }
         handled = true;
         break;
      case frmFilterTreeUpdateCode:
         if( tree )
            {
            tree->filterTree();
            tree->updateTable( table, scrollBar, true );
            }
         handled = true;
         break;
      case frmRedrawUpdateCode:
         FrmDrawForm( FrmGetActiveForm());
         drawLine();
         handled = true;
         break;
      }

   // seems like we always need to redraw that line
   drawLine();
      
   return handled;
   }


/*
static void SECT6 handleLeftRightButton( WinDirectionType direction )
	{
	Int8		column = prefs.list.column;	// we need a signed int temporarily
	
	if( direction == winLeft )
		column--;
	else
		column++;
	
	if( column < colCreator )
		column = colNone;
		
	if( column > colNone )
		column = colCreator;

	prefs.list.column = column;

	setColumnLabel();
	FrmUpdateForm( MainViewForm, frmRedrawTreeUpdateCode );
	}
*/


/**
 * Handles non-list-scrolling key events. The key events that scroll the list
 * are in the TreeView class.
 */
static Boolean SECT6 handleKeyDown( EventPtr event )
   {
   Boolean handled = false;

	// handle if a key wasn't actually pressed
	if( EvtKeydownIsVirtual( event ))
		{
/*
		switch( event->data.keyDown.chr )
			{
			case vchrJogPush:
				// bring up the item details
				handleDetailsButton( event );
				break;

			// handle the 5-way navigation button
			case vchrNavChange:
				// if it's a select event, bring up the item details
				if( event->data.keyDown.keyCode == navChangeSelect && event->data.keyDown.modifiers == commandKeyMask )
					{
					handleDetailsButton( event );
					}
				// see if it's a scroll left event
				else if( event->data.keyDown.keyCode == navChangeLeft && event->data.keyDown.modifiers == commandKeyMask )
					{
					handleLeftRightButton( winLeft );
					}
				// see if it's a scroll right event
				else if( event->data.keyDown.keyCode == navChangeRight && event->data.keyDown.modifiers == commandKeyMask )
					{
					handleLeftRightButton( winRight );
					}
				handled = true;
				break;				
			}
*/
		}

   return handled;
   }


Boolean TreeViewHandleEvent( EventPtr event )
   {
	Boolean handled = false;

   ScrollBarPtr scrollBar = GetObjectPtr<ScrollBarType>( TreeViewScrollBar );
   TablePtr table = GetObjectPtr<TableType>( TreeViewTable );

   if( ResizeHandleEvent( event ))
      return true;

   if( tree )
      {
      handled = tree->handleEvent( event, table, scrollBar );      
      if( handled ) return handled;
      }

	switch( event->eType )
		{
		case frmOpenEvent:
			handled = handleFormOpen( event );
			break;

		case keyDownEvent:
			handled = handleKeyDown( event );
			break;

      case frmCloseEvent:
         handled = handleFormClose( event );
         break;

		case frmUpdateEvent:
         handled = handleFormUpdate( event );
			break;

		case ctlSelectEvent:
         handled = handleSelectEvent( event );
         break;

		case menuEvent:
			handled = handleMenuEvent( event );
         break;

		case winDisplayChangedEvent:
         handleResize();
			break;
    default:
			break;
		}
	
	return handled;
   }


/** The goto form code is here **/

/*
Boolean handleGotoFormOpen( EventPtr event )
	{
	Boolean handled = false;
	
	FormPtr frm = FrmGetActiveForm();
	FrmDrawForm( frm );
	
	return handled;
	}
	
	
Boolean handleGotoSelectEvent( EventPtr event )
	{
	Boolean handled = false;
	
   switch( event->data.ctlEnter.controlID )
      {
      case GotoFormGotoButton:
			
         handled = true;
         break;
      case GotoFormCancelButton:
			FrmReturnToForm( 0 );
         handled = true;
			break;
		}
		
	return handled;
	}


Boolean GotoFormHandleEvent( EventPtr event )
   {
	Boolean handled = false;

   if( ResizeHandleEvent( event ))
      return true;

	switch( event->eType )
		{
		case frmOpenEvent:
			handled = handleGotoFormOpen( event );
			break;

		case ctlSelectEvent:
         handled = handleGotoSelectEvent( event );
         break;
		}
	
	return handled;
   }
*/

