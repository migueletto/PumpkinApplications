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

#ifndef __itemset_h__
#define __itemset_h__

#include <PalmOS.h>					// all the system toolbox headers
#include "Sections.hpp"

class ItemFolder;
class ItemFile;

class ItemSet
   {
   private:

   Item        **items;
   UInt16      count;

   public:

   ItemSet() SECT3;
   ~ItemSet() SECT3;

   void     addItem( Item *item ) SECT3;
   Item*    getItem( UInt16 i ) SECT3;
   UInt16   size() SECT3;
   Boolean  isExpanded( Char *path, Char *name, UInt32 volumeNum ) SECT3;
   };

#endif