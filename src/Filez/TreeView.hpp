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
 * The definition of the TreeView object.
 */
 
#ifndef __treeview_h__
#define __treeview_h__

#include <PalmOS.h>					// all the system toolbox headers
#include "Sections.hpp"

class Item;
class ItemSet;

// different things the other column can show
#define	colCreator	0
#define	colType		1
#define	colSize		2
#define	colRec		3
#define	colAttr		4
#define  colCreate	5
#define  colMod		6
#define  colBackup	7
#define  colNone     8

#define	sortNameAZ	0
#define	sortNameZA	1
#define	sortOtherAZ	2
#define	sortOtherZA	3

class TreeView
   {
   public:

   void init() SECT3;
   void setup( TablePtr tableP, Boolean chooser ) SECT3;
   void redrawTree( TablePtr table ) SECT3;
   void reloadTree( TablePtr table, ScrollBarPtr scrollBar ) SECT3;
   void updateTable( TablePtr table, ScrollBarPtr scrollBar, Boolean redraw ) SECT3;
   void treeViewScroll( TablePtr table, ScrollBarPtr scrollBar, Int16 dir, Boolean fullPage ) SECT3;
   void free() SECT3;
   void rememberExpandedItems() SECT3;
   ItemSet *getSelectedItems() SECT3;
   void jumpToLetter( Char ch, TablePtr table, ScrollBarPtr scrollBar ) SECT3;
   void drawRecord( Int16 row, Int16 column, RectanglePtr bounds, TablePtr table ) SECT3;
   void sendSelectedItems() SECT3;
   void sendToMemo( TableType *table ) SECT3;
   void sendSelectedFiles() SECT3;
   void setAllAttributes( UInt16 attr, Boolean set ) SECT3;
   void reSortTree( Boolean op ) SECT3;
   void reSortTree() SECT3;
   void filterTree() SECT3;
   void selectAll( Boolean on ) SECT3;
   Boolean getFilterOn() SECT3;
   Boolean handleEvent( EventPtr event, TablePtr t, ScrollBarPtr s ) SECT3;
   void collapseAll( Boolean expand ) SECT3;

   private:

   void readRootItems( Boolean expandAll ) SECT3;
   void initTable( TablePtr table ) SECT3;
   void formatDateForTree( Char *dateStr, UInt32 seconds ) SECT3;
   RGBColorType determineColor( Item *item ) SECT3;
   void getSecondColumnText( Char *str, Item *item ) SECT3;
   void drawItemIcon( Item *item, Int16 x, Int16 y ) SECT3;
   void treeViewUpdateScrollers( TablePtr table, ScrollBarPtr scrollBar ) SECT3;
   void disableTable( TablePtr table ) SECT3;
   Boolean handleRepeatEvent( EventPtr event, TablePtr t, ScrollBarPtr s ) SECT3;
   Boolean handleTableEnterEvent( EventPtr event, TablePtr t, ScrollBarPtr s ) SECT3;
   Boolean handleKeyDown( EventPtr event, TablePtr t, ScrollBarPtr s ) SECT3;
   Boolean handleSelectAllEvent( Boolean on, EventPtr event ) SECT3;
   
   Int32          skippedItems;
   UInt16         totalItems;

   Boolean        filterOn;
   Boolean        chooser;

   Item           *rootItem;
   ItemSet        *expandedItems;
   };

#endif
