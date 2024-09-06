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
 * This file contains all the code to support the preference
 *	viewer. The code in this file was taken from PrefEdit
 *	(under GPL) written by Bodo Bellut (bodo@bellut.net).
 *
 * Created on 1/3/03 by Tom Bulatewicz
 */

#ifndef __prefform_h__
#define __prefform_h__

#include "Sections.hpp"

Boolean PrefListHandleEvent( EventPtr e ) SECT8;
Boolean PrefViewHandleEvent( EventPtr e ) SECT8;

#endif
