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
 * my_object.c
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

static void init_id_potions() SEC_1;
static void init_id_scrolls() SEC_1;
static void init_id_weapons() SEC_1;
static void init_id_armors() SEC_1;
static void init_id_wands() SEC_1;
static void init_id_rings() SEC_1;
static void alloc_level_pts() SEC_1;
static void alloc_dungeon_arrays() SEC_1;


struct id * id_potions;
struct id * id_scrolls;
struct id * id_wands;
struct id * id_rings;
struct id * id_weapons;
struct id * id_armors;

extern Boolean *rooms_visited;
extern struct state_of_the_union * sotu;

extern room * rooms;

Char ** wiz_names;

/* actually, const in this file could all be static const. */

static const Short potion_values[POTIONS] = {
  100, 
  250, 
  100, 
  200, 
  10,  
  300, 
  10,  
  25,  
  100, 
  100, 
  10, 
  80, 
  150,
  145
};
// static const Char potion_colors[POTION_COLORS][12] = {
static const Char * potion_colors[POTION_COLORS] = {
// Added more colors from urogue.
  "blue ",
  "red ",
  "green ",
  "grey ",
  "brown ",
  "clear ",
  "pink ",
  "white ",
  "purple ",
  "black ",
  "yellow ",
  "plaid ",
  "burgundy ",
  "beige ",

  "orange ",
  "silver ",
  "gold ",
  "violet ",
  "vermilion ",
  "ecru ",
  "turquoise ",
  "amber ",
  "topaz ",
  "tan ",
  "tangerine ",
  "aquamarine ",
  "scarlet ",
  "khaki ",
  "crimson ",
  "indigo ",
  "lavender ",
  "saffron "
};

/* note: Burgundy was Magenta in urogue */

// static const Char potion_actions[POTIONS][22] = {
static const Char * potion_actions[POTIONS] = {
  "of increase strength ",
  "of restore strength ",
  "of healing ",
  "of extra healing ",
  "of poison ",
  "of raise level ",
  "of blindness ",
  "of hallucination ",
  "of detect monster ",
  "of detect things ",
  "of confusion ",
  "of levitation ",
  "of haste self ",
  "of see invisible "
};

/**********************************************************************
                       INIT_ID_POTIONS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Initialize the array of potion information (what the rogue knows about
 them.)  Do this at the start of a "session" and whenever a new game
 is started from the top-ten list (since the new rogue doesn't know
 what the potions are anymore.)
 **********************************************************************/
static void init_id_potions() {
  Short i, k;
  Boolean used[POTION_COLORS];
  for (i = 0 ; i < POTION_COLORS ; i++)
    used[i] = false;
  for (i = 0 ; i < POTIONS ; i++) {
    id_potions[i].value = potion_values[i];
    id_potions[i].id_status = UNIDENTIFIED;
    StrCopy(id_potions[i].real, potion_actions[i]);
    // select an unused random color
    do {
      k = get_rand(0, POTION_COLORS-1);
    } while (used[k]);
    used[k] = true;
    StrCopy(id_potions[i].title, potion_colors[k]);
  }
  //StrPrintF(id_potions[0].title, "blue "); /* Wouldn't I like to know why. */
}

static const Short scroll_values[SCROLLS] = {
  505,
  200,
  235,
  235,
  175,
  190,
  25,
  610,
  210,
  100,
  25,
  180,
  400
};
// static const Char scroll_actions[SCROLLS][22] = {
static const Char * scroll_actions[SCROLLS] = {
  "of protect armor ",
  "of hold monster ",
  "of enchant weapon ",
  "of enchant armor ",
  "of identify ",
  "of teleportation ",
  "of sleep ",
  "of scare monster ",
  "of remove curse ",
  "of create monster ",
  "of aggravate monster ",
  "of magic mapping ",
  "of create object "
};
/**********************************************************************
                       INIT_ID_SCROLLS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Initialize the array of scroll information (what the rogue knows about
 them.)  Do this at the start of a "session" and whenever a new game
 is started from the top-ten list (since the new rogue doesn't know
 what the scrolls are anymore.)
 **********************************************************************/
static void init_id_scrolls() {
  Short i;
  for (i = 0 ; i < SCROLLS ; i++) {
    id_scrolls[i].value = scroll_values[i];
    id_scrolls[i].id_status = UNIDENTIFIED;
    StrCopy(id_scrolls[i].real, scroll_actions[i]);
  }
  make_scroll_titles();
}

