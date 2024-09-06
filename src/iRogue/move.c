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
 * move.c
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

static Boolean check_hunger(Boolean messages_only) SEC_1;
static void heal() SEC_1;

extern struct state_of_the_union * sotu;
extern Boolean i_am_dead;
extern RoguePreferenceType my_prefs;

//Char you_can_move_again[] = "you can move again";


/**********************************************************************
                       ONE_MOVE_ROGUE
 IN:
 dir = direction, defined by a number 1 to 8, see rogue_defines.h
 pickup = a boolean, whether the rogue will pick up objects
 various globals
 OUT:
 The move failed (stop repeatedly moving in that direction)..
 The move was made and something of interest was encountered (stop)..
 The move was made but nothing of interest was encountered (keep going)..
 PURPOSE:
 Try to move the rogue one space in the indicated direction.
 (Various special cases.)  Then perform a clock tick.
 **********************************************************************/
extern Boolean crosshairs_dirty;
Short one_move_rogue(Short dir, Boolean pickup, Boolean stay_centered) {
  fighter *rogue = sotu->roguep;
  Short row, col;
  object *obj;
  Char desc[DCOLS];
  Short n, status;
  //Boolean object_in_pack = true;
  Boolean pack_too_full = false;

  row = rogue->row;
  col = rogue->col;

  /* If the rogue is confused, move in a random direction... */ 
  if (sotu->confused) {
    dir = get_rand(1, 8);
  }

  /* From 'row' and 'col', calculate the next square in
     direction 'dir', and update 'row' and 'col' with the location.
     'true' means don't check whether it's out of bounds, I guess.  */
  get_dir_rc(dir, &row, &col, true);

  /* Is it possible, in general, to move from here to there? */
  if (!can_move(rogue->row, rogue->col, row, col, dungeon)) {
    return(MOVE_FAILED);
  }

  /* Is it possible for the rogue to move, at all? */
  if (sotu->being_held || sotu->bear_trap) {
    if (!(dungeon[row][col] & MONSTER)) {
      /* you can still attack, even if you can't _move_ */
      if (sotu->being_held) {
	message("you are being held", sotu);
	/* no clock tick. */
      } else {
	message("you are still stuck in the bear trap", sotu);
	reg_move(); /* clock tick */
      }
      return(MOVE_FAILED);
    }
  }

  if (crosshairs_dirty) { invert_crosshairs(); crosshairs_dirty = false; }

  /* Is the rogue under a teleport curse? */
  if (sotu->ring_flags & RING_TELEPORT) {
    if (rand_percent(R_TELE_PERCENT)) {
      tele(sotu); // rogue should be centered
      /* no clock tick? */
      return(STOPPED_ON_SOMETHING);
    }
  }

  /* Is there a monster there to attack? */
  if (dungeon[row][col] & MONSTER) {
    /* 0 means calculate hit probability first; then damage the monster */
    rogue_hit(object_at(level_monsters, row, col), 0);
    reg_move(); /* clock tick */
    refresh(); /* need this to make message visible..? */
    return(MOVE_FAILED);
  }

  /* The rogue can move there and there's no monster there.  So, move. */

  /* Scroll and redraw the screen, if necessary. */
  check_rogue_position(rogue, stay_centered); 

  /* {Light, wake, unlight} a {passage, room} on {entry, exit} */
  if (dungeon[row][col] & DOOR) {
    if (sotu->cur_room == PASSAGE) {
      /* Moving to a room's door */
      sotu->cur_room = get_room_number(row, col, sotu->rooms);
      light_up_room(sotu->cur_room, sotu);
      wake_room(sotu->cur_room, 1, row, col, sotu);
    } else {
      /* Moving to a passage's door */
      light_passage(row, col, sotu);
    }
  } else if ((dungeon[rogue->row][rogue->col] & DOOR) &&
	     (dungeon[row][col] & TUNNEL)) {
    /* Moving from a room's door to a passage */
    light_passage(row, col, sotu);
    wake_room(sotu->cur_room, 0, rogue->row, rogue->col, sotu);
    darken_room(sotu->cur_room, sotu);
    sotu->cur_room = PASSAGE;
  } else if (dungeon[row][col] & TUNNEL) {
    /* Moving in a passage...? */
    light_passage(row, col, sotu);
  }

  /* Update the screen: old and new position of rogue */
  mvaddch(rogue->row, rogue->col, 
	  get_dungeon_char(rogue->row, rogue->col, sotu));
  mvaddch(row, col, rogue->fchar);
  refresh();

  /* Update the rogue's knowledge of its position */
  rogue->row = row;
  rogue->col = col;

  /* If there is an object, maybe pick it up. */
  if (dungeon[row][col] & OBJECT) {
    if (pickup) {
      /* want to pick it up */
      if (sotu->levitate || (sotu->ring_flags & RING_LEVITATE)) {
	/* can't pick it up */
	return(STOPPED_ON_SOMETHING);
      } else {
	/* can pick it up... */
	if ((obj = pick_up(row, col, &status))) {
	  /* ...successfully picked it up */
	  get_desc(obj, desc, sotu);
	  if (obj->what_is == GOLD) {
	    free_object(obj, sotu);
	    //object_in_pack = false;
	  } else {
	    /* add pack letter to message (wonder why I bother) */
	    n = StrLen(desc);
	    desc[n] = '(';
	    desc[n+1] = obj->ichar;
	    desc[n+2] = ')';
	    desc[n+3] = 0;
	  }
	} else if (!status) {
	  /* ...item was destroyed! */
	  goto MVED; /* (message was already printed, skip ahead) */
	} else {
	  /* ...can't carry item! */
	  pack_too_full = true;
	}
      }
    }
    if (!pickup || pack_too_full) {
      obj = object_at(level_objects, row, col);
      StrCopy(desc, "moved onto ");
      get_desc(obj, desc+11, sotu);
    }
    message(desc, sotu);
    (void) reg_move(); /* clock tick */
    return(STOPPED_ON_SOMETHING);
  }

  /* Ok, there was not an object there... */
  if (dungeon[row][col] & (DOOR | STAIRS | TRAP)) {
    /* ...but there was something else "interesting" */
    if (!(sotu->levitate
	  || (sotu->ring_flags & RING_LEVITATE))
	&& (dungeon[row][col] & TRAP)) {
      trap_player(row, col);
    }
    (void) reg_move(); /* clock tick */
    return(STOPPED_ON_SOMETHING);
  }

MVED:	
  if (reg_move()) {
    /* Fainting from hunger is "interesting". */
    return(STOPPED_ON_SOMETHING);
  }
  /* Being confused is "interesting". */
  return((sotu->confused ? STOPPED_ON_SOMETHING : MOVED));
}

