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
 * level.c
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

/* WARNING: t MUST be a local variable declared where this is used.. */
#define swap(x,y) {t = x; x = y; y = t;}

#define has_amulet() (mask_pack(&rogue->pack, AMULET))

/* function decls. local to this file */
static void visit_rooms(Short rn, room * rooms, Boolean * rooms_visited) SEC_2;
static Boolean is_all_connected(room * rooms, Boolean * rooms_visited) SEC_2;
static Boolean same_row(Short room1, Short room2) SEC_2;
static Boolean same_col(Short room1, Short room2) SEC_2;
static void add_mazes() SEC_2;
static void fill_out_level() SEC_2;
static void fill_it(Short rn, Boolean do_rec_de) SEC_2;
static void recursive_deadend(Short rn, Short srow, Short scol) SEC_2;
static void make_maze(Short r, Short c,
		      Short tr, Short br, Short lc, Short rc) SEC_2;
static void mix_random_rooms() SEC_2;

extern struct state_of_the_union * sotu;

Short offsets[4] = {-1, 1, 3, -3}; /* was static in fill_it */

Char random_rooms[MAXROOMS+1] = { 3,7,5,2,0,6,1,4,8 };
Boolean * rooms_visited;


/* Call tree:
   clear_level called in main.c
   make_level called in main.c
   put_player called in main.c -- does it really balong here?
 */

/* Make level... guess it's ok */
/* XXXX is_all_connected, has_amulet, put_amulet, and win are temp. def'n.s */




// make_room has been moved to chuvmey_nolib.c
//void make_room(Short rn, Short r1, Short r2, Short r3,
//	       struct state_of_the_union *sotu);

/***************************************************************
                   VISIT_ROOMS
 IN:
 rn = room number to start at...
 rooms = the array of room areas
 rooms_visited = array to mark accessible/inaccessible
 OUT:
 nothing
 PURPOSE:
 Used in determining whether a dungeon level being constructed
 is fully-connected yet, or whether some rooms are adrift.
****************************************************************/
static void visit_rooms(Short rn, room * rooms, Boolean * rooms_visited) 
{
  Short i;
  Short oth_rn;

  rooms_visited[rn] = 1;

  for (i = 0; i < 4; i++) {
    oth_rn = rooms[rn].doors[i].oth_room;
    if ((oth_rn >= 0) && (!rooms_visited[oth_rn])) {
      visit_rooms(oth_rn, rooms, rooms_visited);
    }
  }
}

/***************************************************************
                   IS_ALL_CONNECTED
 IN:
 rooms = array of room areas
 rooms_visited = array to mark accessible/inaccessible
 OUT:
 true if all rooms are accessible
 (false if some rooms are still unconnected)
 PURPOSE:
 Determine whether all rooms/mazes are accessible.
****************************************************************/
static Boolean is_all_connected(room * rooms, Boolean * rooms_visited) 
{
  Short i, starting_room = 0;

  for (i = 0; i < MAXROOMS; i++) {
    rooms_visited[i] = 0;
    if (rooms[i].is_room & (R_ROOM | R_MAZE)) {
      starting_room = i;
    }
  }

  visit_rooms(starting_room, rooms, rooms_visited);

  for (i = 0; i < MAXROOMS; i++) {
    if ((rooms[i].is_room & (R_ROOM | R_MAZE)) && (!rooms_visited[i])) {
      return false;
    }
  }
  return true;
}


/**********************************************************************
                       MAKE_LEVEL
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Make the map for a new dungeon level.  ...
 **********************************************************************/
