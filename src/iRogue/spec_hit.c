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
 * special_hit.c
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

static void freeze(object *monster) SEC_1;
static void steal_gold(object *monster) SEC_1;
static void steal_item(object *monster) SEC_1;
static Boolean gold_at(Short row, Short col) SEC_1;
static void sting(object *monster) SEC_1;
static void drop_level(object *monster) SEC_1;
static void drain_life() SEC_1;
static void hug(object *monster) SEC_1;
static void paralyze(object *monster) SEC_1;
static void drink_blood(object *monster) SEC_1;
static void slow_rogue() SEC_1;

//Char *breath_name = "flame";
Char breath_name[BREATH_ATTACKS][15] = {
  "flame",
  "acid",
  "lightning bolt",  // hm, same length as max. of current monster names
  "chlorine gas",
  "ice",
  "nerve gas",
  "sleeping gas",
  "slow gas"
  //  "fear gas"
};

//extern Char *you_can_move_again;

/**********************************************************************
                       SPECIAL_HIT
 IN:
 monster = an attacking monster which has a special ability
 various globals
 OUT:
 nothing
 PURPOSE:
 Perform interesting attacks on the hapless rogue
 **********************************************************************/
void special_hit(object * monster) {
  if (i_am_dead) return;
  if ((monster->m_flags2 & CONFUSED) && rand_percent(66)) {
    return;
  }
  if (monster->m_flags1 & RUSTS) {
    rust(monster);
  }
  if ((monster->m_flags1 & HOLDS)
      && !(sotu->levitate
	   || (sotu->ring_flags & (RING_LEVITATE | RING_FREE_ACTION)))) {
    sotu->being_held = true;
  }
  if (monster->m_flags1 & FREEZES) {
    freeze(monster);
  }
  if (monster->m_flags1 & STINGS) {
    sting(monster);
  }
  if (monster->m_flags1 & DRAINS_LIFE) {
    drain_life();
  }
  if (monster->m_flags1 & DROPS_LEVEL) {
    drop_level(monster);
  }
  if (monster->m_flags1 & STEALS_GOLD) {
    steal_gold(monster);
  } else if (monster->m_flags1 & STEALS_ITEM) {
    /* Not sure this is working sufficiently "randomly"! */
    steal_item(monster);
  }
  if (monster->m_flags1 & HUGS) {
    hug(monster);
  }
  if (monster->m_flags1 & PARALYZES) {
    paralyze(monster);
  }
  // if this is last, the vampire can have multiple actions
  if ((monster->m_flags1 & SUCKER) && rand_percent(15)) {
    drink_blood(monster);
  }
}

/**********************************************************************
                       RUST
 IN:
 monster = a monster that causes rust, or a trap if '0'.
 various globals
 OUT:
 nothing
 PURPOSE:
 Cause the rogue's armor to rust, if applicable, unless protected.
 **********************************************************************/
void rust(object *monster) {
  fighter *rogue = sotu->roguep;
  /* Don't rust leather, or non-useful/non-existent armor */
  if ((!rogue->armor) || (get_armor_class(sotu) <= 1) ||
      (!(rogue->armor->o_flags & O_METAL)) ) {
    return;
  }
  /* Some armor is protected against rust */
  if (rogue->armor->is_protected
      || (sotu->ring_flags & RING_MAINTAIN_ARMOR)) {
    if (monster && (!(monster->m_flags2 & RUST_VANISHED))) {
      message("the rust vanishes instantly", sotu);
      monster->m_flags2 |= RUST_VANISHED;
    }
  } else {
    rogue->armor->d_enchant--;
    message("your armor weakens", sotu);
    print_stats(STAT_ARMOR, sotu);
  }
}


/**********************************************************************
                       FREEZE
 IN:
 monster = a monster with a cold attack
 various globals
 OUT:
 nothing
 PURPOSE:
 Make the rogue freeze, maybe.  (I.e. the rogue can't do anything
 for a bit, while any neighboring wakeful monsters whale on it.)
 Also, you can die of hypothermia.
 BUG: If you are frozen in place and too many monsters attack you,
 the event queue can overflow.  Drat.  Find out where the problem is.
 **********************************************************************/
