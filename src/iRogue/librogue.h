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

// multisection code
#include "sections.h"

#ifndef __LIBROGUE_H__
#define __LIBROGUE_H__

//#include "../rogue_defines.h"
#include "rogue_defines.h"

/* random.c  ok */
void seed_random() SEC_L;
Int get_rand(Int x, Int y) SEC_L;
Boolean rand_percent(Int percentage) SEC_L;
Boolean coin_toss() SEC_L;
void rand_around(Short i, Short *r, Short *c) SEC_L;

/* hit.c  ok */
Short get_damage(Char ds[4], Boolean r) SEC_L;
Short get_hit_chance(object *weapon, Boolean thrown,
		     struct state_of_the_union * sotu) SEC_L;
Short get_weapon_damage(object *weapon, Boolean thrown,
			struct state_of_the_union * sotu) SEC_L;
void get_dir_rc(Short dir, Short *row, Short *col, 
		Boolean allow_off_screen) SEC_L;

/* level.c  ok */
void put_door(room * rm, Short dir, Short * row, Short * col,
	      struct state_of_the_union * sotu) SEC_L;
Boolean mask_room(Short rn, Short * row, Short * col, UShort mask,
	      struct state_of_the_union * sotu) SEC_L;
void hide_boxed_passage(Short row1, Short col1,
			Short row2, Short col2, 
			Short n,
			struct state_of_the_union * sotu) SEC_L;
void draw_simple_passage(Short row1, Short col1, 
			 Short row2, Short col2,
			 Short dir,
			 struct state_of_the_union * sotu) SEC_L;
Boolean drop_check(struct state_of_the_union * sotu) SEC_L;
Short hp_raise(struct state_of_the_union * sotu) SEC_L;
Boolean connect_rooms(Short room1, Short room2, 
		      struct state_of_the_union * sotu) SEC_L;
void clear_level(struct state_of_the_union * sotu) SEC_L;
void put_player(Short nr, struct state_of_the_union *sotu) SEC_L;
void add_exp(Int e, Boolean promotion, struct state_of_the_union *sotu) SEC_L;

/* my_message.c  ok */
void check_message() SEC_L;
void message(Char * msg, struct state_of_the_union * sotu) SEC_L;
void print_stats(UInt stat_mask, struct state_of_the_union * sotu) SEC_L;


/* display.c  ok */
void init_display_size();
void update_display_size();
Short adjust_form_size(Boolean draw);
Short adjust_form_position(Boolean draw);
Boolean toggle_itsy() SEC_L;
void check_rogue_position(fighter * rogue, Boolean centered) SEC_L;
void move_visible_window(Short left_x, Short top_y, Boolean centered) SEC_L;
Boolean scroll_window(Short direction, Short steps) SEC_L;
void save_vbuffer(MemPtr p, Int *offsetp, Err *errorp) SEC_L;
void mvaddch(Short row, Short col, Short ch) SEC_L;
void refresh() SEC_L;
Short mvinch(Short row, Short col) SEC_L;
void clear(Boolean all) SEC_L;
/* break abstraction barrier for map of dungeon */
Char peek_me(Short row, Short col) SEC_L;
Short what_row_col(Short screen_x, Short screen_y, Short *rowp, Short *colp) SEC_L;
void what_x_y(Short row, Short col, Short *x, Short *y) SEC_L;

/* monster.c  ok */
void copy_mon_name(Short index, Char *foo) SEC_L;
void print_mon_name(Short index, struct state_of_the_union *sotu) SEC_L;
void mon_name(object *monster, Char *foo, struct state_of_the_union *sotu) SEC_L;
Boolean rogue_is_around(Short row, Short col, fighter *rogue) SEC_L;
void wake_up(object *monster) SEC_L;
Char gr_obj_char() SEC_L;
Boolean mon_sees(object *monster, Short row, Short col, room * rooms) SEC_L;
Char gmc(object *monster, struct state_of_the_union * sotu) SEC_L;
void aim_monster(object *monster, room * rooms) SEC_L; /* only used one place */
Boolean rogue_can_see(Short row, Short col, struct state_of_the_union * sotu) SEC_L;
Boolean no_room_for_monster(Short rn, room * rooms, UShort ** dungeon) SEC_L;
Boolean mon_can_go(object *monster, Short row, Short col,
		   struct state_of_the_union * sotu) SEC_L;