/**********************************************************************
                       MULTIPLE_MOVE_ROGUE (shift, control)
 IN:
 dir = direction, defined by a number 1 to 8, see rogue_defines.h
 [go_speed_racer = an option to follow non-forking twists/turns]
 various globals
 OUT:
 nothing
 PURPOSE:
 Try to move the rogue in the indicated direction, until the rogue
 is "interrupted" or something interesting is encountered, or the
 rogue's progress is impeded.
 **********************************************************************/
void multiple_move_rogue_Shift(Short dir) {
  /* This function is currently unused.  Its notion of "interesting"
     is much more restricted than ..._Ctrl's. */
  if (dir < NORTH || dir > NORTHWEST) return;
  while ( (!sotu->interrupted) &&
	  (one_move_rogue(dir, true, false) == MOVED) ) 
    ;
}

void multiple_move_rogue_Ctrl(Short dirch, Boolean go_speed_racer) {
  fighter *rogue = sotu->roguep;
  Short row, col, new_dir;
  Short m;
  Boolean moved = false;
  if (dirch < NORTH || dirch > NORTHWEST) return;
  /* Move until stopped, or on / *adjacent to* something interesting. */
  do {
    row = rogue->row;
    col = rogue->col;
    m = one_move_rogue(dirch, true, false);
    if ((m == STOPPED_ON_SOMETHING) ||
	sotu->interrupted) {
      break;
    }
    if (m == MOVE_FAILED) {
      /* Can't move that way - optionally, follow non-forking turns. */
      if (!go_speed_racer || sotu->cur_room != PASSAGE)
	break;
      else {
	/* if the passage makes a right turn, take it! */
	new_dir = turns_p(rogue->row, rogue->col, dirch, sotu->dungeon);
	if (new_dir == 0)
	  break;
	dirch = new_dir;
	m = one_move_rogue(dirch, true, false); /* seems to be necessary */
      }
    } else {
      moved = true;
    }
  } while (!next_to_something(row, col, sotu));
  // now, make us be centred again, if desired by player.
  if (moved && my_prefs.stay_centered)
    check_rogue_position(rogue, true); 
}

// moved turns_p to chuvmey_nolib.c

/************************************************
  is_passable MOVED to lib
  next_to_something MOVED to lib
************************************************/

/************************************************
  can_move MOVED to lib
  ************************************************/

/* move_onto replaced by a popup dialog */
/* is_direction no longer used */

