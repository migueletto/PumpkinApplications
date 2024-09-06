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

#ifndef __itemfolder_h__
#define __itemfolder_h__

#include <PalmOS.h>					// all the system toolbox headers
#include <VFSMgr.h>
#include "Sections.hpp"
#include "Item.hpp"

#define volInternal  1
#define volExternal  2

class ItemSet;

class ItemFolder
   {
   public:
   
   UInt32 getItemCount() SECT4;
   Item* getItem( UInt32 target ) SECT4;
   void readItems( ItemSet *expandedItems, Int8 indent, Boolean chooser, Boolean filterOn, Boolean expandAll ) SECT4;
   void sortItems() SECT4;
   void init() SECT4;
   void free() SECT4;
   void remove( Err &err ) SECT4;
   void filter( Boolean turnOn ) SECT4;
   void setAllSelected( Boolean on ) SECT4;
   void setAllAttributes( UInt16 attr, Boolean set ) SECT4;
   void sendToMemo( MemHandle textH, UInt8 depth, BusyIndicator *busyIndicator ) SECT4;
   void getSelectedItems( ItemSet *itemSet ) SECT4;
   void setFolderName( Char *newName ) SECT4;
   void sendSelectedItems() SECT4;
   Int32 findItemWithLetter( Char ch ) SECT4;
   void rememberExpandedItems( ItemSet *expandedItems ) SECT4;
   void readCards( ItemSet *expandedItems, Int8 indent, Boolean chooser, Boolean filterOn, Boolean expandAll ) SECT4;
   void readPalmCard( Int8 indent, Boolean chooser, Boolean filterOn ) SECT4;
   void freeItems() SECT4;

   // accessor methods
   void setPath( Char *pathP, Char *folderName ) SECT4;
   void setPath( Char *pathP ) SECT4;
   Char *getPath() SECT4;
   void setVolumeNum( UInt16 volumeNum ) SECT4;
   UInt16 getVolumeNum() SECT4;
   void setIsVolume( UInt8 vol ) SECT4;
   UInt8 getIsVolume() SECT4;

   Char        *name;               // file name
   Char        *path;               // folder's path ( e.g. /Palm/Photos/ )
   Item        *items;
   UInt32      itemCount;
   Boolean     expanded;            // is this folder expanded in the tree view

   private:

   UInt16      volumeNum;           // the volume on which this folder resides
   UInt8       isVolume;            // is this folder actually a volume? see volume defines above

   UInt32 getItemCountHelper( UInt32 count ) SECT4;
   Item* getItemHelper( UInt32 &count, UInt32 target ) SECT4;
   void getSelectedItemsHelper( ItemSet *itemSet ) SECT4;
   Int32 findItemWithLetterHelper( Char ch, Int32 count ) SECT4;
   Err MyVFSFileDBInfo( Char *fileName, FileRef ref, Char *name, UInt16 *attributes, UInt16 *version, UInt32 *crDate, UInt32 *modDate, UInt32 *bckUpDate, UInt32 *type, UInt32 *creator );

   };

typedef struct
   {
   UInt8    name[dmDBNameLength];
   UInt16   attributes;
   UInt16   version;
   UInt32   creationDate;
   UInt32   modificationDate;
   UInt32   lastBackupDate;
   UInt32   modificationNumber;
   LocalID  appInfoID;
   LocalID  sortInfoID;
   UInt32   type;
   UInt32   creator;
   UInt32   uniqueIDSeed;
   } DatabaseHdrType;

#endif