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
 * hit.c
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

extern Char * hit_message;

/**********************************************************************
                       CHECK_HIT_MESSAGE
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 well, basically to display the hit message if there is one
 ...
 **********************************************************************/
void check_hit_message() {
  if (hit_message[0] != 0) {
    /*    StrPrintF(hit_message + StrLen(hit_message), " FOO!"); */
    message(hit_message, sotu);
    hit_message[0] = 0;
  }
}


/**********************************************************************
                       MON_HIT
 IN:
 monster = the attacking monster
 other = name to use instead of monster's, if nonnull
 breath = whether it's a breath attack (the only time 'other' is nonnull!)
 various globals
 OUT:
 nothing
 PURPOSE:
 Monster hits (or misses) rogue, the rogue takes damage, and hit_message
 gets some stuff added to it.  Also, monsters may do special attacks.
 ('breath' could be omitted since 'other' is used only for breath attack.)
 if 'breath' then it was called by breath_attack
 **********************************************************************/
void mon_hit(object * monster, Char * other, Short breath) {
  fighter *rogue = sotu->roguep;
  Short damage, hit_chance;
  Char mn[18];
  Int minus;

  if (i_am_dead)
    return;

  /* Is the player in 'fight' mode with this monster. */
  if (sotu->fight_monster && (monster != sotu->fight_monster)) {
    sotu->fight_monster = 0;
  }
  /* The monster can hit the rogue so it has no movement goal now. */
  monster->trow = NO_ROOM;

  /* Calculate monster's chance to hit. */
  if (sotu->cur_level >= (AMULET_LEVEL * 2)) {
    hit_chance = 100;
  } else {
    hit_chance = monster->m_hit_chance;
    hit_chance -= (((2 * rogue->exp) + (2 * sotu->ring_dex)) - sotu->r_rings);
  }
  if (IS_WIZARD) {
    hit_chance /= 2;
  }
  if (breath) hit_chance = 100; // XXX just for testing breaths!!!

  /* If the rogue is resting or cruising along or something, make sure
     it stops doing that so the player has a chance to hit back. */
  if (!sotu->fight_monster) {
    sotu->interrupted = true;
  }
/*   mn = mon_name(monster); */
  mon_name(monster, mn, sotu);

  //  message(mn, sotu); // XXX test to see if monsters get here often enough
  // well, every time they get here, I get a hit/miss message also,
  // but they are not getting here very often!  bunch of stoners.

  if (breath) {
    hit_chance -= ((rogue->exp + sotu->ring_dex) - sotu->r_rings);
  }

  /* Monster misses ==> add this fact to hit_message and return */
  if (!rand_percent(hit_chance)) {
    if (!sotu->fight_monster) {
      StrPrintF(hit_message + StrLen(hit_message),
		"the %s misses", (other ? other : mn));
      message(hit_message, sotu);
      hit_message[0] = 0;
    }
    return;
  }
  /* Monster hits ==> add this to hit_message (unless in fight mode) */
  if (!sotu->fight_monster) {
    StrPrintF(hit_message + StrLen(hit_message), "the %s hit",
	    (other ? other : mn));
    message(hit_message, sotu);
    hit_message[0] = 0;
  }
  /* Calculate the damage. */
  if (!((monster->m_flags2 & STATIONARY)
	&& !(monster->m_flags2 & TASTY))) {
    damage = get_damage(monster->m_damage, 1);
    if (breath) {
      if ((damage -= get_armor_class(sotu)) < 0) {
	damage = 1;
      }
    }
    if (sotu->cur_level >= (AMULET_LEVEL * 2)) {
      minus = ((AMULET_LEVEL * 2) - sotu->cur_level);
    } else {
      minus = get_armor_class(sotu) * 3;
      //    minus = minus/100 *  damage; // This will not work due to roundoff!
      minus = (minus * damage)/100;
    }
    damage -= (Short) minus;
  } else {
    /* Most stationary monsters become increasingly dangerous. */
    damage = monster->stationary_damage++;
  }
  if (IS_WIZARD) {
    damage /= 3;
  }
  do_breath(breath, &damage); // may reduce damage.  may print message.
  if (damage > 0) {
    rogue_damage(damage, monster); /* ow! */
  }

  if (!breath && (monster->m_flags1 & SPECIAL_HIT))
    special_hit(monster);


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

}