void make_level() {
  fighter *rogue = sotu->roguep;
  Short i, j;
  Short must_exist1 = 0, must_exist2 = 0, must_exist3 = 0;
  Boolean big_room;
  room *rooms = sotu->rooms;

  /* It's not REALLY turtles all the way down, you know. */
  if (sotu->cur_level < LAST_DUNGEON) {
    sotu->cur_level++;
  }
  /* Keep track of the deepest level the rogue's been to. 
     (Note that if max_level > cur_level, the rogue is either
     heading up with the amulet or flirting with wizard mode.) */
  if (sotu->cur_level > sotu->max_level) {
    sotu->max_level = sotu->cur_level;
  }

  /* Standard dungeon geography requires that there be at least three
     rooms in a straight (north-south or east-west) line. */
  switch(must_exist1) {
  case 0:
    must_exist1 = 0;
    must_exist2 = 1;
    must_exist3 = 2;
    break;
  case 1:
    must_exist1 = 3;
    must_exist2 = 4;
    must_exist3 = 5;
    break;
  case 2:
    must_exist1 = 6;
    must_exist2 = 7;
    must_exist3 = 8;
    break;
  case 3:
    must_exist1 = 0;
    must_exist2 = 3;
    must_exist3 = 6;
    break;
  case 4:
    must_exist1 = 1;
    must_exist2 = 4;
    must_exist3 = 7;
    break;
  case 5:
    must_exist1 = 2;
    must_exist2 = 5;
    must_exist3 = 8;
    break;
  }

  /* Well, ok, there could also be just ONE REALLY BIG ROOM.  The odds
     of that are small (but I should really change that rand_percent(1)
     sometime to make sure PilotRogue won't barf when it does happen..) */

  big_room = ((sotu->cur_level == sotu->party_counter) && rand_percent(1));

  if (big_room) {
    make_room(BIG_ROOM, 0, 0, 0, sotu);
  } else {
    for (i = 0; i < MAXROOMS; i++) {
      make_room(i, must_exist1, must_exist2, must_exist3, sotu);
    }
  }
  /* heisenFatalError is after here */

  if (!big_room) {
    /* I am sorry to say that I have some kind of horrible bug in the
       maze generation which is why it's currently diked out.
       it might take up more memory than it is really worth, too. */
    add_mazes();

/*     message("mazes", sotu); */
    /* heisenFatalError is before here! */

    /* shuffle an array of the room numbers.. */
    mix_random_rooms();

    /* Make sure that every room is reachable. */
    for (j = 0; j < MAXROOMS; j++) {

      i = random_rooms[j];

      if (i < (MAXROOMS-1)) {
	(void) connect_rooms(i, i+1, sotu);
      }
      if (i < (MAXROOMS-3)) {
	(void) connect_rooms(i, i+3, sotu);
      }
      if (i < (MAXROOMS-2)) {
	if (rooms[i+1].is_room & R_NOTHING) {
	  if (connect_rooms(i, i+2, sotu)) {
	    rooms[i+1].is_room = R_CROSS;
	  }
	}
      }
      if (i < (MAXROOMS-6)) {
	if (rooms[i+3].is_room & R_NOTHING) {
	  if (connect_rooms(i, i+6, sotu)) {
	    rooms[i+3].is_room = R_CROSS;
	  }
	}
      }
      if (is_all_connected(rooms, rooms_visited)) {
	break;
      }
    }
    /* Put stuff around... */
    fill_out_level();
  }
  if (!(has_amulet()) && (sotu->cur_level >= AMULET_LEVEL)) {
    put_amulet();
  }
}

/******************************
   make_room MOVED to LibRogue.  then back here.  then chuvmey_nolib.
 ******************************/

/***********************************
   connect_rooms MOVED to LibRogue
 ***********************************/

/******************************
   put_door MOVED to LibRogue
 ******************************/

/***************************************
   draw_simple_passage MOVED to LibRogue
 ***************************************/


/**********************************************************************
                       SAME_ROW
 IN: two rooms
 OUT: 'true' if they're in the same row
 **********************************************************************/
static Boolean same_row(Short room1, Short room2) 
{ 
  return((room1 / 3) == (room2 / 3)); 
}

/**********************************************************************
                       SAME_COL
 IN: two rooms
 OUT: 'true' if they're in the same column
 **********************************************************************/
static Boolean same_col(Short room1, Short room2) 
{
  return((room1 % 3) == (room2 % 3)); 
}


/**********************************************************************
                       ADD_MAZES
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 If it worked, this would add some cute little mazes to the dungeon
 in place of where some room could have been but isn't.
 But there is some horrible bug in it that I haven't been motivated to
 actually fix.  is the space the code takes up actually worth it?
 **********************************************************************/
/* XXXXXXXXXX  SOURCE OF HEISENBUG  XXXXXXXXXXXXX */
#define MILKMAN_DAN 0
static void add_mazes() {
  Short i, j;
  Short start;
  Short maze_percent;
  room *rooms = sotu->rooms;

  if (sotu->cur_level > 1) {
    start = get_rand(0, (MAXROOMS-1));
    maze_percent = (sotu->cur_level * 5) / 4;

    if (sotu->cur_level > 15) {
      maze_percent += sotu->cur_level;
    }
    for (i = 0; i < MAXROOMS; i++) {
      j = ((start + i) % MAXROOMS);
      if (rooms[j].is_room & R_NOTHING) {
	/* XXXXXX REMOVED maze generation for DEBUGGING XXXXXX */
	if (rand_percent(maze_percent) && MILKMAN_DAN) {
	  rooms[j].is_room = R_MAZE;
/* 	  message("make maze", sotu); */
	  /* It seems that make_maze is the problem */
	  make_maze(get_rand(rooms[j].top_row+1, rooms[j].bottom_row-1),
		    get_rand(rooms[j].left_col+1, rooms[j].right_col-1),
		    rooms[j].top_row, rooms[j].bottom_row,
		    rooms[j].left_col, rooms[j].right_col);
/* 	  message("hide boxed", sotu); */
/*   message("foo",0); */
	  hide_boxed_passage(rooms[j].top_row, rooms[j].left_col,
			     rooms[j].bottom_row, rooms[j].right_col,
			     get_rand(0, 2), sotu);
/*   message("box",0); */
	}
      }
    }
  }
}

