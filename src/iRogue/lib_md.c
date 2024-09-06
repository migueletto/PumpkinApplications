/*********************************************************************
 * iRogue - Rogue adapted for the PalmPilot.                         *
 * Copyright (C) 1999 Bridget Spitznagel                             *
 * Note: This program is derived from rogue 5.3-clone for Linux.     *
 *                                                                   *
 * This program is free software; you can redistribute it and/or     *
 * modify it under the terms of the GNU General Public License       *
 * as published by the Free Software Foundation; either version 2    *
 * of the License, or (at your option) any later version.            *
 *                                                                   *
 * This program is distributed in the hope that it will be useful,   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License for more details.                      *
 *                                                                   *
 * You should have received a copy of the GNU General Public License *
 * along with this program; if not, write to                         *
 * The Free Software Foundation, Inc.,                               *
 * 59 Temple Place - Suite 330,                                      *
 * Boston, MA  02111-1307, USA.                                      *
 *********************************************************************/

#include "palm.h"
#include "librogue.h"

/* formerly "machine-dependent" */

/***************************************************************
                   x
 IN:
 n = size to allocate
 OUT:
 pointer to the locked chunk
 PURPOSE:
 Allocate and lock a moveable chunk of memory.
****************************************************************/
Char * md_malloc(Int n) {
  VoidHand h;
  VoidPtr p;

  h = MemHandleNew((ULong) n); /* will this cast work??  apparently. */
  if (!h) {
    /* the caller might want to check this and die. */
    return NULL;
  }

  p = MemHandleLock(h);
  MemSet(p, n, 0); /* just to make really sure the memory is zeroed */
  return p;
}
