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
 * zap.c
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

/**********************************************************************
                       ZAPP
 IN:
 dir = direction, defined by a number 1 to 8, see rogue_defines.h
 index = the index of the wand in the rogue's pack (consider as a list)
 sotu = various globals
 OUT:
 whether it was a "legal move" and you need to tick the clock.
 PURPOSE:
 Zap with a wand!  Possibly hitting a monster.
 **********************************************************************/
Boolean zapp(Short dir, Short index, struct state_of_the_union * sotu) {
  object *wand;
  Short row, col;
  object *monster;
  fighter *rogue = sotu->roguep;
  UShort kind;
  Boolean zlong;

  if (dir == NO_DIRECTION) {
    return false;
  }

  if (!(wand = get_nth_object(index, &rogue->pack))) {
    message("no such item.", sotu);
    return false;
  }
  if ((wand->what_is != WAND)
      && !((wand->what_is == WEAPON) && (wand->o_flags & O_ZAPPED))) {
    // this should never happen...
    message("you can't zap with that", sotu);
    return false;
  }
  if (wand->class <= 0) {
    if (wand->what_is == WEAPON) {
      message("the dancing blue light fades away!", sotu);
      wand->o_flags &= ~O_ZAPPED; // remove the zapped characteristic
    } else
      message("nothing happens", sotu);
  } else {
    wand->class--;
    row = rogue->row;
    col = rogue->col;
    /* is it a wand, or a zapped weapon. */
    kind = (wand->what_is == WAND) ? wand->which_kind : LIGHTNING;
    /* find a monster in this general direction.. if any. */
    zlong = ((kind == SLOW_MONSTER) || (kind == HASTE_MONSTER)
	     || (kind == CONFUSE_MONSTER) || (kind == LIGHTNING));
    monster = get_zapped_monster(dir, &row, &col, zlong ? 164 : 149,
				 zlong, sotu);
    if (monster) {
      wake_up(monster);
      zap_monster(monster, kind);
      relight(sotu); /* an easy way to redraw unhappy monster? */
    }
  }
  return true; /* (caller must call reg_move now) */
}

/* get_zapped_monster MOVED to LibRogue */

/**********************************************************************
                       ZAP_MONSTER
 IN:
 monster = the affected monster
 kind = type of wand it was zapped with
 various globals
 OUT:
 nothing
 PURPOSE:
 Do something interesting to the affected monster.
 **********************************************************************/
void zap_monster(object *monster, UShort kind) {
  Short row, col;
  object *nm;
  Short tc;

  row = monster->row;
  col = monster->col;

  switch(kind) {
  case SLOW_MONSTER:
    if (monster->m_flags2 & HASTED) {
      monster->m_flags2 &= (~HASTED);
    } else {
      monster->slowed_toggle = 0;
      monster->m_flags2 |= SLOWED;
    }
    return;
  case HASTE_MONSTER:
    if (monster->m_flags2 & SLOWED) {
      monster->m_flags2 &= (~SLOWED);
    } else {
      monster->m_flags2 |= HASTED;
    }
    return;
  case TELE_AWAY:
    tele_away(monster, sotu);
    return;
  case CONFUSE_MONSTER:
    monster->m_flags2 |= CONFUSED;
    monster->moves_confused += get_rand(12, 22);
    return;
  case INVISIBILITY:
    monster->m_flags1 |= INVISIBLE;
    return;
  case POLYMORPH:
    if (monster->m_flags1 & HOLDS) {
      sotu->being_held = false;
    }
    nm = monster->next_monster;
    tc = monster->trail_char;
    gr_monster(monster, get_rand(0, MONSTERS-1));
    monster->row = row;
    monster->col = col;
    monster->next_monster = nm;
    monster->trail_char = tc;
    if (!(monster->m_flags1 & IMITATES)) {
      wake_up(monster);
    }
    return;
  case PUT_TO_SLEEP:
    monster->m_flags2 |= (ASLEEP | NAPPING);
    monster->nap_length = get_rand(3, 6);
    return;
  case MAGIC_MISSILE:
    rogue_hit(monster, 1); // same effect as your weapon though.
    return;
  case CANCELLATION:
    if (monster->m_flags1 & HOLDS) {
      sotu->being_held = false;
    }
    if (monster->m_flags1 & STEALS_ITEM) {
      monster->drop_percent = 0;
    }
    // yes, these are all in the first flag
    monster->m_flags1 &= (~(FLIES | FLITS | SPECIAL_HIT | INVISIBLE |
			    FLAMES | IMITATES | CONFUSES | SEEKS_GOLD |
			    SHRIEKS | HOLDS));
    return;
  case DO_NOTHING:
    message("nothing happens", sotu);
    return;
  case LIGHTNING:
    message("ZOT.", sotu);
    rogue_hit(monster, 1); // same effect as your weapon though.
    return;
  case W_SCARE_MONSTER:
    monster->m_flags2 |= FLEEING;
    return;
  }
}