static void freeze(object *monster) {
  fighter *rogue = sotu->roguep;
  Short freeze_percent = 99;
  Short i, n;

  if (rand_percent(12)
      || (rogue->armor && rogue->armor->which_kind == CRYSTAL_ARMOR)
      || (sotu->ring_flags & RING_PROTECT_COLD))
    return;
  
  /* It helps to have a high strength, level, armor class, or hp max. */
  freeze_percent -= (rogue->str_current+(rogue->str_current / 2));
  freeze_percent -= ((rogue->exp + sotu->ring_dex) * 4);
  freeze_percent -= (get_armor_class(sotu) * 5);
  freeze_percent -= (rogue->hp_max / 3);

  if (freeze_percent > 10) {
    monster->m_flags2 |= FREEZING_ROGUE;
    message("you are frozen", sotu);

    n = get_rand(4, 8);
    for (i = 0; i < n; i++) {
      mv_mons(); // if dead, return
      if (i_am_dead) return;
    }
    if (rand_percent(freeze_percent)) {
      for (i = 0; i < 50; i++) {
	mv_mons(); // if dead, return
	if (i_am_dead) return;
      }
      killed_by((object *)0, HYPOTHERMIA, sotu);
      FrmPopupForm(TopTenForm);
      return;
    }
    message("you can move again", sotu);
    monster->m_flags2 &= (~FREEZING_ROGUE);
  }
}

/**********************************************************************
                       STEAL_GOLD
 IN:
 monster = a light-fingered monster (leprechaun)
 various globals
 OUT:
 nothing
 PURPOSE:
 Try to steal money from the rogue; if successful, the monster leaves
 the dungeon (disappear != teleport.)
 (why is this annoying?  "top ten" score is based on gold.)
 **********************************************************************/
static void steal_gold(object *monster) {
  fighter *rogue = sotu->roguep;
  Int amount;

  if ((rogue->gold <= 0) || rand_percent(10)) {
    return;
  }

  amount = get_rand((sotu->cur_level * 10), (sotu->cur_level * 30));

  if (amount > rogue->gold) {
    amount = rogue->gold;
  }
  rogue->gold -= amount;
  message("your purse feels lighter", sotu);
  print_stats(STAT_GOLD, sotu);
  disappear(monster, sotu);
}


/**********************************************************************
                       STEAL_ITEM
 IN:
 monster = a light-fingered monster (nymph)
 various globals
 OUT:
 nothing
 PURPOSE:
 Try to steal an item from the rogue; if successful, the monster leaves
 the dungeon (disappear != teleport.)
 BUG: The item selected does not seem to be very random!
 **********************************************************************/
static void steal_item(object *monster) {
  fighter *rogue = sotu->roguep;
  object *obj;
  //  Short i, n, qty = 0;
  Short i, n, qty = 1;
  Boolean has_stealable_item = 0;
  Char desc[80];

  if (rand_percent(15))
    return; /* won't always steal something */

  obj = rogue->pack.next_object;
  if (!obj) {
    /* rats, there is nothing left to steal */
    disappear(monster, sotu);
    return;
  }
  while (obj) {
    /* don't count objects that are "in use" */
    if (!(obj->in_use_flags & BEING_USED)) {
      has_stealable_item = 1;
      break;
    }
    obj = obj->next_object;
  }
  if (!has_stealable_item) {
    /* rats, rogue is holding everything */
    disappear(monster, sotu);
    return;
  }
  /* pick some random un-used object */
  /* this seems to pick "first item in pack" - WHY? */
  n = get_rand(0, MAX_PACK_COUNT);
  obj = rogue->pack.next_object;
  for (i = 0; i <= n; i++) {
    obj = obj->next_object;
    while ((!obj) || (obj->in_use_flags & BEING_USED)) {
      if (!obj) {
	obj = rogue->pack.next_object;
      } else {
	obj = obj->next_object;
      }
    }
  }
  StrCopy(desc, "she stole ");
  if (obj->what_is != WEAPON) {
    qty = obj->quantity;
    obj->quantity = 1;
  }
  get_desc(obj, desc+10, sotu);
  message(desc, sotu);

  // this "should" cause plural-weapons to all go at once.
  obj->quantity = ((obj->what_is != WEAPON) ? qty : 1);

  vanish(obj, &rogue->pack, sotu);
  //  reg_move(); // NO. BAD. WRONG.

  disappear(monster, sotu);
  
}


