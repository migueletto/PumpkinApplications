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
 * Handles the filtering of files.
 *
 * Created on 5/25/01 by Tom Bulatewicz
 */

#ifndef __filterform_h__
#define __filterform_h__

#include "Sections.hpp"

#define	filterBegins	0
#define	filterContains	1
#define	filterEnds		2

#define	filterName	0
#define	filterCreator	1
#define	filterType		2
#define	filterSize		3
#define	filterRec		4
#define	filterAttr		5

Boolean FilterHandleEvent (EventPtr event) SECT7;

#endif