/* tele_away MOVED to LibRogue */


/* wizardize is ON HOLD.  I'm not requiring a password yet.
 (What the heck is it doing in "zap" anyway?) */

/*
void wizardize() {
  char buf[100];

  if (IS_WIZARD) {
    sotu->conduct &= ~CONDUCT_ISWIZ;
    message("not wizard anymore", 0);
  } else {
    if (get_input_line("wizard's password:", "", buf, "", 0, 0)) {
      (void) xxx(1);
      xxxx(buf, strlen(buf));
      if (!strncmp(buf, "\247\104\126\272\115\243\027", 7)) {
	sotu->conduct |= CONDUCT_ISWIZ | CONDUCT_WASWIZ;
	message("Welcome, mighty wizard!", 0);
      } else {
	message("sorry", 0);
      }
    }
  }
}
*/



/***************************************************************
                   GET_THROWN_AT_MONSTER
 IN:
 obj = object being thrown
 dir = direction of the long-distance attack
 row, col = location where the attack was initiated
 sotu = various globals (dungeon, level_monsters)
 OUT:
 0 if no monster affected (row,col = the wall or whatever)
 else, the monster affected (row,col = the location of the monster)
 PURPOSE:
 The character has performed a directional action (see get_zapped_monster)
 that is effective until it hits a wall or monster.  Follow the
 line of fire and find the monster, if any, that will be hit.
 (Note: a monster that's imitating something won't be hit...)
****************************************************************/
object * get_thrown_at_monster(object *obj, Short dir, Short *row, Short *col,
			       struct state_of_the_union *sotu)
{
  Short orow, ocol;
  Short i, ch;
  UShort ** dungeon = sotu->dungeon;

  orow = *row; ocol = *col;

  ch = get_mask_char(obj->what_is);

  /* Unlike a zap, a thrown object eventually falls to the ground. */
  for (i = 0; i < 24; i++) {
    /* step one square in the indicated direction */
    get_dir_rc(dir, row, col, false);
    /* if you've hit a wall, you're done */
    if ((dungeon[*row][*col] == NOTHING) ||
	((dungeon[*row][*col] & (HORWALL | VERTWALL | HIDDEN)) &&
	 (!(dungeon[*row][*col] & TRAP)))) {
      *row = orow;
      *col = ocol;
      return 0;
    }
    /* Gee, it's animating the arrow.  Maybe I should slow this down. */
    if ((i != 0) && rogue_can_see(orow, ocol, sotu)) {
      mvaddch(orow, ocol, get_dungeon_char(orow, ocol, sotu));
    }
    if (rogue_can_see(*row, *col, sotu)) {
      if (!(dungeon[*row][*col] & MONSTER)) {
	mvaddch(*row, *col, ch);
      }
      SysTaskDelay(SysTicksPerSecond()/10); // XXX animation
      refresh();
    }
    orow = *row; ocol = *col;
    /* if you've hit a monster, you're done */
    if (dungeon[*row][*col] & MONSTER) {
      if (!imitating(*row, *col, sotu)) {
	return(object_at(sotu->level_monsters, *row, *col));
      }
    }
    /* haven't hit anything yet... */
    /* arrows travel 1/3 as far in tunnels as in rooms? */
    if (dungeon[*row][*col] & TUNNEL) {
      i += 2;
    }
  }
  return 0;
}

