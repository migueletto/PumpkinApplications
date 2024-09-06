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
 * This class holds all the information about a filesystem in a tree
 * structure, optimized for being displayed as a tree, similar to typical
 * file management programs. The intent is to make this class the 'model' in
 * the model-view-controller pattern so that it does not contain any form-
 * specific code, allowing multiple instanciations on different forms of the
 * same program.
 *
 * Each tree object has a rootItem object which is the root item of the
 * whole tree (the root is not displayed in the UI). An item is either a file
 * or a folder, and you can tell the difference by whether or not the item
 * has the ItemFile data member or the ItemFolder data member set.
 *
 * Created on 5/3/04 by Tom Bulatewicz
 */

#include <PalmOS.h>
#include <TblGlue.h>

#include "TreeView.hpp"            // the object definitions
#include "Item.hpp"
#include "ItemFolder.hpp"
#include "ItemFile.hpp"
#include "ItemSet.hpp"
#include "BusyIndicator.hpp"
#include "SelectionChecker.hpp"
//#include "PalmChars.h"
#include "TxtGlue.h"

#undef keyBitJogBack
//#include <SonyCLIE.h>

#include "Stuph.h"               // ??
#include "UI.h"
#include "Resource.h"            // for drawing the icons
#include "CardSet.hpp"           // for quick access to card information

#include "debug.h"           // for quick access to card information

#define indentFactor 5


/**
 * Returns an ItemSet populated with pointers to the items that are
 * currently selected.
 */
ItemSet *TreeView::getSelectedItems()
   {
   ItemSet *itemSet = new ItemSet();
   rootItem->getFolder()->getSelectedItems( itemSet );
   return itemSet;   
   }


/**
 * The main initialization method called only when the tree is first
 * created, but called after the init() constructor. Note that this method
 * does not actually draw the tree.
 */
void TreeView::setup( TablePtr table, Boolean chooserp )
   {
   chooser = chooserp;
   /*UInt16 tableRows =*/ calcTableRows( table, chooser );

/*   
   Char msg[256];
   StrPrintF( msg, "bounds.extent.y=%d", tableRows );
   showMessage( msg );
  */ 
  
   initTable( table );
   
   if( !rootItem )
      readRootItems( false );
   }


/**
 * Sets up the table itself and needs to be called only once each time the
 * form is loaded.
 */
void TreeView::initTable( TablePtr table )
   {
	//UInt16		i;
	Int16			row;

   /*UInt16 tableRows =*/ calcTableRows( table, chooser );

	for( row = 0; row < 16; row++ )
		{		
		TblSetItemStyle( table, row, 0, customTableItem );
		TblSetItemStyle( table, row, 1, customTableItem );
		TblSetItemFont( table, row, 0, stdFont );
		TblSetRowUsable( table, row, false );
		}

	TblSetColumnUsable( table, 0, true );
	TblSetColumnUsable( table, 1, true );
   }


/**
 * Searches the tree to find out which folders are currently expanded by the
 * user, and remembers them in the expandedItems ItemSet.
 */
void TreeView::rememberExpandedItems()
   {
   if( expandedItems )
      {
      for( int i=0; i<expandedItems->size(); i++ )
         {
         Item *item = expandedItems->getItem( i );
         item->free();
         }      
      delete expandedItems;
      expandedItems = 0;
      }

   expandedItems = new ItemSet();
   rootItem->getFolder()->rememberExpandedItems( expandedItems );
   }


/**
 * Re-populates the rootItem and all folders that were previously marked as
 * expanded. This way we can re-read all the file information from the device
 * but then still draw the tree with all the appropriate folders expanded.
 */
