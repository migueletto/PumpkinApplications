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
 * room.c
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

room * rooms;
/* "Boolean * rooms_visited" moved to level.c */

/**************************************
  light_up_room MOVED to LibRogue ?
  light_passage MOVED to LibRogue ?
  darken_room MOVED to LibRogue ?
*************************************/

/************************************************
  get_dungeon_char MOVED to LibRogue
************************************************/

/**************************************
  get_mask_char MOVED to lib
  gr_row_col MOVED to lib
*************************************/

/**************************************
  gr_room MOVED to lib
  party_objects MOVED to lib
*************************************/

/**************************************
  get_room_number MOVED to lib
  is_all_connected MOVED to lib
  visit_rooms MOVED to lib
  draw_magic_map MOVED to lib
*************************************/

/**************************************
  dr_course MOVED to lib
  get_oth_room MOVED to lib
*************************************/

/***************************************************************************
 *     Moved all of lib_room.c into here:
 ***************************************************************************/



/***************************************************************
                   GET_ROOM_NUMBER
 IN:
 row, col = the location in question
 rooms = the array of room areas
 OUT:
 the room number or NO_ROOM
 PURPOSE:
 Given a location, return the room that it is in, if any.
****************************************************************/
Short get_room_number(Short row, Short col, room * rooms) {
  Short i;

  for (i = 0; i < MAXROOMS; i++) {
    if ((row >= rooms[i].top_row) && (row <= rooms[i].bottom_row) &&
	(col >= rooms[i].left_col) && (col <= rooms[i].right_col)) {
      return(i);
    }
  }
  return(NO_ROOM);
}


/***************************************************************
                   GET_MASK_CHAR
 IN:
 mask = basically "kind of object"
 OUT:
 character that visually represents that kind of object
 PURPOSE:
 Map a kind-of-object to a visual-representation.
****************************************************************/
Char get_mask_char(UShort mask) {
  switch(mask) {
  case SCROLL:
    return('?');
  case POTION:
    return('!');
  case GOLD:
    return('*');
  case FOOD:
    return(FOODCHAR);
  case WAND:
    return('/');
  case ARMOR:
    return(']');
  case WEAPON:
    return(')');
  case RING:
    return('=');
  case AMULET:
    return(',');
  default:
    return('~');	// unknown, something is wrong
  }
}


/***************************************************************
                   GR_ROOM
 IN:
 rooms = the array of room areas
 OUT:
 random room number
 PURPOSE:
 Pick a random room/maze.
****************************************************************/
Short gr_room(room * rooms) {
  Short i;

  do {
    i = get_rand(0, MAXROOMS-1);
  } while (!(rooms[i].is_room & (R_ROOM | R_MAZE)));

  return(i);
}


/***************************************************************
                   GET_OTH_ROOM
 IN:
 rn = room number
 row, col = a location in the wall of room 'rn'
 room = the array of room areas
 OUT:
 true if a door to another room is found in the indicated wall.
 row,col will be modified to return a location...
 PURPOSE:
 Given a room and a location in its wall, find the door (if any)
 in that wall and return the location of the "other end".
****************************************************************/
static Boolean get_oth_room(Short rn, Short *row, Short *col, room * rooms) SEC_L;
static Boolean get_oth_room(Short rn, Short *row, Short *col, room * rooms) 
{
  Short d = -1;

  if (*row == rooms[rn].top_row) {
    d = UP/2;
  } else if (*row == rooms[rn].bottom_row) {
    d = DOWN/2;
  } else if (*col == rooms[rn].left_col) {
    d = LEFT/2;
  } else if (*col == rooms[rn].right_col) {
    d = RIGHT/2;
  }
  if ((d != -1) && (rooms[rn].doors[d].oth_room >= 0)) {
    *row = rooms[rn].doors[d].oth_row;
    *col = rooms[rn].doors[d].oth_col;
    return true;
  }
  return false;
}


/***************************************************************
                   DR_COURSE
 IN:
 monster = a monster to direct on a course
 entering = whether the monster is entering a room
 row, col = location to start the monster at
 sotu = various globals (rooms, rogue, dungeon)
 OUT:
 PURPOSE:
 When the monster has entered/exited a room, you can call this
 to revise the monster's movement goal (if it can see the rogue
 now, it will head rogueward; otherwise it will explore other
 rooms / passages.)
****************************************************************/
void dr_course(object *monster, Boolean entering, Short row, Short col,
	       struct state_of_the_union * sotu)
{
  Short i, j, k, rn;
  Short r, rr;
  room * rooms;