static const Short weapon_values[WEAPONS] = {
 7,  1,  1,  2,  3,  8, 15, 20,  1,  1,
 15, 1,  2, 25, 12,  6, 18, 20, 21, 21,
 8,  2, 10,  8,  6,  4,  3,  8,  4,  4,
 6,  8,  1,  8, 15,  8, 27, 10,  1, 10,
 3,  7, 10, 20, 12
};
// static const Char weapon_names[WEAPONS][18] = {
static const Char * weapon_names[WEAPONS] = {
// See also: lib_object.c  "weapon_tab"
  "short bow ",
  "darts ",
  "arrows ",
  "daggers ",
  "shurikens ",
  "mace ",
  "long sword ",
  "two-handed sword ",  // the rest are from urogue:
  "rocks ",
  "sling ",
  "crossbow ",
  "crossbow bolts ", 
  "spear ",
  "trident ",
  "spetum ",
  "bardiche ",
  "short pike ",
  "bastard sword ",
  "halberd ",
  "battle axe ",   // (silver)  "arrow ", 3
  "hand axe ",
  "club ",
  "flail ",
  "glaive ",
  "guisarme ",
  "hammer ",
  "javelin ", 
  "morning star ",
  "partisan ",
  "pick ",
  "long pike ",
  "scimitar ",
  "sling bullets ",
  "quarterstaff ",
  "broadsword ",
  "short sword ",  //  "boomerang ",  //  "burning oil ",
  "claymore ",  //  "crysknife ",
  "footbow ",
  "footbow bolts ",
  "katana ",
  "leuku ",
  "tomahawk ",
  "pertuska ",
  "sabre ",
  "cutlass "
};

/**********************************************************************
                       INIT_ID_WEAPONS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Initialize the array of weapon information.  This does not have to be
 redone when a rogue is killed and a new game is started; only at the
 start of a session.
 **********************************************************************/
static void init_id_weapons() {
  Short i;
  for (i = 0 ; i < WEAPONS ; i++) {
    id_weapons[i].value = weapon_values[i];
    StrCopy(id_weapons[i].title, weapon_names[i]);
  }
}

static const Short armor_values[ARMORS] = {
  70, 50, 50, 70, 150, 100, 150, 150, 400, 650, 2000, 3500
};
// static const Char armor_names[ARMORS][24] = {
static const Char * armor_names[ARMORS] = {
  "leather armor ",
  "ring mail ",
  "studded leather armor ", // new
  "scale mail ",
  "padded armor ", // new
  "chain mail ",
  "splint mail ", // swapped
  "banded mail ", // swapped
  "plate mail ",
  "plate armor ", // new
  "mithril ", // new
  "crystalline armor " // new
};
/**********************************************************************
                       INIT_ID_ARMORS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Initialize the array of armor information.  This does not have to be
 redone when a rogue is killed and a new game is started; only at the
 start of a session.
 **********************************************************************/
static void init_id_armors() {
  Short i;
  for (i = 0 ; i < ARMORS ; i++) {
    id_armors[i].value = armor_values[i];
    StrCopy(id_armors[i].title, armor_names[i]);
  }
}

static const Short wand_values[WANDS] = {
  25,
  50,
  45,
  8,
  55,
  2,
  25,
  20,
  20,
  0,
  42,
  42
};
// static const Char wand_actions[WANDS][20] = {
static const Char * wand_actions[WANDS] = {
  "of teleport away ",
  "of slow monster ",
  "of confuse monster ",
  "of invisibility ",
  "of polymorph ",
  "of haste monster ",
  "of sleep ",
  "of magic missile ",
  "of cancellation ",
  "of do nothing ",
  "of lightning ",
  "of terror "
};
/**********************************************************************
                       INIT_ID_WANDS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Initialize the array of wand information.  This does not have to be
 redone when a rogue is killed and a new game is started.  (I guess.)
 **********************************************************************/
static void init_id_wands() {
  Short i;
  for (i = 0 ; i < WANDS ; i++) {
    id_wands[i].value = wand_values[i];
    StrCopy(id_wands[i].real, wand_actions[i]);
  }
}

