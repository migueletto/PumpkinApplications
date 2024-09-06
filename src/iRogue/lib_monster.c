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


/** m_names and mon_name() are probably BAD to have
    in the library, for reusability.. mon_names ok
    if m_names is just a pointer or something */
/* oh, hell with it. */
// This would take up code space instead of data space, due to *:
// const Char *m_names[] = { ... };
// .... well ....
// .... in the brave new world of multigen I don't think I need to care ....

// Hm.  Lowercase-L and uppercase-I will be like
// monarch and viceroy butterflies.
// I should think of some way to make this a "good" thing.
//const Char m_names[MONSTERS][17] = {
// takes less space declared this way:
const Char *m_names[MONSTERS] = {
  "aquator",
  "bat",
  "centaur",
  "dragon",
  "emu",
  "venus fly-trap",
  "griffin",
  "hobgoblin",
  "invisible dude",
  "jabberwock",
  "kestrel",
  "leprechaun",
  "medusa",
  "nymph",
  "orc",
  "phantom",
  "questing beast", // was quagga
  "rattlesnake",
  "snake",
  "troll",
  "black unicorn",
  "vampire",
  "wraith",
  "xeroc",
  "yeti",
  "zombie",

  "great-ant",
  "beetle",
  "chimera",
  "dingo",
  "floating eye",
  "felis silvestris",  // felis margarita? felis chaus?
  "gwyllion",
  "hannya",
  "ice monster",
  "jaguarundi",
  "vermicious knid", // new
  "leech",
  "manticore",
  "narcolep", // new
  "orange mold",
  "python",
  "64-bit int", // new.... needs a special attack
  "rust monster", // new
  "giant spider",
  "todal",
  "upier",
  "vindshield viper", // new
  "killer wail", // new
  "xavier",
  "yellow light",
  "zoomer" // new
};

/***************************************************************
                   COPY_MON_NAME
 IN:
 index = the index of the type of monster whose name is desired
 foo = AT LEAST 17 CHARACTERS of space to put the name in.
 OUT:
 nothing (well, foo)
 PURPOSE:
 Necessary for extra-library users of m_names.
****************************************************************/
/* foo should be at least 17 chars... */
void copy_mon_name(Short index, Char *foo) {
  if ((index < 0) || (index >= MONSTERS))
    return;
  StrCopy(foo, m_names[index]);
}
void print_mon_name(Short index, struct state_of_the_union *sotu) {
  if ((index < 0) || (index >= MONSTERS))
    return;
  message((char *)m_names[index], sotu); // wonder if this will work
}

/***************************************************************
                   MON_NAME
 IN:
 monster = the monster whose name is desired
 foo = space to put the monster name in (at least 15 characters!)
 sotu = various globals (blind, halluc, detect_monster, (r_)see_invisible)
 OUT:
 nothing (well, foo)
 PURPOSE:
 Get the name of the monster, like for when it hits you.
****************************************************************/
void mon_name(object *monster, Char *foo, struct state_of_the_union *sotu) {
  Short ch;

  if (sotu->blind
      || ((monster->m_flags1 & INVISIBLE)
	  && !(sotu->detect_monster
	       || sotu->see_invisible
	       || (sotu->ring_flags & RING_SEE_INVISIBLE)))) {
    StrCopy(foo, "something");
    return;
  }
  if ((sotu->halluc || (sotu->ring_flags & RING_DELUSION))
      && !(sotu->ring_flags & RING_ESP)) {
    //    ch = get_rand('A', 'Z') - 'A';
    ch = get_rand(0, MONSTERS-1);
    StrCopy(foo, m_names[ch]);
    return; // But really it should use the name of the ch you (think you) see!
  }
  //  ch = monster->m_char - 'A';
  ch = monster->m_char - 'A';
  if (ch > 25)
    ch = 26 + monster->m_char - 'a'; // yo ho!
  StrCopy(foo, m_names[ch]);
  return;
}

/* end use of m_names */

/***************************************************************
                   ROGUE_IS_AROUND
 IN:
 row, col = location of interest
 rogue
 OUT:
 true if the rogue is "around" the location of interest
 PURPOSE:
 Determine whether the rogue is on one of the nine squares
 adjacent to / at the location of interest.
 (e.g. if row,col is where a monster is standing, the monster's
 goal is "attack" rather than "wander about semi-aimlessly")
****************************************************************/
Boolean rogue_is_around(Short row, Short col, fighter *rogue) {
  Short rdif, cdif, retval;

  rdif = row - rogue->row;
  cdif = col - rogue->col;

  retval = (rdif >= -1) && (rdif <= 1) && (cdif >= -1) && (cdif <= 1);
  return(retval);
}

