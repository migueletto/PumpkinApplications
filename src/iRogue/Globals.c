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
#include "iRogueRsc.h"

#include "Globals.h"

#include "rogue.h"

// from urogue.  I think that [][] should put this in 'data' not 'code'.
/*
Char fruit_names[21][14] = { "candleberry ", "caprifig ",     "dewberry ", 
			     "elderberry ",  "gooseberry ",   "guanabana ", 
			     "hagberry ",    "ilama ",        "imbu ", 
			     "jaboticaba ",  "jujube ",       "litchi ",
			     "mombin ",      "pitanga ",      "prickly-pear ",
			     "rambutan ",    "sapodilla ",    "soursop ",
			     "sweetsop ",    "whortleberry ", "slime-mold " };
*/

/*****************************************************************************
 *                                                                           *
 *                      Global vars                                          *
 *                                                                           *
 *****************************************************************************/

//DmOpenRef       RogueDB = NULL;

static void init_roguep() SEC_1;
static void player_init() SEC_1;

/* object mon_tab[]; */
/* object level_monsters; */




/* struct state_of_the_union sotu_data; */
/* struct state_of_the_union * sotu = &sotu_data; */

struct state_of_the_union * sotu;

Char * hit_message; /* was in hit.c */


/* Do this ==once per session. */

void alloc_and_init_sotu() {
  Short i;
  fighter *rogue;
  sotu = (struct state_of_the_union *) md_malloc(sizeof(struct 
							state_of_the_union));
  /* perhaps I should check that the result is not NULL. */

  for (i = 0 ; i < SAVED_MSGS ; i++)
    sotu->old_messages[i] = (Char *) md_malloc(sizeof(Char) * SAVED_MSG_LEN);

/*    sotu->fruit = (Char *) md_malloc(sizeof(Char) * 14); */
  rogue = (fighter *) md_malloc(sizeof(fighter));
  sotu->roguep = rogue;

  hit_message = (Char *) md_malloc(sizeof(Char) * 100);

  init_sotu();
}
/*
void init_fruit() {
  StrCopy(sotu->fruit, fruit_names[sotu->fruit_number]);
}
*/
void init_sotu() {
  Short i;
  sotu->birthdate = TimGetSeconds();
  /* hit.c */
  /* init.c */
  /* inventory.c */
  /* level.c */
//  sotu->cur_level = 25;
  sotu->cur_level = 0; /* XXXXXXXXX DEBUGGING */
  sotu->max_level = 1;
  sotu->party_room = NO_ROOM;
  /* main.c */
  /* monster.c */
  /* move.c */
  sotu->m_moves = 0;
  sotu->jump = false;
  /* object.c */
  sotu->foods = 0;
  /* play.c */
  sotu->interrupted = false;
  /* ring.c */
  /* room.c */
  /* spec_hit.c */
  sotu->less_hp = 0;
  /* throw.c */
  /* trap.c */
  sotu->trap_door = false;
  sotu->bear_trap = 0;
  /* use.c */
  sotu->halluc = 0;
  sotu->blind = 0;
  sotu->confused = 0;
  sotu->levitate = 0;
  sotu->haste_self = 0;
  sotu->see_invisible = false;
  sotu->extra_hp = 0;
  sotu->detect_monster = false;
  /* zap.c */
  sotu->conduct = 0;//sotu->wizard = false; sotu->score_status = STATUS_NORMAL;

  i = get_rand(0, 30);
  //  sotu->fruit_number = min(i,20); // make it still frequently slime-mold
  //  init_fruit(); // may be called again if a saved rogue is loaded.

  sotu->hunger_str[0] = 0;

  /* XXX This is where I would put the owner name in username */
  // StrPrintF(sotu->username, "sprite");
  get_username(sotu);

  sotu->party_counter = get_rand(1, PARTY_TIME);
  /* XXXX Some other things from init.c are probably also missing. */

  init_roguep();
  ring_stats(0, sotu);

  for (i = 0 ; i < SAVED_MSGS ; i++)
    sotu->old_messages[i][0] = '\0';
  sotu->last_old_message_shown = SAVED_MSGS-1;
}