/************************************************
  disappear MOVED to LibRogue
  cough_up MOVED to LibRogue
  try_to_cough MOVED to LibRogue
************************************************/

/**********************************************************************
                       SEEK_GOLD
 IN:
 monster = a greedy monster
 various globals
 OUT:
 Returns true if the gold-seeking monster moved.
 PURPOSE:
 The monster "flits" in the direction of gold; when it finds some, it
 goes to sleep on it (no longer gold-seeking, and not self-wakening.)
 **********************************************************************/
Boolean seek_gold(object *monster) {
  Short i, j, rn, s;
  room * rooms = sotu->rooms;

  /* not a room ==> no gold here */
  if ((rn = get_room_number(monster->row, monster->col, rooms)) < 0) {
    return false;
  }
  for (i = rooms[rn].top_row+1; i < rooms[rn].bottom_row; i++) {
    for (j = rooms[rn].left_col+1; j < rooms[rn].right_col; j++) {
      if ((gold_at(i, j)) && !(dungeon[i][j] & MONSTER)) {
	/* there's gold somewhere!  see if you can "flit" to it. */
	monster->m_flags2 |= CAN_FLIT;
	s = mon_can_go(monster, i, j, sotu);
	monster->m_flags2 &= (~CAN_FLIT);
	if (s) {
	  /* the monster moves to the gold, sleeps there. */
	  move_mon_to(monster, i, j, sotu);
	  monster->m_flags2 |= ASLEEP;
	  monster->m_flags1 &= (~SEEKS_GOLD);
	  monster->m_flags2 &= (~WAKENS);
	  return true;
	}
	monster->m_flags1 &= (~SEEKS_GOLD);
	monster->m_flags2 |= CAN_FLIT;
	mv_monster(monster, i, j);
	monster->m_flags2 &= (~CAN_FLIT);
	monster->m_flags1 |= SEEKS_GOLD;
	return true;
      }
    }
  }
  return false;
}

/**********************************************************************
                       GOLD_AT
 IN:
 row,col = place to check for gold
 various globals
 OUT:
 Returns true if there's gold there.
 PURPOSE:
 Used by seek_gold.
 **********************************************************************/
static Boolean gold_at(Short row, Short col) {
  object *obj;
  if (dungeon[row][col] & OBJECT) {

    if ((obj = object_at(level_objects, row, col)) &&
	(obj->what_is == GOLD)) {
      return true;
    }
  }
  return false;
}


/************************************************
  check_gold_seeker MOVED to LibRogue
  check_imitator MOVED to LibRogue
  imitating MOVED to LibRogue
************************************************/


/**********************************************************************
                       STING
 IN:
 monster = a poisonous monster
 various globals
 OUT:
 nothing
 PURPOSE:
 Try to sting the rogue, which may decrement strength.
 **********************************************************************/
