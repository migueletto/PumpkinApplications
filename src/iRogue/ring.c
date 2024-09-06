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

//Char *curse_message = "you can't, it appears to be cursed";
Char *no_ring = "there's no ring on that hand";
Char *no_rings = "not wearing any rings";

/***************************************************************
                   RING_STATS
 IN:
 pr = whether to make the effects visible immediately
 sotu = various globals (lots...)
 OUT:
 nothing
 PURPOSE:
 Reset the stuff affected by ring-wearing...
 Do not call from within do_put_on.
****************************************************************/
void ring_stats(Boolean pr, struct state_of_the_union *sotu) {
  Short i;
  object *ring;
  ULong flag_to_add = 01L;

  sotu->ring_flags = NO_RING_EFFECTS;
  sotu->r_rings = 0; // number of rings worn by rogue - bad for hit/damage??
  sotu->e_rings = 0; // this is to make you digest faster
  sotu->stealthy = 0;
  sotu->add_strength = 0;
  sotu->regeneration = 0;
  sotu->ring_dex = 0;
  sotu->ring_ac = 0;
  sotu->auto_search = 0;

  for (i = 0; i < 8; i++) {
    ring = sotu->roguep->rings[i];
    if (!ring) 
      continue;
    sotu->r_rings++;
    sotu->e_rings++;
    // add flag to ring_flags, according to which_kind.
    if (ring->which_kind >= 0 && ring->which_kind <= R_DELUSION)
      sotu->ring_flags |= (flag_to_add << ring->which_kind);
    // now take care of the non-flagged rings.
    switch(ring->which_kind) {
    case STEALTH:
      sotu->stealthy++;
      break;
    case REGENERATION:
      sotu->regeneration++;
      sotu->e_rings++; // this ring costs extra
      break;
    case SLOW_DIGEST:
      //      sotu->e_rings -= 2;
      sotu->e_rings -= 3; // cancels itself and two other rings
      break;
    case ADD_STRENGTH:
      sotu->add_strength += ring->class;
      break;
    case DEXTERITY:
      sotu->ring_dex += ring->class;
      break;
    case R_PROTECTION:
      sotu->ring_ac += ring->class;
      break;
    case ADORNMENT:
    case R_MOOD:
      sotu->e_rings--; // you can wear these for free
      break;
    case SEARCHING:
      sotu->auto_search += 2;
      break;
    }
  }
  sotu->e_rings /= 2; // you can wear more rings now.  seems more reasonable.
  if (pr) {
    print_stats(STAT_ALL, sotu);
    relight(sotu); /* to see invisible, I reckon */
  }
}


/***************************************************************
                   DO_PUT_ON
 IN:
 ring = the ring being put on
 on_left = put it on the left hand
 sotu = various globals (rogue)
 OUT:
 nothing
 PURPOSE:
 Perform the action of putting on a ring...  does not update stats.
 (Caller will have to call ring_stats.)
 NOTE: 
 Do not call ring_stats() from within do_put_on().  May cause problems
 when do_put_on() is called from read_pack() in restore().  (?)
****************************************************************/
static void do_put_on(object *ring, Short index,
		      struct state_of_the_union *sotu) SEC_L;
static void do_put_on(object *ring, Short index,
		      struct state_of_the_union *sotu)
{
  fighter *rogue = sotu->roguep;
  ULong ring_finger = 01L;

  if (ring && (index >= 0) && (index < 8)) {
    ring_finger = ring_finger << (2 + index);
    ring->in_use_flags |= ring_finger;
    rogue->rings[index] = ring;
  }
}

