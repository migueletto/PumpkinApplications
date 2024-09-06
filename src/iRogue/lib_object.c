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
//#include "../iRogueRsc.h"
#include "iRogueRsc.h"

struct weap_data {
  // name?
  ULong w_flags; // (to be |= with o_flags)
  Char damage[4]; // hit damage   [0] d [1],   hurl damage  [2] d [3]
  Short launcher;
  Short weight;
  // IS_COLLECTION, IS_MISSILE, IS_METAL
};
typedef struct weap_data weapon_data;

// [0,1] is dice to roll for straight damage (e.g. '1' d '4')
// If an IS_MISSILE weapon is properly launched/thrown:
// [2,3] is a fractional multiplier, applied after all other bonuses.
// When it is applied, a 4/3 multiplier should be applied to hit_chance also!
// Note - [0] is also used as a modifier to-hit !!
static weapon_data weapon_tab[WEAPONS] = {
  { (0),                                  {1,1,1,1}, NO_LAUNCHER, 40}, // bow
  { (O_COLLECTION | O_MISSILE),           {1,1,1,3}, NO_LAUNCHER,  5},  // dart
  { (O_COLLECTION | O_MISSILE),           {1,1,1,6}, BOW,          5},  // arrow
  { (O_COLLECTION | O_MISSILE | O_METAL), {1,4,1,6}, NO_LAUNCHER, 10}, // dagger
  { (O_COLLECTION | O_MISSILE),		  {1,2,2,5}, NO_LAUNCHER,  4}, // shuriken
  { (O_METAL),				  {2,4,1,3}, NO_LAUNCHER,100},  // mace
  { (O_METAL),				  {3,4,1,2}, NO_LAUNCHER, 60},  // long sword
  { (O_METAL),				  {3,6,1,2}, NO_LAUNCHER,250},  // two handed sword
  // these taken entirely from urogue:
  { (O_COLLECTION | O_MISSILE),		  {1,2,1,4}, SLING,        5}, // rock
  { (0),				  {0,1,0,1}, NO_LAUNCHER,  5}, // sling
  { (0),				  {1,1,1,1}, NO_LAUNCHER,100}, // crossbow
  { (O_COLLECTION | O_MISSILE),		 {1,2,1,12}, CROSSBOW,     7}, // crossbow bolt
  { (O_METAL | O_MISSILE),		  {1,6,1,8}, NO_LAUNCHER, 50}, // spear
  { (O_METAL),				  {3,4,1,4}, NO_LAUNCHER, 50}, // trident
  { (O_METAL),				  {2,6,1,3}, NO_LAUNCHER, 50}, // spetum
  { (O_METAL),				  {3,4,1,2}, NO_LAUNCHER,125}, // bardiche
  { (O_METAL),				 {1,12,1,8}, NO_LAUNCHER, 80}, // short pike
  { (O_METAL),				  {2,8,1,2}, NO_LAUNCHER,100}, // bastard sword
  { (O_METAL),				  {2,6,1,3}, NO_LAUNCHER,175}, // halberd
  { (O_METAL),				  {1,8,1,3}, NO_LAUNCHER, 80}, // battle axe
  // leaving out the silver arrow for now
  { (O_METAL),			          {1,6,1,2}, NO_LAUNCHER, 50}, // hand axe
  { (0),				  {1,4,1,2}, NO_LAUNCHER, 30}, // club
  { (O_METAL),				  {1,7,1,4}, NO_LAUNCHER,150}, // flail
  { (O_METAL),				 {1,10,1,3}, NO_LAUNCHER, 80}, // glaive
  { (O_METAL),				  {2,4,1,3}, NO_LAUNCHER, 80}, // guisarme
  { (O_METAL | O_MISSILE),		  {1,3,1,5}, NO_LAUNCHER, 50}, // hammer
  { (O_MISSILE),			  {1,4,1,6}, NO_LAUNCHER, 10}, // javelin
  { (O_METAL),				  {2,4,1,3}, NO_LAUNCHER,125}, // morning star
  { (O_METAL),				  {1,6,1,2}, NO_LAUNCHER, 80}, // partisan
  { (O_METAL),				  {2,4,1,2}, NO_LAUNCHER, 60}, // pick
  { (O_METAL),				  {2,8,2,6}, NO_LAUNCHER, 80}, // long pike
  { (O_METAL),				  {1,8,1,2}, NO_LAUNCHER, 40}, // scimitar
  { (O_COLLECTION|O_METAL|O_MISSILE),	  {1,1,1,8}, NO_LAUNCHER,  3}, // sling bullet
  { (0),				  {1,6,1,2}, NO_LAUNCHER, 50}, // quarterstaff
  { (O_METAL),				  {2,4,1,3}, NO_LAUNCHER, 75}, // broadsword
  { (O_METAL),				  {1,6,1,2}, NO_LAUNCHER, 35}, // short sword
  // leaving out the boomerang for now
  // leaving out the burning oil for now
  { (O_METAL),				  {3,7,1,2}, NO_LAUNCHER,300}, // claymore
  // leaving out the crysknife for now
  { (0),                                  {1,1,1,1}, NO_LAUNCHER, 90}, // footbow
  { (O_COLLECTION | O_MISSILE),		 {1,2,1,10}, NO_LAUNCHER,  5}, // footbow bolt
  { (O_METAL),				 {4,12,2,6}, NO_LAUNCHER,350}, // katana
  { (O_METAL),				  {1,6,1,5}, NO_LAUNCHER, 40}, // leuku
  { (0),				  {1,6,1,6}, NO_LAUNCHER, 45}, // tomahawk
  { (O_METAL),				  {2,5,1,3}, NO_LAUNCHER,160}, // pertuska
  { (O_METAL),				  {2,6,1,3}, NO_LAUNCHER,120}, // sabre
  { (O_METAL),				 {1,10,1,2}, NO_LAUNCHER, 55} // cutlass
};
// 45 weapons - not including silver-arrow, boomerang, burning-oil, crysknife.
// Is this too many??