/**********************************************************************
                       CHECK_HUNGER
 IN:
 messages_only = if true, don't kill a sufficiently-faint rogue.
 various globals
 OUT:
 true=the rogue has fainted due to lack of food.
 PURPOSE:
 Display any necessary warnings of hunger.
 If not messages_only, kill a starved rogue, and perform a hunger
 clock-tick (make the rogue slightly more hungry.)
 **********************************************************************/
static Boolean check_hunger(Boolean messages_only) {
  fighter *rogue = sotu->roguep;
  Short i, n;
  Boolean fainted = 0;

  if (rogue->moves_left == HUNGRY) {
    StrCopy(sotu->hunger_str, "hungry");
    message(sotu->hunger_str, sotu);
    print_stats(STAT_HUNGER, sotu);
  }
  if (rogue->moves_left == WEAK) {
    StrCopy(sotu->hunger_str, "weak");
    message(sotu->hunger_str, sotu);
    print_stats(STAT_HUNGER, sotu);
  }
  if (rogue->moves_left <= FAINT) {
    if (rogue->moves_left == FAINT) {
      StrCopy(sotu->hunger_str, "faint");
      message(sotu->hunger_str, sotu);
      print_stats(STAT_HUNGER, sotu);
    }
    /* the rogue will faint-from-hunger with increasing frequency */
    n = get_rand(0, (FAINT - rogue->moves_left));
    if (n > 0) {
      fainted = 1;
      if (rand_percent(40)) {
	rogue->moves_left++;
      }
      message("you faint", sotu);
      for (i = 0; i < n; i++) {
	if (coin_toss()) {
	  mv_mons(); // if dead, return
	  if (i_am_dead) return true;	  
	}
      }
      message("you can move again", sotu);
    }
  }
  if (messages_only) {
    /* We just wanted a message, not a hunger clock-tick. */
    return (fainted);
  }

  /* Kill the rogue from starvation. */
  if (rogue->moves_left <= STARVE) {
    killed_by((object *) 0, STARVATION, sotu);
    FrmPopupForm(TopTenForm);
    return(fainted);
  }

  /* Make the rogue hungrier.
     (Each ring is +1, unless it is SLOW_DIGEST, then it's -1.) */
  // I really need to rework this "algorithm".  Sometime later.
  if (sotu->e_rings <= -8) {
    ; // all SLOW_DIGEST rings.  use no food!
  } else if (sotu->e_rings <= -1) {
    // SLOW_DIGEST rings outweigh other rings.  use half-normal food.
    rogue->moves_left -= (rogue->moves_left % 2);
  } else if (sotu->e_rings == 0) {
    rogue->moves_left--;
  } else if (sotu->e_rings > 1) {
    /* a few rings: subtract 1.5 */
    rogue->moves_left--;
    (void) check_hunger(1); /* do all the message stuff again! */
    rogue->moves_left -= (rogue->moves_left % 2);
  } else {
    /* lots of rings: subtract 2 */
    rogue->moves_left--;
    (void) check_hunger(1); /* do all the message stuff again! */
    rogue->moves_left--;
  }

  return(fainted);
}


/**********************************************************************
                       REG_MOVE
 IN:
 various globals
 OUT:
 true=the rogue has fainted due to lack of food.
 PURPOSE:
 This is the major clock-tick of the game.  It moves monsters, creates
 new wandering monsters, updates "decaying" characteristics of the rogue, 
 etc.
 **********************************************************************/
