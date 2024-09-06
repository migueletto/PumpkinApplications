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
 * main.c
 *
 * Ye Olde rogue comment:
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
#include "debug.h"
#include "iRogueRsc.h"
#include "Globals.h"

#include "rogue.h"
#ifndef I_AM_OS_2
//#include "handera.h"
#endif
extern Short ScrWidth; // display.c
extern Short ScrHeight;
extern Short WalkMagic;

extern struct state_of_the_union * sotu;
extern Boolean do_make_new_level;


#define abs(a)                                  (((a) >= 0) ? (a) : (-(a)))
#define sgn(x) ( (x) >= 0 ? 1 : -1 )


// some stuff that used to be in iRogue.c:
Boolean to_the_biteme = false;
Short direction;
Short task;
Short inventory_item;
Short count = 0;
Short revivify_which = 0;
UChar revivify_p = STATUS_NORMAL; // == 0, do not revivify
Boolean whatsit_p = false;
// end of stuff
extern void LeaveForm(); // iRogue.c
extern Boolean topten_just_browsing;


/**********************************************************************
                       INIT_GAME
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Do all the allocation and initialization needed to begin a session.
 (Later, we will check for a save file, and load the character
 information from it into the allocated and initialized space.)
 **********************************************************************/
void init_game() {
  seed_random();
  alloc_and_init_sotu();
  alloc_id_arrays();
  mix_colors();
  get_wand_and_ring_materials();
}

/**********************************************************************
                       MAKE_NEW_LEVEL
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Make a new dungeon map; place objects, monsters, etc., in it.
 It is called whenever the rogue goes up/down a level (via stairs or
 trapdoor), and at the start of a session IF no save file is found
 (whatever sort of space is allocated here had better also be
 allocated by the reload function or someone will be unhappy later...)
 NOTE: This must be called AFTER the MainForm is drawn - this is
 why there is that global variable do_make_new_level kicking around.
 **********************************************************************/
/* I hope none of these allocate space that chuvmey doesn't. */
extern RoguePreferenceType my_prefs;
void make_new_level() {
  clear_level(sotu);
  make_level(); /* Illegal instruction: 100a */
  put_objects(); 
  put_stairs(sotu); 
  add_traps(sotu); 
  put_mons(); 
  put_player(sotu->party_room, sotu); 
  print_stats(STAT_ALL, sotu); 

  // aggravate!
  if (sotu->ring_flags & RING_AGGRAVATE) {
    if (sotu->ring_flags & RING_ESP)
      message("you sense hostility", sotu);
    // I should also do this when you put the ring on,  But I'm not.
    aggravate(sotu);
  }

  // Also, if we are interested in autosaving, this is the time for it.
  //  message("Autosaving...", sotu);
  if (my_prefs.autosave_on)
    save_character(sotu, 0);
}



/*****************************************************************************
 *                                                                           *
 *                start of a dungeon level                                   *
 *                                                                           *
 *****************************************************************************/
static void start_of_Play_Level_loop() {

  sotu->interrupted = false;
  /*
  if (hit_message[0]) {
    message(hit_message, 1);
    hit_message[0] = 0;
  }
  */

  if (sotu->trap_door) {
    /* not sure about this - from use of '>' */
    free_old_level(sotu);
    do_feep(300, 9);
    make_new_level();
    move_visible_window(sotu->roguep->col, sotu->roguep->row, true);
    /* end not sure */
    sotu->trap_door = 0;
    // XXX Crosshairs
    if (my_prefs.crosshairs_on) draw_crosshairs();
    return;
  }
/*    move(rogue->row, rogue->col); */
  refresh();

}