static void sting(object *monster) {
  fighter *rogue = sotu->roguep;
  Short sting_chance = 35;
  Char msg[80];
  Char mn[18];

  /* Rogue may be immune, or too weak to be affected. */
  if ((rogue->str_current <= 3) || (sotu->ring_flags & RING_SUSTAIN_STR)) {
    return;
  }
  /* Helps to have good armor or high level. */
  sting_chance += (6 * (6 - get_armor_class(sotu)));

  if ((rogue->exp + sotu->ring_dex) > 8) {
    sting_chance -= (6 * ((rogue->exp + sotu->ring_dex) - 8));
  }
  if (rand_percent(sting_chance)) {
    mon_name(monster, mn, sotu);
    StrPrintF(msg, "the %s bite has weakened you", mn); /* fits better */
    /*     StrPrintF(msg, "the %s's bite has weakened you", mn); */
    message(msg, sotu);
    if (rand_percent(50))
      monster->m_flags1 &= (~STINGS);
    rogue->str_current--;
    print_stats(STAT_STRENGTH, sotu);
  }
}

/**********************************************************************
                       DROP_LEVEL
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Possibly decrease the rogue's experience.
 **********************************************************************/
static void drop_level(object *monster) {
  fighter *rogue = sotu->roguep;
  Int hp;
  Boolean q = (monster->m_char == 'q');

  if (rand_percent(q ? 8 : 80) || (rogue->exp <= 5)) {
    return;
  }
  if (q) {
    message("you overflow!", sotu);
    if (rand_percent(75))
      monster->m_flags1 &= (~DROPS_LEVEL);
  }
  rogue->exp_points = sotu->level_points[rogue->exp-2] - get_rand(9, 29);
  rogue->exp -= 2;
  hp = hp_raise(sotu);
  if ((rogue->hp_current -= hp) <= 0) {
    rogue->hp_current = 1;
  }
  if ((rogue->hp_max -= hp) <= 0) {
    rogue->hp_max = 1;
  }
  add_exp(1, 0, sotu);
}

/**********************************************************************
                       DRAIN_LIFE
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Possibly decrease the rogue's strength / hit points.
 **********************************************************************/
static void drain_life() {
  fighter *rogue = sotu->roguep;
  Short n;

  if (rand_percent(60) || (rogue->hp_max <= 30) || (rogue->hp_current < 10)) {
    return;
  }
  n = get_rand(1, 3);		/* 1 Hp, 2 Str, 3 both */

  if ((n != 2) || (!(sotu->ring_flags & RING_SUSTAIN_STR))) {
    message("you feel weaker", sotu);
  }
  if (n != 2) {
    rogue->hp_max--;
    rogue->hp_current--;
    sotu->less_hp++;
  }
  if (n != 1) {
    if ((rogue->str_current > 3) && (!(sotu->ring_flags & RING_SUSTAIN_STR))) {
      rogue->str_current--;
      if (coin_toss()) {
	rogue->str_max--;
      }
    }
  }
  print_stats((STAT_STRENGTH | STAT_HP), sotu);
}

/**********************************************************************
                       M_CONFUSE
 IN:
 monster = a monster with a confusing gaze
 various globals
 OUT:
 true if the rogue is confused.
 PURPOSE:
 The monster (if visible) gets one chance to confuse the rogue.
 **********************************************************************/
Boolean m_confuse(object *monster) {
  Char msg[80];
  Char mn[18];

  /* Gaze attack only works if the rogue can see the monster! */
  if (!rogue_can_see(monster->row, monster->col, sotu)) {
    return false;
  }
  if (rand_percent(45)) {
    monster->m_flags1 &= (~CONFUSES);	/* will not confuse the rogue */
    return false;
  }
  if (rand_percent(55)) {
    monster->m_flags1 &= (~CONFUSES);
    mon_name(monster, mn, sotu);
    StrPrintF(msg, "the gaze of the %s has confused you", mn);
    message(msg, sotu);
    confuse(sotu);
    return true;
  }
  return false;
}
/**********************************************************************
                       HUG
 IN:
 monster = think "I'm being eaten by a boa constrictor"
 various globals
 OUT:
 nothing
 PURPOSE:
 Try to squeeze the player into a nice compact MRE.
 **********************************************************************/