void TreeView::readRootItems( Boolean expandAll )
   {
   Char buf[32];
   // deallocate the current root item if there is one
   if( rootItem )
      {
      rememberExpandedItems();      // store which are expanded, so we can expand
                                    //  them when we reload the tree.
      rootItem->free();
      MemPtrFree( rootItem );
      checkMemPtr( rootItem, "TreeView::readRootItems() a" );
      rootItem = 0;
      }

   // create the root which will contain a folder for each card on the device
   rootItem = (Item*)MemPtrNew( sizeof( Item ));
   checkMemPtr( rootItem, "TreeView::readRootItems() b" );
   rootItem->init();

   rootItem->setFolder((ItemFolder*)MemPtrNew( sizeof( ItemFolder )));
   checkMemPtr( rootItem->getFolder(), "TreeView::readRootItems() c" );

   rootItem->getFolder()->init();
   StrCopy(buf, "/");
   rootItem->getFolder()->setPath( buf );
   rootItem->getFolder()->setIsVolume( true );
   rootItem->setIndent( 0 );
   rootItem->getFolder()->expanded = true;
   StrCopy(buf, "home");
   rootItem->setName( buf );

   rootItem->getFolder()->items = (Item*)MemPtrNew( cardSet.size() * sizeof( Item ));
   checkMemPtr( rootItem->getFolder()->items, "ItemFolder::readItems() d" );

   // the main tree view should start with an indent of -1 so that the
   //  root folders won't appear as selectable, and save some space at the
   //  same time.  But, if this is a chooser tree, then we do want them to
   //  be selectable, so we pass in a 0 in that case;
   Int8 initialIndent = 0;

   rootItem->getFolder()->readCards( expandedItems, initialIndent, chooser, filterOn, expandAll );
   
   // deallocate the expanded items ItemSet
   if( expandedItems )
      {
      for( int i=0; i<expandedItems->size(); i++ )
         {
         Item *item = expandedItems->getItem( i );
         item->free();
         }

      delete expandedItems;
      expandedItems = 0;
      }
   }
   
   
/**
 * Given a letter, this method will search the tree (the expanded parts only)
 * for the first item (file or folder) that begins with that letter, and then
 * scroll the UI so that that item is in view.
 */
void TreeView::jumpToLetter( Char ch, TablePtr table, ScrollBarPtr scrollBar )
	{
	//Char     fc;
	
	if( (UInt8)ch >= 'a' && (UInt8)ch <= 'z' )
      ch -= ( 'a' - 'A' );

   Int32 num = rootItem->getFolder()->findItemWithLetter( ch );

   if( num >= 0 )
      treeViewScroll( table, scrollBar, num - skippedItems, false );
   else
      SndPlaySystemSound( sndWarning );
	}


/**
 * Updates our internal scroll amount indicator: skippedItems.
 */
void TreeView::treeViewScroll( TablePtr table, ScrollBarPtr scrollBar, Int16 dir, Boolean fullPage )
	{
   UInt16 tableRows = calcTableRows( table, chooser );
   if( fullPage )
      dir *= tableRows;
      
   skippedItems += dir;
   updateTable( table, scrollBar, true );
   }


/**
 * Updates the actual UI scrollers based on the skippedItems variable.
 */
void TreeView::treeViewUpdateScrollers( TablePtr table, ScrollBarPtr scrollBar )
	{
   UInt16 tableRows = calcTableRows( table, chooser );

   if( totalItems <= tableRows )
		SclSetScrollBar( scrollBar, 0, 0, 0, tableRows-1 );
	else	
		SclSetScrollBar( scrollBar, skippedItems, 0, totalItems - tableRows, tableRows-1 );
	}


/**
 * Initiates a send (beam/bluetooth/etc.) of all selected items.
 */
void TreeView::sendSelectedItems()
   {
   rootItem->sendSelectedItems();
   }


/**
 * Tells the root item to set the attributes of all visible files accordingly.
 */
void TreeView::setAllAttributes( UInt16 attr, Boolean set )
   {
   rootItem->setAllAttributes( attr, set );
   }


/**
 * Creates a new memo in the Memo Pad application that is a list of all the
 * items currently displayed in the tree.
 */
