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
 * monster.c
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
extern Boolean i_am_dead;

static void put_m_at(Short row, Short col, object *monster) SEC_1;

object * level_monsters;

// I removed 0,0,0 from the end of each struct
// It was o_row, o_col, o; not needed since my_malloc initializes to 0
/* m_flags1, m_flags2, {damage}, hit pts, 'char',
   exp, start-level, end-level, to-hit-you, HOLD-damage, drop-% */
#define M_
object mon_tab[MONSTERS] = {
  {(RUSTS),  (ASLEEP|WAKENS|WANDERS),{0,0,0,0}, 25, 'A',  20, 9, 18,100,0,  0}, // aquatar
  {(FLITS),  (ASLEEP|WANDERS),       {1,3,0,0}, 10, 'B',   2, 1,  8, 60,0,  0}, // bat
  { 0,       (ASLEEP|WANDERS|FLEEING),       {3,3,2,5}, 32, 'C',  15, 7, 16, 85,0, 10}, // centaur
  {(FLAMES), (ASLEEP|WAKENS),        {4,6,4,9},145, 'D',5000,21,126,100,0, 90}, // dragon
  { 0, (ASLEEP|WAKENS),              {1,3,0,0}, 11, 'E',   2, 1,  7, 65,0,  0}, // emu
  {(HOLDS),  (STATIONARY),           {5,5,0,0}, 73, 'F',  91,12,126, 80,0,  0}, // fly-trap
  {(FLIES),  (ASLEEP|WAKENS|WANDERS),{5,5,5,5},115, 'G',2000,20,126, 85,0, 10}, // griffin
  { 0,     (ASLEEP|WAKENS|WANDERS),  {1,3,1,2}, 15, 'H',   3, 1, 10, 67,0,  0}, // hobgoblin
  {(INVISIBLE), (ASLEEP),            {4,4,0,0}, 15, 'I',1000, 17,126,68,0,  0}, // invisible dude
  { 0,     (ASLEEP|WANDERS),         {3,10,4,5},132,'J',3000,21,126,100,0,  0}, // jabberwock
  {(FLIES), (ASLEEP|WAKENS|WANDERS), {1,4,0,0}, 10, 'K',   2, 1,  6, 60,0,  0}, // kestrel
  {(STEALS_GOLD), (ASLEEP),          {0,0,0,0}, 25, 'L',  21, 6, 16, 75,0,  0}, // leprechaun
  {(CONFUSES), (ASLEEP|WAKENS|WANDERS),
                                     {4,4,3,7}, 97, 'M', 250,18,126, 85,0, 25}, // medusa
  {(STEALS_ITEM), (ASLEEP),          {0,0,0,0}, 25, 'N',  39,10, 19, 75,0,100}, // nymph
  {(SEEKS_GOLD), (ASLEEP|WAKENS|WANDERS|TASTY),
                                     {1,6,0,0}, 25, 'O',   5, 4, 13, 70, 1, 10}, // orc
  {(INVISIBLE|FLITS), (ASLEEP|WANDERS),
                                     {5,4,0,0}, 76, 'P', 120,15, 24, 80,0, 50}, // phantom
  { 0, (ASLEEP|WAKENS|WANDERS),      {3,5,0,0}, 30, 'Q',  20, 8, 17, 78,0, 20}, // questing beast
  {(STINGS), (ASLEEP|WAKENS|WANDERS|TASTY),
                                     {2,5,0,0}, 19, 'R',  10, 3, 12, 70, 2,  0}, // rattlesnake
  { 0, (ASLEEP|WAKENS|WANDERS),      {1,3,0,0},  8, 'S',   2, 1,  9, 50,0,  0}, // snake
  { 0, (ASLEEP|WAKENS|WANDERS),      {4,6,1,4}, 75, 'T', 125,13, 22, 75,0, 33}, // troll
  { 0, (ASLEEP|WAKENS|WANDERS),      {4,10,0,0},90, 'U', 200,17, 26, 85,0, 33}, // unicorn
  {(DRAINS_LIFE|SUCKER), (ASLEEP|WAKENS|WANDERS),
                                     {1,14,1,4},55, 'V', 350,19,126, 85,0, 18}, // vampire
  {(DROPS_LEVEL), (ASLEEP|WANDERS),  {2,8,0,0}, 45, 'W',  55,14, 23, 75,0,  0}, // wraith
  {(IMITATES), (ASLEEP),             {4,6,0,0}, 42, 'X', 110,16, 25, 75,0,  0}, // xeroc
  { 0, (ASLEEP|WANDERS),             {3,6,0,0}, 35, 'Y',  50,11, 20, 80,0, 20}, // yeti
  { 0, (ASLEEP|WAKENS|WANDERS),      {1,7,0,0}, 21, 'Z',   8, 5, 14, 69,0,  0}, // zombie