// Here are probabilities used by the various gr_foo.
//static Short whatis_prob[] = {
// order should be: armor, weapon, scroll, potion, *SKIP*, food, wand, ring
//}
static Short scroll_prob[SCROLLS] = {
  // out of 0-85 .. make that 87
  5, 11, 16, 21, 36, 44, 51, 56, 65, 74, 80, 85, 87
};
// All scrolls weigh SCROLL_WEIGHT
#define SCROLL_WEIGHT 4
static Short potion_prob[POTIONS] = {
  // out of 1-118 
  10, 20, 30, 40, 50, 55, 65, 75, 85, 95, 105, 110, 114, 118
};
// All scrolls weigh POTION_WEIGHT
#define POTION_WEIGHT 10
static Short armor_prob[ARMORS] = {
  // out of 1-100
  10, 20, 31, 43, 55, 67, 77, 87, 93, 97, 99, 100
};
static Short armor_class[ARMORS] = {
  2,  3,  3,  4,  4,  5,  6,  6,  7,  8, 9, 10
};
// All wands are equally likely.
// All rings are equally likely.
// All weapons are equally likely, but some are more equal than others.
// Wands/staffs weigh ...
#define WAND_WEIGHT 20
#define STAFF_WEIGHT 50
#define FOOD_WEIGHT 400
#define FRUIT_WEIGHT 300
#define RING_WEIGHT 3
// Rings weigh ... except for the heavy one

static Short armor_weight[ARMORS] = {
  100, 250, 200, 250, 150, 300, 350, 350, 400, 450, 100, 300
};
// in urogue, armor weighs 20% less per + (20% more per -)
// and weapons weigh 50% less if sum of to-hit + and dam + is > 0
// and other stuff weighs 20% less if 'blessed'

// carrying ability modifier = (str - 8) * 50
// add to normal encumberance.
// take 20% off if 'weak', 50% off if 'faint'.
// (so.. what if you're carrying too much to move?)

// from urogue.  I think that [][] should put this in 'data' not 'code'.
// (irrelevant now but also takes a bit less space declared that way)
// also used in lib_use.c
static Char fruit_names[21][14] = {
  "slime-mold ",  "candleberry ",  "caprifig ",     "dewberry ", 
  "elderberry ",  "gooseberry ",   "guanabana ",    "hagberry ",
  "ilama ",       "imbu ",         "jaboticaba ",   "jujube ",
  "litchi ",      "mombin ",       "pitanga ",      "prickly-pear ",
  "rambutan ",    "sapodilla ",    "soursop ",      "sweetsop ",
  "whortleberry "
};
#define POPSICLE 4
static Char carcase_names[5][14] = {
  "mystery meat ",
  "orc chop ",
  "snake steak ",
  "eye surprise ",
  "popsicle "
};

/***************************************************************
                   OBJECT_AT
 IN:
 pack = list of things to check in (level_obj. or level_mons.)
 row,col = location to check
 OUT:
 thing found, if any
 PURPOSE:
 Find the object (or monster?) at the given location, if any.
****************************************************************/
object * object_at(object *pack, Short row, Short col) {
  object *obj;

  obj = pack->next_object;

  while (obj && ((obj->row != row) || (obj->col != col))) {
    obj = obj->next_object;
  }
  return(obj);
}

/***************************************************************
                   IS_VOWEL
 IN:
 character
 OUT:
 true if character is a vowel
 PURPOSE:
 Use to determine whether to prefix a word with "a" or "an"
****************************************************************/
Boolean is_vowel(Short ch) {
  return( (ch == 'a') ||
	  (ch == 'e') ||
	  (ch == 'i') ||
	  (ch == 'o') ||
	  (ch == 'u') );
}

/***************************************************************
                   GET_ID_TABLE
 IN:
 obj = an object of some type
 sotu = various globals (id_foo)
 OUT:
 the id table for that type of object
 PURPOSE:
 Given an object, return the id table corresponding to its type
 (scroll, potion, etc.)
****************************************************************/
/* this was in inventory.c */
struct id * get_id_table(object *obj, struct state_of_the_union * sotu) {
  switch(obj->what_is) {
  case SCROLL:
    return(sotu->id_scrolls);
  case POTION:
    return(sotu->id_potions);
  case WAND:
    return(sotu->id_wands);
  case RING:
    return(sotu->id_rings);
  case WEAPON:
    return(sotu->id_weapons);
  case ARMOR:
    return(sotu->id_armors);
  }
  return((struct id *) 0);
}

/***************************************************************
                   GET_ARMOR_CLASS
 IN:
 obj = armor, I hope.
 OUT:
 number representing an armor class (higher is better)
 PURPOSE:
 Use to figure out how hard something will be to hit...
****************************************************************/
//Short get_armor_class(object *obj) {
Short get_armor_class(struct state_of_the_union * sotu) {
  Short ac = sotu->ring_ac;
  object *obj = sotu->roguep->armor;
  if (obj)
    ac += obj->class + obj->d_enchant;
  return ac;
}

/***************************************************************
                   FREE_OBJECT
 IN:
 obj = object to free
 sotu = various globals... (unused)
 OUT:
 nothing
 PURPOSE:
 please don't crash
****************************************************************/
/* I hope the free_list is reasonably bounded... */
void free_object(object *obj, struct state_of_the_union * sotu) {
  VoidHand h = MemPtrRecoverHandle(obj);
  if (h) 
    MemHandleFree(h);
  else
    FrmGotoForm(WitsEndForm);
}


/***************************************************************
                   ALLOC_OBJECT
 IN:
 sotu = various globals... (unused)
 OUT:
 a newly allocated and initialized object/monster
 PURPOSE:
 Allocate space for a new object or monster.
 Maybe this should check whether md_malloc failed, or is
 md_malloc doing that, I forget.
****************************************************************/
object * alloc_object(struct state_of_the_union * sotu) {
  object *obj;

  obj = (object *) md_malloc(sizeof(object)); // everything's initially 'zero'

  obj->quantity = 1;
  obj->ichar = 'Z';
  obj->picked_up = obj->is_cursed = 0;
  obj->in_use_flags = NOT_USED;
  obj->identified = UNIDENTIFIED;
  obj->damage[0] = 1; //  obj->damage = "1d1";
  obj->damage[1] = 1;
  obj->damage[2] = 1; // thrown damage = 1d1
  obj->damage[3] = 1;
  return(obj);
}