void TreeView::sendToMemo( TableType *table )
	{
	MemHandle	content;
   Char        *text;
  Char buf[32];

   BusyIndicator *busyIndicator = new BusyIndicator( 80, 5 );

	content = MemHandleNew( 1 );
	
	// since we're just concatenating later on, be sure to null terminate the string
	text = (Char*)MemHandleLock( content );
	text[0] = 0;
	MemHandleUnlock( content );
	
   rootItem->sendToMemo( content, 0, busyIndicator );

	text = (Char*)MemHandleLock( content );
   StrCopy(buf, "FileZ file listing");
   SendStringToMemoPad( buf, text );
   MemHandleUnlock( content );
   MemHandleFree( content );

   delete busyIndicator;
   TblDrawTable( table );
	}




/**
 * Deallocate any memory used by the tree.
 */
void TreeView::free()
   {
   Err err = errNone;

   if( rootItem )
      {
      rootItem->free();           // free anything that the root allocated
      err = MemPtrFree( rootItem );    // and free the root folder
      checkError( err, "TreeView::free()", 0 );
      rootItem = 0;
      }

   if( expandedItems )
      {
      MemPtrFree( expandedItems );
      expandedItems = 0;
      }
   }
   

/**
 * Go through the tree and mark any files that do not meet the filter
 *  criteria as "filtered" so that they won't be shown when the tree is
 *  redrawn.
 */
void TreeView::filterTree()
   {
   filterOn = !filterOn;
   
   if( filterOn )
      {
      HideObject( FrmGetActiveForm(), TreeViewFilterButton );
      ShowObject( FrmGetActiveForm(), TreeViewUnFilterButton );
      }
   else
      {
      HideObject( FrmGetActiveForm(), TreeViewUnFilterButton );
      ShowObject( FrmGetActiveForm(), TreeViewFilterButton );      
      }

   rootItem->filter( filterOn );
   }


/**
 * Sets which items in the tree are associated with each row of the table,
 * and then draws the actual table in the UI.
 */
void TreeView::updateTable( TablePtr table, ScrollBarPtr scrollBar, Boolean redraw )
   {
   ErrFatalDisplayIf( !rootItem, "Tree not allocated in TreeView::updateTable()" );

   UInt16 tableRows = calcTableRows( table, chooser );
   totalItems = rootItem->getFolder()->getItemCount();

   if( skippedItems < 0 )     // if we page down, this could end up negative
      skippedItems = 0;

   if( skippedItems + tableRows > totalItems && totalItems >= tableRows )
      skippedItems = totalItems - tableRows;

   if( skippedItems > 0 && totalItems <= tableRows )
      skippedItems = 0;

   disableTable( table );
   
   // you have to always go up to 16 since you may have just resized the table
   //  and you want to make sure all the rows are properly enabled or disabled,
   //  otherwise, you'll only disable some of them, and the (now hidden) ones
   //  will try to draw and get all messed up.
//   for( int row=0; row<16; row++ )
   for( int row=0; row<tableRows; row++ )
      {
		if( row >= totalItems )
         TblSetRowUsable( table, row, false );			// so it won't get drawn
      else
         {
         TblSetItemPtr( table, row, 0, rootItem->getFolder()->getItem( row + skippedItems ));
         TblSetRowUsable( table, row, true );			// so it will get drawn
         TblMarkRowInvalid( table, row );					// so it will get updated
         }
      }

   if( redraw )
      TblDrawTable( table );
   treeViewUpdateScrollers( table, scrollBar );   
   }
      

/**
 * Just redraws the tree itself (which basically just calls the draw method
 * for each row.
 */
void TreeView::redrawTree( TablePtr table )
   {
	TblEraseTable( table );
	TblDrawTable( table );
   }


/**
 * The constructor that initializes the tree object.
 */
void TreeView::init()
   {
   skippedItems=0;
   expandedItems = 0;
   totalItems=0;
   rootItem = 0;
   filterOn = false;
   }
   