  { 0,  (ASLEEP|WAKENS|WANDERS),     {1,4,0,0}, 10, 'a',   2, 1,  8, 60,0,  0}, // ant == bat
  { 0,  (ASLEEP|WAKENS|WANDERS),     {1,4,0,0}, 10, 'b',   2, 1,  8, 60,0,  0}, // beetle == ant
  {(FLAMES|STINGS), (ASLEEP),        {3,6,0,0}, 35, 'c',  60,11, 20, 80,B_FLAME,  0}, // chimera == yeti
  { 0,  (ASLEEP|WAKENS|WANDERS),     {1,4,0,0}, 10, 'd',   2, 1,  8, 60,0,  0}, // dingo == bat
  {(PARALYZES), (STATIONARY|TASTY),  {1,1,0,0}, 10, 'e',   3, 1,  8, 90, 3,  0},// floating eye
  { 0,  (ASLEEP|WAKENS|WANDERS),     {1,4,0,0}, 10, 'f',   2, 1,  8, 60,0,  0}, // feral cat == bat
  {(CONFUSES),  (ASLEEP),            {0,0,0,0}, 20, 'g',   5, 2, 11, 60,0, 90}, // gwyllion
  { 0,  (ASLEEP|WAKENS|WANDERS),     {4,5,1,4}, 70, 'h', 120,13, 22, 75,0, 33}, // hannya == troll
  {(FREEZES), (ASLEEP|TASTY),        {0,0,0,0}, 15, 'i',   5, 3, 12, 68, 4,  0},// ice monster
  { 0,  (ASLEEP|WAKENS|WANDERS),     {2,6,0,0}, 45, 'j',  25, 4, 13, 70,0,  0}, // jaguar == orc
  { 0,  (ASLEEP|WAKENS|WANDERS),     {1,4,0,0},  5, 'k',   1, 98, 99, 10,0,  0},
  {(SUCKER), (ASLEEP|WAKENS|WANDERS),{2,5,0,0}, 19, 'l',  10, 3, 12, 70,0,  0}, // leech == rattlesnake
  {(STINGS), (ASLEEP|WAKENS|WANDERS),{3,6,0,0}, 35, 'm',  60,11, 20, 80,0, 10}, // manticore == yeti
  {(FLAMES), (ASLEEP|WAKENS|WANDERS),{1,4,0,0}, 50, 'n', 100,30,126, 90, B_SLEEP,  0},     // narcolep
  {(SHRIEKS), (STATIONARY),          {1,6,0,0}, 10, 'o',   5,16,126, 70,0,  0}, // orange mold
  {(HUGS),  (ASLEEP|WAKENS|WANDERS|TASTY),
                                     {2,5,0,0}, 19, 'p',  10, 3, 12, 70, 2,  0}, // python == rattlesnake
  {(DROPS_LEVEL),  (ASLEEP|WAKENS|WANDERS),
                                     {1,4,0,0}, 25, 'q',  20, 12,126, 90,0,  0},    // 64-bit int
  {(RUSTS),  (ASLEEP|WAKENS|WANDERS),{0,0,0,0},100, 'r',  40, 16,126, 20,0,  0},
  {(STINGS), (ASLEEP|WAKENS|WANDERS),{2,5,0,0}, 19, 's',  10, 3, 12, 70,0,  0}, // giant spider == rattlesnake
  {(PARALYZES|CONFUSES|FLITS|SHRIEKS|SEEKS_GOLD), (ASLEEP|WAKENS|WANDERS),
                                     {0,0,0,0},145, 't',1000,21,125,100,0, 90}, // todal == dragon
  {(SUCKER),  (WANDERS),             {1,14,1,4},55, 'u', 300,19,126, 85,0, 40}, // upier == vampire
  {(STINGS), (ASLEEP|WAKENS|WANDERS|TASTY),
                                     {3,6,0,0},150, 'v',2000,19,126, 60, 2,  0}, // vindshield viper
  {(SHRIEKS), (ASLEEP|WAKENS),       {4,4,0,0}, 80, 'w', 200,25,126,100,0,  0},
  {(FLAMES), (ASLEEP),               {4,6,0,0},100, 'x',2500,21, 126, 75,B_LGHTN, 90}, // xavier
  {(FLAMES), (ASLEEP),               {1,2,0,0}, 20, 'y',   1, 3,   7, 10,B_SLOW,  0}, // yellow light
  { 0,  (HASTED|WANDERS),            {1,4,0,0}, 15, 'z',   3, 3, 10, 10,0,  0} // zoomer
};

