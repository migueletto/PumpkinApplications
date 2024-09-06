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
 * pack.c
 *
 * This source herein may be modified and/or distributed by anybody who
 * so desires, with the following restrictions:
 *    1.)  No portion of this notice shall be removed.
 *    2.)  Credit shall not be taken for the creation of this source.
 *    3.)  This code is not to be traded, sold, or used for personal
 *         gain or profit.
 *
 */

/* #ifndef CURSES */
/* #include <curses.h> */
/* #endif CURSES */
#include "palm.h"
#include "iRogueRsc.h"
#include "Globals.h"

#include "rogue.h"

extern struct state_of_the_union * sotu;

Char curse_message[] = "you can't, it appears to be cursed";


/************************************************
  add_to_pack MOVED to LibRogue
************************************************/

/************************************************
  take_from_pack MOVED to LibRogue
************************************************/

/**********************************************************************
                       PICK_UP
 IN:
 row,col = location of item to pick up
 status = another return value: whether object was destroyed (special case.)
 various globals
 OUT:
 the object that was picked up, for use by caller...
 PURPOSE:
 If possible, pick up an object...
 **********************************************************************/
object * pick_up(Short row, Short col, Short *status) {
  fighter *rogue = sotu->roguep;
  object *obj;
  Short new_weight;

  obj = object_at(level_objects, row, col);
  *status = 1;
  
  /* Special case: Scare-Monster scroll can't be picked up. */
  if ((obj->what_is == SCROLL) && (obj->which_kind == SCARE_MONSTER) &&
      obj->picked_up) {
    message("the scroll turns to dust as you pick it up", sotu);
    dungeon[row][col] &= (~OBJECT);
    vanish(obj, level_objects, sotu);
    //    reg_move(); // NO. BAD. WRONG.
    *status = 0;
    if (id_scrolls[SCARE_MONSTER].id_status == UNIDENTIFIED) {
      id_scrolls[SCARE_MONSTER].id_status = IDENTIFIED;
    }
    return(0);
  }

  /* Special case: Gold is not a separate object per se. */
  if (obj->what_is == GOLD) {
    rogue->gold += obj->quantity;
    dungeon[row][col] &= ~(OBJECT);
    take_from_pack(obj, level_objects);
    print_stats(STAT_GOLD, sotu);
    return(obj);	/* obj will be free_object()ed in one_move_rogue() */
  }

  /* Another exceptional case: pack won't hold any more objects */
  if (pack_count(obj, rogue, &new_weight) >= MAX_PACK_COUNT) {
    message("pack too full", sotu);
    return(0);
  }
  if (new_weight > max_weight(sotu)) {
    message("pack too heavy", sotu);
    return(0);
  }

  /* Remove the object from the list of stuff-sitting-around and
     add it to the list of stuff-in-pack */
  dungeon[row][col] &= ~(OBJECT);
  take_from_pack(obj, level_objects);
  obj = add_to_pack(obj, &rogue->pack, 1, sotu);
  obj->picked_up = 1;
  return(obj);
}

/**********************************************************************
                       CHECK_DROP_ITEM
 IN:
 various globals
 OUT:
 Whether the rogue can currently drop an object.
 PURPOSE:
 (duh.)
 **********************************************************************/
Boolean check_drop_item() {
  fighter *rogue = sotu->roguep;
  if (dungeon[rogue->row][rogue->col] & (OBJECT | STAIRS | TRAP)) {
    message("there's already something there", sotu);
    return false;
  }
  if (!rogue->pack.next_object) {
    message("you have nothing to drop", sotu);
    return false;
  }
  return true;
}
/**********************************************************************
                       DROP
 IN:
 index = of the item in the pack-list, to be dropped.
 various globals
 OUT:
 nothing
 PURPOSE:
 If possible, drop the item.  Display appropriate message.
 **********************************************************************/