/**
 * Performs a complete re-reading of the device, re-creates the whole tree,
 * and then updates and draws the table. This method performs a complete
 * reload, and as such takes some time to run.
 */
void TreeView::reloadTree( TablePtr table, ScrollBarPtr scrollBar )
   {
   UInt16 tableRows = calcTableRows( table, chooser );

   // make sure the table doesn't try to redraw while we're loading the tree
   for( UInt16 i=0; i<tableRows; i++ )
      TblSetRowUsable( table, i, false ); 
   
   // read the whole tree again
   readRootItems( false );
   
   // and update the table on the screen
   updateTable( table, scrollBar, true );
   }


/**
 * Decide what color the file of the given type should be.
 *
 * @param the 4 byte type code
 * @return the color it should be
 */
RGBColorType TreeView::determineColor( Item *item )
	{
   // if it's not a color device, then don't color it (grayscale looks bad)
	if( colorDepth < 8 )
		return black;
	
   // vfs folders are always black, as are non-database vfs files
   if( item->getFolder())
      return black;
   
   // it must be a file if we're here
	// if this file has no database-related attributes
	if( !item->getFile()->iAttr )
		return black;
	
   // must be an internal file if we're here
	for( int i=0; i<prefs.list.typeColors.count; i++ )
		{      
		if( item->getFile()->iAttr->type == prefs.list.typeColors.types[i] )
			return prefs.list.typeColors.colors[i];
		}
	
	return black;
	}


void TreeView::drawItemIcon( Item *item, Int16 x, Int16 y )
   {
   UInt16   iconID = 0;
   
   if( item->getSelected())
      {
      if( item->getFile() )
         iconID = CheckFileOnBitmap;
      else
         {
         if( item->getFolder()->expanded )
            iconID = FolderExpOnBitmap;
         else
            iconID = FolderColOnBitmap;
         }
      }
   else
      {
      if( item->getFile() )
         iconID = CheckFileOffBitmap;
      else
         {
         if( item->getFolder()->expanded )
            iconID = FolderExpOffBitmap;
         else
            iconID = FolderColOffBitmap;
         }
      }

   DrawBitmap( iconID, x, y );
   }


/**
 * This is called to draw each row of the table.
 */