/*****************************************************************************
               Main Help Handle Cmd

  Caller had darn well better check 'c' is a reasonable character
  c = e->data.keyDown.chr    OR    c = e->data.menu.itemID - 4000
  Returns:  "handled"                           
*****************************************************************************/
static Short projectable_exists(Boolean zap, Boolean stop_at_one);
static Boolean main_help_handle_cmd(Char c) {
  //FormPtr frm;
  to_the_biteme = false;
  //frm = FrmGetActiveForm();

  check_message();

  /* Implemented:    m   d s i f F e q r z w t P R T W < >
     Not impl.:    .   ,   (requires rest(), kick_into_pack())
           also,   c
   */
  /* Removed:        d         e q r   w   P R
   */

  switch(c) {
  case '>':
  case pageDownChr:
    if (drop_check(sotu)) {
      free_old_level(sotu);
      do_feep(300, 9);
      make_new_level();
      move_visible_window(sotu->roguep->col, sotu->roguep->row, true);
    }
    return true;
  case '<':
  case pageUpChr:
    if (check_up()) {
      free_old_level(sotu);
      do_feep(400, 9);
      make_new_level();
      move_visible_window(sotu->roguep->col, sotu->roguep->row, true);
    }
    return true;
  case 'm':
    direction = NO_DIRECTION;
    task = INV_MOVE;
    FrmPopupForm(DirectionForm);
    return true;
  case 'F':
    to_the_biteme = true;
    /* FALL THROUGH to 'f' !!! */
  case 'f':
    direction = NO_DIRECTION;
    task = INV_FIGHT;
    FrmPopupForm(DirectionForm);
    return true;
  case 'i':
    /* Inventory */
    if (sotu->roguep->pack.next_object != NULL) {
      inventory_item = -1;
      task = INV_DROP;
      FrmPopupForm(InvSelectForm);
    } else {
      message("your pack is empty", sotu);
    }
    return true;
    /*
      case 'c':
      inventory_item = -1;
      task = INV_CALL;
      FrmPopupForm(InvSelectForm);
      return true;
      */
  case 's':
    search( ((count > 0) ? count : 1), false);
    count = 0;
    /*    search(1, false); */
    return true;
  case '.':
    rest( (count > 0) ? count : 1 );
    count = 0;
    /*    rest(1); */
    return true;
  case ',':
    kick_into_pack();
    return true;
  case 'Q':
    /* do an Alert-thingy to Confirm, then:
    killed_by((object *) 0, QUIT);
    */
    if (0 == FrmAlert(QuitP)) {
      killed_by((object *) 0, QUIT, sotu);
      FrmPopupForm(TopTenForm);
    }
    return true;
  case '^':
    direction = NO_DIRECTION;
    task = INV_IDTRAP;
    FrmPopupForm(DirectionForm);
    return true;
  case 't':
    if (projectable_exists(false, true)) {
      inventory_item = -1;
      direction = NO_DIRECTION;
      task = INV_THROW;
      FrmPopupForm(DirectionForm);
    } else {
      message("you have no projectile weapons", sotu);
    }
    return true; // hello, why was this false?
  case 'z':
    if (projectable_exists(true, true)) {
      inventory_item = -1;
      direction = NO_DIRECTION;
      task = INV_ZAP;
      FrmPopupForm(DirectionForm);
    } else {
      message("you have nothing to zap", sotu);
    }
    return true; // hello, why was this false?
  // re-added the following so that people can use palm keyboards :-)
  case 'h':
    one_move_rogue(WEST, true, my_prefs.stay_centered);
    return true;
  case 'j':
    one_move_rogue(SOUTH, true, my_prefs.stay_centered);
    return true;
  case 'k':
    one_move_rogue(NORTH, true, my_prefs.stay_centered);
    return true;
  case 'l':
    one_move_rogue(EAST, true, my_prefs.stay_centered);
    return true;
  case 'y':
    one_move_rogue(NORTHWEST, true, my_prefs.stay_centered);
    return true;
  case 'u':
    one_move_rogue(NORTHEAST, true, my_prefs.stay_centered);
    return true;
  case 'b':
    one_move_rogue(SOUTHWEST, true, my_prefs.stay_centered);
    return true;
  case 'n':
    one_move_rogue(SOUTHEAST, true, my_prefs.stay_centered);
    return true;
  case 'H':
    multiple_move_rogue_Ctrl(WEST, my_prefs.dave_mode);
    return true;
  case 'J':
    multiple_move_rogue_Ctrl(SOUTH, my_prefs.dave_mode);
    return true;
  case 'K':
    multiple_move_rogue_Ctrl(NORTH, my_prefs.dave_mode);
    return true;
  case 'L':
    multiple_move_rogue_Ctrl(EAST, my_prefs.dave_mode);
    return true;
  case 'Y':
    multiple_move_rogue_Ctrl(NORTHWEST, my_prefs.dave_mode);
    return true;
  case 'U':
    multiple_move_rogue_Ctrl(NORTHEAST, my_prefs.dave_mode);
    return true;
  case 'B':
    multiple_move_rogue_Ctrl(SOUTHWEST, my_prefs.dave_mode);
    return true;
  case 'N':
    multiple_move_rogue_Ctrl(SOUTHEAST, my_prefs.dave_mode);
    return true;
  }
  return false;
}


/**********************************************************************

                       MAIN FORM

**********************************************************************/


static Boolean maybe_scroll_screen(Short x, Short y, Short count);
static void rogue_relativize(Short *x, Short *y);
//void clear_crosshairs(); // display.c
extern Boolean crosshairs_dirty;
Boolean Main_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
/*   Boolean last_was_digit; */
  Short tmp_x, tmp_y, click_dir, dist;
  Boolean do_run = true;
  Boolean do_crosshairs = false;
  Char buf[40];

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* mine */
    /** place any form-initialization stuff here **/
    /** init_game(); should go somewhere ELSE. */
    /* b/c this stuff will be called when we return from a popup. ? */
    /* end mine */