/***************************************************************
                   PLANT_GOLD
 IN:
 row,col = location to place the gold
 is_maze = whether it's a room or maze; rooms get more gold
 sotu = various globals (dungeon, level_objects, cur_level)
 OUT:
 nothing
 PURPOSE:
 Create and place a random quantity of gold at row,col.
****************************************************************/
static void plant_gold(Short row, Short col, Boolean is_maze,
		struct state_of_the_union *sotu) SEC_L;
static void plant_gold(Short row, Short col, Boolean is_maze,
		struct state_of_the_union *sotu) 
{
  object *obj;

  obj = alloc_object(sotu);
  obj->row = row; 
  obj->col = col;
  obj->what_is = GOLD;
  obj->quantity = get_rand((2 * sotu->cur_level), (16 * sotu->cur_level));
  if (is_maze) {
    obj->quantity += obj->quantity / 2;
  }
  sotu->dungeon[row][col] |= OBJECT;
  (void) add_to_pack(obj, sotu->level_objects, 0, sotu);
}

/***************************************************************
                   PUT_GOLD
 IN:
 sotu = various globals (rooms, dungeon)
 OUT:
 nothing
 PURPOSE:
 Called when a new level is created; randomly places gold in its
 rooms and mazes.
****************************************************************/
void put_gold(struct state_of_the_union *sotu) {
  Short i, j;
  Short row,col;
  Boolean is_maze, is_room;
  room * rooms = sotu->rooms;
  UShort ** dungeon = sotu->dungeon;

  for (i = 0; i < MAXROOMS; i++) {
    is_maze = (rooms[i].is_room & R_MAZE) ? 1 : 0;
    is_room = (rooms[i].is_room & R_ROOM) ? 1 : 0;

    if (!(is_room || is_maze)) {
      continue;
    }
    /* All mazes and some rooms get gold... */
    if (is_maze || rand_percent(GOLD_PERCENT)) {
      for (j = 0; j < 50; j++) {
	row = get_rand(rooms[i].top_row+1,
		       rooms[i].bottom_row-1);
	col = get_rand(rooms[i].left_col+1,
		       rooms[i].right_col-1);
	if ((dungeon[row][col] == FLOOR) ||
	    (dungeon[row][col] == TUNNEL)) {
	  plant_gold(row, col, is_maze, sotu);
	  break;
	}
      }
    }
  }
}


/***************************************************************
                   PLACE_AT
 IN:
 obj = object to place
 row,col = location to put it at
 sotu = various globals (dungeon, level_objects)
 OUT:
 nothing
 PURPOSE:
 Put the object at the given location and add it to level_objects.
****************************************************************/
void place_at(object * obj, Short row, Short col,
	      struct state_of_the_union *sotu) 
{
  obj->row = row;
  obj->col = col;
  sotu->dungeon[row][col] |= OBJECT;
  (void) add_to_pack(obj, sotu->level_objects, 0, sotu);
}

/***************************************************************
                   FREE_STUFF
 IN:
 objlist = list of objects to be freed
 sotu = various globals (not used)
 OUT:
 nothing
 PURPOSE:
 Frees the objects pointed to by objlist (but not objlist itself)
****************************************************************/
void free_stuff(object *objlist, struct state_of_the_union *sotu) {
  object *obj;

  while (objlist->next_object) {
    obj = objlist->next_object;
    objlist->next_object =
      objlist->next_object->next_object;
    free_object(obj, sotu);
  }
}

/***************************************************************
                   FREE_OLD_LEVEL
 IN:
 sotu = various globals (level_objects, level_monsters)
 OUT:
 nothing
 PURPOSE:
 Free space that belongs to the old level: objects that the
 rogue isn't carrying; monsters that weren't killed.
 Called when the rogue goes up/down a level...
****************************************************************/
/* FREE EVERYTHING ALLOCED BY make_new_level (main.c) !!! */
/* FREE EVERYTHING ALLOCED BY anything after that !!! */
void free_old_level(struct state_of_the_union *sotu) {
  /* clear_level - nothing */
  /* make_level - calls some stuff.. */
  /* put_objects - make_party. gr_object. allocates mons and objs */
  /* put_stairs - nothing */
  /* add_traps - nothing */
  /* put_mons - gr_monster allocates a monster. */
  /* put_player - nothing, I think */
  /* print_stats - nothing, I think */

  /* party_monster. wanderer. create_monster. put_mons. */

  free_stuff(sotu->level_objects, sotu);
  free_stuff(sotu->level_monsters, sotu);
}

/***************************************************************
                   PUT_STAIRS
 IN:
 sotu = various globals (dungeon)
 OUT:
 nothing
 PURPOSE:
 When a new level is created, one stairway is randomly placed.
****************************************************************/
void put_stairs(struct state_of_the_union *sotu) {
  Short row, col;

  gr_row_col(&row, &col, (FLOOR | TUNNEL), sotu);
  sotu->dungeon[row][col] |= STAIRS;
}

/************************************************
                item generation
************************************************/

/***************************************************************
                   GR_WHAT_IS
 IN:
 nothing
 OUT:
 a random item-category
 PURPOSE:
 Randomly select from the 7 possible types of items.
****************************************************************/
static UShort gr_what_is() SEC_L;
static UShort gr_what_is() {
  Short percent;
  UShort what_is;

  percent = get_rand(1, 91);
  // I can improve this - make it like armor_prob
  // 1-30 = scroll
  // 31-60 = potion
  // 61-74 = weapon
  // 75-83 = armor
  // 84-88 = food
  // 89-91 = ring
  // order should be: armor, weapon, scroll, potion, *SKIP*, food, wand, ring
  // These puppies are flags, so they will need a "shift<<i".
  if (percent <= 30) {
    what_is = SCROLL;
  } else if (percent <= 60) {
    what_is = POTION;
  } else if (percent <= 64) {
    what_is = WAND;
  } else if (percent <= 74) {
    what_is = WEAPON;
  } else if (percent <= 83) {
    what_is = ARMOR;
  } else if (percent <= 88) {
    what_is = FOOD;
  } else {
    what_is = RING;
  }
  return(what_is);
}