/**********************************************************************
                       FILL_OUT_LEVEL
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 This will make some dead ends and long paths and stuff in some
 empty "rooms" (places where there could be rooms but aren't.)
 **********************************************************************/
static void fill_out_level() {
  Short i, rn;

  /* shuffle an array of the room numbers.. */
  mix_random_rooms();

  /* This represents the last room on a long dead-end path (if any.) */
  sotu->r_de = NO_ROOM;

  for (i = 0; i < MAXROOMS; i++) {
    rn = random_rooms[i];
    if ((sotu->rooms[rn].is_room & R_NOTHING) ||
	((sotu->rooms[rn].is_room & R_CROSS) && coin_toss())) {
      /* Do stuff to all of the "nothing" rooms, and to half of the
	 rooms that have crossing passages. */
      fill_it(rn, 1);
    }
  }
  if (sotu->r_de != NO_ROOM) {
    /* A long dead-end path was created; do something with the last
       room on it, perhaps making it not *really* a dead end? */
    fill_it(sotu->r_de, 0);
  }
}

/**********************************************************************
                       FILL_IT
 IN:
 rn = number of "room" to fill
 do_rec_de = whether to maybe do a long dead-end path, starting here
 various globals
 OUT:
 nothing
 PURPOSE:
 This will probably make a dead end path through or terminating in the
 room 'rn', from some room adjacent to it to the {north, south, east,
 west}.  Maybe connect two rooms, I can't really tell at this time of night.
 **********************************************************************/
static void fill_it(Short rn, Boolean do_rec_de) {
  /* Made offsets a global variable.  also, rn was originally int.  */
  Short i, tunnel_dir, door_dir, drow, dcol;
  Short target_room, rooms_found = 0;
  Short srow, scol, t;
  Boolean did_this = 0;
  room * rooms = sotu->rooms;

  /* randomize the 'offsets' array */
  for (i = 0; i < 10; i++) {
    srow = get_rand(0, 3);
    scol = get_rand(0, 3);
    swap(offsets[srow], offsets[scol]); /* swap via 't' */
  }

  /* There are at most four room-areas reachable from the current
     room-area to the north/south/east/west.  For each of them ... */
  for (i = 0; i < 4; i++) {

    /* Find a target room which is NESW-adjacent to the current room.
       It must be a ROOM or MAZE. */
    target_room = rn + offsets[i];
    if (((target_room < 0) || (target_room >= MAXROOMS)) ||
	(!(same_row(rn,target_room) || same_col(rn,target_room))) ||
	(!(rooms[target_room].is_room & (R_ROOM | R_MAZE)))) {
      continue;
    }

    /* Figure out which direction the target room is from here. */
    if (same_row(rn, target_room)) {
      tunnel_dir = (rooms[rn].left_col < rooms[target_room].left_col) ?
	RIGHT : LEFT;
    } else {
      tunnel_dir = (rooms[rn].top_row < rooms[target_room].top_row) ?
	DOWN : UP;
    }
    /* Figure out what wall in the target room to put the door in.
       If there's already a door there, skip this target room. */
    door_dir = ((tunnel_dir + 4) % DIRS);
    if (rooms[target_room].doors[door_dir/2].oth_room != NO_ROOM) {
      continue;
    }

    /* If there is no 'tunnel' in the current room, or if we're not
       allowed to do a long dead-end path starting from here, or if ?,
       then set srow,scol to the middle of the current room.
       (Otherwise there is an srow,scol already that we should
       continue to use.  ?)  */
    if (((!do_rec_de) || did_this) ||
	(!mask_room(rn, &srow, &scol, TUNNEL, sotu))) {
      srow = (rooms[rn].top_row + rooms[rn].bottom_row) / 2;
      scol = (rooms[rn].left_col + rooms[rn].right_col) / 2;
    }
    /* Put a door somewhere in the appropriate wall of the target,
       connected via a passage to the srow,scol in the current room.
       Label the current room a dead end. */
    put_door(&rooms[target_room], door_dir, &drow, &dcol, sotu);
    rooms_found++;
    draw_simple_passage(srow, scol, drow, dcol, tunnel_dir, sotu);
    rooms[rn].is_room = R_DEADEND;
    dungeon[srow][scol] = TUNNEL; /* see, now we'll reuse srow,scol */

    if ((i < 3) && (!did_this)) {
      /* If this is the first time we've made it this far, and at
	 least one of the four directions hasn't been tried yet:
	 50% chance of going through the loop again; otherwise,
	 break out of the loop after maybe doing the recursive d.e. */
      did_this = 1;
      if (coin_toss()) {
	continue;
      }
    }
    if ((rooms_found < 2) && do_rec_de) {
      /* We've found at least one room or we wouldn't be here.  We've
	 also found at most one room, and do_rec_de says we're allowed
	 to make a long dead-end path from here.
	 Starting at this room, draw a dead-end path through a chain
	 of 1 or more unused room-areas (each area must be
	 NESW-adjacent to the previous one.) */
      recursive_deadend(rn, srow, scol);
    }
    /* Let's see.  If we've made it this far, either we found 1 room
       and made a dead-end path through here from it, or we found 1
       room and weren't allowed to do that; or we got the 50% chance
       to continue and found 2 rooms (which are now connected through
       the point srow,scol even though the current room is supposed to
       be a DEADEND.) */
    break;
  }
}

