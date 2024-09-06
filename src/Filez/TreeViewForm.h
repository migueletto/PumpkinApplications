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

#ifndef __treeviewform_h__
#define __treeviewform_h__

#include <PalmOS.h>					// all the system toolbox headers
#include "Sections.hpp"
#include "TreeView.hpp"

extern TreeView *tree;

// these are my own update codes
#define frmReloadTreeUpdateCode        0x0002
#define frmRedrawTreeUpdateCode			0x0004
#define frmFilterTreeUpdateCode        0x0008
#define frmReSortTreeUpdateCode			0x0016

Boolean TreeViewHandleEvent( EventPtr event ) SECT6;
//Boolean GotoFormHandleEvent( EventPtr event ) SECT6;
void freeTree() SECT6;

#endif
