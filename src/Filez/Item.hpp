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

#ifndef __item_h__
#define __item_h__

#include <PalmOS.h>					// all the system toolbox headers
#include "Sections.hpp"
#include "BusyIndicator.hpp"

class ItemFolder;
class ItemFile;

class Item
   {
   public:
   
   void updateFile( Char *oldName ) SECT3;
   void init() SECT3;
   void free() SECT3;
   Boolean remove( Boolean ask, Err &err ) SECT3;
   void filter( Boolean turnOn ) SECT3;
   void filterCheck() SECT3;
   void setAllSelected( Boolean on ) SECT3;
   void setAllAttributes( UInt16 attr, Boolean set ) SECT3;
   void sendToMemo( MemHandle textH, UInt8 depth, BusyIndicator *busyIndicator ) SECT3;
   void send() SECT3;
   void sendSelectedItems() SECT3;
   
   // accessor methods for local data members
   void setSelected( Boolean s ) SECT3;
   Boolean getSelected() SECT3;
   void setFiltered( Boolean f ) SECT3;
   Boolean getFiltered() SECT3;
   void setIndent( Int8 i ) SECT3;
   Int8 getIndent() SECT3;
   void setFile( ItemFile *f ) SECT3;
   ItemFile* getFile() SECT3;
   void setFolder( ItemFolder *f ) SECT3;
   ItemFolder* getFolder() SECT3;

   // accessor methods for file/folder data members
   void setName( Char *name ) SECT3;
   Char *getName() SECT3;
   void setVolumeNum( UInt32 volumeNum ) SECT3;
   UInt32 getVolumeNum() SECT3;

   private:

   ItemFile    *file;
   ItemFolder  *folder;
   Boolean     selected;               // did the user select this item?
   Boolean     filtered;
   Int8        indent;

   Boolean filterStringCompare( Char s1[], Char s2[] ) SECT3;
   Boolean filterNumberCompare( UInt32 i1, UInt32 i2 ) SECT3;
   Boolean filterAttrCompare( UInt16 attr ) SECT3;
   Err sendVFSFile() SECT3;
   Err sendDBFile() SECT3;
   };

#endif