/**********************************************************************
                       RECURSIVE_DEAD_END
 IN:
 rn = number of "room" to make dead end in
 srow, scol = some target location in the room
 various globals
 OUT:
 nothing
 PURPOSE:
 This will (recursively) draw a long dead-end path through some number
 of otherwise unused room-areas (each one in the sequence has to be
 directly north, south, east, or west of the previous one, and it will
 stop when it can't find a suitable candidate to continue the path.)
 **********************************************************************/
/* I'm not sure a recursive function is a good idea in 2k,
   but it uses up a room-area each time so depth is limited... */
static void recursive_deadend(Short rn, Short srow, Short scol) {
  Short i, de;
  Short drow, dcol, tunnel_dir;
  room * rooms = sotu->rooms;

  rooms[rn].is_room = R_DEADEND;
  dungeon[srow][scol] = TUNNEL;

  for (i = 0; i < 4; i++) {
    /* 'de' will be a NOTHING room-area, in the same row or col as 'rn' */
    de = rn + offsets[i];
    if (((de < 0) || (de >= MAXROOMS)) ||
	(!(same_row(rn, de) || same_col(rn, de)))) {
      continue;
    }
    if (!(rooms[de].is_room & R_NOTHING)) {
      continue;
    }
    /* drow, dcol will be the center of de. */
    drow = (rooms[de].top_row + rooms[de].bottom_row) / 2;
    dcol = (rooms[de].left_col + rooms[de].right_col) / 2;
    if (same_row(rn, de)) {
      tunnel_dir = (rooms[rn].left_col < rooms[de].left_col) ?
	RIGHT : LEFT;
    } else {
      tunnel_dir = (rooms[rn].top_row < rooms[de].top_row) ?
	DOWN : UP;
    }
    /* Connect 'rn' and 'de' */
    draw_simple_passage(srow, scol, drow, dcol, tunnel_dir, sotu);
    /* Update the "last room on the dead-end path" */
    sotu->r_de = de;
    /* Repeat with 'de' as the new 'rn'... */
    recursive_deadend(de, drow, dcol);
  }
}

/***************************************
   mask_room MOVED to LibRogue
 *************************************/


/**********************************************************************
                       MAKE_MAZE
 IN:
 r, c = row and column of "current square"
 tr, br, lc, rc = top row, bottom row, left col, right col (Bounds.) 
 various globals
 OUT:
 nothing
 PURPOSE:
 This is another recursive function.  It's disabled due to bugs.
 Draws some maziness from r,c within the designated bounds.
 **********************************************************************/
