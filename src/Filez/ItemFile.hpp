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

#ifndef __itemfile_h__
#define __itemfile_h__

#include <PalmOS.h>					// all the system toolbox headers
#include "Sections.hpp"
#include "Item.hpp"
#include "ItemFolder.hpp"
#include "Main.h"                // for the DEBUG define

class InternalAttr
   {
   public:
   
	UInt32		type;						// 4-byte file type
	UInt32		creator;					// 4-byte creator
	UInt32		recCount;				// record count
	LocalID		id;						// unique id
	UInt16		attr;						// attributes
   UInt16      version;
	UInt32      created;          // creation date
	UInt32      modified;         // modified date
	UInt32      backedUp;         // last backup date
   };
   

// all vfs files will have an external attr object
// this is how we tell the difference between types of files
class ExternalAttr
   {
   public:
   
	UInt32		attr;                // used by vfs
	UInt32      created;             // creation date
	UInt32      modified;            // modified date
	UInt32      accessed;            // last accessed
   };


class ItemFile
   {
   public:
   
   void updateFile( Char *oldName ) SECT4;         // updates the actual file itself
   void init() SECT4;
   void free() SECT4;
   void setFileName( Char *newName ) SECT4;
   
   void setVolumeNum( UInt32 volumeNum ) SECT4;
   UInt32 getVolumeNum() SECT4;
   void setSize( UInt32 size ) SECT4;
   UInt32 getSize() SECT4;
   
   Char           *name;            // file name
	UInt32         size;             // file size in bytes
   ItemFolder     *parent;
   InternalAttr   *iAttr;           // pdb/prc attributes
   ExternalAttr   *eAttr;           // VFS file attributes   

   private:

   UInt32         volumeNum;        // card or volume number
   };

#endif