static void hug(object *monster) {
  fighter *rogue = sotu->roguep;
  Char msg[80];
  Char mn[18];
  Char dam[4] = {2,8,0,0};
  Short damage;

  if (rand_percent(81)
      || (rogue->armor && rogue->armor->which_kind == CRYSTAL_ARMOR))
    return; // congratuations, you're impervious.
  mon_name(monster, mn, sotu);
  StrPrintF(msg, "the %s squeeeezes you", mn);
  message(msg, sotu);
  if (rand_percent(50))
    monster->m_flags1 &= (~HUGS);
  // subtract 2d8 hit points and, maybe, die
  damage = get_damage(dam, 1);
  rogue_damage(min(damage, rogue->hp_current-1), monster);
}
/**********************************************************************
                       PARALYZE
 IN:
 monster
 various globals
 OUT:
 nothing
 PURPOSE:
 Try to keep the player all tied up
 **********************************************************************/
static void paralyze(object *monster) {
  Char msg[80];
  Char mn[18];

  if ( (rand_percent(81) && (monster->m_char != 'e'))
       || (sotu->ring_flags & RING_FREE_ACTION))
    return; // congratuations, you're still moving
  mon_name(monster, mn, sotu);
  StrPrintF(msg, "the %s's gaze paralyzed you", mn);
  message(msg, sotu);
  if (rand_percent(25))
    monster->m_flags1 &= (~PARALYZES);
  //  take_a_nap();
}
/**********************************************************************
                       DRINK_BLOOD
 IN:
 monster = bloodsucker
 various globals
 OUT:
 nothing
 PURPOSE:
 mmm O-
 **********************************************************************/
static void drink_blood(object *monster) {
  Char msg[80];
  Char mn[18];
  fighter *rogue = sotu->roguep;

  monster->m_flags1 &= (~SUCKER); // it will only do this once
  monster->m_flags2 |= (FLEEING); // it will try to leave the room now, maybe
  mon_name(monster, mn, sotu);
  StrPrintF(msg, "the %s drank your blood", mn);
  message(msg, sotu);
  rogue_damage(min(10, rogue->hp_current-1), monster);
  // the monster should "FLEE" now, but I don't have that implemented!!!
  // so I will make it vanish instead.
  //  disappear(monster, sotu);
}
/**********************************************************************
                       SLOW_ROGUE
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 make the rogue slow down.
 **********************************************************************/
static void slow_rogue() {
  message("you feel yourself slowing down", sotu);
  if (sotu->haste_self > 0)
    sotu->haste_self = 0;
  else
    sotu->haste_self = -4;
  // This should be a random number, but I don't want to think
  // right now about: whether it should be cumulative,
  // what range is fair, how to make it "even" using %
  // and whether I need to change lib_use.c to check for negative.
}

/**********************************************************************
                       BREATH_ATTACK // FLAME_BROIL
 IN:
 monster = a monster with a breath attack
 various globals
 OUT:
 true if the monster Did Its Thing.
 PURPOSE:
 This is a really cool attack, at least from the monster's perspective,
 and animated too.
 **********************************************************************/