void drop(Short index) {
  fighter *rogue = sotu->roguep;
  /* 'index' is zero-based and is the numerical index in the pack list. */
  object *obj, *new;
  Char desc[DCOLS];

  if (index < 0)
    return;

  /* check_drop should display message for these two */
  if (dungeon[rogue->row][rogue->col] & (OBJECT | STAIRS | TRAP))
    return;
  if (!rogue->pack.next_object)
    return;

  /* the inventory select form should make this impossible */
  if (!(obj = get_nth_object(index, &rogue->pack))) {
    message("no such item.", sotu);
    return;
  }

  /* now see if it's something wielded/worn/on */
  if (obj->in_use_flags & BEING_WIELDED) {
    if (obj->is_cursed) {
      message(curse_message, sotu);
      return;
    }
    unwield(rogue->weapon, rogue);
  } else if (obj->in_use_flags & BEING_WORN) {
    if (obj->is_cursed) {
      message(curse_message, sotu);
      return;
    }
    mv_aquatars();
    unwear(rogue->armor, rogue);
    print_stats(STAT_ARMOR, sotu);
  } else if (obj->in_use_flags & ON_EITHER_HAND) {
    if (obj->is_cursed) {
      message(curse_message, sotu);
      return;
    }
    un_put_on(obj, sotu); // lib_ring takes care of printing stats
  }

  /* drop that puppy */
  obj->row = rogue->row;
  obj->col = rogue->col;

  /* if it's multiple, and not a weapon, drop "one" of it */
  if ((obj->quantity > 1) && (obj->what_is != WEAPON)) {
    obj->quantity--;
    new = alloc_object(sotu);
    *new = *obj;
    new->quantity = 1;
    obj = new;
  } else {
    obj->ichar = 'Z';  /* eh? */
    take_from_pack(obj, &rogue->pack);
  }
  place_at(obj, rogue->row, rogue->col, sotu);
  StrCopy(desc, "dropped ");
  get_desc(obj, desc+8, sotu);
  message(desc, sotu);
  reg_move();
}

/************************************************
  check_duplicate MOVED to LibRogue
  next_avail_ichar MOVED to LibRogue
  ************************************************/

/**********************************************************************
                       TAKE_OFF
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 If it's possible, take off armor.
 **********************************************************************/
void take_off() {
  fighter *rogue = sotu->roguep;
  Char desc[DCOLS];
  object *obj;

  if (rogue->armor) {
    if (rogue->armor->is_cursed) {
      message(curse_message, sotu);
    } else {
      mv_aquatars(); /* interesting */
      obj = rogue->armor;
      unwear(rogue->armor, rogue);
      StrCopy(desc, "was wearing ");
      get_desc(obj, desc+12, sotu);
      message(desc, sotu);
      print_stats(STAT_ARMOR, sotu);
      reg_move();
    }
  } else {
    message("not wearing any", sotu);
  }
}

/**********************************************************************
                       WEAR
 IN:
 index = of the item to be worn, in the pack-list
 various globals
 OUT:
 nothing
 PURPOSE:
 If it's possible, wear the (armor) item.  (Not possible if you're
 already wearing some - you have to take it off first.)
 **********************************************************************/
void wear(Short index) {
  fighter *rogue = sotu->roguep;
  object *obj;
  Char desc[DCOLS];

  /* Rogue.c should check first whether you're already wearing some */
  if (rogue->armor)
    return;

  /* the inventory select form should make this impossible */
  if (!(obj = get_nth_object(index, &rogue->pack))) {
    message("no such item.", sotu);
    return;
  }

  if (obj->what_is != ARMOR) {
    message("you can't wear that", sotu);
    return;
  }

  obj->identified = 1;
  StrCopy(desc, "wearing ");
  get_desc(obj, desc + 8, sotu);
  message(desc, sotu);
  do_wear(obj, rogue);
  print_stats(STAT_ARMOR, sotu);
  reg_move();
}

/************************************************
  unwear MOVED to LibRogue
  do_wear ditto
************************************************/


/**********************************************************************
                       WIELD
 IN:
 index = of the item to be wielded, in the pack-list
 various globals
 OUT:
 nothing
 PURPOSE:
 If it's possible, wield the item.  (Possible if you're already wielding
 something else - it will be un-wielded.
 **********************************************************************/
