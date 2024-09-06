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
 * trap.c
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

extern struct state_of_the_union * sotu;

trap * traps;

/************************************************
  trap_at MOVED to LibRogue
************************************************/


static Char trap_names[TRAPS][18] = {
  "trap door",
  "bear trap",
  "teleport trap",
  "poison dart trap",
  "sleeping gas trap",
  "rust trap"
};
/*
static Char trap_effects[TRAPS][55] = {
  "you fell down a trap",
  "you are caught in a bear trap",
  "teleport",
  "a small dart hit you in the shoulder",
  "a strange white mist envelops you and you fall asleep",
  "a gush of water hits you on the head"
};
*/
Char you_fell[21] = "you fell down a trap";

/**********************************************************************
                       TRAP_PLAYER
 IN:
 row, col = location of rogue
 various globals?
 OUT:
 nothing
 PURPOSE:
 What to do when the rogue steps on a trap.
 **********************************************************************/
void trap_player(Short row, Short col) {
  fighter *rogue = sotu->roguep;
  Short t;
  //Char buf[55];
  Char d6dam[4] = {1,6,0,0}; /* six-sided-die (1d6) for damage */

  //buf[0] = 0;
  /* spring the trap (if any), rendering it visible. */
  if ((t = trap_at(row, col, sotu)) == NO_TRAP) {
    return;
  }
  dungeon[row][col] &= (~HIDDEN);
  if (rand_percent(rogue->exp + sotu->ring_dex)) {
    message("the trap failed", sotu);
    return;
  }

  /* do something interesting */
  switch(t) {
  case TRAP_DOOR:
    /* fall down a level! */
    sotu->trap_door = 1;
    sotu->new_level_message = you_fell;
    break;
  case BEAR_TRAP:
    /* get stuck for a few "moves" (must try to move, to get out) */
    if (!(sotu->ring_flags & RING_FREE_ACTION)) {
      message("you are caught in a bear trap", sotu);
      sotu->bear_trap = get_rand(4, 7);
    }
    break;
  case TELE_TRAP:
    /* teleport... */
    mvaddch(rogue->row, rogue->col, '^');
    tele(sotu);
    break;
  case DART_TRAP:
    /* ow! poison darts cause damage and possibly loss of strength */
    message("a small dart hit you in the shoulder", sotu);
    rogue->hp_current -= get_damage(d6dam, 1);
    if (rogue->hp_current <= 0) {
      rogue->hp_current = 0;
    }
    if ((!(sotu->ring_flags & RING_SUSTAIN_STR)) && rand_percent(40) &&
	(rogue->str_current >= 3)) {
      rogue->str_current--;
    }
    print_stats(STAT_HP | STAT_STRENGTH, sotu);
    if (rogue->hp_current <= 0) {
      killed_by((object *) 0, POISON_DART, sotu);
      FrmPopupForm(TopTenForm);
    }
    break;
  case SLEEPING_GAS_TRAP:
    if (!(sotu->ring_flags & (RING_ALERTNESS | RING_BREATHING))) {
      //      check_message(); // cause this one is two lines long
      message("a strange white mist envelops you and you fall asleep", sotu);
      take_a_nap();
    }
    break;
  case RUST_TRAP:
    /* bucket of water... */
    message("a gush of water hits you on the head", sotu);
    rust((object *) 0);
    break;
  }
  do_feep(900,9); /* auditory feepback */
}


/************************************************
  add_traps MOVED to LibRogue
************************************************/

/* id_trap moved to chuvmey.c */

/************************************************
  show_traps MOVED to LibRogue
************************************************/


/**********************************************************************
                       SEARCH
 IN:
 n = number of turns to search
 is_auto = rogue has a ring of searching or something; don't tick clock
 various globals
 OUT:
 nothing
 PURPOSE:
 The rogue looks for traps and hidden doors, and maybe finds one.
 **********************************************************************/
void search(Short n, Boolean is_auto) {
  fighter *rogue = sotu->roguep;
  Short s, i, j, row, col, t;
  Short shown = 0, found = 0;
  static Boolean reg_search; /* you can search 1.5 times per turn */

  if (!is_auto)
    message("searching", sotu);  

  /* Look through the 3x3 square and count the "hidden" things there */
  for (i = -1; i <= 1; i++) {
    for (j = -1; j <= 1; j++) {
      row = rogue->row + i;
      col = rogue->col + j;
      if ((row < MIN_ROW) || (row >= (DROWS-1)) ||
	  (col < 0) || (col >= DCOLS)) {
	continue;
      }
      if (dungeon[row][col] & HIDDEN) {
	found++;
      }
    }
  }

  /* Spend n turns looking for hidden things.
     Stop early if hidden things exist and they're all found,
     or if the rogue is interrupted. */
  for (s = 0; s < n; s++) {
    for (i = -1; i <= 1; i++) {
      for (j = -1; j <= 1; j++) {
	row = rogue->row + i;
	col = rogue->col + j ;
	if ((row < MIN_ROW) || (row >= (DROWS-1)) ||
	    (col < 0) || (col >= DCOLS)) {
	  continue;
	}
	if (dungeon[row][col] & HIDDEN) {
	  if (rand_percent(17 + (rogue->exp + sotu->ring_dex))) {
	    /* "Found" something! */
	    dungeon[row][col] &= (~HIDDEN);
	    if ((!sotu->blind) && ((row != rogue->row) ||
			     (col != rogue->col))) {
	      mvaddch(row, col, get_dungeon_char(row, col, sotu));
	    }
	    shown++;
	    if (dungeon[row][col] & TRAP) {
	      t = trap_at(row, col, sotu);
	      message(trap_names[t], sotu);
	    }
	  }
	}
	if (((shown == found) && (found > 0)) || sotu->interrupted) {
	  return;
	}
      }
    }
    if ((!is_auto) && (reg_search = !reg_search)) {
      (void) reg_move();
    }
  }
}