void TreeView::drawRecord( Int16 row, Int16 column, RectanglePtr boundss, TablePtr table )
	{
   RGBColorType	*textColor=NULL, *backColor=&white, col;

   RectangleType tableBounds;
   TblGetBounds( table, &tableBounds );
   Int16 tableWidth = tableBounds.extent.x;

   RectangleType rowBounds;
   Int16 x = rowBounds.topLeft.x = boundss->topLeft.x;
   Int16 y = rowBounds.topLeft.y = boundss->topLeft.y;
   rowBounds.extent.y = boundss->extent.y;
   rowBounds.extent.x = tableWidth;

   // Specify the background color of the row	
	if( row % 2 == 0 && colorDepth > 1 )
      {
      col = prefs.list.rowColor;
		backColor = &col;
      }
	
	// Draw the background rectangle
	if( colorDepth > 1 && colorMode != colorOldAPI )
		{
		ColorSet( backColor, NULL, NULL, NULL, NULL );
		WinDrawRectangle( &rowBounds, 0 );
		ColorUnset();
		}

   if( colorDepth == 1 )
		WinEraseRectangle( &rowBounds, 0 );      

   Item *item = (Item*)TblGlueGetItemPtr( table, row, 0 );
   if( !item )
      {
      showMessage( "No item in TreeView::drawRecord()." );
      return;
      }

	// Figure out what color this file should be drawn in
	RGBColorType	color;
	color = determineColor( item );
	textColor = &color;

   Int16 indentWidth = ( item->getIndent() * indentFactor );
   if( indentWidth < 0 ) indentWidth = 0;

   // move the x position to the proper indentation
   x += indentWidth;

   // based on the width of the table figure out what the column widths
   //  should be.  there are 3 columns: filename, attributes, and other.

   Int16 columnOtherWidth = 25;
   Int16 columnAttrWidth = 35;
   Int16 checkWidth = 10;
   Int16 columnFilenameWidth = tableWidth - ( indentWidth + checkWidth + columnOtherWidth + columnAttrWidth );

   if( prefs.list.column == colNone )
      columnFilenameWidth = tableWidth - ( indentWidth + checkWidth );

   drawItemIcon( item, x, y );
   x += checkWidth;

   if( !item->getName())
      {
      showMessage( "Item has no name in TreeView::drawRecord()." );
      return;
      }

   Boolean fits = false;
	Int16 textLen = StrLen( item->getName());
	Int16 width = columnFilenameWidth;
	FntCharsInWidth( item->getName(), &width, &textLen, &fits );
   
   ColorSet( 0, backColor, textColor, 0, 0 );
   // if the full filename does not fit, then we shorten it a little
   //  more so that we can draw an elipis after it.
   if( !fits )
      {
      fits = false;
      textLen = StrLen( item->getName());
      width = columnFilenameWidth - 5;
      FntCharsInWidth( item->getName(), &width, &textLen, &fits );
      WinDrawChars( item->getName(), textLen, x, y );
      WinDrawChars( "...", 3, x+width, y );
      }
   else
      WinDrawChars( item->getName(), textLen, x, y );
	ColorUnset();

   // don't draw anything else if we're not supposed to
   if( prefs.list.column == colNone )
      return;

   // move the x position to the start of the attributes column
   x += columnFilenameWidth;

	if( item->getFile() )
      {
      if( item->getFile()->iAttr )
         {
         if( item->getFile()->iAttr->id != 0 )
            {
            if( MemLocalIDKind( item->getFile()->iAttr->id ) != memIDHandle )
               DrawBitmap( ROMFlagBitmap, x, y );
            }
         if( item->getFile()->iAttr->attr & dmHdrAttrCopyPrevention )
            DrawBitmap( CopyFlagBitmap, x+9, y );
         if( item->getFile()->iAttr->attr & dmHdrAttrBackup )
            DrawBitmap( BackupFlagBitmap, x+20, y );
         }
      }

   // move the x position to the start of the second column
   x += columnAttrWidth;

   Char otherColumnText[64];
   getSecondColumnText( otherColumnText, item );
   
   ColorSet( 0, backColor, textColor, 0, 0 );
   WinDrawChars( otherColumnText, StrLen( otherColumnText ), x, y );
	ColorUnset();
   }


/**
 * This draws the second column of each row in the table.
 */
void TreeView::getSecondColumnText( Char *str, Item *item )
   {
   StrCopy( str, " - " );
   
   // this is a folder item, so draw the folder info in the right column
   if( item->getFolder() )
      {
      // i'm not sure what to draw for folders
      }

   // this is a file item, so draw the file info in the right column
   if( item->getFile() )
      {
      switch( prefs.list.column )
         {
         case colCreator:
            if( !item->getFile()->iAttr ) return;
            IntToStr( item->getFile()->iAttr->creator, str );
            break;
				
         case colType:
            if( !item->getFile()->iAttr ) return;
            IntToStr( item->getFile()->iAttr->type, str );				
            break;
				
         case colSize:
            SizeToString( item->getFile()->size, str, 5 );				
            break;
			
         case colRec:
            if( !item->getFile()->iAttr ) return;
            StrIToA( str, item->getFile()->iAttr->recCount );
            break;
			
         case colAttr:
            if( !item->getFile()->iAttr ) return;
            attrToString( str, item->getFile()->iAttr->attr );
            break;
			
         case colCreate:
            if( !item->getFile()->iAttr ) return;
            formatDateForTree( str, item->getFile()->iAttr->created );
            break;
				
         case colMod:
            if( !item->getFile()->iAttr ) return;
            formatDateForTree( str, item->getFile()->iAttr->modified );
            break;
				
         case colBackup:
            if( !item->getFile()->iAttr ) return;
            formatDateForTree( str, item->getFile()->iAttr->backedUp );
            break;
         }
      }
   }