/***************************************************************
                   GR_SCROLL
 IN:
 obj = object destined to be a random scroll
 OUT:
 nothing (scroll)
 PURPOSE:
 Assign a type of scroll to the object.
****************************************************************/
static void gr_scroll(object *obj) SEC_L;
static void gr_scroll(object *obj) {
  Short percent, kind = 0;

  obj->what_is = SCROLL;
  percent = get_rand(0, 87); // was 85
  while ( !(percent <= scroll_prob[kind]) && (kind < SCROLLS-1) )
    kind++; // the kind<SCROLLS-1 is "just in case".
  obj->which_kind = kind;
  obj->weight = SCROLL_WEIGHT;
  // PROTECT_ARMOR     6   5
  // HOLD_MONSTER      6  11
  // ENCH_WEAPON       9  16
  // ENCH_ARMOR       15  21
  // IDENTIFY          8  36
  // TELEPORT          7  44
  // SLEEP             5  51
  // SCARE_MONSTER     9  56
  // REMOVE_CURSE      5  65
  // CREATE_MONSTER    5  74
  // AGGRAVATE_MONSTER 6  80
  // MAGIC_MAPPING     5  85
  // create_obj        2  87
  // If there seem to be too many of them, reduce it to (0,86)
}

/***************************************************************
                   GR_POTION
 IN:
 obj = object destined to be a random potion
 OUT:
 nothing (potion)
 PURPOSE:
 Assign a type of potion to the object.
****************************************************************/
static void gr_potion(object *obj) SEC_L;
static void gr_potion(object *obj) {
  Short percent, kind = 0;

  obj->what_is = POTION;
  percent = get_rand(1, 118);
  while ( !(percent <= potion_prob[kind]) && (kind < POTIONS) )
    kind++; // the kind<SCROLLS is "just in case".
  obj->which_kind = kind;
  obj->weight = POTION_WEIGHT;
  // INCREASE_STRENGTH 10  10
  // RESTORE_STRENGTH  10  20
  // HEALING           10  30
  // EXTRA_HEALING     10  40
  // POISON            10  50
  // RAISE_LEVEL        5  55
  // BLINDNESS         10  65
  // HALLUCINATION     10  75
  // DETECT_MONSTER    10  85
  // DETECT_OBJECTS    10  95
  // CONFUSION         10  105
  // LEVITATION         5  110
  // HASTE_SELF         4  114
  // SEE_INVISIBLE      4  118
}

/***************************************************************
                   GR_WEAPON
 IN:
 obj = object destined to be a random weapon
 assign_wk = if true, randomly assign a type of weapon
 OUT:
 nothing (weapon)
 PURPOSE:
 Initialize a new weapon object with appropriate random values
 (optionally, randomly choose its type.)
****************************************************************/
static void gr_weapon(object *obj, Boolean assign_wk, Char wkind) SEC_L;
static void gr_weapon(object *obj, Boolean assign_wk, Char wkind) {
  Short percent;
  Short i;
  Short blessing, increment;

  obj->what_is = WEAPON;
  if (assign_wk) {
    // 50% chance of just "common" weapons
    obj->which_kind = get_rand(0, (coin_toss() ? 11 : (WEAPONS - 1)) );
  } else {
    obj->which_kind = min(max(wkind,0),WEAPONS-1);
  }
  // copy the weapon-table information
  obj->o_flags |= weapon_tab[obj->which_kind].w_flags;
  obj->weight = weapon_tab[obj->which_kind].weight;

  obj->launcher = weapon_tab[obj->which_kind].launcher;
  for (i = 0 ; i < 4 ; i++)
    obj->damage[i] = weapon_tab[obj->which_kind].damage[i];

  // this is basically the same as it was before
  if (obj->o_flags & O_COLLECTION) {
    obj->quantity = get_rand(3, 15);
    obj->quiver = get_rand(0, 126);
  } else {
    obj->quantity = 1;
  }
  obj->hit_enchant = obj->d_enchant = 0;

  percent = get_rand(1, 96);
  blessing = get_rand(1, 3);

  if (percent <= 16) {
    increment = 1;
  } else if (percent <= 32) {
    increment = -1;
    obj->is_cursed = 1;
  }
  if (percent <= 32) {
    for (i = 0; i < blessing; i++) {
      if (coin_toss()) {
	obj->hit_enchant += increment;
      } else {
	obj->d_enchant += increment;
      }
    }
  }
}

/***************************************************************
                   GR_ARMOR
 IN:
 obj = object destined to be a random armor
 OUT:
 nothing (armor)
 PURPOSE:
 Initialize a new armor object with randomly chosen attributes.
****************************************************************/

static void gr_armor(object *obj, Boolean assign_ak, Char akind) SEC_L;
static void gr_armor(object *obj, Boolean assign_ak, Char akind) {
  Short percent;
  Short blessing;
  Short kind = 0;

  obj->what_is = ARMOR;
  if (assign_ak) {
    percent = get_rand(1,100); // should this be 99? but then !crystalline...
    while ( !(percent <= armor_prob[kind]) && (kind < ARMORS) )
      kind++; // the kind<ARMORS is "just in case".
  } else
    kind = min(max(akind,0), ARMORS-1);
  obj->which_kind = kind; // = get_rand(0, (ARMORS - 1));

  obj->class = armor_class[kind];
  obj->weight = armor_weight[kind] + get_rand(0,20) - 10; // add some individual variation
  obj->is_protected = 0;
  obj->d_enchant = 0;

  percent = get_rand(1, 100);
  blessing = get_rand(1, 3);
  /*
  if (percent <= 16) {
    obj->is_cursed = 1;
    obj->d_enchant -= blessing;
  } else if (percent <= 33) {
    obj->d_enchant += blessing;
  }
  */
  if ((percent < 20) && (kind != MITHRIL)) {
    obj->is_cursed = 1;
    obj->d_enchant -= blessing;
  } else if ((percent < 28) || (kind == MITHRIL)) {
    obj->d_enchant += blessing;
  }
  if (kind == MITHRIL)
    obj->is_protected = 1;
  // if it's "metal", it can rust.  heh.
  if (kind != LEATHER && kind != PADDED_ARMOR
      && kind != CRYSTAL_ARMOR && kind != MITHRIL)
    obj->o_flags |= O_METAL;
}