/**********************************************************************
                       ROGUE_HIT
 IN:
 monster = the victim of the attack
 force_hit = if true, hit probability = 1 (just roll for damage.)
 various globals
 OUT:
 PURPOSE:
 Rogue hits (or misses) monster ... damage ... messages ...
 **********************************************************************/
void rogue_hit(object *monster, Boolean force_hit) {
  fighter *rogue = sotu->roguep;
  Short damage, hit_chance;
  Boolean mon_killed = false;

  if (monster) {
    if (check_imitator(monster, sotu)) {
      /* this wakes up the imitator and reveals its true shape */
      return;
    }
    // also check can_surprise (urogue)

    /* Calculate chance to hit */
    hit_chance = force_hit ? 100 : get_hit_chance(rogue->weapon, false, sotu);
    if (IS_WIZARD) {
      hit_chance *= 2;
    }

    if (!rand_percent(hit_chance)) {
      /* Oops, you miss. */
      if (!sotu->fight_monster) {
	StrCopy(hit_message, "you miss  ");
      }
    } else {
      /* Hit!  Calculate damage. */
      damage = get_weapon_damage(rogue->weapon, false, sotu);
      if (IS_WIZARD) {
	damage *= 3;
      }
      check_shrieker(monster); // nicer to do this BEFORE it's killed...
      if (mon_damage(monster, damage)) {	/* still alive? */
	if (!sotu->fight_monster) {
	  StrCopy(hit_message, "you hit  ");
	}
      } else mon_killed = true;
    }
    if (!mon_killed) {
      /* Even if you miss the monster, you will wake it up. */
      check_gold_seeker(monster); /* turns off gold-seeking. */
      wake_up(monster); // Is this not working???
    }
  }
}

/**********************************************************************
                       CHECK_SHRIEKER
 IN:
 monster = maybe a shrieker
 various globals
 OUT:
 PURPOSE:
 When a shrieker is first hit, it aggravates the dungeon and (small
 probability) shatters crystalline armor.
 **********************************************************************/
// NOTE - 'throw' should also cause something to shriek
void check_shrieker(object *monster) {
  fighter *rogue = sotu->roguep;
  Char msg[80];
  Char mn[18];
  if (monster->m_flags1 & SHRIEKS) {
    monster->m_flags1 &= (~SHRIEKS);
    monster->m_flags2 |= ALREADY_MOVED; // I think this is enough of a move.
    // if player has extra-powerful hearing, he should be 'stunned'
    // by the shriek for d8+4 turns.  if I implement that.
    //    check_message();
    mon_name(monster, mn, sotu);
    StrPrintF(msg, "the %s emits a piercing shriek", mn);
    do_feep(800, 80); // good pitch, maybe a tad short in duration.
    message(msg, sotu);
    // wake up dungeon..
    aggravate(sotu);
    // maybe destroy crystalline armor..
    if (rogue->armor && rogue->armor->which_kind == CRYSTAL_ARMOR
	&& rand_percent(2)) {
      message("your armor shatters!", sotu);
      vanish(rogue->armor, &rogue->pack, sotu);
      print_stats(STAT_ARMOR, sotu); // mv_aquatars?
    }
  }
}

/**********************************************************************
                       ROGUE_DAMAGE
 IN:
 d = the amount of damage
 monster = the one inflicting it
 various globals
 OUT:
 PURPOSE:
 Damage the rogue, possibly killing it.  Update stats display.
 **********************************************************************/
void rogue_damage(Short d, object *monster) {
  fighter *rogue = sotu->roguep;
  Short i;
  if (i_am_dead)
    return;
  if (d >= rogue->hp_current) {
    if (sotu->ring_flags & RING_LIFESAVING) {
      // find the ring; 'vanish' will do all the clean-up
      for (i = 0 ; i < 8 ; i++) {
	if (rogue->rings[i] && (rogue->rings[i]->which_kind == R_LIFESAVING)) {
	  vanish(rogue->rings[i], &rogue->pack, sotu);
	  // restore hit points to maximum, taking no damage from this hit!
	  rogue->hp_current = rogue->hp_max;
	  message("you feel a chill", sotu);
	  print_stats(STAT_HP, sotu);
	  return;
	}
      } // end for.. we didn't find the ring.. bug.. kill the rogue anyway!
    }
    rogue->hp_current = 0;
    print_stats(STAT_HP, sotu);
    i_am_dead = true;
    killed_by(monster, 0, sotu);
    FrmPopupForm(TopTenForm);
    return;
  }
  rogue->hp_current -= d;
  print_stats(STAT_HP, sotu);
}