/* put_mons was here */

/**********************************************************************
                       MV_MONS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 This is called at each "clock tick" to move the monsters on the
 current dungeon level.
 **********************************************************************/
void mv_mons() {
  fighter *rogue = sotu->roguep;
  object *monster, *next_monster;
  Boolean flew;

  if (i_am_dead)
    return;
  //  if (rogue->hp_current <= 0)
  //    return;

  /* When the rogue is magically sped-up, what really happens is that
     the monsters move once for every two clock ticks. */
  if ((sotu->haste_self > 0) && (sotu->haste_self % 2)) {
    return;
  }

  /* Move each monster in the list of monsters.... */
  monster = level_monsters->next_monster;

  while (monster && !i_am_dead) {
    next_monster = monster->next_monster;
    if (monster->m_flags2 & HASTED) {
      /* If the monster is hasted, it will move twice for each tick. */
      sotu->mon_disappeared = false;
      mv_monster(monster, rogue->row, rogue->col);
      if (sotu->mon_disappeared) {
        /* it stole something and vanished.. it doesn't exist anymore. */
	monster = next_monster;
	continue;
      }
    } else if (monster->m_flags2 & SLOWED) {
      /* If the monster is slowed, it moves once per two ticks. */
      monster->slowed_toggle = !monster->slowed_toggle;
      if (monster->slowed_toggle) {
	/* the slow monster doesn't get to move this time */
	monster = next_monster;
	continue;
      }
    }
    if ((monster->m_flags2 & CONFUSED) && move_confused(monster, sotu)) {
      /* confused monster successfully made a confused move.. next! */
      monster = next_monster;
      continue;
    }
    flew = 0;
    if ((monster->m_flags1 & FLIES) && !(monster->m_flags2 & NAPPING) &&
	!mon_can_go(monster, rogue->row, rogue->col, sotu)) {
      /* if it can fly, and can't hit the rogue yet, move toward the rogue */
      flew = 1;
      mv_monster(monster, rogue->row, rogue->col);
    }
    if (!(flew && mon_can_go(monster, rogue->row, rogue->col, sotu))) {
      /* If it didn't just fly toward the rogue,
	 or it just flew toward the rogue but still can't hit it,
	 then move toward the rogue. */
      mv_monster(monster, rogue->row, rogue->col);
    }
    monster = next_monster;
  }
}

/**********************************************************************
                       PARTY_MONSTERS
 IN:
 rn = room number that the monster party will be placed in
 n = usually the number of 'treasure' objects being guarded there
 various globals
 OUT:
 nothing
 PURPOSE:
 This is called sometimes when a new dungeon level is being created.
 It will create the monsters in a "party room" (a room that has a
 bunch of monsters and a bunch of objects.)
 **********************************************************************/