#ifndef I_AM_OS_2
/*
    if (IsVGA) {
      VgaFormModify(frm, vgaFormModify160To240);
      // set height, width, etc
      update_display_size();
      adjust_form_size(true);
    }
*/
#endif
    FrmDrawForm(frm);
debug(1, "XXX", "main 1");
    clear(true); // to make color background start out right.  I hope.

    if (do_make_new_level) {
debug(1, "XXX", "main 2");
      // (startup) We did not find an autosave image in the database, or
      //     the rogue that it represents is dead, or,
      // (during play) we went up or down a dungeon level
      do_make_new_level = false;
debug(1, "XXX", "main 3");
      make_new_level();
debug(1, "XXX", "main 4");
      message("Welcome to iRogue", sotu);
debug(1, "XXX", "main 5");
    } else {
      // (startup) We found a 'live' autosave image, or
      // (during play) we were requested to load a saved rogue.
debug(1, "XXX", "main 6");
      clear_level(sotu);
debug(1, "XXX", "main 7");
      load_character(sotu, revivify_which);
debug(1, "XXX", "main 8");
      revivify_which = 0;
debug(1, "XXX", "main 9");
      if (revivify_p != STATUS_NORMAL)
	do_revivify(sotu); // inflict a penalty, maybe.
debug(1, "XXX", "main 10");
      ring_stats(true, sotu); // recalculate some calculated parts of sotu
debug(1, "XXX", "main 11");
      /* let's see if this will fly..  not bad. */
      move_visible_window(sotu->roguep->col, sotu->roguep->row, true);
debug(1, "XXX", "main 12");
      if (sotu->cur_room != PASSAGE) {
	light_up_room(sotu->cur_room, sotu);
debug(1, "XXX", "main 13");
      } else {
	light_passage(sotu->roguep->row, sotu->roguep->col, sotu);
debug(1, "XXX", "main 14");
	mvaddch(sotu->roguep->row, sotu->roguep->col, sotu->roguep->fchar);
debug(1, "XXX", "main 15");
      }
      print_stats(STAT_ALL, sotu);
debug(1, "XXX", "main 16");
      /* ok, done loading character! */
    }
    crosshairs_dirty = false;
debug(1, "XXX", "main 17");
    //    StrPrintF(buf, "Welcome to iRogue");
    // 171 = << 187 = >>
    // 249 is a pretty nice character.  also 165 and 167 (amuletish?)
    /*
    for (tmp_x = 165, tmp_y = 0 ; tmp_y < 32 ; ) {
      buf[tmp_y++] = tmp_x++;
      buf[tmp_y++] = ' ';
    }
    buf[tmp_y] = 0;
    message(buf, sotu);
    */
    //    message("Welcome to iRogue", sotu);
    start_of_Play_Level_loop();
debug(1, "XXX", "main 18");

    handled = true;
    goto SKIP_THE_REST;
    break;

