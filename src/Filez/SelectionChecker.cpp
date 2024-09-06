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

#include <PalmOS.h>
#include <VFSMgr.h>

#include "Stuph.h"            // for error checking
#include "SelectionChecker.hpp"
#include "Item.hpp"
#include "ItemFile.hpp"
#include "ItemFolder.hpp"
#include "ItemSet.hpp"
#include "CardSet.hpp"

Boolean SelectionChecker::check( UInt8 num, UInt8 inex, UInt8 filefolder, TreeView *tree, Boolean requiresCards, UInt8 volumesOK )
   {
   Char msg[256];
   ItemSet *items = tree->getSelectedItems();

   // check to see if we need more than one card for this operation
   if( requiresCards && cardSet.size() == 1 )
      {
      showMessage( "This operation requires at least one external card" );
      return false;
      }

   // check the number of selected items first
   if( items->size() == 0 )
      {
      showMessage( "Please select an item" );
      return false;
      }
   
   if( items->size() > 1 && num == selectionOne )
      {
      showMessage( "This operation allows only one item to be selected" );
      return false;
      }
      
   for( int i=0; i<items->size(); i++ )
      {
      Item *item = items->getItem( i );

      if( volumesOK == selectionVolNo && item->getFolder() )
         {
         if( item->getFolder()->getIsVolume())
            {
            StrPrintF( msg, "This operation can't be performed on a card (%s)", item->getName());
            showMessage( msg );
            return false;
            }
         }
      
      // if we only allow internal items, but some items are vfs files
      if( inex == selectionInternal && item->getFolder() )
         {
         StrPrintF( msg, "This operation allows only internal files to be selected (%s)", item->getName());
         showMessage( msg );
         return false;
         }
      
      // if we only allow internal items, but some items are vfs files
      if( inex == selectionInternal && item->getFile())
         if( item->getFile()->eAttr )
            {
            StrPrintF( msg, "This operation allows only internal files to be selected (%s)", item->getName());
            showMessage( msg );
            return false;
            }
      
      // if we only allow external items, but some items are internal files
      if( inex == selectionExternal ) {
         if( item->getFile())
            {
            // if this is a file they selected
            if( !item->getFile()->eAttr )
               {
               StrPrintF( msg, "This operation allows only external items to be selected (%s)", item->getName());
               showMessage( msg );
               return false;
               }
            }
         else
            {
            // if this is a folder they selected, we can't assume that it's
            //  external since the internal folder is a folder too.
            if( item->getFolder()->getIsVolume() == volInternal )
               {
               StrPrintF( msg, "This operation allows only external items to be selected (%s)", item->getName());
               showMessage( msg );
               return false;
               }
            }
      }
            
      // if we only allow files, but some items are folders
      if( filefolder == selectionFiles && item->getFolder() )
         {
         StrPrintF( msg, "This operation allows only files to be selected (%s)", item->getName());
         showMessage( msg );
         return false;
         }
      
      // if we only allow folders, but some items are files
      if( filefolder == selectionFolders && item->getFile())
         {
         StrPrintF( msg, "This operation allows only folders to be selected (%s)", item->getName());
         showMessage( msg );
         return false;
         }      
      }
   
   delete items;
   return true;
   }