void party_monsters(Short rn, Short n) {
  /* args were int,int */
  Short i, j;
  Short row = 0, col = 0;
  object *monster;
  Boolean found;
  room * rooms = sotu->rooms;

  /* We'll try to create 2n monsters, where n is usually the number
     of objects in the hoard we've already placed in this room. */
  n += n;

  /* Temporarily change the level that each monster is "allowed" to
     appear at or below.  (A monster can appear sooner in a party?) */
  for (i = 0; i < MONSTERS; i++) {
    mon_tab[i].first_level -= (sotu->cur_level % 3);
  }

  for (i = 0; i < n; i++) {
    /* If the room is full, stop trying to add monsters to it. */
    if (no_room_for_monster(rn, rooms, dungeon)) {
      break;
    }
    /* Randomly try to find a place in the room to put the monster. */
    for (j = found = 0; ((!found) && (j < 250)); j++) {
      row = get_rand(rooms[rn].top_row+1,
		     rooms[rn].bottom_row-1);
      col = get_rand(rooms[rn].left_col+1,
		     rooms[rn].right_col-1);
      if ((!(dungeon[row][col] & MONSTER)) &&
	  (dungeon[row][col] & (FLOOR | TUNNEL))) {
	found = 1;
      }
    }
    /* If we found a place (within 250 tries), put a monster there. */
    if (found) {
      monster = gr_monster((object *) 0, 0);
      if (!(monster->m_flags1 & IMITATES)) {
	monster->m_flags2 |= WAKENS;
      }
      put_m_at(row, col, monster);
    }
  }

  /* Undo the making-party-monsters-funkier trick. */
  for (i = 0; i < MONSTERS; i++) {
    mon_tab[i].first_level += (sotu->cur_level % 3);
  }
}

/**********************************************************************
                       GR_MONSTER
 IN:
 monster = storage space to put the monster in (returned)
 mn = index to the kind of monster in the monster table
 various globals
 OUT:
 the monster
 PURPOSE:
 Generate a monster (possibly reusing a previously alloced space, in
 which case mn is assumed to be meaningful; otherwise it's ignored.)
 **********************************************************************/
object * gr_monster(object * monster, Short mn) {
  /* mn was just 'register' */
  if (!monster) {
    monster = alloc_object(sotu);
    if (mn)
      mn--;
    else {
      for (;;) {
	mn = get_rand(0, MONSTERS-1);
	if ((sotu->cur_level >= mon_tab[mn].first_level) &&
	    (sotu->cur_level <= mon_tab[mn].last_level)) {
	  break;
	}
      }
    }
    // DEBUG - set 'mn' to 0-25 if you want to debug a monster type
    //    mn = 8; // generate only ice monsters, to test them
    //    mn = 23; // generate only xerocs, to test them
  }
  *monster = mon_tab[mn];
  if (monster->m_flags1 & IMITATES) {
    monster->disguise = gr_obj_char();
  }
  if (sotu->cur_level > (AMULET_LEVEL + 10)) {
    monster->m_flags2 |= HASTED;
  }
  monster->trow = NO_ROOM;
  return(monster);
}

/************************************************
  gmc_row_col MOVED to LibRogue
  gmc MOVED to LibRogue
************************************************/

/**********************************************************************
                       MV_AQUATARS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Move all the aquatars on the level.
 (apparently they move toward you when you take off armor.)
 **********************************************************************/
void mv_aquatars() {
  fighter *rogue = sotu->roguep;
  object *monster;

  monster = level_monsters->next_monster;

  while (monster) {
    if ((monster->m_flags1 & RUSTS) &&
	mon_can_go(monster, rogue->row, rogue->col, sotu)) {
      mv_monster(monster, rogue->row, rogue->col);
      monster->m_flags2 |= ALREADY_MOVED;
    }
    monster = monster->next_monster;
  }
}

/**********************************************************************
                       MV_MONSTER
 IN:
 monster = the monster to move
 row, col = location to move toward
 OUT:
 nothing
 PURPOSE:
 Move the monster toward the location... or, if possible, attack it.
 **********************************************************************/