/* XXXXXXXXXX  SOURCE OF HEISENBUG  XXXXXXXXXXXXX */
static void make_maze(Short r, Short c, Short tr, Short br, Short lc, Short rc)
{
  Char dirs[4];
  Short i, t;

  ErrNonFatalDisplayIf( (r < 0 || c < 0 || r >= DROWS || c >= DCOLS) ,
			"make_maze Out Of Bounds" );
//  message("start",0);

  dirs[0] = UP;
  dirs[1] = DOWN;
  dirs[2] = LEFT;
  dirs[3] = RIGHT;

  dungeon[r][c] = TUNNEL;

  if (rand_percent(33)) {
    for (i = 0; i < 10; i++) {
      short t1, t2;

      t1 = get_rand(0, 3);
      t2 = get_rand(0, 3);

      if (t1 != t2)
	swap(dirs[t1], dirs[t2]);
    }
  }
  for (i = 0; i < 4; i++) {
    switch(dirs[i]) {
    case UP:
      if (r <= 1) break;
//      message("up",0);
      ErrNonFatalDisplayIf( (r <= 1) , "Maze Overrun 1");
      if (((r-1) >= tr) &&
	  (dungeon[r-1][c] != TUNNEL) &&
	  (dungeon[r-1][c-1] != TUNNEL) &&
	  (dungeon[r-1][c+1] != TUNNEL) &&
	  (dungeon[r-2][c] != TUNNEL)) {
	make_maze((r-1), c, tr, br, lc, rc);
      }
      break;
    case DOWN:
      if (r+2 >= DROWS) break;
//      message("down",0);
      ErrNonFatalDisplayIf( (r+2 >= DROWS) , "Maze Overrun 2");
      /* 9 times */
      if (((r+1) <= br) &&
	  (dungeon[r+1][c] != TUNNEL) &&
	  (dungeon[r+1][c-1] != TUNNEL) &&
	  (dungeon[r+1][c+1] != TUNNEL) &&
	  (dungeon[r+2][c] != TUNNEL)) {
	make_maze((r+1), c, tr, br, lc, rc);
      }
      break;
    case LEFT:
      if (c <= 1) break;
/*       message("left",0); */
      ErrNonFatalDisplayIf( (c <= 1) , "Maze Overrun 3"); /* once */
      if (((c-1) >= lc) &&
	  (dungeon[r][c-1] != TUNNEL) &&
	  (dungeon[r-1][c-1] != TUNNEL) &&
	  (dungeon[r+1][c-1] != TUNNEL) &&
	  (dungeon[r][c-2] != TUNNEL)) {
	make_maze(r, (c-1), tr, br, lc, rc);
      }
      break;
    case RIGHT:
      if (c+2 >= DCOLS) break;
/*       message("right",0); */
      ErrNonFatalDisplayIf( (c+2 >= DCOLS) , "Maze Overrun 4"); /* once */
      if (((c+1) <= rc) &&
	  (dungeon[r][c+1] != TUNNEL) &&
	  (dungeon[r-1][c+1] != TUNNEL) &&
	  (dungeon[r+1][c+1] != TUNNEL) &&
	  (dungeon[r][c+2] != TUNNEL)) {
	make_maze(r, (c+1), tr, br, lc, rc);
      }
      break;
    }
  }
  /*   message("ya",0); */
}

/***************************************
   hide_boxed_passage MOVED to LibRogue
   put_player MOVED to LibRogue
   *************************************/

/***************************************
      drop_check MOVED to LibRogue
 *************************************/

/**********************************************************************
                       CHECK_UP
 IN:
 various globals
 OUT:
 Returns true if you could in fact go up stairs.
 PURPOSE:
 Call this when the rogue tries to go up - only allowed if you have the
 amulet (and are on stairs), or are in wizard mode.
 Possible outcomes - you can't; you exit and the game is over; you
 go up a level (caller builds a new level if return value is true.)
 **********************************************************************/
Boolean check_up() {
  fighter *rogue = sotu->roguep;
  if ( !(dungeon[rogue->row][rogue->col] & STAIRS) ) {
    message("I see no way up", sotu);
    return false;
  }
  if (!(has_amulet())) {
    message("your way is magically blocked", sotu);
    return false;
  }
  // on stairs, has amulet....
  if (sotu->cur_level == 1) {
    sotu->new_level_message = "Hello again...";
    win(sotu);
    FrmPopupForm(TopTenForm);
  } else {
    sotu->new_level_message = "you feel a wrenching sensation in your gut";
    sotu->cur_level -= 2; /*  because it's ++'d when a new level is made  */
    return true;
  }
  return false;
}

/************************************************
  get_exp_level, add_exp MOVED to LibRogue
*************************************************/

/***************************************
   hp_raise MOVED to LibRogue
 *************************************/

/**********************************************************************
                       MIX_RANDOM_ROOMS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 This just shuffles an array that contains the indices of the 9
 "possible room" areas in the dungeon.  Then the caller can iterate
 on the array to get room indices in a random order...
 **********************************************************************/
static void mix_random_rooms() {
  Short i, t;
  Short x, y;

  for (i = 0; i < (3 * MAXROOMS); i++) {
    do {
      x = get_rand(0, (MAXROOMS-1));
      y = get_rand(0, (MAXROOMS-1));
    } while (x == y);
    swap(random_rooms[x], random_rooms[y]);
  }
}

/***************************************************************************
 *     Moved all of lib_level.c into here:
 ***************************************************************************/


/* t must be some local variable where this is used.. */
#define swap(x,y) {t = x; x = y; y = t;}

/* Moved make_room back into ../level.c !!! */

/***************************************************************
                   PUT_DOOR
 IN:
 rm = a room
 dir = which wall to put the door in
 row,col = (return) the location the door was put in
 sotu = various globals (dungeon, cur_level)
 OUT:
 nothing (row,col)
 PURPOSE:
 Place a door in the indicated wall of the given room, reporting
 to the caller the door's location.  Possibly, hide the door.
****************************************************************/
void put_door(room * rm, Short dir, Short * row, Short * col,
	      struct state_of_the_union * sotu)
{
  Short wall_width;
  UShort ** dungeon = sotu->dungeon;

