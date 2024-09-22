/* Emacs style mode select   -*- C -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze, Andrey Budko
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Ex Demo Wad Table
 *
 *---------------------------------------------------------------------
 */

#ifndef __WADTBL__
#define __WADTBL__

#include "doomtype.h"
#include "w_wad.h"

typedef struct
{
  wadinfo_t header;
  filelump_t *lumps;
  char* data;
  int datasize;
} wadtbl_t;

#define PWAD_SIGNATURE "PWAD"

void InitPWADTable(wadtbl_t *wadtbl);
void FreePWADTable(wadtbl_t *wadtbl);
void AddPWADTableLump(wadtbl_t *wadtbl, const char *name, const byte* data, size_t size);
wadinfo_t *ReadPWADTable(char *buffer, size_t size);

#endif