void mv_monster(object *monster, Short row, Short col) {
  fighter *rogue = sotu->roguep;
  Short i, n;
  Boolean tried[6];
  Short tropism;
/*    Char buf[40]; */

  if (i_am_dead)
    return;

  if (monster->m_flags2 & ASLEEP) {
    /* potentially wake up sleeping monsters */
    if (monster->m_flags2 & NAPPING) {
      if (--monster->nap_length <= 0) {
	monster->m_flags2 &= (~(NAPPING | ASLEEP));
      }
      return;
    }
    if ((monster->m_flags2 & WAKENS) &&
	rogue_is_around(monster->row, monster->col, rogue) &&
	rand_percent(((sotu->stealthy > 0) ?
		      (WAKE_PERCENT / (STEALTH_FACTOR + sotu->stealthy)) :
		      WAKE_PERCENT))) {
      wake_up(monster);
    }
    return;
  } else if (monster->m_flags2 & ALREADY_MOVED) {
    /* this keeps wakeful aquatars from moving twice... see 'mv_aquatars' */
    monster->m_flags2 &= (~ALREADY_MOVED);
    return;
  }

  if ( ((monster->m_flags1 & FLITS) && flit(monster, sotu))
       || ((monster->m_flags2 & STATIONARY) &&
	   (!mon_can_go(monster, rogue->row, rogue->col, sotu)))
       || (monster->m_flags2 & FREEZING_ROGUE)
       || ((monster->m_flags1 & CONFUSES) && m_confuse(monster))  ) {
    /* monster is pre-occupied or can't move... */
    return;
  }

  /*
  if ((monster->m_flags2 & FLEEING)
      && mon_sees(monster, rogue->row, rogue->col, sotu->rooms)) {
    StrPrintF(buf, "the %c quivers", monster->m_char);
    message(buf, sotu);
  }
  if ((monster->m_flags1 & SUCKER)
      && mon_sees(monster, rogue->row, rogue->col, sotu->rooms)) {
    StrPrintF(buf, "the %c looks hungry", monster->m_char);
    message(buf, sotu);
  }
  */

  if (!(monster->m_flags2 & FLEEING)
      // || (monster->m_flags2 & STATIONARY)
      ) {
    // monsters that are trying to flee won't hit you.

    if (mon_can_go(monster, rogue->row, rogue->col, sotu)) {
      /* monster can reach rogue ==> spend this turn in melee */
      mon_hit(monster, (char *) 0, 0);
      return;
    }
    if ((monster->m_flags1 & FLAMES) && breath_attack(monster)) {
      /* monster can't reach rogue, but breath attack is possible */
      return;
    }
    if ((monster->m_flags1 & SEEKS_GOLD) && seek_gold(monster)) {
      /* gold-seeking monster can't reach rogue, but found gold to nap on */
      return;
    }
  }

  /* The monster can't do anything "interesting", so, just move. */

  /* trying to implement 'fleeing' monsters, approximately */
  if (monster->m_flags2 & FLEEING)
    tropism = -1; // add/subtract 'tropism' instead of 1
  else
    tropism = 1;

  /* row,col is the default destination (used if ->trow is invalid);
     usually the caller has passed in the rogue's location;
     a monster can also try to reach some other goal which is
     maintained in ->trow,->tcol. */
  if ((monster->trow == monster->row) &&
      (monster->tcol == monster->col)) {
    monster->trow = NO_ROOM;
  } else if (monster->trow != NO_ROOM) {
    row = monster->trow;
    col = monster->tcol;
  }
  /* Figure out which adjacent row to move to, or stay in this row  */
  if (monster->row > row) {
    row = monster->row - tropism;
  } else if (monster->row < row) {
    row = monster->row + tropism;
  }
  /* (You can't move diagonally through a door.  Go n/s if one is there.) */
  if ((dungeon[row][monster->col] & DOOR) &&
      mtry(monster, row, monster->col, sotu)) {
    return;
  }
  /* Figure out which adjacent column to move to, or stay in this column  */
  if (monster->col > col) {
    col = monster->col - tropism;
  } else if (monster->col < col) {
    col = monster->col + tropism;
  }
  /* (You can't move diagonally through a door.  Go e/w if one is there.) */
  if ((dungeon[monster->row][col] & DOOR) &&
      mtry(monster, monster->row, col, sotu)) {
    return;
  }
  /* Try to move to the calculated adjacent location. */
  if (mtry(monster, row, col, sotu)) {
    return;
  }
  /*
  StrPrintF(buf, "at %d,%d goal %d,%d @ %d,%d %d,%d",
	    monster->row, monster->col,
	    monster->trow, monster->tcol,
	    rogue->row, rogue->col, row, col);
  message(buf, sotu);
  */
  /* Alas, the monster was unable to move directly towards its goal.
     Try to move to a square that is in the same row or the same
     column as the square that the monster wanted to move to.  (yes,
     this could cause the monster to move in the opposite direction,
     or to try a square twice.) */

  for (i = 0; i <= 5; i++)
    tried[i] = 0;

  for (i = 0; i < 6; i++) {
    do {
      /* Find a number we haven't tried yet. */
      n = get_rand(0, 5);
    } while (tried[n]);
    tried[n] = 1; /* ok now remember that we've tried this one */
    switch(n) {
    case 0:
      if (mtry(monster, row, monster->col-1, sotu)) {
	/* go to o when the monster has successfully moved. */
	goto O;
      }
      break;
    case 1:
      if (mtry(monster, row, monster->col, sotu)) {
	goto O;
      }
      break;
    case 2:
      if (mtry(monster, row, monster->col+1, sotu)) {
	goto O;
      }
      break;
    case 3:
      if (mtry(monster, monster->row-1, col, sotu)) {
	goto O;
      }
      break;
    case 4:
      if (mtry(monster, monster->row, col, sotu)) {
	goto O;
      }
      break;
    case 5:
      if (mtry(monster, monster->row+1, col, sotu)) {
	goto O;
      }
      break;
    }
  } /* end for i - did we find one that worked? */
 O:
  if ((monster->row == monster->o_row) && (monster->col == monster->o_col)) {
    /* The monster did NOT successfully move on this turn. */
    if (++(monster->o) > 4) {
      /* The monster will attempt a non-random or rogueward goal
	 for 5? consecutive failures before trying something else. */
      if ((monster->trow == NO_ROOM) &&
	  (!mon_sees(monster, rogue->row, rogue->col, sotu->rooms))) {
	/* Monster's goal has been to move toward the unseen rogue;
	   it's failed for several turns.  Try a totally random goal.
	   (choose new random goals each turn until one works.)  */
	monster->trow = get_rand(1, (DROWS - 2));
	monster->tcol = get_rand(0, (DCOLS - 1));
      } else {
	/* The monster can see the rogue, or the monster has a
	   non-rogue goal that hasn't been working.  On the next turn,
	   try to move toward the rogue again. */
	monster->trow = NO_ROOM;
	monster->o = 0;
      }
    }
  } else {
    /* The monster moved.. update the "old" row and col for next time. */
    monster->o_row = monster->row;
    monster->o_col = monster->col;
    /* Reset the consecutive-failure counter. */
    monster->o = 0;
  }
}