  wall_width = (rm->is_room & R_MAZE) ? 0 : 1;

  switch(dir) {
  case UP:
  case DOWN:
    *row = ((dir == UP) ? rm->top_row : rm->bottom_row);
    do {
      *col = get_rand(rm->left_col+wall_width,
		      rm->right_col-wall_width);
    } while (!(dungeon[*row][*col] & (HORWALL | TUNNEL)));
    break;
  case RIGHT:
  case LEFT:
    *col = (dir == LEFT) ? rm->left_col : rm->right_col;
    do {
      *row = get_rand(rm->top_row+wall_width,
		      rm->bottom_row-wall_width);
    } while (!(dungeon[*row][*col] & (VERTWALL | TUNNEL)));
    break;
  }
  if (rm->is_room & R_ROOM) {
    dungeon[*row][*col] = DOOR;
  }
  if ((sotu->cur_level > 2) && rand_percent(HIDE_PERCENT)) {
    dungeon[*row][*col] |= HIDDEN;
  }
  rm->doors[dir/2].door_row = *row;
  rm->doors[dir/2].door_col = *col;

}


/**********************************************************************
                       MASK_ROOM
 IN:
 rn = number of room-area
 row, col = return location fitting the mask, if one is found
 mask = some attribute of a dungeon square...
 sotu = various globals
 OUT:
 true if a square fitting the mask is found (row,col indicate location)
 false if no square is found that fits the mask
 PURPOSE:
 Search the room-area to see if a particular kind of square is present
 there, and if it is, return one location of such a square.
 Actually, this is used ONLY by fill_it in level.c, to find whether
 there is a 'TUNNEL' in the room-area.
 **********************************************************************/
Boolean mask_room(Short rn, Short * row, Short * col, UShort mask,
		  struct state_of_the_union *sotu)
{
  Short i, j;
  UShort ** dungeon = sotu->dungeon;
  room * rooms = sotu->rooms;

  for (i = rooms[rn].top_row; i <= rooms[rn].bottom_row; i++) {
    for (j = rooms[rn].left_col; j <= rooms[rn].right_col; j++) {
      if (dungeon[i][j] & mask) {
	*row = i;
	*col = j;
	return(true);
      }
    }
  }
  return(false);
}

/***************************************************************
                   HIDE_BOXED_PASSAGE
 IN:
 row1,col1 = one end
 row2,col2 = the other end
 n = number of points to (try to) hide the passage at
 sotu = various globals (
 OUT:
 nothing
 PURPOSE:
 This will make some random square(s) of the passage "hidden",
 usually.
****************************************************************/
void hide_boxed_passage(Short row1, Short col1,
			Short row2, Short col2, 
			Short n,
			struct state_of_the_union * sotu)
{
  Short i, j, t;
  Short row, col, row_cut, col_cut;
  Short h, w;
  UShort ** dungeon = sotu->dungeon;

  if (n <= 0)
    return;

  if (sotu->cur_level > 2) {
    if (row1 > row2) {
      swap(row1, row2);
    }
    if (col1 > col2) {
      swap(col1, col2);
    }
    h = row2 - row1;
    w = col2 - col1;

    if ((w >= 5) || (h >= 5)) {
      row_cut = ((h >= 2) ? 1 : 0);
      col_cut = ((w >= 2) ? 1 : 0);

      for (i = 0; i < n; i++) {
	for (j = 0; j < 10; j++) {
	  row = get_rand(row1 + row_cut, row2 - row_cut);
	  col = get_rand(col1 + col_cut, col2 - col_cut);
	  if (dungeon[row][col] == TUNNEL) {
	    dungeon[row][col] |= HIDDEN;
	    break;
	  }
	}
      }
    }
  }
}



/***************************************************************
                   DRAW_SIMPLE_PASSAGE
 IN:
 row1, col1 = one end
 row2, col2 = the other end
 dir = left/right or up/down
 sotu = various globals (dungeon)
 OUT:
 nothing
 PURPOSE:
 Draw a simple passage  ----|______ between the two endpoints,
 and maybe hide part of it. 
****************************************************************/
void draw_simple_passage(Short row1, Short col1, 
			 Short row2, Short col2,
			 Short dir,
			 struct state_of_the_union * sotu)
{
  Short i, middle, t;
  UShort ** dungeon = sotu->dungeon;