/***************************************************************
                   UN_PUT_ON
 IN:
 ring = ring to remove
 sotu = various globals (rogue)
 OUT:
 nothing
 PURPOSE:
 Inverse of do_put_on (except that it's ok to call ring_stats.)
 Removes the ring from whichever hand and updates stats.
****************************************************************/
void un_put_on(object *ring, struct state_of_the_union *sotu) {
  fighter *rogue = sotu->roguep;
  ULong ring_flag;
  Short i;

  if (ring && (ring->in_use_flags & ON_EITHER_HAND)) {
    ring_flag = ring->in_use_flags >> 2; // divide by 4
    // Right-shift it into nothing; the index is #shifts - 1
    // (I think!  XXXX)
    for (i = 0 ; (i < 8) && ring_flag ; i++) {
      ring_flag = ring_flag >> 1; // divide by 2
      if (!ring_flag) {
	rogue->rings[i] = 0; // we found it, I hope.
	ring->in_use_flags = 0; // (you can't wear or wield it anyway).
	break;
      }
    }
  } // else someone screwed up, but we'll ignore it.
  ring_stats(1, sotu);
}


/***************************************************************
                   REMOVE_RING
 IN:
 index = which ring to remove
 sotu = various globals (rogue)
 OUT:
 true means the action succeeded --> caller must call reg_move
 PURPOSE:
 Remove a ring, first checking to see that it's possible to do so.
****************************************************************/
Boolean remove_ring(Short index, struct state_of_the_union *sotu)
{
  Char buf[DCOLS];
  object *ring;
  fighter *rogue = sotu->roguep;

  if (sotu->r_rings == 0) {
    message(no_rings, sotu);
    return false;
  }
  if (!(ring = get_nth_object(index, &rogue->pack))) {
    message("no such item.", sotu);
    return false;
  }
  /* //the following is guaranteed by the caller to NEVER HAPPEN:
  if (ring->what_is != RING) {
    message("that's not a ring", sotu);
    return false;
  }
  */
  if (ring->is_cursed) {
    message("you can't, it appears to be cursed", sotu);
    return false;
  }

  un_put_on(ring, sotu);
  // reuse some message-sending from move.c
  switch (ring->which_kind) {
  case R_LEVITATE:
    if (!sotu->levitate)
      sotu->levitate = 1;
    break;
  case R_DELUSION:
    if (!sotu->halluc)
      sotu->halluc = 1;
    break;
  }
  StrCopy(buf, "removed ");
  get_desc(ring, buf + 8, sotu);
  message(buf, sotu);
  return true; /* (void) reg_move(); */
}

/***************************************************************
                   PUT_ON_RING
 IN:
 index = index of item in rogue's pack list.. should be a ring..
 left = put it on the left hand
 sotu = various globals (rogue, r_rings)
 OUT:
 true means the action succeeded --> caller must call reg_move
 PURPOSE:
 Put a ring on, checking first to see that it's possible.
 (Stats are updated here, not in do_put_on.)
****************************************************************/
/* If there are two slots, is_left indicates which */
Boolean put_on_ring(Short inv_index, Short finger_index, 
		    struct state_of_the_union *sotu)
{
  Char desc[DCOLS];
  object *ring;
  fighter *rogue = sotu->roguep;
  
  if (finger_index < 0 || finger_index >= 8) {
    //    StrPrintF(desc, "bad index %d", finger_index);
    message("bug?", sotu);
    return false;
  }
  if (sotu->r_rings == 8) {
    message("wearing eight rings already", sotu);
    return false;
  }
  if (!(ring = get_nth_object(inv_index, &rogue->pack))) {
    message("no such item.", sotu);
    return false;
  }
  /* //the following is guaranteed by the caller to NEVER HAPPEN:
  if (ring->what_is != RING) {
    message("that's not a ring", sotu);
    return false;
  }
  if (ring->in_use_flags & ON_EITHER_HAND) {
    message("that ring is already being worn", sotu);
    return false;
  }
  if (rogue->rings[finger_index]) {
    check_message();
    message("there's already a ring on that finger", sotu);
    return false;
  }
  */
  do_put_on(ring, finger_index, sotu);
  ring_stats(true, sotu);
  //  check_message();
  get_desc(ring, desc, sotu);
  message(desc, sotu);
  if (ring->which_kind & RING_AGGRAVATE) {
    message("you sense hostility", sotu);
    aggravate(sotu);
  }
  return true; /*   reg_move(); */
}