/**
 * Makes a date string look nice.
 */
void TreeView::formatDateForTree( Char *dateStr, UInt32 seconds )
   {
   DateTimeType	d;
   Char           tempStr[64];

   if( !seconds )
      {
      StrCopy( dateStr, "Never" );
      return;
      }

   TimSecondsToDateTime( seconds, &d );
   DateToAscii( d.month, d.day, d.year, ShortDateFormat, tempStr );

   Char *strPtr = tempStr;
   
   // remove the year from the date string
   if(( ShortDateFormat == dfYMDWithSlashes ) ||
      ( ShortDateFormat == dfYMDWithDots ) ||
      ( ShortDateFormat == dfYMDWithDashes ))
      strPtr += 3;
   else
      strPtr[StrLen( tempStr ) - 3] = 0;
   
   StrCopy( dateStr, strPtr );
   }


/**
 * Re-sorts the tree. If op == true, then the user wants to sort by file name,
 * otherwise the user wants to sort by whatever is currently displayed in the
 * second column (size, type, etc.).
 */
void TreeView::reSortTree( Boolean op )
   {
	if( op )
		{
		if( prefs.list.sortOrder == sortNameAZ )
			prefs.list.sortOrder = sortNameZA;
		else
			prefs.list.sortOrder = sortNameAZ;
		}
	else
		{
		if( prefs.list.sortOrder == sortOtherAZ )
			prefs.list.sortOrder = sortOtherZA;
		else
			prefs.list.sortOrder = sortOtherAZ;
		}

   reSortTree();
   }


// just resorts the list as is
void TreeView::reSortTree()
   {
	PrefSetAppPreferences( appCreatorID, appPrefID, appPrefVersionNum, &prefs, sizeof(prefs), true );
   rootItem->getFolder()->sortItems();
   }
   

/**
 * Sets all visible items to selected or not selected, depending on the
 * argument.
 */
void TreeView::selectAll( Boolean on )
   {
   rootItem->setAllSelected( on );
   }


void TreeView::collapseAll( Boolean expand )
   {
   free();
   readRootItems( expand );
   }


/**
 * Return true/false if the filter is currently on or not.
 */
Boolean TreeView::getFilterOn()
   {
   return filterOn;
   }


/**
 * Handles what happens when the user presses a key. We only want to handle
 * key presses here that scroll the list. The other key presses should be
 * handled by the form itself.
 */
Boolean TreeView::handleKeyDown( EventPtr event, TablePtr table, ScrollBarPtr scrollBar )
   {
   Boolean handled = false;

	// handle if a key wasn't actually pressed
	if( EvtKeydownIsVirtual( event ))
		{
		// handle the scroll hardware buttons
		switch( event->data.keyDown.chr )
			{
			// since the jog dial spins, we just scroll one line for each event
/*
			case vchrJogUp:
				treeViewScroll( table, scrollBar, -1, false );
				break;				
			case vchrJogDown:
				treeViewScroll( table, scrollBar, 1, false );
				break;
*/
			
			// these events should be received for all devices, scrolls one page
			case vchrPageUp:
				treeViewScroll( table, scrollBar, -1, true );
				handled = true;
				break;
			case vchrPageDown:
				treeViewScroll( table, scrollBar, 1, true );
				handled = true;
				break;
			}
		}

	// handle if an actual key was pressed by scrolling to the first file/folder
	//  that starts with that character.
	// we need to the modifiers check because the 5-way nav pad issues extra key down events
	//  that are mistakenly considered as print characters.
	if( TxtGlueCharIsPrint( event->data.keyDown.chr ) && event->data.keyDown.modifiers != commandKeyMask )
		{
		jumpToLetter( event->data.keyDown.chr, table, scrollBar );
		}
	
   return handled;
   }