/* Do this ==once per session -- initialize from save file if avail. */
static void init_roguep() {
  fighter *rogue = sotu->roguep;
  Short i;
  /** md_malloc initializes stuff to 0 by default. **/
  rogue->armor = 0;
  rogue->weapon = 0;
  for (i = 0 ; i < 8 ; i++) {
    rogue->rings[i] = 0;
  }
  rogue->hp_current = INIT_HP;
  rogue->hp_max = INIT_HP;
  rogue->str_current = 16;
  rogue->str_max = 16;
  /*   rogue->pack = {0}; */
  rogue->gold = 0;
  rogue->exp = 1;
  rogue->exp_points = 0;
  /*   row = 0; */
  /*   col = 0; */
  rogue->fchar = '@';
  rogue->moves_left = 1250;
  player_init(); /* WAS in init.c */
}
/* player_init WAS in init.c */
/* Do this at most once per session -- not at all if restoring save */
static void player_init() {
  fighter *rogue = sotu->roguep;
  object *obj;
  //  Short i;

  rogue->pack.next_object = 0;

  obj = alloc_object(sotu);
  get_food(obj, 1);
  (void) add_to_pack(obj, &rogue->pack, 1, sotu);

  obj = alloc_object(sotu);   /* initial armor */
  obj->what_is = ARMOR;
  obj->which_kind = RING_MAIL;
  obj->class = 3; // see lib_object.c for the armor_class table.
  obj->is_protected = 0;
  obj->d_enchant = 1;
  obj->o_flags = O_METAL;
  (void) add_to_pack(obj, &rogue->pack, 1, sotu);
  do_wear(obj, rogue);

  obj = alloc_object(sotu);   /* initial weapons */
  obj->what_is = WEAPON;
  obj->which_kind = MACE;
  /* 	obj->damage = "2d3"; */
  obj->damage[0] = 2;
  obj->damage[1] = 3;
  obj->damage[2] = 0;
  obj->damage[3] = 0;
  obj->hit_enchant = obj->d_enchant = 1;
  obj->identified = 1;
  obj->o_flags |= O_METAL;
  obj->launcher = NO_LAUNCHER;
  (void) add_to_pack(obj, &rogue->pack, 1, sotu);
  do_wield(obj, rogue);

  obj = alloc_object(sotu);
  obj->what_is = WEAPON;
  obj->which_kind = BOW;
  /* 	obj->damage = "1d2"; */
  obj->damage[0] = 1;
  obj->damage[1] = 2;
  obj->damage[2] = 0;
  obj->damage[3] = 0;
  obj->hit_enchant = 1;
  obj->d_enchant = 0;
  obj->identified = 1;
  obj->launcher = NO_LAUNCHER;
  (void) add_to_pack(obj, &rogue->pack, 1, sotu);

  obj = alloc_object(sotu);
  obj->what_is = WEAPON;
  obj->which_kind = ARROW;
  obj->quantity = get_rand(25, 35);
  /* 	obj->damage = "1d2"; */
  obj->damage[0] = 1;
  obj->damage[1] = 2;
  obj->damage[2] = 0;
  obj->damage[3] = 0;
  obj->hit_enchant = 0;
  obj->d_enchant = 0;
  obj->identified = 1;
  obj->o_flags |= (O_COLLECTION | O_MISSILE);
  obj->launcher = BOW;
  (void) add_to_pack(obj, &rogue->pack, 1, sotu);

  /* To test additional potion colors / ring gems / etc */
  /*
  for (i = 0 ; i < SCROLLS ; i++) {
    obj = alloc_object(sotu);
    obj->what_is = SCROLL;
    obj->which_kind = i;
    (void) add_to_pack(obj, &rogue->pack, 1, sotu);
  }
  */
  /*
    obj = alloc_object(sotu);
    obj->what_is = SCROLL;
    obj->which_kind = AGGRAVATE_MONSTER;
    (void) add_to_pack(obj, &rogue->pack, 1, sotu);
    obj = alloc_object(sotu);
    obj->what_is = POTION;
    obj->which_kind = DETECT_MONSTER;
    (void) add_to_pack(obj, &rogue->pack, 1, sotu);
  */
}

/* init_sotu was here */