void wake_room(Short rn, Boolean entering, Short row, Short col,
	       struct state_of_the_union *sotu) SEC_L;
void show_monsters(struct state_of_the_union *sotu) SEC_L;
void aggravate(struct state_of_the_union *sotu) SEC_L;
Char gmc_row_col(Short row, Short col, struct state_of_the_union *sotu) SEC_L;
void move_mon_to(object *monster, Short row, Short col,
		 struct state_of_the_union *sotu) SEC_L;
Boolean mtry(object *monster, Short row, Short col,
	     struct state_of_the_union *sotu) SEC_L;
Boolean move_confused(object *monster, struct state_of_the_union *sotu) SEC_L;
Boolean flit(object *monster, struct state_of_the_union *sotu) SEC_L;

/* room.c  ok */
Short get_room_number(Short row, Short col, room * rooms) SEC_L;
Char get_mask_char(UShort mask) SEC_L;
Short gr_room(room * rooms) SEC_L;
void dr_course(object *monster, Boolean entering, Short row, Short col,
	       struct state_of_the_union * sotu) SEC_L;
Char get_dungeon_char(Short row, Short col,
		      struct state_of_the_union * sotu) SEC_L;
void light_up_room(Short rn, struct state_of_the_union *sotu) SEC_L;
void light_passage(Short row, Short col, struct state_of_the_union *sotu) SEC_L;
void darken_room(Short rn, struct state_of_the_union *sotu) SEC_L;
void gr_row_col(Short *row, Short *col, UShort mask,
		struct state_of_the_union *sotu) SEC_L;
void draw_magic_map(struct state_of_the_union *sotu) SEC_L; /* YAY! */
Short party_objects(Short rn, struct state_of_the_union *sotu) SEC_L;

/* move.c  ok */
Boolean can_move(Short row1, Short col1, Short row2, Short col2,
		 UShort ** dungeon) SEC_L;
Boolean is_passable(Short row, Short col, UShort ** dungeon) SEC_L;
Boolean next_to_something(Short drow, Short dcol,
			  struct state_of_the_union * sotu) SEC_L;

/* object.c  ok */
object * object_at(object *pack, Short row, Short col) SEC_L;
Boolean is_vowel(Short ch) SEC_L;
struct id * get_id_table(object *obj, struct state_of_the_union * sotu) SEC_L;
Short get_armor_class(struct state_of_the_union * sotu) SEC_L;
void free_object(object *obj, struct state_of_the_union * sotu) SEC_L;
object * alloc_object(struct state_of_the_union * sotu) SEC_L;
void put_gold(struct state_of_the_union *sotu) SEC_L; 
void place_at(object * obj, Short row, Short col,
	      struct state_of_the_union *sotu) SEC_L;
void free_stuff(object *objlist, struct state_of_the_union *sotu) SEC_L;
void free_old_level(struct state_of_the_union *sotu) SEC_L;
void put_stairs(struct state_of_the_union *sotu) SEC_L;
void get_food(object *obj, Boolean force_ration) SEC_L;
object * gr_object(struct state_of_the_union *sotu) SEC_L;
object * get_nth_object(Short n, object *pack) SEC_L; /* I made this */
void new_object_for_wizard(Short chtype, Short whatkind,
			   struct state_of_the_union *sotu) SEC_L;
void copy_name_of(object *obj, Char *foo,
		  struct state_of_the_union *sotu) SEC_L;