void wield(Short index) {
  fighter *rogue = sotu->roguep;
  object *obj;
  Char desc[DCOLS];

  /* ? */
  if (rogue->weapon && rogue->weapon->is_cursed) {
/*     message(curse_message, sotu); */
    return;
  }

  if (!(obj = get_nth_object(index, &rogue->pack))) {
    /* should be impossible */
    message("no such item.", sotu);
    return;
  }

  /* You can wield most things; even, say, slime-molds */
  if (obj->what_is & (ARMOR | RING)) {
    StrPrintF(desc, "you can't wield %s",
	      ((obj->what_is == ARMOR) ? "armor" : "rings"));
    message(desc, sotu);
    return;
  }

  if (obj->in_use_flags & BEING_WIELDED) {
    message("in use", sotu);
  } else {
    unwield(rogue->weapon, rogue);
    StrCopy(desc, "wielding ");
    get_desc(obj, desc + 9, sotu);
    message(desc, sotu);
    do_wield(obj, rogue);
    reg_move();
  }
}

/************************************************
  do_wield ditto
  unwield ditto
************************************************/










/* XXX I am going to DISABLE this for now
   so I don't have to revise get_input_line yet. */
/*
call_it()
{
	short ch;
	register object *obj;
	struct id *id_table;
	char buf[MAX_TITLE_LENGTH+2];

	ch = pack_letter("call what?", (SCROLL | POTION | WAND | RING));

	if (ch == CANCEL) {
		return;
	}
	if (!(obj = get_letter_object(ch))) {
		message("no such item.", 0);
		return;
	}
	if (!(obj->what_is & (SCROLL | POTION | WAND | RING))) {
		message("surely you already know what that's called", 0);
		return;
	}
	id_table = get_id_table(obj, sotu);

	if (get_input_line("call it:","",buf,id_table[obj->which_kind].title,1,1)) {
		id_table[obj->which_kind].id_status = CALLED;
		(void) strcpy(id_table[obj->which_kind].title, buf);
	}
}
*/
/*
void call_it() {
  Short ch;
  object *obj;
  struct id *id_table;
  Char buf[MAX_TITLE_LENGTH+2];

//       Make a dialog with:
//       1. scrollable select
//       2. input field.
//       3. OK/cancel.
//       maybe I can rework the inventory-frobbing dialog Yet Again.
//       on OK, see if something is selected.
//       if it's not {scrolls, potions, wands, rings}, get sassy.
//       otherwise copy into id_table...... hm.......

  ch = pack_letter("call what?", (SCROLL | POTION | WAND | RING));

  if (ch == CANCEL) {
    return;
  }
  if (!(obj = get_letter_object(ch))) {
    message("no such item.", 0);
    return;
  }
  if (!(obj->what_is & (SCROLL | POTION | WAND | RING))) {
    message("surely you already know what that's called", 0);
    return;
  }
  id_table = get_id_table(obj, sotu); // sotu->id_<foo>

  if (get_input_line("call it:","",buf,id_table[obj->which_kind].title,1,1)) {
    id_table[obj->which_kind].id_status = CALLED;
    (void) strcpy(id_table[obj->which_kind].title, buf);
  }

  // so when is title (re-)set?
  //   init_id_potions, init_id_scrolls (make_scroll_titles) need to be done.
  //   get_wand_and_ring_materials is done - ok.

}
*/

/************************************************
  pack_count MOVED to LibRogue
************************************************/

/************************************************
  mask_pack MOVED to LibRogue
************************************************/

/* is_pack_letter */
/* No longer used - since I'm getting rid of pack_letter */

/**************************************************************************
  has_amulet MOVED to LibRogue.... on second thought, no.
*************************************************************************/
// used only in level.c... made it into a macro there.
/*
Boolean has_amulet() {
  return(mask_pack(&rogue->pack, AMULET));
}
*/