  monster->row = row;
  monster->col = col;
  rooms = sotu->rooms;

  /* The monster can see the rogue ==> Use the default goal, the rogue. */
  if (mon_sees(monster, sotu->roguep->row, sotu->roguep->col, sotu->rooms)) {
    monster->trow = NO_ROOM;
    return;
  }

  /* The monster can't see the rogue ==> Find some other goal */

  rn = get_room_number(row, col, rooms);

  if (entering) {
    /* The monster has just entered room 'rn'. */

    /* First, look through all other rooms/mazes for a door to 'rn',
       and find the door at the other end belonging to 'rn'; if it's
       not the door we just came in, make it the goal and return. */
    r = get_rand(0, MAXROOMS-1);
    for (i = 0; i < MAXROOMS; i++) {
      rr = (r + i) % MAXROOMS;
      if ((!(rooms[rr].is_room & (R_ROOM | R_MAZE))) || (rr == rn)) {
	continue;
      }
      for (k = 0; k < 4; k++) {
	if (rooms[rr].doors[k].oth_room == rn) {
	  monster->trow = rooms[rr].doors[k].oth_row;
	  monster->tcol = rooms[rr].doors[k].oth_col;
	  if ((monster->trow == row) &&
	      (monster->tcol == col)) {
	    continue;
	  }
	  return;
	}
      }
    }

    /* We could not find a door that leads to a room/maze and
       isn't the door we just came in. */

    /* Try to find any door that isn't the one we just came in;
       make it the goal and return. */       
    for (i = rooms[rn].top_row; i <= rooms[rn].bottom_row; i++) {
      for (j = rooms[rn].left_col; j <= rooms[rn].right_col; j++) {
	if ((i != monster->row) && (j != monster->col) &&
	    (sotu->dungeon[i][j] & DOOR)) {
	  monster->trow = i;
	  monster->tcol = j;
	  return;
	}
      }
    }
    
    /* We couldn't find any door except the one we just came in. */

    /* The monster will backtrack to the room that it came from... */
    for (i = 0; i < MAXROOMS; i++) {
      for (j = 0; j < 4; j++) {
	if (rooms[i].doors[j].oth_room == rn) {
	  for (k = 0; k < 4; k++) {
	    if (rooms[rn].doors[k].oth_room == i) {
	      monster->trow = rooms[rn].doors[k].oth_row;
	      monster->tcol = rooms[rn].doors[k].oth_col;
	      return;
	    }
	  }
	}
      }
    }

    /* No place to send monster! */
    monster->trow = -1;
  } else {

    /* Exiting room - the goal is "the other end" (if any) */

    if (!get_oth_room(rn, &row, &col, rooms)) {
      monster->trow = NO_ROOM;
    } else {
      monster->trow = row;
      monster->tcol = col;
    }
  }
}

// visit_rooms and is_all_conn moved BACK into ../level.c
/***************************************************************
                   GET_DUNGEON_CHAR
 IN:
 row,col = the location in question
 sotu = various globals (dungeon, level_objects)
 OUT:
 character that is the current visual representation of the location
 PURPOSE:
 Given a location, determine what the rogue should "see" there
 (a monster, an object, the floor, ...)
****************************************************************/
Char get_dungeon_char(Short row, Short col,
		      struct state_of_the_union * sotu) 
{
  UShort mask = sotu->dungeon[row][col];

  /* Monsters have highest priority */
  if (mask & MONSTER) {
    return(gmc_row_col(row, col, sotu));
  }
  /* Objects are visible if there's no monster standing on 'em */
  if (mask & OBJECT) {
    object *obj;

    obj = object_at(sotu->level_objects, row, col);
    return(get_mask_char(obj->what_is));
  }
  /* Otherwise display some feature of the dungeon. */
  if (mask & (TUNNEL | STAIRS | HORWALL | VERTWALL | FLOOR | DOOR)) {
    if ((mask & (TUNNEL| STAIRS)) && (!(mask & HIDDEN))) {
      return(((mask & STAIRS) ? '%' : '#'));
    }
    if (mask & HORWALL) {
      return(EMDASH); /*       return('-'); */
    }
    if (mask & VERTWALL) {
      return('|');
    }
    /* Traps and doors could be hidden. */
    if (mask & FLOOR) {
      if (mask & TRAP) {
	if (!(sotu->dungeon[row][col] & HIDDEN)) {
	  return('^');
	}
      }
      return('.');
    }
    if (mask & DOOR) {
      if (mask & HIDDEN) {
	if (((col > 0) && (sotu->dungeon[row][col-1] & HORWALL)) ||
	    ((col < (DCOLS-1)) && (sotu->dungeon[row][col+1] & HORWALL))) {
	  return(EMDASH); /* 	  return('-'); */
	} else {
	  return('|');
	}
      } else {
	return('+');
      }
    }
  }
  return(' ');
}



