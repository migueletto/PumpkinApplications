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

#ifndef __selectionchecker_h__
#define __selectionchecker_h__

#include "Sections.hpp"
#include "TreeView.hpp"

#define selectionOne       1     // only one item can be selected
#define selectionMultiple  2     // multiple items can be selected
#define selectionInternal  3     // only internal items can be selected
#define selectionExternal  4     // only external items can be selected
#define selectionIntExt    5     // allow both internal and external to be selected
#define selectionFiles     6     // allow only files to be selected
#define selectionFolders   7     // allow only folders to be selected
#define selectionFileFold  8     // allow both files and folders to be selected
#define selectionVolOK     9     // volumes are ok
#define selectionVolNo     10    // volumes cannot be selected

class SelectionChecker
	{
   public:

   static Boolean check( UInt8 num, UInt8 inex, UInt8 filefolder, TreeView *tree, Boolean requiresCards, Boolean volumesOK ) SECT5;
	};
   
#endif