static const Short ring_values[RINGS] = {
  250,  100,  255,  295,  200,
  250,  250,  25,  300,  290,
  270,  0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0
};
// static const Char ring_actions[RINGS][21] = {
static const Char * ring_actions[RINGS] = {
  "of sustain strength ",
  "of see invisible ",
  "of maintain armor ",
  "of levitation ",
  "of free action ",
  "of breathing ",
  "of alertness ",
  "of resist fire ",
  "of resist cold ",
  "of resist shock ",
  "of lifesaving ",
  "of esp ",
  // the following are cursed or useless:
  "of warning ",
  "of mood ",
  "of teleportation ",
  "of aggravate ",
  "of delusion ",
  "of burden ",
  "of adornment ",
  // the following have class or are otherwise cumulative
  "of stealth ",
  "of regeneration ",
  "of slow digestion ",
  "of add strength ",
  "of dexterity ",
  "of searching ",
  "of protection "
};
/**********************************************************************
                       INIT_ID_RINGS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Initialize the array of ring information.  This does not have to be
 redone when a rogue is killed and a new game is started.  (I guess.)
 **********************************************************************/
static void init_id_rings() {
  Short i;
  for (i = 0 ; i < RINGS ; i++) {
    id_rings[i].value = ring_values[i];
    StrCopy(id_rings[i].real, ring_actions[i]);
  }
}

static const Long lev_pts[MAX_EXP_LEVEL] = {
  10L,
  20L,
  40L,
  80L,
  160L,
  320L,
  640L,
  1300L,
  2600L,
  5200L,
  10000L,
  20000L,
  40000L,
  80000L,
  160000L,
  320000L,
  1000000L,
  3333333L,
  6666666L,
  MAX_EXP,
  99900000L
};
/**********************************************************************
                       ALLOC_LEVEL_PTS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Allocate and initialize the list of what experience is needed for
 each new level (this could probably be done some better way.)
 Do this exactly once at the start of a session.
 **********************************************************************/
static void alloc_level_pts() {
  Short i;
  sotu->level_points = (Long *) md_malloc(sizeof(Long) * MAX_EXP_LEVEL);
  for (i = 0 ; i < MAX_EXP_LEVEL ; i++) {
    sotu->level_points[i]  = lev_pts[i];    
  }
  // MAX_EXP_LEVEL should be 21
}

// alloc_trap_strings has been MOVED to a static array in trap.c, dammit

/**********************************************************************
                       ALLOC_DUNGEON_ARRAYS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Allocate and initialize the data structures that represent the dungeon
 (rooms, traps, objects, monsters.)
 Do this exactly once at the start of a session.
 **********************************************************************/
static void alloc_dungeon_arrays() {
  /* Wonder if this will work. */
  /* UShort dungeon[DROWS][DCOLS]; */
  Short i;
  /* object.c */
  dungeon = (UShort **) md_malloc(sizeof(UShort *) * DROWS);
  for (i = 0 ; i < DROWS ; i++) {
    dungeon[i] = (UShort *) md_malloc(sizeof(UShort) * DCOLS);
  }
  /* room.c */
  rooms = (room *) md_malloc(sizeof(room) * MAXROOMS);
  rooms_visited = (Boolean *) md_malloc(sizeof(Boolean) * MAXROOMS);
  /* trap.c */
  traps = (trap *) md_malloc(sizeof(trap) * MAX_TRAPS);

  sotu->dungeon = dungeon;
  sotu->rooms = rooms;
  sotu->traps = traps;

  level_objects = (object *) md_malloc(sizeof(object));
  level_monsters = (object *) md_malloc(sizeof(object));
  sotu->level_objects = level_objects;
  sotu->level_monsters = level_monsters;
  level_objects->next_object = 0; /* just to be sure. */
  level_monsters->next_monster = 0;
}

/**********************************************************************
                       ALLOC_ID_ARRAYS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Allocate and initialize the arrays that hold the information about
 the various types of objects.
 Do this exactly once at the start of a session.
 **********************************************************************/