/***************************************************************
                   GR_WAND
 IN:
 obj = object destined to be a random wand
 OUT:
 nothing (wand)
 PURPOSE:
 Assign a type of wand and number of shots to the object.
****************************************************************/
static void gr_wand(object *obj, struct state_of_the_union *sotu) SEC_L;
static void gr_wand(object *obj, struct state_of_the_union *sotu) {
  obj->what_is = WAND;
  obj->which_kind = get_rand(0, (WANDS - 1));
  obj->weight = (sotu->is_wood[obj->which_kind]) ? STAFF_WEIGHT : WAND_WEIGHT;
  if (obj->which_kind == MAGIC_MISSILE) {
    obj->class = get_rand(6, 12);
  } else if (obj->which_kind == CANCELLATION) {
    obj->class = get_rand(5, 9);
  } else {
    obj->class = get_rand(3, 6);
  }
}

/***************************************************************
                   GR_FOOD
 IN:
 obj = object destined to be a random food
 force_ration = chance of ration is 100% instead of 80%
 OUT:
 nothing (food)
 PURPOSE:
 Initialize a new food object (choosing ration/fruit.)
****************************************************************/
void get_food(object *obj, Boolean force_ration) {
  obj->what_is = FOOD;

  if (force_ration || rand_percent(80)) {
    obj->which_kind = RATION;
    obj->weight = FOOD_WEIGHT;
  } else {
    obj->which_kind = FRUIT;
    obj->weight = FRUIT_WEIGHT;
    obj->o_unused = get_rand(0,20); // pick a random fruit name
  }
}

/***************************************************************
                   GR_RING
 IN:
 obj = object destined to be a random ring
 assign_wk = whether to randomly assign its type
 OUT:
 nothing (ring)
 PURPOSE:
 Initialize a new ring object with random attributes.
****************************************************************/
static void gr_ring(object *ring, Boolean assign_wk) SEC_L;
static void gr_ring(object *ring, Boolean assign_wk) {
  ring->what_is = RING;
  if (assign_wk) {
    ring->which_kind = get_rand(0, (RINGS - 1));
  }
  ring->class = 0;
  ring->weight = RING_WEIGHT;

  switch(ring->which_kind) {
    /*
      case STEALTH:
      break;
      case SLOW_DIGEST:
      break;
      case REGENERATION:
      break;
      case R_SEE_INVISIBLE:
      break;
      case SUSTAIN_STRENGTH:
      break;
      case R_MAINTAIN_ARMOR:
      break;
      case SEARCHING:
      break;
      */
  case R_BURDEN:
    ring->weight = 300;
    // no break
  case R_TELEPORT:
  case R_AGGRAVATE:
  case R_DELUSION:
    ring->is_cursed = 1;
    break;
  case ADD_STRENGTH:
  case DEXTERITY:
  case R_PROTECTION:
    while ((ring->class = (get_rand(0, 4) - 2)) == 0) 
      ;
    ring->is_cursed = (ring->class < 0);
    break;
  case ADORNMENT:
  case R_MOOD:
    //  case R_LEVITATE:
    ring->is_cursed = coin_toss();
    break;
  }
}

/***************************************************************
                   GR_OBJECT
 IN:
 sotu = various globals (foods, cur_level)
 OUT:
 a randomly generated object!
 PURPOSE:
 Randomly generate a new object - allocate, determine category
 and specific type, and assign all attributes.
****************************************************************/
object * gr_object(struct state_of_the_union *sotu) {
  object *obj;

  obj = alloc_object(sotu); // everything should be 0 initially

  if (sotu->foods < (sotu->cur_level/2 )) {
    obj->what_is = FOOD;
    sotu->foods++;
  } else {
    obj->what_is = gr_what_is();
  }
  switch(obj->what_is) {
  case SCROLL:
    gr_scroll(obj);
    break;
  case POTION:
    gr_potion(obj);
    break;
  case WEAPON:
    gr_weapon(obj, true, 0);
    break;
  case ARMOR:
    gr_armor(obj, true, 0);
    break;
  case WAND:
    gr_wand(obj, sotu);
    break;
  case FOOD:
    get_food(obj, 0);
    break;
  case RING:
    gr_ring(obj, 1);
    break;
  }
  return(obj);
}

/***************************************************************
                   GET_NTH_OBJECT
 IN:
 n = index
 pack = list
 OUT:
 nth item in the list, or 0
 PURPOSE:
 Walk down the list to the nth object and return it.
 (Or walk off the list and return 0.)
****************************************************************/
object * get_nth_object(Short n, object *pack) {
  object *obj;
  Short i;

  obj = pack->next_object;
  i = 0;
  while (obj && i < n) {
    i++;
    obj = obj->next_object;    
  }

  return(obj);
}

/***************************************************************
                   NEW_OBJECT_FOR_WIZARD
 IN:
 chtype = the category of object to create
 whatkind = the kind of <chtype> to create
 sotu = various globals (rogue)
 OUT:
 nothing
 PURPOSE:
 In wizard mode, create the specified object.  (Useful for
 testing/debugging purposes.)
****************************************************************/
/* Create a new object in wizard-mode. */
void new_object_for_wizard(Short chtype, Short whatkind,
			   struct state_of_the_union *sotu)
{
  Short new_weight, max = 0;
  object *obj;
  Char buf[80];

  // I'll allow the wizard to create things even when the pack is
  // too heavy, because it's too much trouble to check :-)
  if (pack_count((object *) 0, sotu->roguep, &new_weight) >= MAX_PACK_COUNT) {
    message("pack full", sotu);
    return;
  }
  if (new_weight > max_weight(sotu)) {
    message("pack too heavy", sotu);
    return;
  }
  if (chtype < 0 || chtype > 7)
    return;

  obj = alloc_object(sotu);