/***************************************************************
                   WAKE_UP
 IN:
 monster = to be woken
 OUT:
 nothing
 PURPOSE:
 Wake the indicated monster (if it's not napping)...
****************************************************************/
void wake_up(object *monster) {
  if (monster && !(monster->m_flags2 & NAPPING)) {
    monster->m_flags1 &= (~IMITATES);
    monster->m_flags2 &= (~(ASLEEP | WAKENS));
  }
}

/***************************************************************
                   GR_OBJ_CHAR
 IN:
 nothing
 OUT:
 the visual representation of a randomly selected object type
 PURPOSE:
 Generate random object-char (useful for hallucinations)
****************************************************************/
Char gr_obj_char() {
  Short r;
  Char *rs = "%!?]=/):*";

  r = get_rand(0, 8);
  if (r==7)
    return FOODCHAR; /* replace ':' */
  else
    return(rs[r]);
}

/***************************************************************
                   MON_SEES
 IN:
 monster = the monster in question
 row,col = the location that the monster maybe sees
 rooms
 OUT:
 true if the monster can see that location from where it is
 PURPOSE:
 Determine whether the given monster can see the given location
 (is in the same room with it, or is within a "lighted" tunnel
 distance from it).  Used e.g. to determine movement goals.
****************************************************************/
Boolean mon_sees(object *monster, Short row, Short col, room * rooms) {
  Short rn, rdif, cdif, retval;

  rn = get_room_number(row, col, rooms);

  if (	(rn != NO_ROOM) &&
	(rn == get_room_number(monster->row, monster->col, rooms)) &&
	!(rooms[rn].is_room & R_MAZE)) {
    return true;
  }
  rdif = row - monster->row;
  cdif = col - monster->col;

  retval = (rdif >= -1) && (rdif <= 1) && (cdif >= -1) && (cdif <= 1);
  return(retval);
}

/***************************************************************
                   GMC
 IN:
 monster = the monster whose visual representation is needed
 sotu = various globals (detect_monster, (r_)see_invisible, blind)
 OUT:
 a character, the visual representation of the monster
 PURPOSE:
 Determine how the rogue currently sees the monster (or would
 see it if the monster and the rogue were in the same room.)
****************************************************************/
Char gmc(object *monster, struct state_of_the_union * sotu) 
{
  if (sotu->blind
      || ((monster->m_flags1 & INVISIBLE)
	  && !(sotu->detect_monster
	       || sotu->see_invisible
	       || (sotu->ring_flags & RING_SEE_INVISIBLE)))) {
    return(monster->trail_char);
  }
  if (monster->m_flags1 & IMITATES) {
    return(monster->disguise);
  }
  return(monster->m_char);
}

/***************************************************************
                   AIM_MONSTER
 IN:
 monster = the monster that wants aiming
 rooms
 OUT:
 nothing
 PURPOSE:
 This will supply the monster with an initial movement goal:
 a random door in the current room that leads to.. somewhere.
****************************************************************/
void aim_monster(object *monster, room * rooms) {
  Short i, rn, d, r;

  rn = get_room_number(monster->row, monster->col, rooms);
  r = get_rand(0, 12);

  for (i = 0; i < 4; i++) {
    d = (r + i) % 4;
    if (rooms[rn].doors[d].oth_room != NO_ROOM) {
      monster->trow = rooms[rn].doors[d].door_row;
      monster->tcol = rooms[rn].doors[d].door_col;
      break;
    }
  }
}

/***************************************************************
                   ROGUE_CAN_SEE
 IN:
 row,col = the location in question
 sotu = various globals (blind, rooms, cur_room, rogue)
 OUT:
 true if the rogue can see the location in question
 PURPOSE:
 Determine whether the rogue can currently see the given location
 (e.g. if something at that location changes, update-p the display)
****************************************************************/
Boolean rogue_can_see(Short row, Short col, struct state_of_the_union * sotu) 
{
  Boolean retval;

  retval = !sotu->blind &&
    (((get_room_number(row, col, sotu->rooms) == sotu->cur_room) &&
      !(sotu->rooms[sotu->cur_room].is_room & R_MAZE)) ||
     rogue_is_around(row, col, sotu->roguep));

  return(retval);
}