/***************************************************************************
 *     Moved all of lib_zap.c into here:
 ***************************************************************************/


/***************************************************************
                   GET_ZAPPED_MONSTER
 IN:
 dir = direction of the long-distance attack
 row, col = location where the attack was initiated
 sotu = various globals (dungeon, level_monsters)
 OUT:
 0 if no monster affected (row,col = the wall or whatever)
 else, the monster affected (row,col = the location of the monster)
 PURPOSE:
 The character has performed a directional action (see get_thrown_at_monster)
 that is effective until it hits a wall or monster.  Follow the
 line of fire and find the monster, if any, that will be hit.
 (Note: a monster that's imitating something won't be hit...)
****************************************************************/
object * get_zapped_monster(Short dir, Short *row, Short *col,
			    Short ch, Boolean long_zap,
			    struct state_of_the_union *sotu) 
{
  Short orow, ocol, rrow, rcol;
  Short i;
  object *mon;

  rrow = *row; rcol = *col;
  for (i = 0 ; ; i++) {
    //    if ((ch && !long_zap) && (i > 0) && rogue_can_see(orow, ocol, sotu)) {
    //      mvaddch(orow, ocol, get_dungeon_char(orow, ocol, sotu));
    //      //SysTaskDelay(SysTicksPerSecond()/15); // XXX animation
    //      //refresh();
    //    }
    orow = *row; ocol = *col;
    /* step one square in the indicated direction */
    get_dir_rc(dir, row, col, false);
    /* if you've hit a wall or a monster, you're done */
    if (((*row == orow) && (*col == ocol)) ||
	(sotu->dungeon[*row][*col] & (HORWALL | VERTWALL)) ||
	(sotu->dungeon[*row][*col] == NOTHING)) {
      mon = 0;
      break;
    }
    if (ch && rogue_can_see(*row, *col, sotu)) {
      /* Draw the zap */
      if (!(sotu->dungeon[*row][*col] & MONSTER))
	mvaddch(*row, *col, ch);
      refresh(); // BEFORE the wait
      SysTaskDelay(SysTicksPerSecond()/15); // XXX animation
      /* Undraw the zap (if it's a short zap) */
      if (!long_zap && !(sotu->dungeon[*row][*col] & MONSTER))
	mvaddch(*row, *col, get_dungeon_char(*row, *col, sotu));
    }
    if (sotu->dungeon[*row][*col] & MONSTER) {
      if (!imitating(*row, *col, sotu)) {
	mon = object_at(sotu->level_monsters, *row, *col);
	break;
      }
    }
    /* haven't hit anything yet... */
  }
  /* "Zap"s can travel "forever", unlike thrown objects. */
  /* Now undraw the zap (if we drew it) */
  if (ch && long_zap) {
    for ( ; i > 0 ; i--) {
      get_dir_rc(dir, &rrow, &rcol, false);
      if (rogue_can_see(rrow, rcol, sotu)) {
	mvaddch(rrow, rcol, get_dungeon_char(rrow, rcol, sotu));
	refresh();
	SysTaskDelay(SysTicksPerSecond()/15); // XXX animation
      }
    }
  }
  return mon;
}

/***************************************************************
                   TELE_AWAY
 IN:
 monster = the affected monster
 sotu = various globals (dungeon, being_held, detect_monster)
 OUT:
 nothing
 PURPOSE:
 This will teleport the affected monster to a random location.
****************************************************************/
void tele_away(object *monster, struct state_of_the_union *sotu) {
  Short row, col;

  if (monster->m_flags1 & HOLDS) {
    sotu->being_held = false;
  }
  gr_row_col(&row, &col, (FLOOR | TUNNEL | STAIRS | OBJECT), sotu);
  mvaddch(monster->row, monster->col, monster->trail_char);
  sotu->dungeon[monster->row][monster->col] &= ~MONSTER;
  monster->row = row; 
  monster->col = col;
  sotu->dungeon[row][col] |= MONSTER;
  monster->trail_char = mvinch(row, col);
  if (sotu->detect_monster || rogue_can_see(row, col, sotu)) {
    mvaddch(row, col, gmc(monster, sotu));
  }
}