void get_desc(object *obj, Char *desc, struct state_of_the_union *sotu) SEC_L;
void get_desc_brief(object *obj, Char *desc, struct state_of_the_union *sotu) SEC_L;

void flop_weapon(object *weapon, Short row, Short col,
		 struct state_of_the_union *sotu) SEC_L; /* was in throw.c */
Boolean eat(Short index, struct state_of_the_union *sotu) SEC_L;
void drink_monblood(object *monster, struct state_of_the_union *sotu) SEC_L;

/* pack.c  ok */
void take_from_pack(object *obj, object *pack) SEC_L;
Boolean mask_pack(object *pack, UShort mask) SEC_L;
object * add_to_pack(object *obj, object *pack, Boolean condense,
		     struct state_of_the_union * sotu) SEC_L;
void unwear(object *obj, fighter *rogue) SEC_L;
void do_wear(object *obj, fighter *rogue) SEC_L;
void do_wield(object *obj, fighter *rogue) SEC_L;
void unwield(object *obj, fighter *rogue) SEC_L;
Short pack_count(object *new_obj, fighter *rogue, Short *weight) SEC_L;
Short max_weight(struct state_of_the_union * sotu) SEC_L;

/* spec_hit.c  ok */
void get_closer(Short *row, Short *col, Short trow, Short tcol) SEC_L;
Boolean imitating(Short row, Short col, struct state_of_the_union * sotu) SEC_L;
void disappear(object *monster, struct state_of_the_union *sotu) SEC_L;
void cough_up(object *monster, struct state_of_the_union *sotu) SEC_L;
void check_gold_seeker(object *monster) SEC_L;
Boolean check_imitator(object *monster, struct state_of_the_union *sotu) SEC_L;

/* machdep.c  ok */
Char * md_malloc(Int n) SEC_L;

/* traps.c  ok */
Short trap_at(Short row, Short col, struct state_of_the_union *sotu) SEC_L;
void add_traps(struct state_of_the_union *sotu) SEC_L;
void show_traps(struct state_of_the_union *sotu) SEC_L;

/* use.c  ok */
void hold_monster(struct state_of_the_union *sotu) SEC_L;
void tele(struct state_of_the_union *sotu) SEC_L;
void hallucinate(struct state_of_the_union *sotu) SEC_L;
void unhallucinate(struct state_of_the_union *sotu) SEC_L;
void unblind(struct state_of_the_union *sotu) SEC_L;
void relight(struct state_of_the_union *sotu) SEC_L;
void go_blind(struct state_of_the_union *sotu) SEC_L;
void confuse(struct state_of_the_union *sotu) SEC_L;
void unconfuse(struct state_of_the_union *sotu) SEC_L;
void uncurse_all(struct state_of_the_union *sotu) SEC_L;
Char * get_ench_color(struct state_of_the_union *sotu) SEC_L;
void vanish(object *obj, object *pack, struct state_of_the_union *sotu) SEC_L;
Boolean quaff(Short index, struct state_of_the_union *sotu) SEC_L;

/* zap.c  ok */
object * get_zapped_monster(Short dir, Short *row, Short *col,
			    Short ch, Boolean long_zap,
			    struct state_of_the_union *sotu) SEC_L;
void tele_away(object *monster, struct state_of_the_union *sotu) SEC_L;

/* ring.c  ok */
void ring_stats(Boolean pr, struct state_of_the_union *sotu) SEC_L;
void un_put_on(object *ring, struct state_of_the_union *sotu) SEC_L;
Boolean remove_ring(Short index, struct state_of_the_union *sotu) SEC_L;
Boolean put_on_ring(Short index, Short finger_index, 
		    struct state_of_the_union *sotu) SEC_L;



// win.c
void qWinEraseRectangle(RectanglePtr r, Word cornerDiam) SEC_2;
void qWinDrawLine(short x1, short y1, short x2, short y2) SEC_2;
void qWinDrawChars(CharPtr chars, Word len, SWord x, SWord y) SEC_2;

#endif