Boolean breath_attack(object *monster) {
  fighter *rogue = sotu->roguep;
  Short row, col;
  Short i;

  /* The monster must see the rogue to emply this attack. */
  /* so, um, somehow this happened across a winding tunnel. I think. */
  if ((!mon_sees(monster, rogue->row, rogue->col, sotu->rooms)) 
      || coin_toss()) {
    return false;
  }

  /* (It seems that what I want here is absolute value.) */
  row = rogue->row - monster->row;
  col = rogue->col - monster->col;
  if (row < 0) {
    row = -row;
  }
  if (col < 0) {
    col = -col;
  }

  /* The monster can only attack in the eight proper directions,
     and only if the rogue is close enough. */
  if (((row != 0) && (col != 0) && (row != col)) ||
      ((row > 7) || (col > 7))) {
    return false;
  }

  /* Animation. */
  if ((!sotu->blind) && 
      (!rogue_is_around(monster->row, monster->col, rogue))) 
    {
      row = monster->row;
      col = monster->col;
      get_closer(&row, &col, rogue->row, rogue->col);
      //      standout();
      do {
	//	mvaddch(row, col, '~');
	mvaddch(row, col, FLAMECHAR);  // XXX animation
	refresh();
	SysTaskDelay(SysTicksPerSecond()/10);
	get_closer(&row, &col, rogue->row, rogue->col);
      } while ((row != rogue->row) || (col != rogue->col));
      //      standend();
      row = monster->row; col = monster->col;
      get_closer(&row, &col, rogue->row, rogue->col);
      do {
	mvaddch(row, col, get_dungeon_char(row, col, sotu));
	refresh();
	get_closer(&row, &col, rogue->row, rogue->col);
      } while ((row != rogue->row) || (col != rogue->col));
    }

  /* WHOOOMP! ow!   mon_hit will roll to hit and damage the rogue. */

  i = monster->breath_type;
  if (i == B_RANDOM || (i < 1 || i > BREATH_ATTACKS))
    i = get_rand(1, BREATH_ATTACKS); // yeah, it's not zero-based.  sue me.
  // if the player is affected by the chosen attack, 
  mon_hit(monster, breath_name[i-1], i); // hit.c
  // else display a message

  // also do special effects, if any, from gasses.
  return true;
}

// Called by mon_hit if the breath hits the player.
void do_breath(Short i, Short *damage) {
  fighter *rogue = sotu->roguep;
  Char buf[80];
  if (i == B_FLAME) {
    if (sotu->ring_flags & RING_PROTECT_FIRE) {
      *damage = 0;
      message("the flame flickers and fades", sotu);
    }
    return;
  }
  if (i == B_ICE) {
    if (sotu->ring_flags & RING_PROTECT_COLD) {
      *damage = 0;
      message("the ice cracks and melts", sotu);
    }
    return;
  }
  if (i == B_ACID) {
    if (rogue->armor && (rogue->armor->which_kind == CRYSTAL_ARMOR)) {
      *damage = 0;
      //      check_message();
      message("the acid splashes harmlessly against your armor", sotu);
    }
    return;
  }
  if (i == B_LGHTN) {
    if ((rogue->armor && rogue->armor->which_kind == CRYSTAL_ARMOR)
	|| (sotu->ring_flags & RING_PROTECT_SHOCK)) {
      *damage = 0;
      //      check_message();
      //      message("your armor is covered with dancing blue light!", sotu);
      if (!rogue->weapon) {
	//	check_message();
	message("your armor is covered with dancing blue light", sotu);
      } else {
	StrPrintF(buf, "your armor and %sare covered with dancing blue light",
		  id_weapons[rogue->weapon->which_kind].title);
	//	check_message();
	message(buf, sotu);
	// charge the weapon.... add 10-25 charges, up to a maximum of 50
	if (rogue->weapon->what_is == WEAPON) {
	  rogue->weapon->o_flags |= O_ZAPPED;
	  rogue->weapon->class += 10 + get_rand(0,15);
	  if (rogue->weapon->class > 50) rogue->weapon->class = 50;
	}
      }
    }
    return;
  }
  // chlorine gas has no interesting effect, I guess.
  //
  // this rand_percent should be a save-vs-breath
  if (rand_percent(15)
      || (sotu->ring_flags & RING_BREATHING))
    return;
  if (i == B_NERVE) {
    if (!(sotu->ring_flags & RING_FREE_ACTION)) {
      message("the nerve gas paralyzed you", sotu);
      //      take_a_nap();
    }
  }
  if (i == B_SLEEP) {
    if (!(sotu->ring_flags & RING_ALERTNESS)) {
      message("you fell asleep", sotu);
      //      take_a_nap();
    }
  }
  if (i == B_SLOW) {
    if (!(sotu->ring_flags & RING_FREE_ACTION)) {
      slow_rogue();
    }
  }
  //  if (i == B_FEAR) {
  //    message("the fear gas terrifies you", sotu);
  //  }
}



