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
 * use.c
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
//extern Char *you_can_move_again;

/************************************************
  quaff MOVED to LibRogue
  also Char* strange_feeling.
************************************************/

/**********************************************************************
                       READ_SCROLL
 IN:
 index = of object in rogue's pack (considered as a list)
 sotu = various globals
 identfy = another return value: is it an "identify" scroll.
 OUT:
 Was the action legal (should a clock tick be performed)
 (Also, was the scroll an "identify" scroll.)
 PURPOSE:
 Various interesting effects from reading a scroll.
 **********************************************************************/
Boolean read_scroll(Short index, struct state_of_the_union *sotu,
		    Boolean * identfy) 
{
  object *obj;
  Char msg[DCOLS];
  fighter *rogue = sotu->roguep;
  Char buf[20];

  *identfy = false;
  if (sotu->blind) {
    message("you can't see to read the scroll", sotu);
    return false;
  }
  if (!(obj = get_nth_object(index, &rogue->pack))) {
    message("no such item.", sotu);
    return false;
  }
  if (obj->what_is != SCROLL) {
    /* I could prevent this by pruning the presented inventory */
    message("you can't read that", sotu);
    return false;
  }
  switch(obj->which_kind) {
  case SCARE_MONSTER:
    /* purpose of this scroll is, monsters can't move onto it. */
    message("you hear a maniacal laughter in the distance", sotu);
    break;
  case HOLD_MONSTER:
    hold_monster(sotu);
    break;
  case ENCH_WEAPON:
    if (rogue->weapon) {
      if (rogue->weapon->what_is == WEAPON) {
	copy_name_of(rogue->weapon, buf, sotu);
	StrPrintF(msg, "your %sglow%s %sfor a moment",
		  buf,
		  ((rogue->weapon->quantity <= 1) ? "s" : ""),
		  get_ench_color(sotu));
	message(msg, sotu);
	if (coin_toss()) {
	  rogue->weapon->hit_enchant++;
	} else {
	  rogue->weapon->d_enchant++;
	}
      }
      rogue->weapon->is_cursed = 0;
    } else {
      message("your hands tingle", sotu);
    }
    break;
  case ENCH_ARMOR:
    if (rogue->armor) {
      StrPrintF(msg, "your armor glows %sfor a moment",
		get_ench_color(sotu));
      message(msg, sotu);
      rogue->armor->d_enchant++;
      rogue->armor->is_cursed = 0;
      print_stats(STAT_ARMOR, sotu);
    } else {
      message("your skin crawls", sotu);
    }
    break;
  case IDENTIFY:
    message("this is a scroll of identify", sotu);
    obj->identified = 1;
    sotu->id_scrolls[obj->which_kind].id_status = IDENTIFIED;
    *identfy = true; /* through a major hack, we revisit inventory-select */
    break;
  case TELEPORT:
    tele(sotu);
    break;
  case SLEEP:
    if (!(sotu->ring_flags & RING_ALERTNESS)) {
      message("you fall asleep", sotu);
      take_a_nap();
    }
    break;
  case PROTECT_ARMOR:
    if (rogue->armor) {
      message( "your armor is covered by a shimmering gold shield", sotu);
      rogue->armor->is_protected = 1;
      rogue->armor->is_cursed = 0;
    } else {
      message("your acne seems to have disappeared", sotu);
    }
    break;
  case REMOVE_CURSE:
    message((!sotu->halluc) ?
	    "you feel as though someone is watching over you" :
	    "you feel in touch with the universal oneness", sotu);
    uncurse_all(sotu);
    break;
  case CREATE_MONSTER:
    create_monster(0);
    break;
  case AGGRAVATE_MONSTER:
    message("you hear a high pitched humming noise", sotu);
    aggravate(sotu);
    break;
  case MAGIC_MAPPING:
    message("this scroll seems to have a map on it", sotu);
    draw_magic_map(sotu); /* XXX maybe make this pop up the overview map */
    break;
  case CREATE_OBJ:
    message("you have the power to create:", sotu);
    FrmPopupForm(WizForm); // TESTING
  }
  if (sotu->id_scrolls[obj->which_kind].id_status != CALLED) {
    sotu->id_scrolls[obj->which_kind].id_status = IDENTIFIED;
  }
  vanish(obj, &rogue->pack, sotu);
  /* This determines whether the caller should call reg_move...
     basically, the caller should except when it was a SLEEP scroll */
  return (obj->which_kind != SLEEP);
}