/***************************************************************
                   NO_ROOM_FOR_MONSTER
 IN:
 rn = room number
 rooms
 dungeon
 OUT:
 true if 'rn' is chock full of nuts^H^H^H^Hmonsters
 PURPOSE:
 Determine whether the given room has room for another monster,
 or not (used in monster placement.)
****************************************************************/
/* rn was int */
Boolean no_room_for_monster(Short rn, room * rooms, UShort ** dungeon) {
  Short i, j;

  for (i = rooms[rn].top_row+1; i < rooms[rn].bottom_row; i++) {
    for (j = rooms[rn].left_col+1; j < rooms[rn].right_col; j++) {
      if (!(dungeon[i][j] & MONSTER)) {
	return false;
      }
    }
  }
  return true;
}

/***************************************************************
                   MON_CAN_GO
 IN:
 monster = the monster that wants to go
 row,col = the location it wants to go to
 sotu = various globals (dungeon, level_objects)
 OUT:
 true if the monster can go to that location
 PURPOSE:
 Used when the monster is deciding where to move, etc. ...
****************************************************************/
Boolean mon_can_go(object *monster, Short row, Short col,
		   struct state_of_the_union *sotu) {
  fighter *rogue = sotu->roguep;
  object *obj;
  Short dr, dc;
  UShort ** dungeon = sotu->dungeon;

  /* The monster can't move by more than 1 row and/or column. */
  dr = monster->row - row;
  dc = monster->col - col;
  if ((dr >= 2) || (dr <= -2) || (dc >= 2) || (dc <= -2)) {
    return false;
  }

  /* In a tunnel, the monster can't move diagonally. */
  if ((!dungeon[monster->row][col]) || (!dungeon[row][monster->col])) {
    return false;
  }

  /* Check whether the destination is impassable or occupied */
  if ((!is_passable(row, col, dungeon)) || (dungeon[row][col] & MONSTER)) {
    return false;
  }

  /* The monster can't go diagonally through a door. */
  if ( (monster->row != row) && 
       (monster->col != col) && 
       ( (dungeon[row][col] & DOOR) ||
	 (dungeon[monster->row][monster->col] & DOOR))) {
    return false;
  }

  /* If the monster is goal-oriented and its goal is to move rogueward,
     don't allow it to move in a strictly antirogueward direction. */
  if (!(monster->m_flags1 & (FLITS | CAN_FLIT))
      && (!(monster->m_flags2 & CONFUSED))
      && (monster->trow != NO_ROOM)) {
    // XXXX or should it be == NO_ROOM ?
    if ((monster->row < rogue->row) && (row < monster->row)) return false;
    if ((monster->row > rogue->row) && (row > monster->row)) return false;
    if ((monster->col < rogue->col) && (col < monster->col)) return false;
    if ((monster->col > rogue->col) && (col > monster->col)) return false;
  }

  /* Also, monsters can't move onto a SCARE_MONSTER scroll. */
  if (dungeon[row][col] & OBJECT) {
    obj = object_at(sotu->level_objects, row, col);
    if ((obj->what_is == SCROLL) && (obj->which_kind == SCARE_MONSTER)) {
      return false;
    }
  }

  /* Looks like it is ok to go there! */
  return true;
}


/***************************************************************
                   WAKE_ROOM
 IN:
 rn = room number
 entering = whether the rogue is entering or leaving the room
 row,col = where the rogue is entering/leaving from
 sotu = various globals (party_room, stealthy, level_monsters, rooms)
 OUT:
 nothing
 PURPOSE:
 The rogue is entering/leaving a room and there is a chance that this
 will wake some monsters there.  (All monsters in the room get their
 movement goal updated!)
****************************************************************/
void wake_room(Short rn, Boolean entering, Short row, Short col,
	       struct state_of_the_union *sotu) {
  object *monster;
  Short wake_percent;
  Boolean in_room;

  /* Calculate probability of rogue waking monsters. */
  wake_percent = (rn == sotu->party_room) ? PARTY_WAKE_PERCENT : WAKE_PERCENT;
  if (sotu->stealthy > 0) {
    wake_percent /= (STEALTH_FACTOR + sotu->stealthy);
  }

  monster = sotu->level_monsters->next_monster;

  while (monster) {
    in_room = (rn == get_room_number(monster->row, monster->col, sotu->rooms));
    if (in_room) {
      if (entering) {
	/* the rogue's visible from here */
	monster->trow = NO_ROOM;
      } else {
	/* set movement goal to the rogue's last known location (a door) */
	monster->trow = row;
	monster->tcol = col;
      }
      if (monster->m_flags2 & WAKENS) {
	if (rand_percent(wake_percent)) {
	  wake_up(monster);
	}
      }
    }
    monster = monster->next_monster;
  }
}