/********************************************
 get_damage has MOVED to LibRogue/lib_hit.c 
 get_w_damage has MOVED to LibRogue/lib_hit.c 
 to_hit has MOVED to LibRogue/lib_hit.c 
 damage_for_strength has MOVED to LibRogue/lib_hit.c 
 ********************************************/



/**********************************************************************
                       MON_DAMAGE
 IN:
 monster = the one receiving the damage
 d = the amount of damage
 various globals
 OUT:
 PURPOSE:
 Damage the monster, possibly killing it, in which case there are
 umpteen things to do to restore balance to the universe
 **********************************************************************/
Boolean mon_damage(object *monster, Short damage) {
  Char mn[18];
  Short row, col;

  monster->hp_to_kill -= damage;

  if (monster->hp_to_kill <= 0) {
    row = monster->row;
    col = monster->col;
    dungeon[row][col] &= ~MONSTER;
    mvaddch(row, col, (Int) get_dungeon_char(row, col, sotu));

    sotu->fight_monster = 0;
    cough_up(monster, sotu); /* treasure! */
    mon_name(monster, mn, sotu);
    StrPrintF(hit_message + StrLen(hit_message), "defeated the %s", mn);
    message(hit_message, sotu);
    hit_message[0] = 0;
    // Also if you are undead, small % chance of drinking blood
    //if ((sotu->score_status & STATUS_UNDEAD) && rand_percent(25))
    if ((sotu->conduct & CONDUCT_UNDEAD) && rand_percent(25))
      drink_monblood(monster, sotu);

    add_exp(monster->kill_exp, 1, sotu);
    take_from_pack(monster, level_monsters);
    /* How about refreshing the screen??  eh? */

    if (monster->m_flags1 & HOLDS) {
      sotu->being_held = false;
    }
    free_object(monster, sotu);
    return(0);
  }
  return(1);
}


/**********************************************************************
                       FIGHT
 IN:
 dir = the direction to attack in
 to_the_death = true if you don't care how much damage you take..
 various globals
 OUT:
 PURPOSE:
 Fight on autopilot so the player doesn't have to keep tapping the
 screen and seeing hit/miss messages.  Unless to_the_death is true,
 the fight will be interrupted when the monster might be able to
 kill the rogue with one hit.
 **********************************************************************/
/* not for use in lib due to one_move_rogue etc */
void fight(Short dir, Boolean to_the_death) {
  fighter *rogue = sotu->roguep;
  Short c;
  Short row, col;
  Short possible_damage;
  object *monster;

  /* Convert direction to a row.col */
  if (dir == NO_DIRECTION) {
    return;
  }
  row = rogue->row; col = rogue->col;
  get_dir_rc(dir, &row, &col, false);

  /* Get the monster there, if any */
  c = mvinch(row, col);
  if (  (Not_Alpha(c)) ||
      (!can_move(rogue->row, rogue->col, row, col, dungeon))) {
    message("I see no monster there", sotu);
    return;
  }
  if (!((sotu->fight_monster = object_at(sotu->level_monsters, row, col)))) {
    return;
  }

  /* Figure out the maximum damage the monster can inflict in one hit */
  if (!(sotu->fight_monster->m_flags2 & STATIONARY)) {
    possible_damage = ((get_damage(sotu->fight_monster->m_damage, 0) * 2) / 3);
  } else {
    possible_damage = sotu->fight_monster->stationary_damage - 1;
  }

  while (sotu->fight_monster && !i_am_dead) {
    /* (almost) as if the player initiated a move in that direction. */
    one_move_rogue(dir, false, false);
    /* Stop fighting if interrupted, or the monster moves, or
       the rogue could be killed in one hit (and the player wanted
       the fight to stop before the rogue is dead.) */
    if (((!to_the_death) && (rogue->hp_current <= possible_damage)) ||
	sotu->interrupted || (!(dungeon[row][col] & MONSTER))) {
      sotu->fight_monster = 0;
    } else {
      /* Make sure the monster at that location is still the same one! */
      monster = object_at(sotu->level_monsters, row, col);
      if (monster != sotu->fight_monster) {
	sotu->fight_monster = 0;
      }
    }
  }
}


/************************************************
  get_dir_rc MOVED to LibRogue
************************************************/

/********************************************
 get_hit_chance has MOVED to LibRogue/lib_hit.c 
 get_weapon_damage has MOVED to LibRogue/lib_hit.c 
 ********************************************/