/*************************************************
  vanish MOVED to LibRogue 
  potion_heal MOVED to LibRogue
************************************************/

/**********************************************************************
                       IDNTFY
 IN:
 index = of object in pack (consider as list)
 sotu = various globals
 OUT:
 nothing
 PURPOSE:
 Identify an item...
 **********************************************************************/
void idntfy(Short index, struct state_of_the_union *sotu) {
  object *obj;
  struct id *id_table;
  Char desc[DCOLS];
  fighter *rogue = sotu->roguep;

  if (!(obj = get_nth_object(index, &rogue->pack))) {
    message("no such item.", sotu);
    return;
  }
  obj->identified = 1;
  if (obj->what_is & (SCROLL | POTION | WEAPON | ARMOR | WAND | RING)) {
    id_table = get_id_table(obj, sotu);
    id_table[obj->which_kind].id_status = IDENTIFIED;
  }
  get_desc(obj, desc, sotu);
  message(desc, sotu);
}


/************************************************
  eat MOVED to LibRogue
************************************************/


/************************************************* 
   MOVED to LibRogue:
   hold_monster()
   tele()
   hallucinate()
   unhallucinate()
   unblind()
   relight()
*************************************************/

/**********************************************************************
                       TAKE_A_NAP
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Move monsters (but, like sleeping beauty, DON'T tick the rogue's clock)
 until rogue wakes.  This may be called by monsters with paralyze
 or sleep gas attacks; I've fixed it so that they aren't cumulative,
 cause that would most likely be the Kiss of Death.
 **********************************************************************/
void take_a_nap() {
  static Boolean napping = false;
  Short i;
  Long delay = SysTicksPerSecond();

  if (napping) return; // (to prevent needless cruelty)
  napping = true;
  i = get_rand(2, 5);
  SysTaskDelay(delay); // there should be some "visible delay"

  while (i--) {
    mv_mons();
    if (i_am_dead)
      return; // if dead, return
  }
  SysTaskDelay(delay); // there should be some "visible delay", again
  message("you can move again", sotu);
  napping = false;
}

/************************************************* 
   MOVED to LibRogue:
   go_blind() 
   get_ench_color() 
   confuse()
   unconfuse()
   uncurse_all() 
*************************************************/

/***************************************************************************
 *     Moved all of lib_use.c into here:
 ***************************************************************************/


Char *strange_feeling = "you have a strange feeling for a moment, then it passes";

extern Char fruit_names[21][14]; // in lib_object.c
extern Char carcase_names[4][14];

/***************************************************************
                   HOLD_MONSTER
 IN:
 sotu = various globals (rogue, dungeon, level_monsters)
 OUT:
 nothing
 PURPOSE:
 For each monster within two squares of the rogue,
 the monster freezes (becomes asleep and won't wake on its own).
****************************************************************/
void hold_monster(struct state_of_the_union *sotu) {
  Short i, j;
  Short mcount = 0;
  object *monster;
  Short row, col;

  for (i = -2; i <= 2; i++) {
    for (j = -2; j <= 2; j++) {
      row = sotu->roguep->row + i;
      col = sotu->roguep->col + j;
      if ((row < MIN_ROW) || (row > (DROWS-2)) || (col < 0) ||
	  (col > (DCOLS-1))) {
	continue;
      }
      if (sotu->dungeon[row][col] & MONSTER) {
	monster = object_at(sotu->level_monsters, row, col);
	monster->m_flags2 |= ASLEEP;
	monster->m_flags2 &= (~WAKENS);
	mcount++;
      }
    }
  }
  if (mcount == 0) {
    message("you feel a strange sense of loss", sotu);
  } else if (mcount == 1) {
    message("the monster freezes", sotu);
  } else {
    message("the monsters around you freeze", sotu);
  }
}

/***************************************************************
                   TELE
 IN:
 sotu = various globals (rogue, cur_room, rooms, being_held, bear_trap)
 OUT:
 nothing
 PURPOSE:
 This will teleport the rogue to a random location.
****************************************************************/
void tele(struct state_of_the_union *sotu) {
  fighter *rogue = sotu->roguep;

  mvaddch(rogue->row, rogue->col, 
	  get_dungeon_char(rogue->row, rogue->col, sotu));
  if (sotu->cur_room >= 0) {
    darken_room(sotu->cur_room, sotu);
  }
  put_player(get_room_number(rogue->row, rogue->col, sotu->rooms), sotu);
  sotu->being_held = false;
  sotu->bear_trap = false;
}