/***************************************************************
                   LIGHT_UP_ROOM
 IN:
 rn = the room to light ... usually 'cur_room'
 sotu = various globals (rogue, dungeon, rooms, level_monsters, blind)
 OUT:
 nothing
 PURPOSE:
 When the rogue enters a room (or when certain visual effects
 wear off) the room is 'lit up' - can see "everything" in it.
****************************************************************/
void light_up_room(Short rn, struct state_of_the_union * sotu) {
  room * rooms = sotu->rooms;
  UShort ** dungeon = sotu->dungeon;
  Short i, j;

  if (!sotu->blind) {
    for (i = rooms[rn].top_row;
	 i <= rooms[rn].bottom_row; i++) {
      for (j = rooms[rn].left_col;
	   j <= rooms[rn].right_col; j++) {
	if (dungeon[i][j] & MONSTER) {
	  object *monster;

	  if ((monster = object_at(sotu->level_monsters, i, j))) {
	    dungeon[monster->row][monster->col] &= (~MONSTER);
	    monster->trail_char =
	      get_dungeon_char(monster->row, monster->col, sotu);
	    dungeon[monster->row][monster->col] |= MONSTER;
	  }
	}
	mvaddch(i, j, get_dungeon_char(i, j, sotu));
      }
    }
    mvaddch(sotu->roguep->row, sotu->roguep->col, sotu->roguep->fchar);
  }
}

/***************************************************************
                   LIGHT_PASSAGE
 IN:
 row,col = the location of the light source...
 sotu = various globals (dungeon, blind)
 OUT:
 nothing
 PURPOSE:
 When the rogue is in a passage, some nearby squares are 'lit.'
****************************************************************/
void light_passage(Short row, Short col,
		   struct state_of_the_union *sotu) 
{
  Short i, j, i_end, j_end;

  if (sotu->blind) {
    return;
  }
  i_end = (row < (DROWS-2)) ? 1 : 0;
  j_end = (col < (DCOLS-1)) ? 1 : 0;

  for (i = ((row > MIN_ROW) ? -1 : 0); i <= i_end; i++) {
    for (j = ((col > 0) ? -1 : 0); j <= j_end; j++) {
      if (can_move(row, col, row+i, col+j, sotu->dungeon)) {
	mvaddch(row+i, col+j, get_dungeon_char(row+i, col+j, sotu));
      }
    }
  }
}

/***************************************************************
                   DARKEN_ROOM
 IN:
 rn = room to darken
 sotu = various globals (rooms, dungeon, blind, detect_monster)
 OUT:
 nothing
 PURPOSE:
 The inverse of lighting a room.  The rogue is in a 'lit' room
 that needs to be made 'dark' again (the rogue left it, or has
 been blinded, or something.)
****************************************************************/
void darken_room(Short rn, struct state_of_the_union *sotu) {
  Short i, j;
  room * rooms = sotu->rooms;
  UShort ** dungeon = sotu->dungeon;

  for (i = rooms[rn].top_row + 1; i < rooms[rn].bottom_row; i++) {
    for (j = rooms[rn].left_col + 1; j < rooms[rn].right_col; j++) {
      if (sotu->blind) {
	mvaddch(i, j, ' ');
      } else {
	if (!(dungeon[i][j] & (OBJECT | STAIRS)) &&
	    !(sotu->detect_monster && (dungeon[i][j] & MONSTER))) {
	  if (!imitating(i, j, sotu)) {
	    mvaddch(i, j, ' ');
	  }
	  if ((dungeon[i][j] & TRAP) && (!(dungeon[i][j] & HIDDEN))) {
	    mvaddch(i, j, '^');
	  }
	}
      }
    }
  }
}


