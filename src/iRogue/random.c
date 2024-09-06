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

/***************************************************************
                   SEED_RANDOM
 IN:
 nothing
 OUT:
 nothing
 PURPOSE:
 This should be called at the start of each game session BEFORE
 doing anything that generates random dungeon guts.  It will
 seed the random number generator.
   (SysRandom returns a random Int between 0 and 'sysRandomMax'
 and takes one argument which, if nonzero, is used to (re)seed it.)
****************************************************************/
void seed_random() {
  SysRandom(TimGetSeconds());
}

/***************************************************************
                   GET_RAND
 IN:
 x, y = bounds for the return value
 OUT:
 a random number
 PURPOSE:
 Used to obtain a random number between x and y INCLUSIVE.
****************************************************************/
Int get_rand(Int x, Int y) {
  Int r, t;
  if (x == y) return x;
  if (x > y) {
    t = y; y = x; x = t;
  }
  t = (y-x) + 1;
  r = SysRandom(0);
  r = (r % t) + x;
  return r;
}

/***************************************************************
                   RAND_PERCENT
 IN:
 percentage = event's chance of "success" (think percentile dice.)
 OUT:
 true if a number <= the goal is "rolled"
 PURPOSE:
 Caller is contemplating an event with a percentage% probability
 of occurring; determine whether the event will occur.
****************************************************************/
Boolean rand_percent(Int percentage) {
  return(get_rand(1, 100) <= percentage);
}

/***************************************************************
                   COIN_TOSS
 IN:
 nothing
 OUT:
 true if the virtual coin came up "heads"
 PURPOSE:
 Toss a coin... is it heads or tails?
****************************************************************/
Boolean coin_toss() {
  return(((SysRandom(0) % 2) ? true : false));
}

/***************************************************************
                   RAND_AROUND
 IN:
 i = 
 r, c = initial location
 OUT:
 nothing (really r,c modified to be an adjacent location)
 PURPOSE:
 If i is nonzero, this will return (in r,c) a random choice of
 nine locations (eight adjacent squares plus original square.)
 It does not bounds-check the returned location.
 (If i is zero, it re-shuffles its "random" numbers.)
****************************************************************/
/* oddly, this was in throw.c */
void rand_around(Short i, Short *r, Short *c) {
  static Char pos[] = "\010\007\001\003\004\005\002\006\0";
  static Short row, col;
  Short j;

  if (i == 0) {
    Short x, y, o, t;

    row = *r;
    col = *c;

    o = get_rand(1, 8);

    for (j = 0; j < 5; j++) {
      x = get_rand(0, 8);
      y = (x + o) % 9;
      t = pos[x];
      pos[x] = pos[y];
      pos[y] = t;
    }
  }
  switch((short)pos[i]) {
  case 0:
    *r = row + 1;
    *c = col + 1;
    break;
  case 1:
    *r = row + 1;
    *c = col - 1;
    break;
  case 2:
    *r = row - 1;
    *c = col + 1;
    break;
  case 3:
    *r = row - 1;
    *c = col - 1;
    break;
  case 4:
    *r = row;
    *c = col + 1;
    break;
  case 5:
    *r = row + 1;
    *c = col;
    break;
  case 6:
    *r = row;
    *c = col;
    break;
  case 7:
    *r = row - 1;
    *c = col;
    break;
  case 8:
    *r = row;
    *c = col - 1;
    break;
  }
}