/**
 * Handles repeat events (i.e. the scrollers).
 */
Boolean TreeView::handleRepeatEvent( EventPtr event, TablePtr table, ScrollBarPtr scrollBar )
   {
   treeViewScroll( table, scrollBar, event->data.sclRepeat.newValue - event->data.sclRepeat.value, false );
   return false;
   }


/**
 * Handles what happens when the user taps on an item in the tree/table.
 */
Boolean TreeView::handleTableEnterEvent( EventPtr event, TablePtr table, ScrollBarPtr scrollBar )
   {
   UInt16   row = event->data.tblSelect.row;
   //UInt16   column = event->data.tblSelect.column;

   Item *item = (Item*)TblGlueGetItemPtr( table, row, 0 );
   if( !item )
      {
      showMessage( "Error: unable to find item" );
      return true;
      }

   UInt16   x = event->screenX;
   //UInt16   y = event->screenY;
   Boolean  tappedFolderIcon = false, userSelecting = false;

   if( item->getFolder())
      {
      if( x < item->getIndent() * indentFactor + 10 )
         tappedFolderIcon = true;

      // so now we do the logic of how to interpret the user's tap
      // if they tapped the icon, and that is the selection action, then
      //  we're selecting.
      if( tappedFolderIcon && prefs.list.folderSelect == folderSelectIcon )
         userSelecting = true;

      // or, if they tapped the folder name, and that is the selection action,
      //  then we're selecting.
      if( !tappedFolderIcon && prefs.list.folderSelect == folderSelectName )
         userSelecting = true;
      
      if( userSelecting )
         {
         // this selects/deselects it
         item->setSelected( !item->getSelected());
         TblMarkRowInvalid( table, row );
         TblRedrawTable( table );
         }
      else
         {
//showMessage( "1" );
         // this expands/collapses it
         if( item->getFolder()->expanded )
            {
            item->getFolder()->expanded = false;
            item->getFolder()->freeItems();
            }
         else
            {
//showMessage( "2" );
            item->getFolder()->expanded = true;
            if( item->getFolder()->getIsVolume() == volInternal )
					{
               item->getFolder()->readPalmCard( item->getIndent(), chooser, filterOn );
//showMessage( "3" );
					}
            else
					{
               item->getFolder()->readItems( expandedItems, item->getIndent(), chooser, filterOn, false );
//showMessage( "4" );
					}
				}

//showMessage( "5" );

         updateTable( table, scrollBar, true );
         }
      }
   else
      {
      // if it's a file, then we select/deselect it
      item->setSelected( !item->getSelected());
      TblMarkRowInvalid( table, row );
      TblRedrawTable( table );
      }

   return true;
   }
   
   
/**
 * Disables the tree so that the rows are not redrawn.
 */
void TreeView::disableTable( TablePtr table )
   {
   //UInt16 tableRows = calcTableRows( table, chooser );
   UInt16 tableRows = 16;

   for( int i=0; i<tableRows; i++ )
      TblSetRowUsable( table, i, false );
   }


/**
 * Handles any events that should be handled by the tree. The updateCode
 * should be set to nonzero if an update is necessary. It will be passed on to
 * the owner form after this method returns to update the tree UI.
 */
Boolean TreeView::handleEvent( EventPtr event, TablePtr table, ScrollBarPtr scrollBar )
   {
   bool handled = false;

   switch( event->eType )
		{
      case sclRepeatEvent:
			handled = handleRepeatEvent( event, table, scrollBar );
			break;

      case tblEnterEvent:
         handled = handleTableEnterEvent( event, table, scrollBar );
         break;

      case keyDownEvent:
         handled = handleKeyDown( event, table, scrollBar );
         break;
      default:
         break;
      }

   return handled;
   }