/***************************************************************
                   HALLUCINATE
 IN:
 sotu = various globals (rogue, level_objects, level_monsters, blind)
 OUT:
 nothing
 PURPOSE:
 This is called on each clock tick when the rogue is hallucinating.
 It will update the display, randomly changing the appearance of
 each object and monster.
****************************************************************/
void hallucinate(struct state_of_the_union *sotu) {
  object *obj, *monster;
  Short ch;
  fighter *rogue = sotu->roguep;

  if (sotu->blind)
    return;

  obj = sotu->level_objects->next_object;

  while (obj) {
    ch = mvinch(obj->row, obj->col);
    if ( (Not_Alpha(ch)) && ((obj->row != rogue->row)
			     || (obj->col != rogue->col)) ) {
      if ((ch != ' ') && (ch != '.') && (ch != '#') && (ch != '+')) {
	mvaddch(obj->row, obj->col, gr_obj_char()); // was addch(gr_obj_char())
      }
    }
    obj = obj->next_object;
  }
  monster = sotu->level_monsters->next_monster;

  while (monster) {
    ch = mvinch(monster->row, monster->col);
    if ( Is_Alpha(ch) ) {
      if (!(sotu->ring_flags & RING_ESP))
	ch = (coin_toss()) ? get_rand('A','Z') : get_rand('a','z'); // yo ho!
      mvaddch(monster->row, monster->col, ch);
    }
    monster = monster->next_monster;
  }
}

/***************************************************************
                   UNHALLUCINATE
 IN:
 sotu = various globals (halluc)
 OUT:
 nothing
 PURPOSE:
 This is called on the turn that the rogue is done hallucinating.
 Redraw the room/passage (is this necessary?)
****************************************************************/
void unhallucinate(struct state_of_the_union *sotu) {
  sotu->halluc = 0;
  relight(sotu);
  message("everything looks SO boring now", sotu);
}

/***************************************************************
                   UNBLIND
 IN:
 sotu = various globals (blind, halluc, detect_monster)
 OUT:
 nothing
 PURPOSE:
 This is called on the turn that the rogue recovers from
 temporary blindness.  Redraw the room/passage as visible.
****************************************************************/
void unblind(struct state_of_the_union *sotu) {
  sotu->blind = 0;
  message("the veil of darkness lifts", sotu);
  relight(sotu);
  if (sotu->halluc || (sotu->ring_flags & RING_DELUSION)) {
    hallucinate(sotu);
  }
  if (sotu->detect_monster) {
    show_monsters(sotu);
  }
}

/***************************************************************
                   RELIGHT
 IN:
 sotu = various globals (rogue, cur_room)
 OUT:
 nothing
 PURPOSE:
 Redraw the room or passage that the rogue is currently in.
****************************************************************/
void relight(struct state_of_the_union *sotu) {
  fighter *rogue = sotu->roguep;

  if (sotu->cur_room == PASSAGE) {
    light_passage(rogue->row, rogue->col, sotu);
  } else {
    light_up_room(sotu->cur_room, sotu);
  }
  mvaddch(rogue->row, rogue->col, rogue->fchar);
}

/***************************************************************
                   GO_BLIND
 IN:
 sotu = various globals (cur_room, rooms, rogue, blind,
                         detect_monster, level_monsters)
 OUT:
 nothing
 PURPOSE:
 Make the rogue blind for a randomly determined duration.
 If the rogue is in a room, darken its interior (except '@').
****************************************************************/
void go_blind(struct state_of_the_union *sotu) {
  Short i, j;
  room * rooms = sotu->rooms;
  fighter *rogue = sotu->roguep;

  if (!sotu->blind) {
    message("a cloak of darkness falls around you", sotu);
  }
  //  sotu->blind += get_rand(500, 800);
  sotu->blind += get_rand(250, 400); // above was, like, WAY too long.

  if (sotu->detect_monster) {
    object *monster;

    monster = sotu->level_monsters->next_monster;

    while (monster) {
      mvaddch(monster->row, monster->col, monster->trail_char);
      monster = monster->next_monster;
    }
  }
  if (sotu->cur_room >= 0) {
    for (i = rooms[sotu->cur_room].top_row + 1;
	 i < rooms[sotu->cur_room].bottom_row; i++) {
      for (j = rooms[sotu->cur_room].left_col + 1;
	   j < rooms[sotu->cur_room].right_col; j++) {
	mvaddch(i, j, ' ');
      }
    }
  }
  mvaddch(rogue->row, rogue->col, rogue->fchar);
}