/************************************************
  mtry MOVED to LibRogue
  move_mon_to MOVED to LibRogue
  mon_can_go MOVED to LibRogue
  wake_up MOVED to lib
  wake_room MOVED to lib
*************************************************/
/*****************************************
  mon_name has NOW moved to lib_monster
  rogue_is_around has MOVED to lib_monster
*****************************************/


/**********************************************************************
                       WANDERER
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Create a new "wandering" monster.  This is *after* level creation,
 i.e. periodically while the rogue is moving about the level.
 (There is some possibility that it won't create one.)
 **********************************************************************/
void wanderer() {
  object *monster;
  Short row, col, i;
  Boolean found = 0;

  /* Try to create a monster that could be described as a "wanderer". */
  for (i = 0; ((i < 15) && (!found)); i++) {
    monster = gr_monster((object *) 0, 0);
    if (!(monster->m_flags2 & (WAKENS | WANDERS))) {
      free_object(monster, sotu);
    } else {
      found = 1;
    }
  }
  if (found) {
    found = 0;
    wake_up(monster);
    /* Try to put the monster somewhere that the rogue can't see it. */
    for (i = 0; ((i < 25) && (!found)); i++) {
      gr_row_col(&row, &col, (FLOOR | TUNNEL | STAIRS | OBJECT), sotu);
      if (!rogue_can_see(row, col, sotu)) {
	put_m_at(row, col, monster);
	found = 1;
      }
    }
    /* Couldn't find a good place to put it. */
    if (!found) {
      free_object(monster, sotu);
    }
  }
}