/**********************************************************************
                       ID_TRAP
 IN:
 dir = direction of the adjacent square supposedly containing a trap
 sotu = various globals (rogue, dungeon, trap_strings)
 OUT:
 nothing
 PURPOSE:
 If there's a trap in the adjacent square in the direction indicated,
 tell the player what kind of trap it is.
 **********************************************************************/
void id_trap(Short dir, struct state_of_the_union *sotu) {
  Short row, col;
  Short t;

  if (dir == NO_DIRECTION) {
    return;
  }
  row = sotu->roguep->row;
  col = sotu->roguep->col;

  get_dir_rc(dir, &row, &col, false);

  if ((sotu->dungeon[row][col] & TRAP) 
      && (!(sotu->dungeon[row][col] & HIDDEN))) {
    t = trap_at(row, col, sotu);
    message(trap_names[t], sotu);
  } else {
    message("no trap there", sotu);
  }
}


/***************************************************************************
 *     Moved all of lib_traps.c into here:
 ***************************************************************************/


/***************************************************************
                   TRAP_AT
 IN:
 row,col = the location to check for traps
 sotu = various globals (traps)
 OUT:
 the type of trap, or NO_TRAP.
 PURPOSE:
 Determine whether there's a trap at the indicated location,
 and what type of trap it is.
****************************************************************/
Short trap_at(Short row, Short col, struct state_of_the_union *sotu) {
  short i;
  trap * traps = sotu->traps;

  /* Actually, it's the other way around - look through the
     array of traps and see if one of them is at this location */
  for (i = 0; ((i < MAX_TRAPS) && (traps[i].trap_type != NO_TRAP)); i++) {
    if ((traps[i].trap_row == row) && (traps[i].trap_col == col)) {
      return(traps[i].trap_type);
    }
  }
  return(NO_TRAP);
}

/***************************************************************
                   ADD_TRAPS
 IN:
 sotu = various globals (traps, rooms, dungeon, cur_level, party_room)
 OUT:
 nothing
 PURPOSE:
 Called when a level is created, to randomly hide some traps in it.
****************************************************************/
void add_traps(struct state_of_the_union *sotu) {
  Short i, n, tries = 0;
  Short row, col;
  trap * traps = sotu->traps;
  room * rooms = sotu->rooms;
  UShort ** dungeon = sotu->dungeon;

  /* Figure out how many traps to put on this level. */
  if (sotu->cur_level <= 2) {
    n = 0;
  } else if (sotu->cur_level <= 7) {
    n = get_rand(0, 2);
  } else if (sotu->cur_level <= 11) {
    n = get_rand(1, 2);
  } else if (sotu->cur_level <= 16) {
    n = get_rand(2, 3);
  } else if (sotu->cur_level <= 21) {
    n = get_rand(2, 4);
  } else if (sotu->cur_level <= (AMULET_LEVEL + 2)) {
    n = get_rand(3, 5);
  } else {
    n = get_rand(5, MAX_TRAPS);
  }
  /* For each trap, pick a type and try to put it somewhere. */
  for (i = 0; i < n; i++) {
    traps[i].trap_type = get_rand(0, (TRAPS - 1));

    /* A party room *always* gets a trap, if we can find a space for it. */
    if ((i == 0) && (sotu->party_room != NO_ROOM)) {
      do {
	row = get_rand((rooms[sotu->party_room].top_row+1),
		       (rooms[sotu->party_room].bottom_row-1));
	col = get_rand((rooms[sotu->party_room].left_col+1),
		       (rooms[sotu->party_room].right_col-1));
	tries++;
      } while (((dungeon[row][col] & (OBJECT|STAIRS|TRAP|TUNNEL)) ||
		(dungeon[row][col] == NOTHING)) && (tries < 15));
      if (tries >= 15) {
	gr_row_col(&row, &col, (FLOOR | MONSTER), sotu);
      }
    } else {
      gr_row_col(&row, &col, (FLOOR | MONSTER), sotu);
    }
    traps[i].trap_row = row;
    traps[i].trap_col = col;
    dungeon[row][col] |= (TRAP | HIDDEN);
  }
}

/***************************************************************
                   SHOW_TRAPS
 IN:
 sotu = various globals (dungeon)
 OUT:
 nothing
 PURPOSE:
 Make visible all the traps on this level (rogue used a magic item.)
****************************************************************/
void show_traps(struct state_of_the_union *sotu) {
  Short i, j;

  for (i = 0; i < DROWS; i++) {
    for (j = 0; j < DCOLS; j++) {
      if (sotu->dungeon[i][j] & TRAP) {
	mvaddch(i, j, '^');
      }
    }
  }
}