  switch(chtype) {
    /* 	LIST "!" "?" "," ":" ")" "]" "/" "=" */
    /* ':' is now replaced by FOODCHAR */
  case 0:
    obj->what_is = POTION;
    max = POTIONS - 1;
    break;
  case 1:
    obj->what_is = SCROLL;
    max = SCROLLS - 1;
    break;
  case 2:
    gr_weapon(obj, false, whatkind); // whatkind should NOT be used below!
    max = WEAPONS - 1;
    break;
  case 3:
    gr_armor(obj, false, whatkind); // whatkind should NOT be used below!
    max = ARMORS - 1;
    break;
  case 4:
    gr_wand(obj, sotu);
    max = WANDS - 1;
    break;
  case 5:
    max = RINGS - 1;
    obj->what_is = RING;
    break;
  case 6:
    get_food(obj, 0);
    break;
  case 7:
    obj->what_is = AMULET;
    break;
  default:
    return;
  }
  if ((chtype != 2) && (chtype != 3)) {
    if ((whatkind >= 0) && (whatkind <= max)) {
      obj->which_kind = (UShort) whatkind;
      if (obj->what_is == RING) {
	gr_ring(obj, 0);
      }
    } else {
      message("oops", sotu); // note: this should no longer happen
      free_object(obj, sotu);
      return;
    }
  }
  /* whee */
  get_desc(obj, buf, sotu);
  message(buf, sotu);

  (void) add_to_pack(obj, &sotu->roguep->pack, 1, sotu);
}



/***************************************************************
                   NAME_OF
 IN:
 obj = object to get the type of
 sotu = various globals (fruit, is_wood, id_weapons)
 OUT:
 string representing a general description of the object
 PURPOSE:
 Given an object, get a string describing it e.g. "scrolls",
 "shuriken", "slime-mold"
****************************************************************/
/* I may regret this.. */
static Char * name_of(object *obj, struct state_of_the_union *sotu) SEC_L;
static Char * name_of(object *obj, struct state_of_the_union *sotu) 
{
  Char *retstring;

  switch(obj->what_is) {
  case SCROLL:
    retstring = obj->quantity > 1 ? "scrolls " : "scroll ";
    break;
  case POTION:
    retstring = obj->quantity > 1 ? "potions " : "potion ";
    break;
  case FOOD:
    if (obj->which_kind == RATION) {
      retstring = "food ";
    } else if (obj->which_kind == FRUIT) {
      retstring = fruit_names[obj->o_unused]; // is this ok? used only locally
    } else {
      retstring = carcase_names[obj->o_unused];
    }
    break;
  case WAND:
    retstring = sotu->is_wood[obj->which_kind] ? "staff " : "wand ";
    break;
  case WEAPON:
    retstring = sotu->id_weapons[obj->which_kind].title;
    if ((obj->o_flags & O_COLLECTION) && (obj->quantity <= 1)) {
      // XXXXX make it singular!!!
      // retstring = sotu->id_weapons[obj->which_kind].title;      
      switch(obj->which_kind) {
      case DART:
	retstring = "dart ";
	break;
      case ARROW:
	retstring = "arrow ";
	break;
      case DAGGER:
	retstring = "dagger ";
	break;
      case SHURIKEN:
	retstring = "shuriken ";
	break;
      case ROCK:
	retstring = "rock ";
	break;
      case CROSSBOW_BOLT:
	retstring = "crossbow bolt ";
	break;
      case SLING_BULLET:
	retstring = "sling bullet ";
	break;
      case FOOTBOW_BOLT:
	retstring = "footbow bolt ";
	break;
	// don't need a default
      }
    }
    break;
  case ARMOR:
    retstring = "armor ";
    break;
  case RING:
    retstring = "ring ";
    break;
  case AMULET:
    retstring = "amulet ";
    break;
  default:
    retstring = "unknown ";
    break;
  }
  return(retstring);
}



/***************************************************************
                   COPY_NAME_OF
 IN:
 obj = object to get name_of
 foo = space supplied by caller to put name_of result in
 sotu = various globals
 OUT:
 nothing (foo)
 PURPOSE:
 well, this is one of those funky porting hack things.
****************************************************************/
void copy_name_of(object *obj, Char *foo,
		  struct state_of_the_union *sotu)
{
  StrCopy(foo, name_of(obj, sotu));
}


/************************************************
            this WAS in inventory.c
************************************************/


/***************************************************************
                   GET_DESC_EITHER
               (GET_DESC, GET_DESC_BRIEF)
 IN:
 obj = the object to get a description of
 desc = space to return the description in
 sotu = various globals (wizard)
 [brief = false in GET_DESC, true in GET_DESC_BRIEF]
 OUT:
 nothing (desc)
 PURPOSE:
 Given an object, return a string describing it to the best of
 the rogue's knowledge - used in inventory lists, picking up,
 dropping, all that sort of thing
 (What 'brief' does is to leave the "entitled: " out of the scroll
 descriptions; they are more than long enough without it.)
****************************************************************/
static void get_desc_either(object *obj, Char *desc,
			    struct state_of_the_union *sotu,
			    Boolean brief) SEC_L;
static void get_desc_either(object *obj, Char *desc,
			    struct state_of_the_union *sotu, Boolean brief)
{
  Char *item_name;
  struct id *id_table;
  Char more_info[32];
  Short i;

  if (obj->what_is == AMULET) {
    StrCopy(desc, "the amulet of Yendor ");
    return;
  }
  item_name = name_of(obj, sotu); /* exists now */

  if (obj->what_is == GOLD) {
    StrPrintF(desc, "%d pieces of gold", obj->quantity);
    return;
  }

  if (obj->what_is != ARMOR) {
    if (obj->quantity == 1) {
      StrCopy(desc, "a ");
    } else {
      StrPrintF(desc, "%d ", obj->quantity);
    }
  }
  if (obj->what_is == FOOD) {
    if (obj->which_kind == RATION) {
      if (obj->quantity > 1) {
	StrPrintF(desc, "%d rations of ", obj->quantity);
      } else {
	StrCopy(desc, "some ");
      }
    } else {
      StrCopy(desc, "a ");
    }
    StrCat(desc, item_name);
    goto ANA;
  }
  id_table = get_id_table(obj, sotu);

