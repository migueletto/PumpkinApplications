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

// Sections.hpp
/*
 segment map
 Segment 1 - 
 Segment 2 - Main.c
 Segment 3 - TreeView.hpp Item.hpp ItemSet.hpp
 Segment 4 - ItemFolder.hpp ItemFile.hpp CardSet.hpp
 Segment 5 - SelectionChecker.hpp
 Segment 6 - TreeViewForm.cpp InformationForm.c PreferencesForm.c
 Segment 7 - MenuForm.c DetailsRForm.c DetailsVFSForm.c FolderDetails.c FilterRForm.c
 Segment 8 - Chooser.c RecListForm.c PrefForm.c HexForm.c SetAttributes.c
*/


#ifndef __sections_hpp__
#define __sections_hpp__

//#ifdef __MWERKS__

#define SECT2
#define SECT3
#define SECT4
#define SECT5
#define SECT6
#define SECT7
#define SECT8

//#else

//#define SECT2 __attribute__ ((section("SECT2")))
//#define SECT3 __attribute__ ((section("SECT3")))
//#define SECT4 __attribute__ ((section("SECT4")))
//#define SECT5 __attribute__ ((section("SECT5")))
//#define SECT6 __attribute__ ((section("SECT6")))
//#define SECT7 __attribute__ ((section("SECT7")))
//#define SECT8 __attribute__ ((section("SECT8")))

//#endif

#endif