#ifndef I_AM_OS_2
/*
  case displayExtentChangedEvent:
    if (IsVGA) {
      // need to clear the "old" screen first?
      // set height, width, etc
      update_display_size();
      adjust_form_size(true);
      // redraw the "new" screen
      move_visible_window(sotu->roguep->col, sotu->roguep->row, true);
      refresh(); // probably need this
      print_stats(STAT_ALL, sotu); 
      check_message();
    }
    //    handled = true;
    goto SKIP_THE_REST;
    break;
*/
#endif

  case menuEvent:
    MenuEraseStatus(NULL); /* clear menu status from display */
    // if there's a --more-- we need to show the next message
    whatsit_p = false;
    if (sotu->last_old_message_shown < SAVED_MSGS-1) {
      check_message();  // clear the old message
      message(0, sotu); // show the next buffered message
      //      message(0, sotu); // try to show a second one, but maybe fail
      handled = true;
      count = 0;
      goto SKIP_THE_REST;
    }
    switch(e->data.menu.itemID) {

    case menu_mainJump:
      // This menu item is (was) for debugging only!!!
      /*
      {
	Char buf[40];
	Coord         x, y;
	WinGetDisplayExtent(&x, &y);
	StrPrintF(buf, "%d %d", x, y);
	WinDrawChars(buf, StrLen(buf), 20, 20);
      }
      */
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainAbout:
      FrmPopupForm(AboutForm);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainMoveInstruct:
      FrmHelp(MoveStr);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainWiztog:
      handled = true;
      // IF character hasn't been a wizard before, warn / ask for confirmation.
      if (!(sotu->conduct & STATUS_WASWIZ) && !IS_WIZARD)  // if (!(sotu->score_status & STATUS_WASWIZ) && !sotu->wizard)
	if (0 == FrmAlert(WizardP))
	  goto SKIP_THE_REST; // player said "oops, cancel"
      // sotu->wizard = !sotu->wizard; sotu->score_status |= STATUS_WASWIZ;
      sotu->conduct |= CONDUCT_WASWIZ;
      if (IS_WIZARD) sotu->conduct &= ~CONDUCT_ISWIZ;
      else           sotu->conduct |=  CONDUCT_ISWIZ;
      check_message(); 
      if (IS_WIZARD) //if (sotu->wizard)
	message("wizard on", sotu);
      else
	message("wizard off", sotu);
      goto SKIP_THE_REST;
    case menu_mainWiz:
      check_message();
      if (IS_WIZARD)
	FrmPopupForm(WizForm);
      else
	message("nice try", sotu);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainAutosave:
      save_character(sotu, 0);
      message("autosaved.", sotu);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainSave:
      //      check_message();
      FrmPopupForm(SnapshotForm);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainRestore:
      //      check_message();
      FrmPopupForm(RevivifyForm);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainSettings:
      FrmPopupForm(PrefsForm);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainBindings:
      FrmPopupForm(HwButtonsForm);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainGraffiti:
      SysGraffitiReferenceDialog(referenceDefault);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainWhatsit:
      check_message();
      message("tap on a character for clarification", sotu);
      whatsit_p = true;
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainFont:
      if (toggle_itsy())
	move_visible_window(sotu->roguep->col, sotu->roguep->row, true);
      else
	FrmAlert(FontAlert);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainRefresh:
      // Redraw the screen, centered; and the stats; also clear the message
      move_visible_window(sotu->roguep->col, sotu->roguep->row, true);
      print_stats(STAT_ALL, sotu); 
      check_message();
      // XXX Crosshairs
      //      if (my_prefs.crosshairs_on) draw_crosshairs();
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainMap:
      FrmPopupForm(MapForm);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainMsgs:
      FrmPopupForm(MsgLogForm);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainScores:
      topten_just_browsing = true;
      FrmPopupForm(TopTenForm);
      handled = true;
      goto SKIP_THE_REST;
    case menu_mainScroll:
      direction = NO_DIRECTION;
      task = SCROLL_WINDOW;
      FrmPopupForm(DirectionForm);
      handled = true;
      goto SKIP_THE_REST;
    default:
      /* this is a Monstrous Hack. */
      tmp_x = e->data.menu.itemID - MAGIC_MENU_NUMBER;
      if (tmp_x > 0 && tmp_x < 128)
	handled = main_help_handle_cmd((Char) tmp_x);
      /*      StrPrintF(buf, "you typed %c", e->data.menu.itemID - 4000);
	      message(buf, sotu); */
      count = 0;
    }
    handled = true;
    break;

  case keyDownEvent:
    // if there's a --more-- we need to show the next message
    whatsit_p = false;
    if (sotu->last_old_message_shown < SAVED_MSGS-1) {
      check_message();  // clear the old message
      message(0, sotu); // show the next buffered message
      //      message(0, sotu); // try to show a second one, but maybe fail
      handled = true;
      count = 0;
      goto SKIP_THE_REST;
    }
    /* graffiti or up/down hardware button. */
    /* I farmed this out so that the Monstrous Hack menu can use it too. */    
    handled = main_help_handle_cmd(e->data.keyDown.chr);
    if (handled)
      count = 0;
    else if ('0' <= e->data.keyDown.chr
	     && e->data.keyDown.chr <= '9') {
      if (count <= 0) {
	count = e->data.keyDown.chr - '0';
      } else if (count <= 9) {
	count = count * 10 + e->data.keyDown.chr - '0';
      } else {
	count = 99; /* maxed-out */
      } // xxx Hey, I need to make "backspace" work too.
      check_message();
      StrPrintF(buf, "count: %d", count);
      message(buf, sotu);
      handled = true;
      goto SKIP_THE_REST;
    }
    break;

  case penDownEvent:
    // if there's a --more-- we need to show the next message
    if (sotu->last_old_message_shown < SAVED_MSGS-1) {
      check_message();  // clear the old message
      message(0, sotu); // show the next buffered message
      //      message(0, sotu); // try to show a second one, but maybe fail
      handled = true;
      count = 0;
      goto SKIP_THE_REST;
    }
    if (whatsit_p) {
      explain_it(e->screenX, e->screenY, sotu);
      whatsit_p = false;
      goto SKIP_THE_REST;
    }
    check_message();
    /* xxxx Rogue-relative:  need to change some stuff around here.  */
    tmp_x = e->screenX - ScrWidth/2;
    tmp_y = e->screenY - ScrHeight/2;
    // First, if scroll mode is on, AND the tap is on the edge of screen,
    // calculate the screen-relative direction,
    // and iff we can scroll any further in that direction,
    // scroll and consider yourself done (goto not-wasting-a-turn.)
    // (Let's just pull all that out into a "maybe_scroll_screen"
    // and have it return a boolean.)
    if (my_prefs.scroll_mode &&	(abs(tmp_x) > 70 || abs(tmp_y) > 70))
      if (maybe_scroll_screen(tmp_x, tmp_y, count)) {
	handled = true;
	count = 0;
	goto SKIP_THE_REST;
      }
    // Next, decide whether to be screen-relative or rogue-relative.
    if (my_prefs.rogue_relative)
      rogue_relativize(&tmp_x, &tmp_y);
    dist = tmp_x * tmp_x + tmp_y * tmp_y;
    // (rogue relative tmp_x and tmp_y may range from -160 to 160 !!!
    // but this should not matter since we got rid of scroll-mode already!)
    // Calculate the direction.
    // Decide whether to search, walk, or run.
    if (dist < my_prefs.walk_search_border * WalkMagic
               * my_prefs.walk_search_border * WalkMagic) {
      /*       message("searching", sotu);  */
      do_feep(200,9);
      search( ((count > 0) ? count : 1), false);
      handled = true;
      count = 0;
      break;
    } else if (dist < my_prefs.run_walk_border * 10
	              * my_prefs.run_walk_border * 10) {
      do_run = false;
    }
    if ((abs(tmp_x)) > 2 * (abs(tmp_y))) {
      click_dir = tmp_x > 0 ? EAST : WEST;
    } else if ((abs(tmp_y)) > 2 * (abs(tmp_x))) {
      click_dir = tmp_y > 0 ? SOUTH : NORTH;
    } else {
      if (tmp_y > 0)
	click_dir = tmp_x > 0 ? SOUTHEAST : SOUTHWEST;
      else
	click_dir = tmp_x > 0 ? NORTHEAST : NORTHWEST;	
    }
    // we have already handled the scroll-mode case.
    if (my_prefs.run_on && do_run) {
      multiple_move_rogue_Ctrl(click_dir, my_prefs.dave_mode); /*stop when interesting*/
      do_feep(200,9);
    } else {
      one_move_rogue(click_dir, true, my_prefs.stay_centered); /* tap tap tap tap */
      do_feep(200,9);
    }
    // XXX Crosshairs
    if (my_prefs.crosshairs_on) do_crosshairs = true;

    handled = true;
    count = 0;
    break;

  default:
    break;
  }