/***************************************************************
                   CONFUSE
 IN:
 sotu = various globals (confused)
 OUT:
 nothing
 PURPOSE:
 Make the rogue confused for a randomly determined duration.
 ('confused' will be checked when the rogue tries to move.)
****************************************************************/
void confuse(struct state_of_the_union *sotu) {
  sotu->confused += get_rand(12, 22);
}

/***************************************************************
                   UNCONFUSE
 IN:
 sotu = various globals (confused, halluc)
 OUT:
 nothing
 PURPOSE:
 Called when the rogue is no longer confused (because it wore
 off or some potion dispelled it.)
****************************************************************/
void unconfuse(struct state_of_the_union *sotu) {
  Char msg[80];

  sotu->confused = 0;
  StrPrintF(msg, "you feel less %s now", (sotu->halluc ? "trippy" : "confused"));
  message(msg, sotu);
}

/***************************************************************
                   UNCURSE_ALL
 IN:
 sotu = various globals (rogue)
 OUT:
 nothing
 PURPOSE:
 For each object that the rogue is carrying, make it not cursed.
****************************************************************/
void uncurse_all(struct state_of_the_union *sotu) {
  object *obj;

  obj = sotu->roguep->pack.next_object;

  while (obj) {
    obj->is_cursed = 0;
    obj = obj->next_object;
  }
}

/***************************************************************
                   GET_ENCH_COLOR
 IN:
 sotu = various globals (halluc, id_potions)
 OUT:
 string representing a color
 PURPOSE:
 This returns a color that an item being enchanted will (briefly)
 glow - just so that, if the rogue is hallucinating, the color
 will be random instead of blue :-)
****************************************************************/
Char * get_ench_color(struct state_of_the_union *sotu) {
  if (sotu->halluc || (sotu->ring_flags & RING_DELUSION)) {
    return(sotu->id_potions[get_rand(0, POTIONS-1)].title);
  }
  return("blue ");
}

/***************************************************************
                   POTION_HEAL
 IN:
 extra = true if the healing potion was "extra-strength"
 sotu = various globals ()
 OUT:
 nothing
 PURPOSE:
 Heal the rogue of some damage, and maybe cure other problems.
****************************************************************/
static void potion_heal(Boolean extra, struct state_of_the_union *sotu) SEC_L;
static void potion_heal(Boolean extra, struct state_of_the_union *sotu) {
  Int ratio; // * 16 or so, to avoid working with actual Floats
  Short add; // (max hp is 800, this should not overflow an int or short)
  fighter *rogue = sotu->roguep;

  /* First, add a number of hit points equal to the rogue's level. */
  rogue->hp_current += rogue->exp;

  /* Figure out how badly hurt the rogue is now. */
  ratio = (16 * rogue->hp_current + 8) / rogue->hp_max; // add 8 to round up

  if (ratio >= 16) { // if ratio >= 1.0 * 16
    // no damage
    /* Rogue is healed "better than new" */
    rogue->hp_max += (extra ? 2 : 1);
    sotu->extra_hp += (extra ? 2 : 1);
    rogue->hp_current = rogue->hp_max;
  } else if (ratio >= 14) { // if ratio >= 0.9 * 16 .. really 14.4
    // 10% or less damage
    /* Rogue is healed "as good as new" (better if 'extra') */
    rogue->hp_max += (extra ? 1 : 0);
    sotu->extra_hp += (extra ? 1 : 0);
    rogue->hp_current = rogue->hp_max;
  } else {
    /* Rogue will be healed, at minimum:
       normal => add 1/3 of the remaining hit-point deficit
       extra  => add 2/3 of the remaining hit-point deficit */
    if (ratio < 5) ratio = 5; // ratio = max(ratio,0.33); .. really 5.28
    if (extra) ratio += ratio;
    add = (Short) ((ratio * (rogue->hp_max - rogue->hp_current) + 8) / 16);
    rogue->hp_current += add;
    if (rogue->hp_current > rogue->hp_max) {
      /* At maximum, rogue will be healed "as good as new" */
      rogue->hp_current = rogue->hp_max;
    }
  }
  /* Normal and extra-strength cure blindness. */
  if (sotu->blind) {
    unblind(sotu);
  }
  /* Normal half-cures, and extra-strength cures, confusion. */
  if (sotu->confused && extra) {
    unconfuse(sotu);
  } else if (sotu->confused) {
    sotu->confused = (sotu->confused / 2) + 1;
  }
  /* Normal half-cures, and extra-strength cures, hallucination. */
  if (sotu->halluc && extra) {
    unhallucinate(sotu);
  } else if (sotu->halluc) {
    sotu->halluc = (sotu->halluc / 2) + 1;
  }
}