/***************************************************************************
 *     Moved all of lib_hit.c into here:
 ***************************************************************************/

static Short damage_for_strength(struct state_of_the_union * sotu) SEC_L;

/***************************************************************
                   GET_DAMAGE
 IN:
 ds = representation of how many dice of what kind to roll.
 r = if true, don't roll; return the maximum possible damage.
 OUT:
 calculated damage
 PURPOSE:
 Calculate damage, either from rolling the dice represented
 by ds, or (if 'r') use the maximum possible.
****************************************************************/
Short get_damage(Char ds[4], Boolean r) {
  Short i = 0, j, n, d, total = 0;
  Boolean repeat = false;

  if (ds[1]==0) return 0; /* "no dice */
  if (!r) {
    /* don't roll, calc. max. damage */
     total = ds[0] * ds[1] + ds[2] * ds[3]; /* x d y + w d z */
     return total;
  }

  /* "do this one more time if there's a second pair" */
  do {
    repeat = !repeat; /* true first time, false second time */
    n = ds[i++];
    d = ds[i++];
    for (j = 0; j < n; j++) {
      total += get_rand(1, d);
    }
  } while (repeat && ds[3] != 0);

  return total;
}


/***************************************************************
                   DAMAGE_FOR_STRENGTH
 IN:
 sotu = various globals (rogue, add_strength)
 OUT:
 the damage modifier due to exceptional strength
 PURPOSE:
 Use this when the rogue hits something: if the rogue is
 exceptionally strong, extra damage is done; if the rogue is
 exceptionally weak, less than the normal amount will be inflicted.
****************************************************************/
// ELIMINATE
static Short damage_for_strength(struct state_of_the_union * sotu) {
  Short strength;

  strength = sotu->roguep->str_current + sotu->add_strength;

  if (strength <= 6) {
    return(strength-5);
  }
  if (strength <= 14) {
    return(1);
  }
  if (strength <= 17) {
    return(3);
  }
  if (strength <= 18) {
    return(4);
  }
  if (strength <= 20) {
    return(5);
  }
  if (strength <= 21) {
    return(6);
  }
  if (strength <= 30) {
    return(7);
  }
  return(8);
}
/*
 (str-10) / 3 = 
 <=  4 --> -2
 <=  7 --> -1
 <= 10 -->  0
 <= 13 -->  1
 <= 16 -->  2
 <= 19 -->  3
 <= 22 -->  4
 <= 25 -->  5
 <= 28 -->  6
*/


/***************************************************************
                   GET_HIT_CHANCE
 IN:
 sotu = various globals (rogue, ring_dex, r_rings)
 OUT:
 rogue's chance of hitting something
 PURPOSE:
 Used when the rogue tries to hit a monster, to determine
 the chance that the rogue actually will hit it.
****************************************************************/
// change for throwing?
Short get_hit_chance(object *weapon, Boolean thrown,
		     struct state_of_the_union * sotu) {
  Short hit_chance;
  Short to_hit = 1;

  // I'm not keen on the weapon->damage[0] here
  if (weapon) {
    to_hit = weapon->damage[0];
    if (!thrown && !(weapon->o_flags & O_MISSILE)) {
	to_hit += 3 * weapon->hit_enchant;
    }
    if ( thrown &&  (weapon->o_flags & O_MISSILE)) { 
      if ((weapon->launcher != NO_LAUNCHER)
	  && (sotu->roguep->weapon->which_kind == weapon->launcher)) {
	to_hit += 2 * (sotu->roguep->weapon->hit_enchant
		       + weapon->hit_enchant);
      }
      if ((weapon->launcher == NO_LAUNCHER)
	  && (weapon->in_use_flags & BEING_WIELDED)) {
	to_hit += 3 * weapon->hit_enchant;	
      }
    }
  }

  hit_chance = 50;
  hit_chance += to_hit;

  // placeholder for a dexterity bonus
  hit_chance += min( 5, ((sotu->roguep->str_current
			  + sotu->add_strength - 10))); // can be <0

  hit_chance += (((2 * sotu->roguep->exp) +
		  (2 * sotu->ring_dex)) - sotu->r_rings);
  return hit_chance;
  // if thrown and properly-launched, hit chance *= 4/3!
}