Boolean reg_move() {
  fighter *rogue = sotu->roguep;
  Boolean fainted;
  Char glowbuf[48];
  Char *color;

  if ((rogue->moves_left <= HUNGRY) || (sotu->cur_level >= sotu->max_level)) {
    fainted = check_hunger(0);
    if (i_am_dead)
      return true; // ...since you can die of hunger
  } else {
    /* Apparently you don't get hungrier if you're going back up
       (i.e. you've found the amulet) and you're currently well-fed. */
    fainted = 0;
  }

  /* Move the monsters! */
  mv_mons(); // if dead, return
  if (i_am_dead)
    return true;

  /* If the rogue is "slow", move the monsters again! */
  if (sotu->haste_self < 0) {
    mv_mons(); // if dead, return
    if (i_am_dead)
      return true;
  }

  /* Periodically, create a new "wandering" monster */
  if (++sotu->m_moves >= 120) {
    sotu->m_moves = 0;
    wanderer();
  }

  /* Decrement all decaying characteristics */

  if ((sotu->halluc > 1) || (sotu->ring_flags & RING_DELUSION))
    hallucinate(sotu); // revise the display for all monsters/items
  if (sotu->halluc) {
    sotu->halluc--; // decrement even if we have a ring of delusion on
    if (!sotu->halluc && !(sotu->ring_flags & RING_DELUSION))
      unhallucinate(sotu);
  }

  if (sotu->blind) {
    if (!(--sotu->blind)) {
      unblind(sotu);
    }
  }
  if (sotu->confused) {
    if (!(--sotu->confused)) {
      unconfuse(sotu);
    }
  }
  if (sotu->bear_trap) {
    sotu->bear_trap--;
  }
  if (sotu->levitate) {
    if (!(--sotu->levitate) && !(sotu->ring_flags & RING_LEVITATE)) {
      message("you float gently to the ground", sotu);
      if (dungeon[rogue->row][rogue->col] & TRAP) {
	trap_player(rogue->row, rogue->col); /* ow */
      }
    }
  }
  if (sotu->haste_self > 0) {
    if (!(--sotu->haste_self)) {
      message("you feel yourself slowing down", sotu); // end of Haste
    }
  } else if (sotu->haste_self < 0) {
    if (!(++sotu->haste_self)) {
      message("you feel yourself speeding up", sotu); // end of Slow
    }
  }
  /* Make the mood ring glow periodically */
  if (rand_percent(1) && (sotu->ring_flags & RING_MOOD) && !sotu->blind) {
    color = sotu->id_potions[get_rand(0, POTIONS-1)].title;
    StrPrintF(glowbuf, "your ring shines with %s %sglow",
	      (is_vowel(color[0]) ? "an" : "a"),
	      color);
    message(glowbuf, sotu);
  }
  /* If the warning level is high enough, or it's been a while,
     emit a ring-of-warning message */
  if ((sotu->ring_flags & RING_WARNING) && !sotu->blind)
    calculate_warning(sotu);

  /* Heal the rogue a bit. */
  heal();

  /* Free search if auto-search is on */
  if (sotu->auto_search > 0) {
    search(sotu->auto_search, sotu->auto_search);
  }

  refresh();  /* I guess I need this for hallucinating? */
  return(fainted);
}

/**********************************************************************
                       REST
 IN:
 count = number of turns to rest, unless interrupted.
 various globals
 OUT:
 nothing
 PURPOSE:
 Tick the clock while the rogue just sits there and, hopefully,
 something unpleasant wears off / hit points are regained.
 The rogue's nap may be interrupted by e.g. a monster.
 **********************************************************************/
void rest(Short count) {
  Short i;
/*    Char buf[10];  */

  message("resting", sotu);
  sotu->interrupted = false;

  for (i = 0; i < count; i++) {
    if (sotu->interrupted) {
      break;
    }
    reg_move(); /* clock tick */
  }
  /*
  StrPrintF(buf, "rested %d", i);
  message(buf, sotu); 
  */
}

/**********************************************************************
                       HEAL
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Cause the rogue to regain some hit points (if applicable.)
 **********************************************************************/
Short map_healexp_to_n[12] = {20, 18, 17, 14, 13, 10, 9, 8, 7, 4, 3};
static void heal() {
  fighter *rogue = sotu->roguep;
  static Short heal_exp = -1;
  static Short n; /* you heal somewhat, every n turns */
  static Short c = 0; /* this is the counter that needs to reach n */
  static Boolean alt; /* you heal an average of 1.5 points each time */

  if (rogue->hp_current == rogue->hp_max) {
    c = 0;
    return;
  }
  if (rogue->exp != heal_exp) {
    heal_exp = rogue->exp; /* heal_exp == rogue's level... */

    /* this used to be a twelve-case 'switch' statement... */
    n = 2; /* (default) */
    if (1 <= heal_exp && heal_exp < 12) {
      n = map_healexp_to_n[heal_exp-1];
    }

  }
  /* every n turns, you heal "1.5" points, plus regeneration if any. */
  if (++c >= n) {
    c = 0;
    rogue->hp_current++;
    if ((alt = !alt)) {
      rogue->hp_current++;
    }
    if ((rogue->hp_current += sotu->regeneration) > rogue->hp_max) {
      rogue->hp_current = rogue->hp_max;
    }
    print_stats(STAT_HP, sotu);
  }
}

/***************************************************************************
 *     Moved all of lib_move.c into here:
 ***************************************************************************/