/*   last_was_digit = false; */

  start_of_Play_Level_loop();
  if (do_crosshairs) draw_crosshairs(); // cleverly we wait til after refresh()
  check_hit_message();

  /* don't do anything after this point! */
SKIP_THE_REST:

  return handled;
}





// x, y range from 70 to 80 in absolute value (but may be positive or neg.)
// they are screen-relative, and represent where you tapped.
static Boolean maybe_scroll_screen(Short tmp_x, Short tmp_y, Short count)
{
  Short scroll_dir;
  // Calculate the direction of the tap.
  if ((abs(tmp_x)) > 2 * (abs(tmp_y))) { // mostly x
    scroll_dir = tmp_x > 0 ? EAST : WEST;
  } else if ((abs(tmp_y)) > 2 * (abs(tmp_x))) { // mostly y
    scroll_dir = tmp_y > 0 ? SOUTH : NORTH;
  } else { // mostly diagonal
    if (tmp_y > 0) { // south-something
      scroll_dir = tmp_x > 0 ? SOUTHEAST : SOUTHWEST;
    } else { // north-something
      scroll_dir = tmp_x > 0 ? NORTHEAST : NORTHWEST;	
    }
  }
  // If we can scroll in that direction, then do so and return true.
  // Else return false.
  if (scroll_window(scroll_dir, max(1, count))) {
    refresh();
    return true;
  } else
    return false;
}

// Convert x,y from screen relative coordinates (ranging -80 to 80)
// to rogue-relative coordinates (ranging -160 to 160)
static void rogue_relativize(Short *x, Short *y)
{
  Short rx, ry;
  Short tap_x = *x + ScrWidth/2;
  Short tap_y = *y + ScrHeight/2;
  // first, where the heck is the rogue?
  what_x_y(sotu->roguep->row, sotu->roguep->col, &rx, &ry);
  // HOWEVER, that is one corner of the cell, and we want the "center",
  // whatever that means.  I should fix what_x_y to do that.
  *x = tap_x - rx;
  *y = tap_y - ry;
}