void kick_into_pack() {
  fighter *rogue = sotu->roguep;
  object *obj;
  Char desc[DCOLS];
  Short n, status;

  if (sotu->levitate || (sotu->ring_flags & RING_LEVITATE)) {
    message("you're floating in the air!", sotu);
    return;
  }
  if (!(dungeon[rogue->row][rogue->col] & OBJECT)) {
    message("nothing here", sotu);
    return;
  }
  if ((obj = pick_up(rogue->row, rogue->col, &status))) {
    get_desc(obj, desc, sotu);
    if (obj->what_is == GOLD) {
      free_object(obj, sotu);
    } else {
      n = StrLen(desc);
      desc[n] = '(';
      desc[n+1] = obj->ichar;
      desc[n+2] = ')';
      desc[n+3] = 0;
    }
    message(desc, sotu);
  }
  if (obj || (!status))
    reg_move();
}
/*
void kick_into_pack() {
  object *obj;
  Char desc[DCOLS];
  Short n, stat;

  if (!(dungeon[rogue->row][rogue->col] & OBJECT)) {
    message("nothing here", 0);
  } else {
    if (obj = pick_up(rogue->row, rogue->col, &stat)) {
      get_desc(obj, desc);
      if (obj->what_is == GOLD) {
	message(desc, 0);
	free_object(obj, sotu);
      } else {
	n = strlen(desc);
	desc[n] = '(';
	desc[n+1] = obj->ichar;
	desc[n+2] = ')';
	desc[n+3] = 0;
	message(desc, 0);
      }
    }
    if (obj || (!stat)) {
      reg_move();
    }
  }
}
*/

/***************************************************************************
 *     Moved all of lib_pack.c into here:
 ***************************************************************************/


/***************************************************************
                   CHECK_DUPLICATE
 IN:
 obj = object to check for a collection of
 pack = the list to check in
 OUT:
 "collection" object if found; 0 if not
 PURPOSE:
 For objects that can be collected into a group.  If the given object
 can be part of a collection, search the pack list for another object
 of the same kind, and add to it the quantity of the given object
 (combining the two into a single collection.)
 Returns the collection object found in the pack list, if any
 (freeing the given object is the caller's problem.)
****************************************************************/
static object * check_duplicate(object *obj, object *pack) SEC_L;
static object * check_duplicate(object *obj, object *pack) {
  object *op;

  if (!(obj->what_is & (WEAPON | FOOD | SCROLL | POTION))) {
    return(0);
  }
  if ((obj->what_is == FOOD) && (obj->which_kind != RATION)) {
    return(0);
  }
  op = pack->next_object;

  while (op) {
    if ((op->what_is == obj->what_is) && 
	(op->which_kind == obj->which_kind)) {

      if ((obj->what_is != WEAPON)
	  || ((obj->what_is == WEAPON)
	      && (obj->o_flags & O_COLLECTION)
	      && (obj->quiver == op->quiver)) ) {
	op->quantity += obj->quantity;
	return(op);
      }
    }
    op = op->next_object;
  }
  return(0);
}


/***************************************************************
                   TAKE_FROM_PACK
 IN:
 obj = object to remove
 pack = list to remove it from
 OUT:
 nothing
 PURPOSE:
 Remove the given object from the given list.
****************************************************************/
void take_from_pack(object *obj, object *pack) {
  while (pack->next_object != obj) {
    pack = pack->next_object;
    if (!pack) return; /* it wasn't IN the pack after all */
  }
  pack->next_object = pack->next_object->next_object;
}

/***************************************************************
                   MASK_PACK
 IN:
 pack = list to look in
 mask = the type of object desired
 OUT:
 true if an object of 'mask' type is found in 'pack'
 PURPOSE:
 Are there any objects of this type in the pack?
****************************************************************/
Boolean mask_pack(object *pack, UShort mask) {
  while (pack->next_object) {
    pack = pack->next_object;
    if (pack->what_is & mask) {
      return true;
    }
  }
  return false;
}