/************************************************
  show_monsters MOVED to LibRogue
************************************************/

/**********************************************************************
                       CREATE_MONSTER
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Create and place a monster *as a result of the rogue using some
 magical item.*
 **********************************************************************/
void create_monster(Short mn) {
  fighter *rogue = sotu->roguep;
  Short row, col;
  Short i;
  Boolean found = 0;
  object *monster;

  row = rogue->row;
  col = rogue->col;

  /* Try to find a good adjacent place to put the monster. */
  for (i = 0; i < 9; i++) {
    rand_around(i, &row, &col);
    if (((row == rogue->row) && (col = rogue->col)) ||
	(row < MIN_ROW) || (row > (DROWS-2)) ||
	(col < 0) || (col > (DCOLS-1))) {
      continue;
    }
    if ((!(dungeon[row][col] & MONSTER)) &&
	(dungeon[row][col] & (FLOOR|TUNNEL|STAIRS|DOOR))) {
      found = 1;
      break;
    }
  }
  if (found) {
    /* Found a place, put a monster there. */
    monster = gr_monster((object *) 0, min(mn, 52));
    put_m_at(row, col, monster);
    mvaddch(row, col, gmc(monster, sotu));
    if (monster->m_flags2 & (WANDERS | WAKENS)) {
      wake_up(monster);
    }
  } else {
    /* Couldn't find a good place (within 9 random tries).  Aww. */
    message("you hear a faint cry of anguish in the distance", sotu);
  }
}

/**********************************************************************
                       PUT_M_AT
 IN:
 row, col = location to put the monster at
 monster = monster to put there
 various globals
 OUT:
 nothing
 PURPOSE:
 This will put a newly created monster in its initial location.
 (Whether it was created when the level was, or during exploration.)
 **********************************************************************/
static void put_m_at(Short row, Short col, object *monster) {
  monster->row = row;
  monster->col = col;
  dungeon[row][col] |= MONSTER;
  monster->trail_char = mvinch(row, col);
  (void) add_to_pack(monster, level_monsters, 0, sotu);
  /* Give the monster some initial goal to move toward, like a door */
  aim_monster(monster, sotu->rooms);
}

/************************************************
  aim_monster MOVED to LibRogue
  rogue_can_see MOVED to LibRogue
  move_confused MOVED to LibRogue
  flit MOVED to LibRogue
  gr_obj_char MOVED to lib
  no_room_for_monster MOVED to lib
  aggravate MOVED to lib
  mon_sees MOVED to lib
*************************************************/


/**********************************************************************
                       PUT_MONS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 This will create and place some the initial non-party monsters on a
 level, at level creation time. 
 **********************************************************************/
void put_mons() {
  Short i;
  Short n;
  object *monster;
  Short row, col;

  n = get_rand(4, 6);

  for (i = 0; i < n; i++) {
    monster = gr_monster((object *) 0, 0);
    if ((monster->m_flags2 & WANDERS) && coin_toss()) {
      /* linker can't find wake_up if put_mons is back where it was */
      /* (I wrote that comment before making a wake_up jump island?) */
      wake_up(monster);
    }
    gr_row_col(&row, &col, (FLOOR | TUNNEL | STAIRS | OBJECT), sotu);
    put_m_at(row, col, monster);
  }
}