/* This should only be called for:
   e->eType is keyDownEvent,
   keyDown.modifiers does not include poweredOnKeyMask
*/
/*
Short hardware_dudes[8] = {
  HWB_THROW, // up ... why are throw/zap not working, I wonder.
  HWB_ZAP, // down
  HWB_NOOP, // calc
  HWB_MAP, // find
  HWB_W, // date
  HWB_N, // addr
  HWB_S, // todo
  HWB_E // memo
};
*/
Boolean buttonsHandleEvent(EventPtr e)
{
  Boolean handled = false;
  Boolean took_time = true;
  Short dispatch_type = 0;

  /*
  if (!(ChrIsHardKey(e->data.keyDown.chr))
      && (e->data.keyDown.chr != findChr)
      && (e->data.keyDown.chr != pageUpChr)
      && (e->data.keyDown.chr != pageDownChr))
    return false; // it's NOT a hardware button.. probably graffiti.
  */
  // I've decided that "if (!(ChrIsHardKey(e->data.keyDown.chr)) ..)" is 
  // not so good, since that includes hardPowerChr, hardCradleChr etc.!
  if ( ((e->data.keyDown.chr < hard1Chr) || (e->data.keyDown.chr > hard4Chr))
       && (e->data.keyDown.chr != calcChr)
       && (e->data.keyDown.chr != findChr)
       && (e->data.keyDown.chr != pageUpChr)
       && (e->data.keyDown.chr != pageDownChr))
    return false; // it's NOT a hardware button.. probably graffiti.

  // <incs>/UI/Chars.h is useful....   ChrIsHardKey(c)
  // hard[1-4]Chr, calcChr, findChr.
  switch (e->data.keyDown.chr) {
  case hard1Chr:      // datebook
    dispatch_type = my_prefs.hardware[0];
    break;
  case hard2Chr:      // address
    dispatch_type = my_prefs.hardware[1];
    break;
  case hard3Chr:      // todo
    dispatch_type = my_prefs.hardware[2];
    break;
  case hard4Chr:      // memos
    dispatch_type = my_prefs.hardware[3];
    break;
  case pageUpChr:
    dispatch_type = my_prefs.hardware[4];
    break;
  case pageDownChr:
    dispatch_type = my_prefs.hardware[5];
    break;
  case calcChr:
    dispatch_type = my_prefs.hardware[6];
    break;
  case findChr:
    dispatch_type = my_prefs.hardware[7];
    break;
  default:
    return false;
  }


  if (FrmGetActiveFormID() != MainForm) {
    // IF the key is not bound, return false, else return TRUE to MASK it.
    // unless it is up/down.
    took_time = false;
    if (FrmGetActiveFormID() == MapForm && dispatch_type != HWB_NOOP) {
      // any bound key will dismiss the map form.
      LeaveForm();
      return true;
    } else
      return ( dispatch_type != HWB_NOOP &&
	       e->data.keyDown.chr != pageUpChr &&
	       e->data.keyDown.chr != pageDownChr );
  }

  // OK, we ARE in the main form.  
  check_message();
  switch (dispatch_type) {
  case HWB_NOOP:
    return false;
    // n = 1 e = 3 s = 5 w = 7
    // x = {1 2 3 4} -> x * 2 - 1
  case HWB_N:
  case HWB_S:
  case HWB_E:
  case HWB_W:
    one_move_rogue(dispatch_type * 2 - 1, true, my_prefs.stay_centered);
    //    do_feep(200,9); // maybe take this out
    handled = true;
    break;
  case HWB_UP:
    handled = main_help_handle_cmd('<');
    break;
  case HWB_DOWN:
    handled = main_help_handle_cmd('>');
    break;
  case HWB_SEARCH:
    do_feep(200,9);
    search(10, false); // handled = main_help_handle_cmd('s');
    handled = true;
    break;
  case HWB_REST:
    //    do_feep(200,9);
    rest(10); // handled = main_help_handle_cmd('.');
    handled = true;
    break;
  case HWB_THROW:
    handled = main_help_handle_cmd('t');
    break;
  case HWB_ZAP:
    handled = main_help_handle_cmd('z');
    break;
  case HWB_INV:
    handled = main_help_handle_cmd('i');
    break;
  case HWB_MAP:
    FrmPopupForm(MapForm);
    handled = true;
    took_time = false;
    break;
  case HWB_SCROLL:
    count = 5;
    direction = NO_DIRECTION;
    task = SCROLL_WINDOW;
    FrmPopupForm(DirectionForm);
    handled = true;
    took_time = false;
    break;
  }
  // do that loop thing too.  yeah.
  if (handled && took_time) {
    count = 0;
    start_of_Play_Level_loop();
    check_hit_message();
  }
  return handled;
}