  if ((dir == LEFT) || (dir == RIGHT)) {
    if (col1 > col2) {
      swap(row1, row2);
      swap(col1, col2);
    }
    /* decide where to zigzag */
    middle = get_rand(col1+1, col2-1);
    /* make the three segments of the passage */
    for (i = col1+1; i != middle; i++) {
      dungeon[row1][i] = TUNNEL;
    }
    for (i = row1; i != row2; i += (row1 > row2) ? -1 : 1) {
      dungeon[i][middle] = TUNNEL;
    }
    for (i = middle; i != col2; i++) {
      dungeon[row2][i] = TUNNEL;
    }
  } else {
    if (row1 > row2) {
      swap(row1, row2);
      swap(col1, col2);
    }
    /* decide where to zigzag */
    middle = get_rand(row1+1, row2-1);
    /* make the three segments of the passage */
    for (i = row1+1; i != middle; i++) {
      dungeon[i][col1] = TUNNEL;
    }
    for (i = col1; i != col2; i += (col1 > col2) ? -1 : 1) {
      dungeon[middle][i] = TUNNEL;
    }
    for (i = middle; i != row2; i++) {
      dungeon[i][col2] = TUNNEL;
    }
  }
  if (rand_percent(HIDE_PERCENT)) {
    hide_boxed_passage(row1, col1, row2, col2, 1, sotu);
  }
}



/***************************************************************
                   DROP_CHECK
 IN:
 sotu = various globals (cur_level, wizard, dungeon, rogue, levitate)
 OUT:
 true if the rogue can go down a level from here.
 PURPOSE:
 Basically, return true if you can go down stairs (you are a
 wizard or standing on stairs, and you haven't hit bottom.)
****************************************************************/
Boolean drop_check(struct state_of_the_union * sotu) 
{
  if (sotu->cur_level >= 99) {
    message("don't you think you've gone down far  enough?", sotu);
    return false; /* come, let's be reasonable */
  }
  if (IS_WIZARD) {
    return true;
  }
  if (sotu->dungeon[sotu->roguep->row][sotu->roguep->col] & STAIRS) {
    if (sotu->levitate
	|| (sotu->ring_flags & RING_LEVITATE)) {
      message("you're floating in the air!", sotu);
      return false;
    }
    return true;
  }
  message("I see no way down", sotu);
  return false;
}

/***************************************************************
                   HP_RAISE
 IN:
 sotu = various globals (wizard)
 OUT:
 number of hit points to add to character
 PURPOSE:
 Return hit points to add.  (hp was int.  used in level and spec_hit,
 when the character goes up a level... or drops 2 and goes up 1...)
****************************************************************/
Short hp_raise(struct state_of_the_union * sotu)
{
  Short hp;
  
  hp = (IS_WIZARD ? 10 : get_rand(3, 10));
  return(hp);
}


/*
// Are the two rooms in the same row?
static Boolean same_row(Short room1, Short room2) SEC_L;
static Boolean same_row(Short room1, Short room2) 
{ 
  return((room1 / 3) == (room2 / 3)); 
}
// Are the two rooms in the same column?
static Boolean same_col(Short room1, Short room2) SEC_L;
static Boolean same_col(Short room1, Short room2) 
{
  return((room1 % 3) == (room2 % 3)); 
}
*/

/***************************************************************
                   CONNECT_ROOMS
 IN:
 room1, room2 = the rooms to attempt to connect
 sotu = various globals (rooms)
 OUT:
 true if the connection was successful
 PURPOSE:
 Connect rooms... return false if they're impossible to connect.
 (If they're connectable, 4% chance of making two connections.)
****************************************************************/
Boolean connect_rooms(Short room1, Short room2, 
		      struct state_of_the_union * sotu) 
{
  Short row1, col1, row2, col2, dir;
  room * rooms = sotu->rooms;

  if ((!(rooms[room1].is_room & (R_ROOM | R_MAZE))) ||
      (!(rooms[room2].is_room & (R_ROOM | R_MAZE)))) {
    /* Can't connect non-rooms. */
    return false;
  }

  /* Figure out how the rooms are oriented wrt each other. */
  if (same_row(room1, room2) &&
      (rooms[room1].left_col > rooms[room2].right_col)) {
    put_door(&rooms[room1], LEFT, &row1, &col1, sotu);
    put_door(&rooms[room2], RIGHT, &row2, &col2,sotu);
    dir = LEFT;
  } else if (same_row(room1, room2) &&
	     (rooms[room2].left_col > rooms[room1].right_col)) {
    put_door(&rooms[room1], RIGHT, &row1, &col1, sotu);
    put_door(&rooms[room2], LEFT, &row2, &col2, sotu);
    dir = RIGHT;
  } else if (same_col(room1, room2) &&
	     (rooms[room1].top_row > rooms[room2].bottom_row)) {
    put_door(&rooms[room1], UP, &row1, &col1, sotu);
    put_door(&rooms[room2], DOWN, &row2, &col2, sotu);
    dir = UP;
  } else if (same_col(room1, room2) &&
	     (rooms[room2].top_row > rooms[room1].bottom_row)) {
    put_door(&rooms[room1], DOWN, &row1, &col1, sotu);
    put_door(&rooms[room2], UP, &row2, &col2, sotu); /* BROKEN? */
    dir = DOWN;
  } else {
    /* Can't connect rooms that aren't NSEW-adjacent. */
    return false;
  }