  if (IS_WIZARD) {
    goto ID; /* not tested */
  }
  if (obj->what_is & (WEAPON | ARMOR | WAND | RING)) {
    goto CHECK;
  }

  switch(id_table[obj->which_kind].id_status) {
  case UNIDENTIFIED:
  CHECK:
  switch(obj->what_is) {
  case SCROLL:
    StrCat(desc, item_name);
    if (!brief)
      StrCat(desc, "entitled: ");
    StrCat(desc, id_table[obj->which_kind].title);
    break;
  case POTION:
    StrCat(desc, id_table[obj->which_kind].title);
    StrCat(desc, item_name);
    break;
  case WAND:
  case RING:
    if (obj->identified ||
	(id_table[obj->which_kind].id_status == IDENTIFIED)) {
      goto ID;
    }
    /* I've disabled 'call' elsewhere, for a while */
    if (id_table[obj->which_kind].id_status == CALLED) {
      goto CALL;
    }
    StrCat(desc, id_table[obj->which_kind].title);
    StrCat(desc, item_name);
    break;
  case ARMOR:
    if (obj->identified) {
      goto ID;
    }
    StrCopy(desc, id_table[obj->which_kind].title);
    break;
  case WEAPON:
    if (obj->identified) {
      goto ID;
    }
    StrCat(desc, name_of(obj, sotu));
    break;
  }
  break;
  case CALLED:
  CALL:	switch(obj->what_is) {
  case SCROLL:
  case POTION:
  case WAND:
  case RING:
    StrCat(desc, item_name);
    StrCat(desc, "called ");
    StrCat(desc, id_table[obj->which_kind].title);
    break;
  }
  break;
  case IDENTIFIED:
ID:
    switch(obj->what_is) {
    case SCROLL:
    case POTION:
      StrCat(desc, item_name);
      StrCat(desc, id_table[obj->which_kind].real);
      break;
    case RING:
      if (IS_WIZARD || obj->identified) {
	if ((obj->which_kind == ADD_STRENGTH) ||
	    (obj->which_kind == DEXTERITY) ||
	    (obj->which_kind == R_PROTECTION)) {
	  StrPrintF(more_info, "%s%d ", ((obj->class > 0) ? "+" : ""),
		    obj->class);
	  StrCat(desc, more_info);
	}
      }
      StrCat(desc, item_name);
      StrCat(desc, id_table[obj->which_kind].real);
      break;
    case WAND:
      StrCat(desc, item_name);
      StrCat(desc, id_table[obj->which_kind].real);
      if (IS_WIZARD || obj->identified) {
	StrPrintF(more_info, "[%d]", obj->class);
	StrCat(desc, more_info);
      }
      break;
    case ARMOR:
      StrPrintF(desc, "%s%d ", ((obj->d_enchant >= 0) ? "+" : ""),
		obj->d_enchant);
      StrCat(desc, id_table[obj->which_kind].title);
      StrPrintF(more_info, "[%d] ", obj->class + obj->d_enchant);
      StrCat(desc, more_info);
      break;
    case WEAPON:
      StrPrintF(desc+StrLen(desc), "%s%d,%s%d ",
		((obj->hit_enchant >= 0) ? "+" : ""),
		obj->hit_enchant,
		((obj->d_enchant >= 0) ? "+" : ""),
		obj->d_enchant);
      StrCat(desc, name_of(obj, sotu));
      break;
    }
  break;
  }
ANA:
  if (!StrNCompare(desc, "a ", 2)) {
    if (is_vowel(desc[2])) {
      /* hm, how annoying. but not the common case, I trust */
      for (i = StrLen(desc) + 1; i > 1; i--) {
	desc[i] = desc[i-1];
      }
      desc[1] = 'n';
    }
  }
  if (brief && (obj->in_use_flags & BEING_USED)) {
    i = StrLen(desc);
    desc[i-1] = ':';
    desc[i] = ' ';
    desc[i+1] = 0;
  }
  if (obj->in_use_flags & BEING_WIELDED) {
    StrCat(desc, "in hand");
  } else if (obj->in_use_flags & BEING_WORN) {
    StrCat(desc, (brief ? "worn" : "being worn"));
  } else if (obj->in_use_flags & ON_LEFT_HAND) {
    // I don't think I'll bother saying which finger it's on.
    StrCat(desc, (brief ? "left" : "on left hand"));
  } else if (obj->in_use_flags & ON_RIGHT_HAND) {
    StrCat(desc, (brief ? "right" : "on right hand"));
  }
}
/* Return a full description. */
void get_desc(object *obj, Char *desc, struct state_of_the_union *sotu) {
  return get_desc_either(obj, desc, sotu, false);
}
/* Return a brief description. */
void get_desc_brief(object *obj, Char *desc, struct state_of_the_union *sotu) {
  return get_desc_either(obj, desc, sotu, true);
}




/***************************************************************
                   FLOP_WEAPON
 IN:
 weapon = the weapon to be flopped
 row,col = the location that it missed
 sotu = various globals (dungeon, rogue, level_monsters)
 OUT:
 nothing
 PURPOSE:
 This is called when the rogue threw something and did not
 successfully hit a monster with it... a new object will
 (usually) appear on the ground near there, and the quantity of
 the thrown object will be decremented.
 ('Vanish'ing the inventory object is the caller's responsibility)
****************************************************************/
void flop_weapon(object *weapon, Short row, Short col,
		 struct state_of_the_union *sotu)
{
  object *new_weapon, *monster;
  Char msg[80];
  Boolean found = false;
  Short mch, dch, t, i=0;
  UShort mon;
  UShort **dungeon = sotu->dungeon;

  /* Try to find a place near row,col to flop the weapon at. */
  while ((i < 9) && dungeon[row][col] & ~(FLOOR | TUNNEL | DOOR | MONSTER)) {
    rand_around(i++, &row, &col);
    if ((row > (DROWS-2)) || (row < MIN_ROW) ||
	(col > (DCOLS-1)) || (col < 0) || (!dungeon[row][col]) ||
	(dungeon[row][col] & ~(FLOOR | TUNNEL | DOOR | MONSTER))) {
      continue;
    }
    found = 1;
    break;
  }