/**********************************************************************
                       INIT_DIRECTION_SELECT_LIST
 IN:
 zap = whether to check for things to 'throw' or for 'zap'
 existsp = we just want to know "are there 0 or non-zero"
 various globals
 OUT:
 number of things that are specially meant to be 'thrown' or 'zapped'
 PURPOSE:
 This is called when the direction select form's list is initialized,
 to figure out the number of items on it, and also to determine whether
 to let that form pop up at all.
 **********************************************************************/
// if (zap) return the number of zappable things in inventory
// else return the number of missile types in inventory
static Short projectable_exists(Boolean zap, Boolean stop_at_one)
{
  Char cur_ichar = 'a';
  Short numItems = 0;
  object *obj = sotu->roguep->pack.next_object;
  for ( ; obj != NULL ; obj = obj->next_object) {
    obj->ichar = cur_ichar++; /* re-order the inventory letters */
    if (cur_ichar > 'z')
      cur_ichar = 'A';
    if ( (zap && ((obj->what_is & WAND)
		  || ((obj->what_is & WEAPON)
		      && (obj->o_flags & O_ZAPPED))))
	 || ((!zap && (obj->what_is & WEAPON)
	      && (obj->o_flags & O_MISSILE)))  ) {
      if (stop_at_one) return 1;
      numItems++;
    }
  }
  return numItems;
}


/**********************************************************************
                       INIT_DIRECTION_SELECT_LIST
 IN:
 frm = the direction select form
 zap = whether to initialize a list for 'throw' or for 'zap'
 various globals
 OUT:
 nothing
 PURPOSE:
 This is called when the direction select form is initialized.
 It initializes the popup list with inventory items that are
 appropriate for "zapping" or "throwing".
 **********************************************************************/
Char ** direction_select_items;
Short direction_select_numItems;
static void init_direction_select_list(FormPtr frm, Boolean zap)
{
  ListPtr lst; // popup list: list_d
  ControlPtr ctl; // popup trigger: popup_d
  CharPtr label;
  Char buf[80];
  object *obj;
  Word i, numItems = 0;

  numItems = projectable_exists(zap, false);

  /* Hey!  Who's freeing this puppy? */
  direction_select_items = (Char **) md_malloc(sizeof(Char *) * numItems);
  for (obj = sotu->roguep->pack.next_object, i = 0 ;
       (obj != NULL && i < numItems) ;
       obj = obj->next_object) {
    if (   (zap  && ((obj->what_is & WAND)
		  || ((obj->what_is & WEAPON)
		      && (obj->o_flags & O_ZAPPED))))
	   || ((!zap && (obj->what_is & WEAPON)
		 && (obj->o_flags & O_MISSILE)))  ) {
      buf[0] = obj->ichar; 
      buf[1] = ')';
      buf[2] = ' ';
      get_desc_brief(obj, buf+3, sotu); /* er.  will it overflow 80 chars? */
      /* Insert inventory code to truncate it to a reasonable length? */
      //      make_it_fit(buf, 140, 80);
      make_it_fit(buf, ((140/2)*(ScrWidth/2))/(160/4) ,  80);

      direction_select_items[i] = md_malloc(sizeof(Char)*(StrLen(buf)+1));
      // malloc failed
      ErrNonFatalDisplayIf ( (direction_select_items[i] == NULL),
			     "XXXXXX SERIOUS EVIL");
      StrCopy(direction_select_items[i], buf);

      i++;
    }
  }

  lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_d));
  LstSetListChoices(lst, direction_select_items, numItems);
  if (numItems > 0) {
    if (numItems > 10) numItems = 10;
    LstSetHeight(lst, numItems);
  }
  LstSetSelection (lst, 0);
  ctl = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, popup_d));
  label = LstGetSelectionText (lst, 0);
  if (label) {
    if (label[0] > 'Z')
      inventory_item = label[0] - 'a';
    else
      inventory_item = label[0] - 'A';
  }
  CtlSetLabel(ctl, label);
  LstSetSelection(lst, 0);
}