/***************************************************************
                   VANISH
 IN:
 obj = the item that needs to "vanish"
 pack = the object-list that the vanishing item is held in.
 sotu = various globals (rogue)
 OUT:
 nothing
 PURPOSE:
 Destroys the indicated item (could be an object in the rogue's
 inventory; could be an object or monster in the dungeon level.)
 If the rogue is currently "using" the item, it will be un-used.
 NOTE:
 vanish() does NOT destroy a quiver of weapons with more than one
 arrow (or whatever) in the quiver.  It only decrements the count.
****************************************************************/
void vanish(object *obj, object *pack, struct state_of_the_union *sotu) 
{
  fighter * rogue = sotu->roguep;
  if (!obj)
    return;

  if (obj->quantity > 1) {
    obj->quantity--;
  } else {
    if (obj->in_use_flags & BEING_WIELDED) {
      unwield(obj, rogue);
    } else if (obj->in_use_flags & BEING_WORN) {
      unwear(obj, rogue);
    } else if (obj->in_use_flags & ON_EITHER_HAND) {
      un_put_on(obj, sotu);
    }
    take_from_pack(obj, pack);
    free_object(obj, sotu);
  }
}

/***************************************************************
                   SHOW_OBJECTS
 IN:
 sotu = various globals (rogue, level_objects, level_monsters, dungeon)
 OUT:
 nothing
 PURPOSE:
 This will make all objects (and imitating monsters) visible,
 unless the rogue or a visible monster is standing on them..
****************************************************************/
/* this WAS in object.c */
static void show_objects(struct state_of_the_union *sotu) SEC_L;
static void show_objects(struct state_of_the_union *sotu) {
  object *obj;
  Short mc, rc, row, col;
  object *monster;
  fighter *rogue = sotu->roguep;

  obj = sotu->level_objects->next_object;

  while (obj) {
    row = obj->row;
    col = obj->col;

    rc = get_mask_char(obj->what_is);

    if (sotu->dungeon[row][col] & MONSTER) {
      if ((monster = object_at(sotu->level_monsters, row, col))) {
	monster->trail_char = rc;
      }
    }
    mc = mvinch(row, col);
    if ( (Not_Alpha(mc))
	 && ((row != rogue->row) || (col != rogue->col))) {
      mvaddch(row, col, rc);
    }
    obj = obj->next_object;
  }

  monster = sotu->level_monsters->next_object;

  while (monster) {
    if (monster->m_flags1 & IMITATES) {
      mvaddch(monster->row, monster->col, (Short) monster->disguise);
    }
    monster = monster->next_monster;
  }
}