  if (found || (i == 0)) {
    /* Either row,col itself is ok, or we found a place near it... */
    /* Create and place the flopped weapon. */
    new_weapon = alloc_object(sotu);
    *new_weapon = *weapon;
    new_weapon->in_use_flags = NOT_USED;
    new_weapon->quantity = 1;
    new_weapon->ichar = 'Z';
    place_at(new_weapon, row, col, sotu);
    /* Update the display, if the rogue can see row,col */
    if (rogue_can_see(row, col, sotu) &&
	((row != sotu->roguep->row) || (col != sotu->roguep->col))) {
      mon = dungeon[row][col] & MONSTER;
      dungeon[row][col] &= (~MONSTER);
      dch = get_dungeon_char(row, col, sotu);
      if (mon) {
	mch = mvinch(row, col);
	if ((monster = object_at(sotu->level_monsters, row, col))) {
	  monster->trail_char = dch;
	}
	if ( Not_Alpha(mch) ) {
	  mvaddch(row, col, dch);
	}
      } else {
	mvaddch(row, col, dch);
      }
      dungeon[row][col] |= mon;
    }
  } else {
    /* We did not find a place to flop the weapon. */
    t = weapon->quantity;
    weapon->quantity = 1;
    StrPrintF(msg, "the %s vanished as it hit the ground",
	      name_of(weapon, sotu));
    weapon->quantity = t;
    message(msg, sotu);
  }
}

// moved get_thrown_at_monster to ..

/***************************************************************
                   EAT
 IN:
 index = of the food item, in the rogue's pack
 sotu = various globals ()
 OUT:
 true if caller should call reg_move()
 (eat can't call reg_move itself; reg_move isn't in this library.)
 PURPOSE:
 The rogue ate something.  Make the rogue less hungry.
// (was in lib_use.c)
****************************************************************/
Boolean eat(Short index, struct state_of_the_union *sotu) {
  Char buf[70];
  Short moves;
  object *obj;
  fighter *rogue = sotu->roguep;

  if (!(obj = get_nth_object(index, &rogue->pack))) {
    message("no such item.", sotu);
    return false;
  }
  /* //the following is guaranteed by the caller to NEVER HAPPEN:
  if (obj->what_is != FOOD) {
    message("you can't eat that", sotu);
    return false;
  }
  */
  /* Slime molds are always good; 40% of rations are stale. */
  /* I should make it so CARCASEs are even worse */

  // record conduct
  if (obj->which_kind == RATION) {
    sotu->conduct |= CONDUCT_ATE_VEG;
    sotu->conduct |= CONDUCT_ATE_MEAT;
  } else if (obj->which_kind == FRUIT) {
    sotu->conduct |= CONDUCT_ATE_VEG;
  } else {
    // a popsicle doesn't count as anything.
    if (obj->o_unused != POPSICLE)
      sotu->conduct |= CONDUCT_ATE_MEAT;    
  }

  if (sotu->conduct & CONDUCT_UNDEAD) {//if (sotu->score_status & STATUS_UNDEAD)
    if (obj->which_kind == RATION) {
      moves = get_rand(100, 200);
      message("hm, that was strangely unsatisfying", sotu);
    } else if (obj->which_kind == FRUIT) {
      moves = get_rand(50, 150);
      StrPrintF(buf, "you gnaw on the vile %s", fruit_names[obj->o_unused]);
      message(buf, sotu);
    } else {
      moves = get_rand(900, 1100);
      StrPrintF(buf, "you enjoy the %s", carcase_names[obj->o_unused]);
      message(buf, sotu);
    }
  } else {
    if ((obj->which_kind == FRUIT) || rand_percent(60)) {
      moves = get_rand(900, 1100);
      if (obj->which_kind == RATION) {
	message("yum, that tasted good", sotu);
      } else if (obj->which_kind == FRUIT) {
	StrPrintF(buf, "my, that was a yummy %s", fruit_names[obj->o_unused]);
	message(buf, sotu);
      } else {
	StrPrintF(buf, "that %s wasn't too bad", carcase_names[obj->o_unused]);
	message(buf, sotu);
	// if it was a floating-eye, act like a potion of see invisible!!!!
      }
    } else {
      moves = get_rand(700, 900);
      message("yuk, that food tasted awful", sotu);
      add_exp(2, 1, sotu);
      // add a small chance that the carcass will poison you!!!!
    }
  }
  rogue->moves_left /= 3; /* i.e. there's no sense in stuffing yourself */
  rogue->moves_left += moves;
  sotu->hunger_str[0] = 0;
  if (rogue->moves_left <= FAINT)
    StrCopy(sotu->hunger_str, "faint");
  else if (rogue->moves_left <= WEAK)
    StrCopy(sotu->hunger_str, "weak");
  else if (rogue->moves_left <= HUNGRY)
    StrCopy(sotu->hunger_str, "hungry");
  
  print_stats(STAT_HUNGER, sotu);

  vanish(obj, &rogue->pack, sotu);
  return true;
}

void drink_monblood(object *monster, struct state_of_the_union *sotu)
{
  Short moves;
  fighter *rogue = sotu->roguep;
  Char mn[18];
  Char buf[70];
  Short ch;

  ch = monster->m_char - 'A';
  if (ch > 25)
    ch = 26 + monster->m_char - 'a'; // yo ho!
  /* if the monster is the wrong kind, return */
  switch(ch) {
  case 5: case 8: case 15: case 22: case 25:
  case 26: case 27: case 31: case 40: case 42:
  case 45: case 48: case 49: case 50:
    return;
  }
  mon_name(monster, mn, sotu);
  StrPrintF(buf, "you drink the %s's blood", mn);
  sotu->conduct |= CONDUCT_ATE_BLOOD;
  message(buf, sotu);
  moves = get_rand(500, 700);
  rogue->moves_left /= 3; /* i.e. there's no sense in stuffing yourself */
  rogue->moves_left += moves;
  sotu->hunger_str[0] = 0;
  print_stats(STAT_HUNGER, sotu);
}