/***************************************************************
                   GET_WEAPON_DAMAGE
 IN:
 obj = the weapon being used (or 0)
 sotu = various globals (rogue, ring_dex, r_rings)
 OUT:
 The damage inflicted, including modifiers
 PURPOSE:
 Called after the rogue hits something, to calculate the total
 damage inflicted (including weapon, strength, & magical effects)
****************************************************************/
// REWRITE
// see roll_em in urogue.
Short get_weapon_damage(object *obj, Boolean thrown,
			struct state_of_the_union * sotu) {
  Short damage;
  //object *use_my_bonus = obj;

  Char dice[4] = {0,0,0,0};
  if ((!obj) || (obj->what_is != WEAPON)) {
    // no weapon!  caller is clueless.  do 'hands' do any damage?
    damage = damage_for_strength(sotu);
    damage += ((((sotu->roguep->exp + sotu->ring_dex) - sotu->r_rings) + 1)/2);
    return (damage > 0 ? damage - 1 : 0);
  }

  // default is not-thrown, or improperly-launched-missile.
  dice[0] = obj->damage[0];
  dice[1] = obj->damage[1] + ( (!(obj->o_flags
				  & O_MISSILE)) ? obj->d_enchant : 0 );
  // NOTE FOR FUTURE - suggest unlaunched-arrow damage be DECREASED a LOT.
  // Well - I looked at lib_object.c and that is ALREADY the case..
  // it is 1d1 without bow compared to 1d6 with bow.  So there!
  if (thrown) {
    if (!(obj->o_flags & O_MISSILE)) {
      // thrown non-missile?
      dice[0] = obj->damage[2];
      dice[1] = obj->damage[3]; // no hit+, no dam+
    } else {
      if (sotu->roguep->weapon) {
	if (obj->launcher != NO_LAUNCHER) {
	  // e.g. properly-launched arrow
	  if (sotu->roguep->weapon->which_kind == obj->launcher) {
	    //use_my_bonus = sotu->roguep->weapon;
	    dice[0] = obj->damage[2];
	    dice[1] = obj->damage[3] + (sotu->roguep->weapon->d_enchant
					+ obj->d_enchant);
	  } 
	} else if (obj->in_use_flags & BEING_WIELDED) {
	  // e.g. properly-launched shuriken
	  dice[0] = obj->damage[2];
	  dice[1] = obj->damage[3] + obj->d_enchant;
	}
      }
    }
  }
  damage = get_damage(dice, 1);

  damage += damage_for_strength(sotu); // inline!
  damage += ((((sotu->roguep->exp + sotu->ring_dex) - sotu->r_rings) + 1) / 2);
  return max(damage,0);
}


/***************************************************************
                   GET_DIR_RC
 IN:
 dir = direction
 row,col = location (modified)
 allow_off_screen = whether to do bounds-checking
 OUT:
 nothing (really row,col)
 PURPOSE:
 Given a location and a direction, find the location that is
 one move away in the given direction (and return in row,col.)
 (If allow_off_screen is false, the location will be unchanged
 if the move would be outside the dungeon proper.)
****************************************************************/
void get_dir_rc(Short dir, Short *row, Short *col, 
		Boolean allow_off_screen) {
  switch(dir) {
  case WEST:
    if (allow_off_screen || (*col > 0)) {
      (*col)--;  /* 'h' */
    }
    break;
  case SOUTH:
    if (allow_off_screen || (*row < (DROWS-2))) {
      (*row)++;  /* 'j' */
    }
    break;
  case NORTH:
    if (allow_off_screen || (*row > MIN_ROW)) {
      (*row)--;  /* 'k' */
    }
    break;
  case EAST:
    if (allow_off_screen || (*col < (DCOLS-1))) {
      (*col)++;  /* 'l' */
    }
    break;
  case NORTHWEST:
    if (allow_off_screen || ((*row > MIN_ROW) && (*col > 0))) {
      (*row)--;  /* 'y' */
      (*col)--;
    }
    break;
  case NORTHEAST:
    if (allow_off_screen || ((*row > MIN_ROW) && (*col < (DCOLS-1)))) {
      (*row)--;  /* 'u' */
      (*col)++;
    }
    break;
  case SOUTHEAST:
    if (allow_off_screen || ((*row < (DROWS-2)) && (*col < (DCOLS-1)))) {
      (*row)++;  /* 'n' */
      (*col)++;
    }
    break;
  case SOUTHWEST:
    if (allow_off_screen || ((*row < (DROWS-2)) && (*col > 0))) {
      (*row)++;  /* 'b' */
      (*col)--;
    }
    break;
  }
  /* anything else, don't move. */
}