/***************************************************************
                   CAN_MOVE
 IN:
 row1, col1 = initial location
 row2, col2 = desired location
 dungeon
 OUT:
 true if you can move from one to the other
 PURPOSE:
 Determine whether you can move to the desired destination...
****************************************************************/
Boolean can_move(Short row1, Short col1, Short row2, Short col2,
		 UShort ** dungeon) {
  if (!is_passable(row2, col2, dungeon)) {
    return false;
  }
  if ((row1 != row2) && (col1 != col2)) {
    /* You can't move diagonal through a door... */
    if ((dungeon[row1][col1]&DOOR)||(dungeon[row2][col2]&DOOR)) {
      return false;
    }
    /* ... or a tunnel */
    if ((!dungeon[row1][col2]) || (!dungeon[row2][col1])) {
      return false;
    }
  }
  return true;
}

/***************************************************************
                   IS_PASSABLE
 IN:
 row, col = location in question
 dungeon
 OUT:
 true if it's ok for a monster/rogue to go to there
 PURPOSE:
 Determine whether the given location can be moved-to.
****************************************************************/
Boolean is_passable(Short row, Short col, UShort ** dungeon) {
  if ((row < MIN_ROW) || (row > (DROWS - 2)) || (col < 0) ||
      (col > (DCOLS-1))) {
    return false;
  }
  if (dungeon[row][col] & HIDDEN) {
    return((dungeon[row][col] & TRAP) ? true : false);
  }
  return(dungeon[row][col] & (FLOOR | TUNNEL | DOOR | STAIRS | TRAP));
}

/***************************************************************
                   NEXT_TO_SOMETHING
 IN:
 drow, dcol = previous location of rogue
 sotu = various globals (rogue, dungeon, confused, blind)
 OUT:
 PURPOSE:
 Determine whether the rogue should drop out of autopilot,
 i.e. there's something of interest to the player.
****************************************************************/
Boolean next_to_something(Short drow, Short dcol,
			  struct state_of_the_union * sotu) {
  Short i, j, i_end, j_end, row, col;
  Short pass_count = 0;
  UShort s;
  fighter * rogue = sotu->roguep;
  UShort ** dungeon = sotu->dungeon;

  if (sotu->confused) {
    /* Do not let a confused rogue move under autopilot :-) */
    return true;
  }
  if (sotu->blind) {
    /* If you can't see, you can't see interesting things next to you. */
    return false;
  }

/*    i_end = (rogue->row < (DROWS-2)) ? true : false; */
/*    j_end = (rogue->col < (DCOLS-1)) ? true : false; */
  i_end = (rogue->row < (DROWS-2)) ? 1 : 0;
  j_end = (rogue->col < (DCOLS-1)) ? 1 : 0;

  for (i = ((rogue->row > MIN_ROW) ? -1 : 0); i <= i_end; i++) {
    for (j = ((rogue->col > 0) ? -1 : 0); j <= j_end; j++) {
      if ((i == 0) && (j == 0)) {
	/* "Stand in the place where you are..." */
	continue;
      }
      if (((rogue->row+i) == drow) && ((rogue->col+j) == dcol)) {
	/* The place where you were... */
	continue;
      }
      row = rogue->row + i;
      col = rogue->col + j;
      s = dungeon[row][col];
      if (s & HIDDEN) {
	/* Something interesting, but you can't see it. */
	continue;
      }
      /* Monsters, objects, and stairs are all interesting. */
      if (s & (MONSTER | OBJECT | STAIRS)) {
	/* If the rogue's previous location is directly north/south or
	   east/west of the location under examination, then don't
	   bother stopping (the player's already seen the thing, and is
	   currently moving away from it.) */
	if (((row == drow) || (col == dcol)) &&
	    (!((row == rogue->row) || (col == rogue->col)))) {
	  continue;
	}
	return true;
      }
      /* Non-hidden traps are interesting. */
      if (s & TRAP) {
	if (!(s & HIDDEN)) {
	  /* Same "don't bother" as above. */
	  if (((row == drow) || (col == dcol)) &&
	      (!((row == rogue->row) || (col == rogue->col)))) {
	    continue;
	  }
	  return true;
	}
      }
      /* If we're in a tunnel, and this isn't the square we came from,
	 and it's n/s/e/w (not diagonal), then it is a Way To Go and
	 is interesting if there's more than one (i.e. a branch.) */
      if ((((i - j) == 1) || ((i - j) == -1)) && (s & TUNNEL)) {
	if (++pass_count > 1) {
	  return true;
	}
      }
      /* If you are n/s/e/w of a door, that is also interesting. */
      if ((s & DOOR) && ((i == 0) || (j == 0))) {
	return true;
      }
    }
  }
  /* Nothing interesting - keep on truckin' */
  return false;
}
