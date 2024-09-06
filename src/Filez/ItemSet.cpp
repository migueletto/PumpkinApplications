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

#include "Item.hpp"
#include "ItemFile.hpp"
#include "ItemFolder.hpp"
#include "ItemSet.hpp"

#include "Stuph.h"            // for error checking


void ItemSet::addItem( Item *item )
   {
   Err   err = errNone;

   if( count == 0 )
      {
      if( items )
         showMessage( "items is not null in ItemSet::addItem()" );
      items = (Item**)MemPtrNew( sizeof( Item* ));
      checkMemPtr( items, "ItemSet::addItem()" );
      }
   else
      {
      Item **old = items;
      items = (Item**)MemPtrNew( sizeof( Item* ) * (count + 1));
      err = MemMove( items, old, sizeof( Item* ) * count );
      checkError( err, "ItemSet::addItem() a", 0 );
      err = MemPtrFree( old );
      checkError( err, "ItemSet::addItem() b", 0 );
      }

   items[count] = item;
   count++;
   }
   

Boolean ItemSet::isExpanded( Char *path, Char *name, UInt32 volumeNum )
   {
   for( int i=0; i<count; i++ )
      {
      if( items[i]->getFolder())
         {
         if( volumeNum == items[i]->getVolumeNum() && !StrCompare( path, items[i]->getFolder()->path ) && !StrCompare( name, items[i]->getName() ) )
            return true;
         }
      }

   return false;
   }


ItemSet::~ItemSet()
   {
   Err   err = 0;

   if( items )
      {
      err = MemPtrFree( items );
      checkError( err, "ItemSet::~ItemSet()", 0 );
      }
   count = 0;
   items = 0;
   }
   

UInt16 ItemSet::size()
   {
   return count;
   }


Item* ItemSet::getItem( UInt16 i )
   {
   return items[i];
   }


ItemSet::ItemSet()
   {
   count = 0;
   items = 0;
   }