/***************************************************************
                   QUAFF
 IN:
 index = the index of the item in the rogue's inventory-list
 sotu = various globals ()
 OUT:
 true if the caller should call reg_move().
 (quaff can't call reg_move itself; reg_move isn't in this library.)
 PURPOSE:
 The rogue drank the indexed potion: determine the potion's effect
 and causes it to be performed.
****************************************************************/
Boolean quaff(Short index, struct state_of_the_union *sotu) {
  //  Char buf[80]; /* buf: 80 and not DCOLS ? */
  object *obj;
  fighter *rogue = sotu->roguep;

  if (!(obj = get_nth_object(index, &rogue->pack))) {
    message("no such item.", sotu);
    return false;
  }
  /* //the following is guaranteed by the caller to NEVER HAPPEN:
  if (obj->what_is != POTION) {
    message("you can't drink that", sotu);
    return false;
  }
  */
  /* Figure out what the potion does, and do it... */
  switch(obj->which_kind) {
  case INCREASE_STRENGTH:
    /* Increment the rogue's current (and perhaps maximum) strength */
    message("you feel stronger now, what bulging muscles!", sotu);
    rogue->str_current++;
    if (rogue->str_current > rogue->str_max) {
      rogue->str_max = rogue->str_current;
    }
    break;
  case RESTORE_STRENGTH:
    /* Restore the rogue's strength to its maximum */
    rogue->str_current = rogue->str_max;
    message("this tastes great, you feel warm all over", sotu);
    break;
  case HEALING:
    /* Heal the rogue (q.v.) */
    message("you begin to feel better", sotu);
    potion_heal(false, sotu);
    break;
  case EXTRA_HEALING:
    /* "Extra-strength"-heal the rogue */
    message("you begin to feel much better", sotu);
    potion_heal(true, sotu);
    break;
  case POISON:
    /* Decrease strength (unless "sustained").  Also cures hallucination! */
    if (!(sotu->ring_flags & RING_SUSTAIN_STR)) {
      rogue->str_current -= get_rand(1, 3);
      if (rogue->str_current < 1) {
	rogue->str_current = 1;
      }
    }
    message("you feel very sick now", sotu);
    if (sotu->halluc) {
      unhallucinate(sotu);
    }
    break;
  case RAISE_LEVEL:
    /* Quantum jump to (just barely) the next experience level */
    rogue->exp_points = sotu->level_points[rogue->exp - 1];
    add_exp(1, 1, sotu);
    break;
  case BLINDNESS:
    /* Rogue is temporarily blinded */
    go_blind(sotu);
    break;
  case HALLUCINATION:
    /* Objects and monsters don't appear as their true selves */
    message("oh wow, everything seems so cosmic", sotu);
    sotu->halluc += get_rand(500, 800);
    break;
  case DETECT_MONSTER:
    /* Monsters throughout the level are made visible (unless blind) */
    show_monsters(sotu);
    if (!(sotu->level_monsters->next_monster)) {
      message(strange_feeling, sotu);
    }
    break;
  case DETECT_OBJECTS:
    /* Objects throughout the level are made visible (unless blind) */
    if (sotu->level_objects->next_object) {
      if (!sotu->blind) {
 	show_objects(sotu);
      }
    } else {
      message(strange_feeling, sotu);
    }
    break;
  case CONFUSION:
    /* Rogue's moves will be in random directions for a while */
    message((sotu->halluc ? "what a trippy feeling" :
	     "you feel confused"), sotu);
    confuse(sotu);
    break;
  case LEVITATION:
    /* Rogue can't reach traps, stairs, objects for a while */
    message("you start to float in the air", sotu);
    sotu->levitate += get_rand(15, 30);
    sotu->being_held = sotu->bear_trap = false;
    break;
  case HASTE_SELF:
    /* Rogue moves twice per turn for a while */
    message("you feel yourself moving much faster", sotu);
    sotu->haste_self += get_rand(11, 21);
    if (!(sotu->haste_self % 2)) {
      sotu->haste_self++;
    }
    break;
  case SEE_INVISIBLE:
    /* Rogue can see invisible monsters for a while.  Cures blindness. */
    //    StrPrintF(buf, "hmm, this potion tastes like %sjuice",
    //	      fruit_names[get_rand(0,20)]);
    message("hmm, this potion tastes like slime-mold juice", sotu);
    if (sotu->blind) {
      unblind(sotu);
    }
    sotu->see_invisible = true;
    relight(sotu);
    break;
  }
  /* Update stats display, "identify" the potion, and destroy it */
  print_stats((STAT_STRENGTH | STAT_HP), sotu);
  if (sotu->id_potions[obj->which_kind].id_status != CALLED) {
    sotu->id_potions[obj->which_kind].id_status = IDENTIFIED;
  }
  vanish(obj, &rogue->pack, sotu);
  return true;
}

/*********************************************
  read_scroll moved BACK into ../use.c
  because moving/creating monsters is a pain
**********************************************/

// I have MOVED "eat" into lib_object.c because it is Happier There.