/* To Do:

   Split out the "animation" code in Arrow and Dragon Breath;
   parameterize appropriately.
   Reuse for some kinds of zap?
   Add a lightning-bolt wand.
   Add zapped-weapon lightning attack which is equivalent.

 */

/* Special breath thingies:

   the damage is set as the monster's current hit points!!!  OW.
   instances of hit points are: 9, 45, 56, 64, 72, 80, 88, 96, 128, 168
   you get to 'save vs breath' to halve it.

   you get to 'save vs magic' as the to-hit;
   mithril armor helps.

   "the $name hits you"
   if crystal_armor && acid
      "the acid splashes harmlessly against your armor"
      damage = 0
   if crystal_armor (or electric ring) && lightning bolt
      "your armor [and $weapon] is[are]covered with dancing blue lights!"
      damage = 0
      charge the current weapon if any
   if applicable rings && fire/ice
      message of harmlessness
      damage = 0
   damage the rogue! [maybe die]
   apply special effects from some gasses:
      nerve = paralyze (frob the ice monster)
      sleeping = take a nap
      slow = slow
      fear = 'flee' (not implemented)

 */

/*********************************************************
  get_closer moved to LibRogue/lib_hit.c 
  ********************************************************/

/***************************************************************************
 *     Moved all of lib_spechit.c into here:
 ***************************************************************************/



/***************************************************************
                   GET_CLOSER
 IN:
 row, col = the current location
 trow, tcol = the goal
 OUT:
 nothing (really row,col)
 PURPOSE:
 Figure out which of the nine squares "reachable" from row,col
 is closest to trow,tcol.  Return that location in row,col.
****************************************************************/
void get_closer(Short *row, Short *col, Short trow, Short tcol) {
  if (*row < trow) {
    (*row)++;
  } else if (*row > trow) {
    (*row)--;
  }
  if (*col < tcol) {
    (*col)++;
  } else if (*col > tcol) {
    (*col)--;
  }
}

/***************************************************************
                   IMITATING
 IN:
 row, col = location to check for imitator
 sotu = various globals (dungeon, level_monsters)
 OUT:
 true if there's an imitator there
 PURPOSE:
 Given a location, determine whether there's an imitator there
 (a monster that can camoflague itself as something else.)
****************************************************************/
Boolean imitating(Short row, Short col, struct state_of_the_union * sotu) {
  object *monster;
  if (sotu->dungeon[row][col] & MONSTER) {
    if ((monster = object_at(sotu->level_monsters, row, col))) {
      if (monster->m_flags1 & IMITATES) {
	return true;
      }
    }
  }
  return false;
}

/***************************************************************
                   DISAPPEAR
 IN:
 monster = the one that's disappearing
 sotu = various globals (dungeon, level_monsters, mon_disappeared)
 OUT:
 nothing
 PURPOSE:
 Remove the given monster from the dungeon level.
 (mon_disappeared is checked if the monster is hasted, since it
 could disappear and then get a second chance to move..)
****************************************************************/
void disappear(object *monster, struct state_of_the_union *sotu) {
  Short row, col;

  row = monster->row;
  col = monster->col;

  sotu->dungeon[row][col] &= ~MONSTER;
  if (rogue_can_see(row, col, sotu)) {
    mvaddch(row, col, get_dungeon_char(row, col, sotu));
  }
  take_from_pack(monster, sotu->level_monsters);
  free_object(monster, sotu);
  sotu->mon_disappeared = true;
}

/***************************************************************
                   TRY_TO_COUGH
 IN:
 row, col = a location near a dying monster
 obj = an object that the monster will (maybe) leave behind
 sotu = various globals (
 OUT:
 true if the object could be placed at the location
 PURPOSE:
 This will try to place the given object at the given location
 and (if there's not a rogue or monster standing on it) update
 the display.
****************************************************************/
static Boolean try_to_cough(Short row, Short col, object *obj,
			    struct state_of_the_union *sotu) SEC_L;