/***************************************************************
                   SHOW_MONSTERS
 IN:
 sotu = various globals (detect_mosnter, blind, level_monsters
 OUT:
 nothing
 PURPOSE:
 Reveal all monsters on the level (even imitators).
 (even invisible ones?)
****************************************************************/
void show_monsters(struct state_of_the_union *sotu) {
  object *monster;

  sotu->detect_monster = true;

  if (sotu->blind) {
    return;
  }
  monster = sotu->level_monsters->next_monster;

  while (monster) {
    mvaddch(monster->row, monster->col, monster->m_char);
    if (monster->m_flags1 & IMITATES) {
      monster->m_flags1 &= (~IMITATES);
      monster->m_flags2 |= WAKENS;
    }
    monster = monster->next_monster;
  }
}

/***************************************************************
                   AGGRAVATE
 IN:
 sotu = various globals (level_monsters)
 OUT:
 nothing
 PURPOSE:
 Wake all monsters on the level (even imitators) and make sure
 the visible ones are displayed properly.
****************************************************************/
void aggravate(struct state_of_the_union *sotu) {
  object *monster;

  //  message("you hear a high pitched humming noise", sotu);

  monster = sotu->level_monsters->next_monster;

  while (monster) {
    wake_up(monster);
    monster->m_flags1 &= (~IMITATES);
    if (rogue_can_see(monster->row, monster->col, sotu)) {
      mvaddch(monster->row, monster->col, monster->m_char);
    }
    monster = monster->next_monster;
  }
}

/***************************************************************
                   GMC_ROW_COL
 IN:
 row,col = location to check for monster
 sotu = various globals (level_monsters, detect_monster,
                         (r_)see_invisible, blind)
 OUT:
 character representing the monster as the rogue would see it
 PURPOSE:
 Given a location, see if there is a monster there, and get
 the visual representation of it.
****************************************************************/
/* was register,register */
Char gmc_row_col(Short row, Short col, struct state_of_the_union *sotu) {
  object *monster;
  Short retval;

  if ((monster = object_at(sotu->level_monsters, row, col))) {
    if (sotu->blind
	|| ((monster->m_flags1 & INVISIBLE)
	    && !(sotu->detect_monster
		 || sotu->see_invisible
		 || (sotu->ring_flags & RING_SEE_INVISIBLE)))) {
      retval = monster->trail_char;
      return(retval);
    }
    if (monster->m_flags1 & IMITATES) {
      return(monster->disguise);
    }
    return(monster->m_char);
  } else {
    return('&');	/* BUG if this ever happens */
  }
}



/***************************************************************
                   MOVE_MON_TO
 IN:
 monster = the monster to move
 row,col = the location to move it to
 sotu = various globals (dungeon, detect_monster, blind, rooms, cur_room,
                         (r_)see_invisible)
 OUT:
 nothing
 PURPOSE:
 Move the monster to the location (caller should have determined
 already whether it's possible/appropriate to do so.)
****************************************************************/
void move_mon_to(object *monster, Short row, Short col,
		 struct state_of_the_union *sotu) 
{
  Short c;
  Short mrow, mcol;
  UShort ** dungeon = sotu->dungeon;

  mrow = monster->row;
  mcol = monster->col;

  dungeon[mrow][mcol] &= ~MONSTER;
  dungeon[row][col] |= MONSTER;

  c = mvinch(mrow, mcol);

  if ( Is_Alpha(c) ) {
    if (!sotu->detect_monster) {
      // the rogue can see the square, use the trail character
      mvaddch(mrow, mcol, monster->trail_char);
    } else {
      if (rogue_can_see(mrow, mcol, sotu)) {
	mvaddch(mrow, mcol, monster->trail_char);
      } else {
	// the rogue can see the monster via magic, _can't_ see the trail.
	if (monster->trail_char == '.') {
	  monster->trail_char = ' ';
	}
	mvaddch(mrow, mcol, monster->trail_char);
      }
    }
  }
  monster->trail_char = mvinch(row, col);// why is the Xeroc not doing this???
  if (monster->m_flags1 & IMITATES) {
    monster->trail_char = '"';
  }
  