/***************************************************************
                   NEXT_AVAIL_ICHAR
 IN:
 sotu = various globals (rogue)
 OUT:
 the alphabetically-first unused inventory character
 PURPOSE:
 Get an unused inventory character to assign to a picked-up item.
****************************************************************/
/* Get next available inventory character.. */
/*
static Char next_avail_ichar(struct state_of_the_union * sotu) {
  object *obj;
  Short i;
  Boolean ichars[26];

  for (i = 0; i < 26; i++) {
    ichars[i] = 0;
  }
  obj = sotu->roguep->pack.next_object;
  while (obj) {
    ichars[(obj->ichar - 'a')] = 1;
    obj = obj->next_object;
  }
  for (i = 0; i < 26; i++) {
    if (!ichars[i]) {
      return(i + 'a');
    }
  }
  return('?');
}
*/
static Char next_avail_ichar(struct state_of_the_union * sotu) SEC_L;
static Char next_avail_ichar(struct state_of_the_union * sotu) {
  object *obj;
  //  Short i;
  Char cur_ichar = 'a';

  obj = sotu->roguep->pack.next_object;
  while (obj) {
    obj->ichar = cur_ichar++;
    obj = obj->next_object;
    if (cur_ichar > 'z')
      cur_ichar = 'A'; // I should check that it never reaches '['...
  }
  return (cur_ichar <= 'z') ? cur_ichar : '?';

}

/***************************************************************
                   ADD_TO_PACK
 IN:
 obj = the object that's being added to a list
 pack = the list to put it in
 condense = whether to condense it into a "collection" (e.g. 3 rations)
            [this is true iff we're adding an object to rogue's pack..]
 sotu = various globals...
 OUT:
 the object (original, or a different "collection" object.)
 PURPOSE:
 Add an object or monster to a list (rogue->pack, level_objects,
 level_monsters.)  Optionally condenses some types of objects
 into collections.
****************************************************************/
extern RoguePreferenceType my_prefs;
const UShort sort_kinds[8] = {FOOD, WEAPON, ARMOR, SCROLL,
			      POTION, WAND, RING, AMULET};
object * add_to_pack(object *obj, object *pack, Boolean condense,
		     struct state_of_the_union * sotu) 
{
  object *op;

  if (condense) {
    if ((op = check_duplicate(obj, pack))) {
      free_object(obj, sotu);
      return(op);
    } else {
      obj->ichar = next_avail_ichar(sotu); // inventory form updates it anyway.
    }
  }
  obj->next_object = 0;
  if (pack->next_object == 0) {
    pack->next_object = obj;
  } else {
    op = pack->next_object;
    
    if (condense && my_prefs.sort_pack) {
      Short i = 0, j = 0;
      Char ichar = 'a'; // might as well set this correctly, while we're at it.
      op = pack;
      while ((sort_kinds[j] != obj->what_is) && (j < (8-1)))
	j++;
      while (op->next_object) {
	while ((sort_kinds[i] != op->next_object->what_is) && (i < 8))
	  i++;
	if (i > j)
	  break;
	op = op->next_object;
	ichar = ((ichar=='z') ? 'A' : ichar+1);
      }
      obj->ichar = ichar;
      obj->next_object = op->next_object; // could be NULL
      op->next_object = obj;
    } else { // pack is not rogue's pack, or we're not sorting rogue's pack
      // Append to end of list.
      while (op->next_object) {
	op = op->next_object;
      }
      op->next_object = obj;
      obj->next_object = NULL;
    }
  }

  return(obj);
}

/***************************************************************
                   UNWEAR
 IN:
 obj = the armor being worn, or 0.
 rogue
 OUT:
 nothing
 PURPOSE:
 Take off the armor.
****************************************************************/
void unwear(object *obj, fighter *rogue) {
  if (obj) {
    obj->in_use_flags &= (~BEING_WORN);
  }
  rogue->armor = (object *) 0;
}

/***************************************************************
                   DO_WEAR
 IN:
 obj = the armor to be worn
 rogue
 OUT:
 nothing
 PURPOSE:
 Put on the armor.  This also identifies it.
****************************************************************/
void do_wear(object *obj, fighter *rogue) {
  rogue->armor = obj;
  obj->in_use_flags |= BEING_WORN;
  obj->identified = 1;
}

/***************************************************************
                   DO_WIELD
 IN:
 obj = the object (not an armor or ring) to be wielded
 OUT:
 nothing
 PURPOSE:
 Wield the object.
****************************************************************/
void do_wield(object *obj, fighter *rogue) {
  rogue->weapon = obj;
  obj->in_use_flags |= BEING_WIELDED;
}