static Boolean try_to_cough(Short row, Short col, object *obj,
			    struct state_of_the_union *sotu) 
{
  if ((row < MIN_ROW) || (row > (DROWS-2)) || (col < 0) || (col>(DCOLS-1))) {
    return false;
  }
  if ((!(sotu->dungeon[row][col] & (OBJECT | STAIRS | TRAP))) &&
      (sotu->dungeon[row][col] & (TUNNEL | FLOOR | DOOR))) {
    place_at(obj, row, col, sotu);
    if (((row != sotu->roguep->row) || (col != sotu->roguep->col)) &&
	(!(sotu->dungeon[row][col] & MONSTER))) {
      mvaddch(row, col, get_dungeon_char(row, col, sotu));
    }
    return true;
  }
  return false;
}

/***************************************************************
                   COUGH_UP
 IN:
 monster = dying beastie that leaves some treasure behind
 sotu = various globals (cur_level, max_level)
 OUT:
 nothng
 PURPOSE:
 (Maybe) Create and place gold, or an object, near a dying monster.
****************************************************************/
void cough_up(object *monster, struct state_of_the_union *sotu) {
  object *obj;
  Short row, col, i, n;

  if (sotu->cur_level < sotu->max_level) {
    /* you don't get treasure coughed up on your way home */
    return;
  }

  /* Some monsters cough up gold.
     Others have a percentage chance of leaving an object behind. */
  if (monster->m_flags1 & STEALS_GOLD) {
    obj = alloc_object(sotu);
    obj->what_is = GOLD;
    obj->quantity = get_rand((sotu->cur_level * 15), (sotu->cur_level * 30));
  } else if ((monster->m_flags2 & TASTY) && rand_percent(15)) {
    obj = alloc_object(sotu);
    get_food(obj, 1);
    obj->which_kind = CARCASE;
    obj->o_unused = monster->carcase_type;
  } else {
    if (!rand_percent((Int) monster->drop_percent)) {
      return;
    }
    obj = gr_object(sotu);
  }
  row = monster->row;
  col = monster->col;

  /* Try to place the object somewhere near the dying monster. */
  for (n = 0; n <= 5; n++) {
    for (i = -n; i <= n; i++) {
      if (try_to_cough(row+n, col+i, obj, sotu)) {
	return;
      }
      if (try_to_cough(row-n, col+i, obj, sotu)) {
	return;
      }
    }
    for (i = -n; i <= n; i++) {
      if (try_to_cough(row+i, col-n, obj, sotu)) {
	return;
      }
      if (try_to_cough(row+i, col+n, obj, sotu)) {
	return;
      }
    }
  }
  /* If there isn't room nearby, don't cough it up after all. */
  free_object(obj, sotu);
}

/***************************************************************
                   CHECK_GOLD_SEEKER
 IN:
 monster = the one to check
 OUT:
 nothing
 PURPOSE:
 "check" seems misleading to me.  The effect is to make a monster
 not-gold-seeking.
****************************************************************/
void check_gold_seeker(object *monster) {
  if (monster)
    monster->m_flags1 &= (~SEEKS_GOLD);
}

/***************************************************************
                   CHECK_IMITATOR
 IN:
 monster = the monster in question
 sotu = various globals (blind)
 OUT:
 true if the monster is an imitator...
 PURPOSE:
 Given a monster, if it is an imitator, wake it up and reveal
 (if the rogue isn't blind) its true appearance.  
****************************************************************/
Boolean check_imitator(object *monster, struct state_of_the_union *sotu) {
  Char msg[80];
  Char mn[16];

  if (monster && monster->m_flags1 & IMITATES) {
    wake_up(monster);
    if (!sotu->blind) {
      mvaddch(monster->row, monster->col,
	      get_dungeon_char(monster->row, monster->col, sotu));
      //      check_message();
      mon_name(monster, mn, sotu);
      StrPrintF(msg, "wait, that's a %s!", mn);
      message(msg, sotu);
    }
    return true;
  }
  return false;
}