/***************************************************************
                   GR_ROW_COL
 IN:
 row, col = (return value) a random location
 mask = locations corresponding to this mask will be selected
 sotu = various globals (rogue, rooms, dungeon)
 OUT:
 nothing
 PURPOSE:
 Return in row,col a random location that matches 'mask'
 (and isn't where the rogue is standing.)
****************************************************************/
void gr_row_col(Short *row, Short *col, UShort mask,
		struct state_of_the_union * sotu) {
  Short rn;
  Short r, c;
  fighter *rogue = sotu->roguep;
  room *rooms = sotu->rooms;
  UShort ** dungeon = sotu->dungeon;

  do {
    r = get_rand(MIN_ROW, DROWS-2);
    c = get_rand(0, DCOLS-1);
    rn = get_room_number(r, c, rooms);
  } while ((rn == NO_ROOM) ||
	   (0==(dungeon[r][c] & mask)) ||
	   (0!=(dungeon[r][c] & (~mask))) ||
	   (!(rooms[rn].is_room & (R_ROOM | R_MAZE))) ||
	   ((r == rogue->row) && (c == rogue->col)) );

  *row = r;
  *col = c;
}



/***************************************************************
                   DRAW_MAGIC_MAP
 IN:
 sotu = various globals (dungeon, level_monsters)
 OUT:
 nothing
 PURPOSE:
 Reveal the dungeon level (including hidden traps/doors;
 otherwise basically like the rogue had explored it all.) 
****************************************************************/
void draw_magic_map(struct state_of_the_union * sotu) {
  Short i, j, ch, och;
  UShort mask = (HORWALL | VERTWALL | DOOR | TUNNEL | TRAP | STAIRS |
			 MONSTER);
  UShort s;
  object *monster;

  for (i = 0; i < DROWS; i++) {
    for (j = 0; j < DCOLS; j++) {
      s = sotu->dungeon[i][j];
      if (s & mask) {
	if (((ch = mvinch(i, j)) == ' ')
	    || ( Is_Alpha(ch) )
	    || (s & (TRAP | HIDDEN))) {

	  och = ch;
	  sotu->dungeon[i][j] &= (~HIDDEN);
	  if (s & HORWALL) {
	    ch = EMDASH;
 	    /* ch = '-'; */
	  } else if (s & VERTWALL) {
	    ch = '|';
	  } else if (s & DOOR) {
	    ch = '+';
	  } else if (s & TRAP) {
	    ch = '^';
	  } else if (s & STAIRS) {
	    ch = '%';
	  } else if (s & TUNNEL) {
	    ch = '#';
	  } else {
	    continue;
	  }
	  if ((!(s & MONSTER)) || (och == ' ')) {
	    mvaddch(i,j,ch); // was addch(ch)
	  }
	  if (s & MONSTER) {
	    if ((monster = object_at(sotu->level_monsters, i, j))) {
	      monster->trail_char = ch;
	    }
	  }
	}
      }
    }
  }
}


/***************************************************************
                   PARTY_OBJECTS
 IN:
 rn = "party room" to place the treasure hoard in
 sotu = various globals (rooms, dungeon)
 OUT:
 number of objects placed in the hoard
 PURPOSE:
 Given a room in which a monster party will be placed, generate
 a treasure hoard of objects and place them in the room.
****************************************************************/
Short party_objects(Short rn, struct state_of_the_union *sotu) {
  Short i, j, nf = 0;
  object *obj;
  Short n, max_n, row = 0, col = 0;
  Boolean found;
  room * rooms = sotu->rooms;

  /* calculate the area of the room */
  max_n = ((rooms[rn].bottom_row - rooms[rn].top_row) - 1) *
          ((rooms[rn].right_col - rooms[rn].left_col) - 1);
  /* select a number of objects to attempt to place */
  n =  get_rand(5, 10);
  if (n > max_n) {
    n = max_n - 2;
  }
  for (i = 0; i < n; i++) {
    for (j = found = 0; ((!found) && (j < 250)); j++) {
      row = get_rand(rooms[rn].top_row+1,
		     rooms[rn].bottom_row-1);
      col = get_rand(rooms[rn].left_col+1,
		     rooms[rn].right_col-1);
      if ((sotu->dungeon[row][col] == FLOOR) || 
	  (sotu->dungeon[row][col] == TUNNEL)) {
	found = 1;
      }
    }
    if (found) {
      obj = gr_object(sotu);
      place_at(obj, row, col, sotu);
      nf++;
    }
  }
  return(nf);
}
