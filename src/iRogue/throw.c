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
 * throw.c
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

//extern Char *curse_message;
extern Char *hit_message;

static Boolean throw_at_monster(object *monster, object *weapon,
				struct state_of_the_union *sotu) SEC_2;

/* XXX BUG */
/**********************************************************************
                       THROW_AT_MONSTER
 IN:
 monster = the one being attacked
 weapon = the thing being thrown
 sotu = various globals
 OUT:
 Whether the monster was hit
 PURPOSE:
 Calculate chance to hit monster, and damage the monster if successful.
 **********************************************************************/
static Boolean throw_at_monster(object *monster, object *weapon,
				struct state_of_the_union *sotu) {
  Short damage, hit_chance;
  Short t;

  hit_chance = get_hit_chance(weapon, true, sotu);
  damage = get_weapon_damage(weapon, true, sotu);

  /* Create an appropriate message. */
  /* THE BUG IS BELOW HERE */
  t = weapon->quantity;
  weapon->quantity = 1;
  /*   StrPrintF(hit_message, "the %s", name_of(weapon)); */
  StrCopy(hit_message, "the ");
  copy_name_of(weapon, hit_message+4, sotu); /* bad sprite, !biscuit */
  /* THE BUG IS ABOVE HERE */
  weapon->quantity = t;
  if (!rand_percent(hit_chance)) {
    StrCat(hit_message, "misses  ");
    return false;
  }
  StrCat(hit_message, "hit  ");

  /* Damage the monster. */
  if ((weapon->what_is == WAND) && rand_percent(75)) {
    /* It seems that thrown wands will usually "go off". */
    zap_monster(monster, weapon->which_kind);
  } else {
    mon_damage(monster, damage);
  }
  return true;
}

/**********************************************************************
                       THROW
 IN:
 dir = direction (1-8) item is thrown
 index = of the item in the rogue's pack (like a list)
 sotu = various globals
 OUT:
 Whether the action was legal and requires a clock tick by caller.
 PURPOSE:
 Throw something, possibly hitting and damaging a monster.
 **********************************************************************/
Boolean throw(Short dir, Short index, struct state_of_the_union * sotu) {
  fighter *rogue = sotu->roguep;
  object *weapon;
  Short row, col;
  object *monster;

  if (dir == NO_DIRECTION) {
    return false;
  }
  if (!(weapon = get_nth_object(index, &rogue->pack))) {
    message("no such item.", sotu);
    return false;
  }
  if ((weapon->in_use_flags & BEING_USED) && weapon->is_cursed) {
    message("you can't, it appears to be cursed", sotu);
    return false;
  }
  row = rogue->row;
  col = rogue->col;

  /* If the item is "in use", un-use it. */
  if ((weapon->in_use_flags & BEING_WIELDED) && (weapon->quantity <= 1)) {
    unwield(rogue->weapon, rogue);
  } else if (weapon->in_use_flags & BEING_WORN) {
    mv_aquatars(); /* interesting... */
    unwear(rogue->armor, rogue);
    print_stats(STAT_ARMOR, sotu);
  } else if (weapon->in_use_flags & ON_EITHER_HAND) {
    un_put_on(weapon, sotu);
  }

  /* Figure out if there is a monster there to throw the item at.
     Get the row,col the item will end up at. */
  monster = get_thrown_at_monster(weapon, dir, &row, &col, sotu);
  /* g-t-a-m redraws some stuff - make sure rogue is still visible */
  mvaddch(rogue->row, rogue->col, rogue->fchar);    /* ? */
  refresh();

  if (rogue_can_see(row, col, sotu) && 
          ((row != rogue->row) || (col != rogue->col))) {
    mvaddch(row, col, get_dungeon_char(row, col, sotu));    /* ? */
  }
  if (monster) {
    wake_up(monster);
    check_gold_seeker(monster);
    check_shrieker(monster);
    if (!throw_at_monster(monster, weapon, sotu)) {
      /* it missed... */
      flop_weapon(weapon, row, col, sotu);
    }
  } else {
    flop_weapon(weapon, row, col, sotu);
  }
  /* remove item from pack, and caller must call reg_move. */
  vanish(weapon, &rogue->pack, sotu);
  return true; /* caller must call reg_move */
}


/************************************************
  get_thrown_at_monster MOVED to LibRogue/lib_object.c
  flop_weapon MOVED to LibRogue/lib_object.c
************************************************/


/****************************************
  rand_around has MOVED to lib_random.c
****************************************/