  if (!sotu->blind && 
      (sotu->detect_monster || rogue_can_see(row, col, sotu)))
    {
      if ( ( !(monster->m_flags1 & INVISIBLE) ||
	     (sotu->detect_monster || 
	      sotu->see_invisible || 
	      (sotu->ring_flags & RING_SEE_INVISIBLE)))) 
	{
	  mvaddch(row, col, gmc(monster, sotu));
	}
    }
  if ((dungeon[row][col] & DOOR) &&
      (get_room_number(row, col, sotu->rooms) != sotu->cur_room) &&
      (dungeon[mrow][mcol] == FLOOR) && !sotu->blind) {
    mvaddch(mrow, mcol, ' ');
  }
  if (dungeon[row][col] & DOOR) {
    dr_course(monster, ((dungeon[mrow][mcol] & TUNNEL) ? 1 : 0),
	      row, col, sotu);
  } else {
    monster->row = row;
    monster->col = col;
  }
}



/***************************************************************
                   MTRY
 IN:
 monster = the monster to (try to) move
 row,col = the location to (try to) move it to
 sotu = various globals...
 OUT:
 true if the monster can and did move there.
 PURPOSE:
 Determine whether the monster can be moved to the location;
 if it can, move it there (and return true).
****************************************************************/
/* try to move monster */
Boolean mtry(object *monster, Short row, Short col,
	     struct state_of_the_union *sotu)
{
  if (mon_can_go(monster, row, col, sotu)
      && (row != sotu->roguep->row || col != sotu->roguep->col)) {
    move_mon_to(monster, row, col, sotu);
    return true;
  }
  return false;
}

/***************************************************************
                   MOVE_CONFUSED
 IN:
 monster = the confused monster
 sotu = various globals (
 OUT:
 true if the monster "moved".
 PURPOSE:
 Usually, cause the monster to move in a random direction.
 (if "false" is returned, I think the caller will move the monster in
 a "non-confused" direction.)
****************************************************************/
/* shrug */
Boolean move_confused(object *monster, struct state_of_the_union *sotu)
{
  Short i, row, col;

  if (!(monster->m_flags2 & ASLEEP)) {
    /* See if the monster's done being confused yet. */
    if (--monster->moves_confused <= 0) {
      monster->m_flags2 &= (~CONFUSED);
    }
    if (monster->m_flags2 & STATIONARY) {
      /* 50% chance a stationary monster decides not to move;
	 otherwise, well, it still isn't going anywhere. */
      return(coin_toss() ? true : false);
    } else if (rand_percent(15)) {
      /* 15% chance that a self-propelled monster decides to stay put. */
      return true;
    }
    row = monster->row;
    col = monster->col;

    for (i = 0; i < 9; i++) {
      rand_around(i, &row, &col);
      if ((row == sotu->roguep->row) && (col == sotu->roguep->col)) {
	return false;
      }
      if (mtry(monster, row, col, sotu)) {
	return true;
      }
    }
  }
  return false;
}

/***************************************************************
                   FLIT
 IN:
 monster = the flittable monster
 sotu = various globals (rogue)
 OUT:
 true if the monster decided to flit about.
 PURPOSE:
 Make a flitting monster move in a random flitty sort of way.
****************************************************************/
Boolean flit(object *monster, struct state_of_the_union *sotu)
{
  Short i, row, col;

  if (!rand_percent(FLIT_PERCENT)) {
    /* 100-FLIT_PERCENT chance the flitter decides to move rationally */
    return false;
  }
  if (rand_percent(10)) {
    /* 10% chance the flitter decides not to move */
    return true;
  }
  row = monster->row;
  col = monster->col;

  /* Pick a random direction to move in */
  for (i = 0; i < 9; i++) {
    rand_around(i, &row, &col);
    if ((row == sotu->roguep->row) && (col == sotu->roguep->col)) {
      continue;
    }
    if (mtry(monster, row, col, sotu)) {
      return true;
    }
  }
  /* Couldn't find a random direction - pretend we decided not to move */
  return true;
}