/***************************************************************
                   UNWIELD
 IN:
 obj = The object being wielded, or 0.
 OUT:
 PURPOSE:
 Un-wield the object.
****************************************************************/
void unwield(object *obj, fighter *rogue) 
{
  if (obj) {
    obj->in_use_flags &= (~BEING_WIELDED);
  }
  rogue->weapon = (object *) 0;
}


/***************************************************************
                   PACK_COUNT
 IN:
 new_obj = 0 or an object that might be put in the pack
 rogue
 OUT:
 The number of items in the pack (- 1 for the collection the new_obj
 would be put in, if any.)
 PURPOSE:
 Figure out whether there is space in the rogue's pack for new_obj.
 (If there's no new_obj, just count the items in the pack.)
 For collections of non-weapon items, the quantity is counted.  
 For collections of weapon items, if there's no new_obj the collection
 always counts as one object; otherwise, if the new_obj would be put
 in that collection, we don't count that collection at all (so that
 the return value indicates there's space for it.)
// Also - return what the pack will weigh if this item is added!
****************************************************************/
Short pack_count(object *new_obj, fighter *rogue, Short *weight) {
  object *obj;
  Short count = 0;

  *weight = (new_obj) ? new_obj->weight : 0;
  obj = rogue->pack.next_object;

  while (obj) {
    *weight = *weight + (obj->quantity * obj->weight); // (all qtys are initially 1 so this is ok)
    if (obj->what_is != WEAPON) {
      count += obj->quantity; // e.g. several scrolls, potions, rations
    } else if (!new_obj) {
      count++;
    } else if ((new_obj->what_is != WEAPON)
	       || (!(obj->o_flags & O_COLLECTION))
	       || (new_obj->which_kind != obj->which_kind)
	       || (obj->quiver != new_obj->quiver)) {
      count++;
    }
    obj = obj->next_object;
  }
  return(count);
}
/***************************************************************
                   MAX_WEIGHT
 IN:
 various globals
 OUT:
 the maximum weight this rogue can carry
 PURPOSE:
 Figure out whether the pack is too heavy.
****************************************************************/
Short max_weight(struct state_of_the_union * sotu) {
  Short w;
  Char h;
  w = BASE_ROGUE_CARRYABLE;
  w += (sotu->roguep->str_current - 8) * 50;
  h = sotu->hunger_str[0];
  if (h == 'w')
    w -= w/10; // hunger = weak
  else if (h == 'f')
    w /= 2; // hunger = faint
  return w;
}


/***************************************************************
 *                      sort_pack
 * In: (unsorted pack)
 * Out: (sorted pack)
 ***************************************************************/
/*
  While there are Unclassified objects,
  
  If the first unclassified object o == Class,
  Stick o at the END of Classified list.
  Set the unclassified list to point to o's Next
  Set o's next to Null.
  Else
  Let o = o's Next.
  When you run out, Classified is a list, Unclassified is a list.
  Let o = first Unclassified and repeat with a new value for Class.
*/
void sort_pack(object *pack)
{
  // the "pack" struct itself has nothing except the pointer next_object.
  Short i;
  object classified, unclassified; // these hold ptrs to sorted + unsorted foo
  object *o, *prev, *classified_tail; /* the object being considered;
					 the one that points to it; list tail*/
  // Initially, the unclassified list is the list of all objects in pack..
  unclassified.next_object = pack->next_object;
  // And the classified list is empty.
  classified_tail = &classified;
  classified_tail->next_object = NULL;
  // We will pass through unclassified objects once per kind of object,
  // appending objects of that kind to 'classified' list.
  for (i = 0 ; i < 8 ; i++) {
    // start (over) at the first still-unclassified object
    prev = &unclassified;
    o = prev->next_object;
    while (o) {
      if (o->what_is == sort_kinds[i]) {
	prev->next_object = o->next_object; // cut o out of unclassified list
	classified_tail->next_object = o; // append o to the classified list..
	classified_tail = o;
	classified_tail->next_object = NULL; // ..done
      } else {
	prev = o; // move forward..
      }
      o = prev->next_object; // (formerly known as o->next_object.)
    }
  }
  // add anything that's still unclassified to the end
  classified_tail->next_object = unclassified.next_object; // maybe NULL
  // set the pack to point to classified+unclassified list
  pack->next_object = classified.next_object;

}