/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
static void init_direction_select(FormPtr frm, Short task) {
/*   ControlPtr btn; */
  ControlPtr ctl; // popup trigger: popup_d.  Hide it by default.
  ctl = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, popup_d));
  CtlHideControl(ctl);
  switch(task) {
  case INV_ZAP:
    FrmCopyTitle(frm, "zap:  direction?");
    /* example of changing a label: */
    /*     btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_d_cancel)); */
    /*     CtlSetLabel(btn, "NO ZAP"); */
    if (inventory_item == -1)
      init_direction_select_list(frm, true);
    break;
  case INV_THROW:
    FrmCopyTitle(frm, "throw:  direction?");
    if (inventory_item == -1)
      init_direction_select_list(frm, false);
    break;
  case INV_MOVE:
    FrmCopyTitle(frm, "move:  direction?");
    break;
  case INV_FIGHT:
    FrmCopyTitle(frm, "fight:  direction?");
    break;
  case INV_IDTRAP:
    FrmCopyTitle(frm, "i.d. trap:  direction?");
    break;
  case SCROLL_WINDOW:
    FrmCopyTitle(frm, "scroll view:  direction?");
    break;
  }
}

/*****************************************************************************
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
Boolean Direction_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  ControlPtr ctl;
  CharPtr txt;
  static Boolean pick_item = false;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* mine */
    /** place any form-initialization stuff here **/
    pick_item = (inventory_item == -1) ? true : false;
    init_direction_select(frm, task);
    /* end mine */	
#ifndef I_AM_OS_2
/*
    if (IsVGA) {
      VgaFormModify(frm, vgaFormModify160To240);
      adjust_form_position(false);
    }
*/
#endif
    //   adjust_form_position(false); // temporarily ADDED for DEBUGGING

    FrmDrawForm(frm);
    if ((task == INV_ZAP || task == INV_THROW) && pick_item) {
      ctl = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, popup_d));
      CtlShowControl(ctl);
    }
    handled = true;
    break;

#ifndef I_AM_OS_2
/*
  case displayExtentChangedEvent:
    if (IsVGA) {
      update_display_size();
      // I am not sure whether adjust_form_position will work after drawn.
      // Perhaps it will if I call FrmEraseForm first?
      frm = FrmGetActiveForm();
      FrmEraseForm(frm);
      adjust_form_position(true);
      FrmDrawForm(frm);
      // Also, will I have to do that CtlShowControl again after? or not?
    }
    break;
*/
#endif

  case popSelectEvent:
    frm = FrmGetActiveForm();
    if (e->data.popSelect.listID == list_d) {
      if (pick_item && (task == INV_ZAP || task == INV_THROW)) {
	lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_d));
	txt = LstGetSelectionText (lst, e->data.popSelect.selection);
	if (txt[0]) {
	  inventory_item = txt[0] - 'a'; // dangerous
	} else {
	  inventory_item = -1;
	}
      }
    }
    /*      handled = true; */
    break;

  case ctlSelectEvent:
    direction = -1;
    switch(e->data.ctlSelect.controlID) {
    case btn_d_n:
      direction = NORTH;
      break;
    case btn_d_ne:
      direction = NORTHEAST;
      break;
    case btn_d_e:
      direction = EAST;
      break;
    case btn_d_se:
      direction = SOUTHEAST;
      break;
    case btn_d_s:
      direction = SOUTH;
      break;
    case btn_d_sw:
      direction = SOUTHWEST;
      break;
    case btn_d_w:
      direction = WEST;
      break;
    case btn_d_nw:
      direction = NORTHWEST;
      break;
    case btn_d_cancel:
      direction = NO_DIRECTION;
      break;
    default:
      break;
    }
    if (direction != -1) {
      LeaveForm();
      if (direction != NO_DIRECTION) {
	switch(task) {
	  /*
	    case INV_ZAP:
	    case INV_THROW:
	    inventory_item = -1;
	    FrmPopupForm(InvSelectForm);
	    break;
	  */
	case INV_ZAP:
	  /* inventory_item should be set already */
	  if (inventory_item != -1) {
	    if (zapp(direction, inventory_item, sotu))
	      reg_move();
	  }
	  break;
	case INV_THROW:
	  /* inventory_item should be set already */
	  if (inventory_item != -1) {
	    if (throw(direction, inventory_item, sotu))
	      reg_move();
	  }
	  break;
	case INV_MOVE:
	  /* replacing move_onto() */
	  one_move_rogue(direction, false, my_prefs.stay_centered);
	  break;
	case INV_FIGHT:
	  fight(direction, to_the_biteme); /* true = to-the-death */
	  break;
	case INV_IDTRAP:
	  id_trap(direction, sotu);
	  break;
	case SCROLL_WINDOW:
	  scroll_window(direction, max(1, count));
	  count = 0;
	  break;
	}
      }
      handled = true;
    }
    break;

  default:
    break;
  }
    
  return handled;
}
