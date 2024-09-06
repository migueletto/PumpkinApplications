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

#ifndef __ROGUEPROTO_H__
#define __ROGUEPROTO_H__
//#include "Lib/librogue.h"
#include "librogue.h"

/* main.c */
void init_game();
void make_new_level();
//void free_old_level();
//void start_rogue();

/* level.c  ok */
void make_level() SEC_2;
Boolean check_up() SEC_2;


/* object.c  ok */
void put_objects() SEC_1;
void put_amulet() SEC_1;

/* pack.c  ok */
object * pick_up(Short row, Short col, Short *status) SEC_1;
Boolean check_drop_item() SEC_1;
void drop(Short index) SEC_1;
void take_off() SEC_1;
void wear(Short index) SEC_1;
void wield(Short index) SEC_1;
void kick_into_pack() SEC_1;
void sort_pack(object *pack) SEC_2;

/* monster.c  ok */
void mv_mons() SEC_1;
void party_monsters(Short rn, Short n) SEC_1;
object * gr_monster(object * monster, Short mn) SEC_1;
void mv_aquatars() SEC_1;
void mv_monster(object *monster, Short row, Short col) SEC_1;
void wanderer() SEC_1;
void create_monster(Short mn) SEC_1;
void put_mons() SEC_1;

/* my_object.c - for initializing id_foo arrays  ok */
void alloc_id_arrays() SEC_1;
void clear_identifications() SEC_1;
void init_wiz_list(FormPtr frm, Short some_list_id, Short object_type) SEC_1;

/* Globals.c  ok */
void alloc_and_init_sotu() SEC_1;
void init_sotu() SEC_1;


/* trap.c  ok */
void trap_player(Short row, Short col) SEC_1;
void search(Short n, Boolean is_auto) SEC_1;
void id_trap(Short dir, struct state_of_the_union *sotu) SEC_1;


/* move.c  ok */
Short one_move_rogue(Short dirch, Boolean pickup, Boolean stay_ctr) SEC_1;
void multiple_move_rogue_Shift(Short dirch) SEC_1;
void multiple_move_rogue_Ctrl(Short dirch, Boolean go_speed_racer) SEC_1;
Boolean reg_move() SEC_1;
void rest(Short count) SEC_1;

/* throw.c  ok */
Boolean throw(Short dir, Short index,
	      struct state_of_the_union * sotu) SEC_2;

/* spec_hit.c  ok */
void special_hit(object * monster) SEC_1;
void rust(object *monster) SEC_1;
Boolean seek_gold(object *monster) SEC_1;
Boolean m_confuse(object *monster) SEC_1;
Boolean breath_attack(object *monster) SEC_1;
void do_breath(Short i, Short *damage) SEC_1;

/* hit.c  ok */
void check_hit_message() SEC_2;
void mon_hit(object * monster, Char * other, Short breath) SEC_2;
void rogue_hit(object *monster, Boolean force_hit) SEC_2;
void check_shrieker(object *monster) SEC_2;
void rogue_damage(Short d, object *monster) SEC_2;
Boolean mon_damage(object *monster, Short damage) SEC_2;
void fight(Short dir, Boolean to_the_death) SEC_2;

/* inventory.c  ok */
void mix_colors() SEC_1;
void make_scroll_titles() SEC_1;
void get_wand_and_ring_materials() SEC_1;

/* use.c  ok */
Boolean read_scroll(Short index, struct state_of_the_union *sotu,
		    Boolean * identify) SEC_2;
void idntfy(Short index, struct state_of_the_union *sotu) SEC_2;
void take_a_nap() SEC_2;

/* zap.c  ok */
Boolean zapp(Short dir, Short index, struct state_of_the_union * sotu) SEC_2;
void zap_monster(object *monster, UShort kind) SEC_2;
object * get_thrown_at_monster(object *obj, Short dir, Short *row, Short *col,
			       struct state_of_the_union *sotu) SEC_2;

/* chuvmey.c  ok */
void killed_by(object *monster, Short other, 
	       struct state_of_the_union * sotu) SEC_2;
void init_topten_view(FormPtr frm) SEC_2;
void create_top_ten_record() SEC_2;
void win(struct state_of_the_union *sotu) SEC_2;

/* save.c  ok */
void create_filecount_record();
Short get_file_count();
void delete_saved_rogue(Short filenum) SEC_1;
Short get_next_saverec() SEC_1;
void undisplay_saved_rogue() SEC_1;
void display_saved_rogue(Short filenum) SEC_1;
void kill_saved_rogue(UChar status, Short filenum) SEC_1;
UChar get_saved_rogue_status(Short filenum) SEC_1;
void save_character(struct state_of_the_union * sotu, Short filenum); // no sec
void load_character(struct state_of_the_union * sotu, Short filenum); // no sec
void snapshot_update_list(ListPtr lst, Boolean include_new);
void snapshot_delete_listi(FormPtr frm, Short i);
void reviv_frob_btns(FormPtr frm, Short filenum);
void snapshot_frob_btns(FormPtr frm, Boolean del_ok);
void topten_frob_btns(FormPtr frm);
void do_revivify(struct state_of_the_union * sotu);

/* chuvmey_nolib.c  ok */
Short turns_p(Short row, Short col, Short dir, UShort ** dungeon) SEC_1;
void sell_pack(struct state_of_the_union *sotu) SEC_1;
void make_room(Short rn, Short r1, Short r2, Short r3,
	       struct state_of_the_union *sotu) SEC_1;
void usr_form_init(FormPtr frm, struct state_of_the_union *sotu) SEC_1;
void usr_form_update(FormPtr frm, struct state_of_the_union *sotu) SEC_1;
void explain_it(Short x, Short y, struct state_of_the_union *sotu) SEC_1;
void calculate_warning(struct state_of_the_union *sotu) SEC_1;
void update_field_scrollers(FormPtr frm, FieldPtr fld,
			    Word up_scroller, Word down_scroller) SEC_1;
#ifndef I_AM_COLOR
void page_scroll_field(FormPtr frm, FieldPtr fld, DirectionType dir) SEC_1;
#else /* I_AM_COLOR */
void page_scroll_field(FormPtr frm, FieldPtr fld, WinDirectionType dir) SEC_1;
#endif /* I_AM_COLOR */
void get_username(struct state_of_the_union *sotu) SEC_1;
void show_dungeon(struct state_of_the_union * sotu) SEC_1;
void make_it_fit(CharPtr buf, Short avail_space, Short maxbuflen) SEC_1;
void draw_circle(Short x, Short y, Short radius, Boolean b) SEC_1;
void draw_crosshairs() SEC_2;
void invert_crosshairs() SEC_2;

/* iRogue.c */
void do_feep(Long frq, UInt dur);

/* color.c */
#ifdef I_AM_COLOR
void look_for_memo();
void look_for_tiles();
//IndexedColorType get_color(Short c);
#endif // I_AM_COLOR





#endif

