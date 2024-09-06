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
/*
 * object.c
 *
 * This source herein may be modified and/or distributed by anybody who
 * so desires, with the following restrictions:
 *    1.)  No portion of this notice shall be removed.
 *    2.)  Credit shall not be taken for the creation of this source.
 *    3.)  This code is not to be traded, sold, or used for personal
 *         gain or profit.
 *
 */

#include "palm.h"
#include "iRogueRsc.h"
#include "Globals.h"

#include "rogue.h"

static void make_party() SEC_1;
static void rand_place(object *obj) SEC_1;
static Short next_party() SEC_1;

extern struct state_of_the_union * sotu;

object * level_objects;
UShort ** dungeon;

/* id_foo are now dynamically allocated. */

extern Char *error_file;


/**********************************************************************
                       PUT_OBJECTS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Sometimes, randomly create + place a "party" of monsters in a "party room."
 Randomly create and place objects throughout the dungeon.  Ditto gold.
 **********************************************************************/
void put_objects() {
  Short i, n;
  object *obj;

  if (sotu->cur_level < sotu->max_level) {
    return;
  }
  n = coin_toss() ? get_rand(2, 4) : get_rand(3, 5);
  while (rand_percent(33)) {
    n++;
  }

  /* I guess this is in the object file because a party room
     tends to have extra treasure */
  if (sotu->cur_level == sotu->party_counter) {
    make_party();
    if ( ((sotu->ring_flags & (RING_WARNING | RING_ESP))
	  && rand_percent(90))
	 || ((sotu->ring_flags & (RING_MOOD | RING_DELUSION))
	     && rand_percent(40)) )
      message("you hear something...", sotu);
    sotu->party_counter = next_party();
  } else if ((sotu->ring_flags & RING_DELUSION)
	     && rand_percent(20)) {
    // careful on the false positives, since there are more !party than party.
    message("you hear something...", sotu);
  }

  for (i = 0; i < n; i++) {
    obj = gr_object(sotu);
    rand_place(obj);
  }

  put_gold(sotu);
}


/************************************************
  plant_gold MOVED to LibRogue
  place_at MOVED to LibRogue
************************************************/

/************************************************
  object_at MOVED to LibRogue
  free_stuff ditto
  get_nth_object ditto (replacement for get_letter_object)
  ************************************************/

/************************************************
  name_of MOVED to lib_object.c
************************************************/

/*************************************************
  moved:
  gr_object
  gr_what_is
  gr_scroll
  gr_potion
  gr_weapon
  gr_armor
  gr_wand
  get_food
*************************************************/

/************************************************
  put_stairs MOVED to LibRogue
  get_armor_class MOVED to LibRogue
************************************************/

/************************************************
  alloc_object MOVED to LibRogue
  free_object MOVED to LibRogue
************************************************/


/**********************************************************************
                       MAKE_PARTY
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Call other stuff to make a party room....
 **********************************************************************/
static void make_party() {
  Short n;

  sotu->party_room = gr_room(sotu->rooms);

  n = rand_percent(99) ? party_objects(sotu->party_room, sotu) : 11;
  if (rand_percent(99)) {
    party_monsters(sotu->party_room, n);
  }

  /* party_monsters is in monster.c.  oddly, party_objects is in room.c */

}


/************************************************
  show_objects MOVED to libRogue/use.c
************************************************/

/**********************************************************************
                       PUT_AMULET
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Place the amulet of yendor!
 would this be better inline? nah
 **********************************************************************/
void put_amulet() {
  object *obj;

  obj = alloc_object(sotu);
  obj->what_is = AMULET;
  rand_place(obj);
}

/**********************************************************************
                       RAND_PLACE
 IN:
 object = the thing to put somewhere
 various globals
 OUT:
 nothing
 PURPOSE:
 Place the object at a random dungeon location...
 **********************************************************************/
static void rand_place(object *obj) {
  Short row, col;

  gr_row_col(&row, &col, (FLOOR | TUNNEL), sotu);
  place_at(obj, row, col, sotu);
}

/* MOVED new_object_for_wizard */

/**********************************************************************
                       NEXT_PARTY
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Calculate the next time to have a party room...
 **********************************************************************/
static Short next_party() {
  int n;

  n = sotu->cur_level;
  while (n % PARTY_TIME) {
    n++;
  }
  return (get_rand((n + 1), (n + PARTY_TIME)));
}