  /* Connect them.  4% chance of extra connecting passage(s). */
  do {
    draw_simple_passage(row1, col1, row2, col2, dir, sotu);
  } while (rand_percent(4));

  rooms[room1].doors[dir/2].oth_room = room2;
  rooms[room1].doors[dir/2].oth_row = row2;
  rooms[room1].doors[dir/2].oth_col = col2;

  rooms[room2].doors[(((dir+4)%DIRS)/2)].oth_room = room1;
  rooms[room2].doors[(((dir+4)%DIRS)/2)].oth_row = row1;
  rooms[room2].doors[(((dir+4)%DIRS)/2)].oth_col = col1;
  return true;
}



/***************************************************************
                   CLEAR_LEVEL
 IN:
 sotu = various globals (rooms, traps, dungeon, ...)
 OUT:
 nothing
 PURPOSE:
 This is called when you go up/down to a new level.  It will clear
 various things in preparation for generating the new level.
****************************************************************/
void clear_level(struct state_of_the_union * sotu) 
{
  Short i, j;

  for (i = 0; i < MAXROOMS; i++) {
    sotu->rooms[i].is_room = R_NOTHING;
    for (j = 0; j < 4; j++) {
      sotu->rooms[i].doors[j].oth_room = NO_ROOM;
    }
  }
  for (i = 0; i < MAX_TRAPS; i++) {
    sotu->traps[i].trap_type = NO_TRAP;
  }
  for (i = 0; i < DROWS; i++) {
    for (j = 0; j < DCOLS; j++) {
      sotu->dungeon[i][j] = NOTHING;
    }
  }
  sotu->detect_monster = sotu->see_invisible = false;
  sotu->being_held = sotu->bear_trap = false;
  sotu->party_room = NO_ROOM;
  sotu->roguep->row = sotu->roguep->col = -1;
  clear(false);  /* XXX need a clear() for the pilot */
}




/* Put player in level --- try to avoid room 'nr'. */
/* The stuff it calls is in room.c and monster.c mostly. XXXX */
void put_player(Short nr, struct state_of_the_union *sotu)
{
  Short rn = nr, misses;
  Short row, col;
  fighter *rogue = sotu->roguep;

  for (misses = 0; ((misses < 2) && (rn == nr)); misses++) {
    gr_row_col(&row, &col, (FLOOR | TUNNEL | OBJECT | STAIRS), sotu);
    rn = get_room_number(row, col, sotu->rooms);
  }
  rogue->row = row;
  rogue->col = col;

  if (sotu->dungeon[rogue->row][rogue->col] & TUNNEL) {
    sotu->cur_room = PASSAGE;
  } else {
    sotu->cur_room = rn;
  }
  if (sotu->cur_room != PASSAGE) {
    light_up_room(sotu->cur_room, sotu);
  } else {
    light_passage(rogue->row, rogue->col, sotu);
  }
  wake_room(get_room_number(rogue->row, rogue->col, sotu->rooms), 1, 
	    rogue->row, rogue->col, sotu);
  if (sotu->new_level_message) {
    message(sotu->new_level_message, sotu);
    sotu->new_level_message = 0;
  }
  mvaddch(rogue->row, rogue->col, rogue->fchar);

  check_rogue_position(rogue, true);
//  move_visible_window(rogue->row, rogue->col, true); /* TESTING.... */

}

/* could be static */
static Short get_exp_level(Long e, struct state_of_the_union *sotu) SEC_L;
static Short get_exp_level(Long e, struct state_of_the_union *sotu) {
  Short i;

  for (i = 0; i < (MAX_EXP_LEVEL - 1); i++) {
    if (sotu->level_points[i] > e) {
      break;
    }
  }
  return(i+1);
}
/* used in many places */
void add_exp(Int e, Boolean promotion, struct state_of_the_union *sotu) {
  Char mbuf[40];
  Short new_exp;
  Short i, hp;
  fighter * rogue = sotu->roguep;

  rogue->exp_points += e;

  if (rogue->exp_points >= sotu->level_points[rogue->exp-1]) {
    new_exp = get_exp_level(rogue->exp_points, sotu);
    if (rogue->exp_points > MAX_EXP) {
      rogue->exp_points = MAX_EXP + 1;
    }
    for (i = rogue->exp+1; i <= new_exp; i++) {
      StrPrintF(mbuf, "welcome to level %d", i);
      message(mbuf, sotu);
      if (promotion) {
	hp = hp_raise(sotu);
	rogue->hp_current += hp;
	rogue->hp_max += hp;
      }
      rogue->exp = i;
      print_stats(STAT_HP | STAT_EXP, sotu);
    }
  } else {
    print_stats(STAT_EXP, sotu);
  }
}