void alloc_id_arrays() {
  Short i;
  alloc_dungeon_arrays();

  /* through the magic of me, allocated memory is guaranteed zeroed. */
  /* perhaps I should CHECK that these are not NULL. */
  id_potions = (struct id *) md_malloc(sizeof(struct id) * POTIONS);
  id_scrolls = (struct id *) md_malloc(sizeof(struct id) * SCROLLS);
  id_weapons = (struct id *) md_malloc(sizeof(struct id) * WEAPONS);
  id_armors  = (struct id *) md_malloc(sizeof(struct id) * ARMORS);
  id_wands   = (struct id *) md_malloc(sizeof(struct id) * WANDS);
  id_rings   = (struct id *) md_malloc(sizeof(struct id) * RINGS);
  sotu->is_wood = (Boolean *) md_malloc(sizeof(Boolean) * WANDS);

  sotu->id_potions = id_potions;
  sotu->id_scrolls = id_scrolls;
  sotu->id_weapons = id_weapons;
  sotu->id_armors = id_armors;
  sotu->id_wands = id_wands;
  sotu->id_rings = id_rings;

  sotu->conduct &= ~CONDUCT_ISWIZ;// wizard = false;

  init_id_potions();
  init_id_scrolls();
  init_id_weapons();
  init_id_armors();
  init_id_wands();
  init_id_rings();

  alloc_level_pts();
  //  alloc_trap_strings();

  // allocate space for
  // Char[ max(objecttypeses) ][ max(all those names) ]
  // currently, WEAPONS is the most and 24 chars is longest.
  wiz_names = (Char **) md_malloc(WEAPONS * sizeof(Char *));
  for (i = 0 ; i < WEAPONS ; i++) {
    wiz_names[i] = (Char *) md_malloc(24 * sizeof(Char));
  }
}

/* rats.  jump-island necessary -- see next file monster.c */


/**********************************************************************
                       CLEAR_IDENTIFICATIONS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Reinitialize the existing potion and scroll arrays (when the player
 has started a new game from the top-ten list.)
 **********************************************************************/
void clear_identifications() {
  init_id_potions(); // also re-randomizes the colors
  init_id_scrolls();
}

/**********************************************************************
                       INIT_WIZ_LIST
 IN:
 form
 id of the list to set
 type of object to set the list to
 various globals
 OUT:
 nothing
 PURPOSE:
 Actually, I have no idea whether this will work or not.
 **********************************************************************/
static const Short object_chars[10] = {
  '!', '?', ')', ']', '/', '=', FOODCHAR, ',', 'a', 'A'
};
void init_wiz_list(FormPtr frm, Short some_list_id, Short object_type) {
  ListPtr lst;
  Short len, i, selected;
  Char c = 'A';
  lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, some_list_id));
  selected = 0;
  switch (object_type) {
  case 0:
    len = POTIONS;
    for (i = 0 ; i < len ; i++)
      StrCopy(wiz_names[i], potion_actions[i]);
    FrmCopyTitle(frm, "Create Potion");
    break;
  case 1:
    len = SCROLLS;
    for (i = 0 ; i < len ; i++)
      StrCopy(wiz_names[i], scroll_actions[i]);
    FrmCopyTitle(frm, "Create Scroll");
    break;
  case 2:
    len = WEAPONS;
    for (i = 0 ; i < len ; i++)
      StrCopy(wiz_names[i], weapon_names[i]);
    FrmCopyTitle(frm, "Create Weapon");
    break;
  case 3:
    len = ARMORS;
    for (i = 0 ; i < len ; i++)
      StrCopy(wiz_names[i], armor_names[i]);
    FrmCopyTitle(frm, "Create Armor");
    break;
  case 4:
    len = WANDS;
    for (i = 0 ; i < len ; i++)
      StrCopy(wiz_names[i], wand_actions[i]);
    FrmCopyTitle(frm, "Create Wand");
    break;
  case 5:
    len = RINGS;
    for (i = 0 ; i < len ; i++)
      StrCopy(wiz_names[i], ring_actions[i]);
    FrmCopyTitle(frm, "Create Ring");
    break;
  case 6:
    // food
    len = 1;
    StrCopy(wiz_names[0], "food");
    FrmCopyTitle(frm, "Create Food");
    break;
  case 7:
    // amulet
    if (IS_WIZARD) {
      len = 1;
      StrCopy(wiz_names[0], "amulet");
      FrmCopyTitle(frm, "Create Amulet");
      break;
    } // else act as default!
  case 8:
    c = 'a';
  case 9:
    // monster
    if (IS_WIZARD) {
      for (i = 0 ; i < 26 ; i++) {
	wiz_names[i][0] = c + i;
	wiz_names[i][1] = '\0';
      }
      len = 26;
      FrmCopyTitle(frm, "Create Monster");
      break;
    } // else act as default!
  default:
    len = (IS_WIZARD) ? 10 : 7; // only wizard gets amulet.  nyaah.
    for (i = 0 ; i < len ; i++) {
      wiz_names[i][0] = object_chars[i];
      wiz_names[i][1] = '\0';
    }
    FrmCopyTitle(frm, "Create Object");
    selected = -1;
    break;
  }
  LstSetListChoices(lst, wiz_names, len);
  LstSetSelection (lst, selected);
}
