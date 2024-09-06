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
#include "debug.h"
#include "Graffiti.h" /* just for GrfGetState */
//#include <CharAttr.h> /* to keep 3.1 happy about ChrIsHardKey */
#ifndef I_AM_COLOR
#include <System/Globals.h> // for font munging
#else /* I_AM_COLOR */
// hm, do I need something?
#endif /* I_AM_COLOR */
#include "iRogueRsc.h"
#include "Globals.h"
//extern DmOpenRef       RogueDB;
DmOpenRef RogueDB = NULL;
DmOpenRef RogueTileDB = NULL;

#include "rogue.h"
#ifndef I_AM_OS_2
//#include "handera.h"
#endif
extern Short ScrWidth; // display.c
extern Short ScrHeight;
extern Short WalkMagic;


// declarations
Boolean buttonsHandleEvent(EventPtr e); // now in main.c
void           do_feep(Long frq, UInt dur);
//static void    start_of_Play_Level_loop(); // now in main.c
//static Boolean main_help_handle_cmd(Char c); // now in main.c
Boolean        Main_Form_HandleEvent(EventPtr e); // now in main.c
extern void    LeaveForm ();
//static void    init_direction_select_list(FormPtr frm, Boolean zap); //main.c
//static void    init_direction_select(FormPtr frm, Short task); //main.c
Boolean        Direction_Form_HandleEvent(EventPtr e); // now in main.c
Boolean        MsgLog_Form_HandleEvent(EventPtr e);
Boolean        InvSelect_Form_HandleEvent(EventPtr e); // now inventory.c
Boolean        Wiz_Form_HandleEvent(EventPtr e);
Boolean        Prefs_Form_HandleEvent(EventPtr e);
Boolean        About_Form_HandleEvent(EventPtr e);
Boolean        Map_Form_HandleEvent(EventPtr e);
Boolean        HwButtons_Form_HandleEvent(EventPtr e);
Boolean        TopTen_Form_HandleEvent(EventPtr e);
static Boolean OpenDatabase(void);
static Boolean ApplicationHandleEvent(EventPtr e);
static void    readRoguePrefs();
static void    writeRoguePrefs();
static Word    StartApplication(void);
static void    free_everything();
static void    StopApplication(void);
static void    EventLoop(void);
DWord          PilotMain(Word cmd, MemPtr cmdPBP, Word launchFlags);
// end

extern struct state_of_the_union * sotu;
Short x, y;
extern Short inventory_item; // main.c; used here lots

RoguePreferenceType my_prefs = {
  true, // run_on
  true, // formerly save_on_exit, now autosave - in 0.42
  4, // run_walk_border
  1, // walk_search_border
  false, // dave_mode
  false, // scroll_mode
  true, // sound_on
  false, // use hardware
  {0, 0, 0, 0, // the hardware buttons
   0, 0, 0, 0},
  false, // stay centered
  false, // use large (normal) font - new in 0.42
  false, // use rogue-relative movement - new in 0.42
  //
  0, // color on - new in 0.45 - changed to UChar in 0.47
  false, // crosshairs on - ditto
  false  // sort pack - new in 0.46
};
/*
Boolean run_on = true;
Boolean save_on_exit = true;
Short run_walk_border = 4;
Short walk_search_border = 1;
Boolean dave_mode = false;
Boolean scroll_mode = false;
*/


Boolean inventory_not_messages = true;

Boolean do_make_new_level = false;
Boolean save_me = true;
Boolean i_am_dead = false;
extern UChar revivify_p; // main.c
extern Short revivify_which; // main.c


/* define INV_foo MOVED to rogue_defines.h */

/**********************************************************************
                       DO_FEEP
 IN:
 frq = frequency
 dur = duration
 OUT:
 nothing
 PURPOSE:
 This will produce a beep of the requested frequency and duration,
 if the pilot owner has "game sound" turned on in the Preferences app.
 (Try to keep it down to chirps...)
 **********************************************************************/
/* SoundLevelType: slOn, slOff
   gameSoundLevel found in SystemPreferencesType
 */
void do_feep(Long frq, UInt dur) {
  SystemPreferencesChoice allgamesound;

  if (!my_prefs.sound_on)
    return;

#ifdef I_AM_OS_2
  allgamesound = prefGameSoundLevel;
#else
  allgamesound = prefGameSoundLevelV20;
#endif

  if (PrefGetPreference(allgamesound) != slOff) {
    /* click: 200, 9
       confirmation: 500, 70  */
    SndCommandType sndCmd;
    sndCmd.cmd = sndCmdFreqDurationAmp; /* "play a sound" */
    sndCmd.param1 = frq; /* frequency in Hz */
    sndCmd.param2 = dur; /* duration in milliseconds */
    sndCmd.param3 = sndDefaultAmp; /* amplitude (0 to sndMaxAmp) */
    SndDoCmd(0, &sndCmd, true);
  }
}


/* how to exit a popup form that you can enter from more than one place */
extern void LeaveForm()
{
   FormPtr frm;
   frm = FrmGetActiveForm();
   FrmEraseForm (frm);
   FrmDeleteForm (frm);
   FrmSetActiveForm (FrmGetFirstForm ());
}

/***********************************************************************
 * swiped from HandEra example...
 *
 * FUNCTION:    PrvMoveObject
 *
 * DESCRIPTION: This routine moves an object vertically within a form.
 *
 * PARAMETERS:  
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
#ifndef I_AM_OS_2
void PrvMoveObject(FormPtr frmP, UInt objid, Coord y_diff, Boolean draw)
{
    RectangleType r;
    UInt objIndex = FrmGetObjectIndex(frmP, objid);
    if (y_diff == 0) return;    

    FrmGetObjectBounds(frmP, objIndex, &r);
    if (draw)
    {
        RctInsetRectangle(&r, -2);   //need to erase the frame as well
        WinEraseRectangle(&r, 0);
        RctInsetRectangle(&r, 2);
    }    
    r.topLeft.y += y_diff;
    FrmSetObjectBounds(frmP, objIndex, &r);
}
void PrvResizeList(FormPtr frm, UInt objid, UInt bot_objid, Coord y_diff)
{
  // Wonder if I need to erase the old space.
  Short lines, x, y, y_bot;
  ListPtr lst;
  UInt objIndex = FrmGetObjectIndex(frm, objid);
  lst = FrmGetObjectPtr(frm, objIndex);
  // Ok, the lists I'm calling this on all are positioned just below the
  // title bar, and end just above a single row of buttons, one of which
  // is supplied in bot_objid.  Make the list as big as possible!
  //
  // In 160x160 these (x, y)'s start out at (0, 15) and (5, 141).
  // We really only care about the y's.
  FrmGetObjectPosition(frm, objIndex, &x, &y);
  FrmGetObjectPosition(frm, FrmGetObjectIndex(frm, bot_objid), &x, &y_bot);
  // Ok.  y_bot - y is the absolute max space available to the list.
  // Figure out how many lines we can put into that space (yay int division).
  lines = (y_bot - y) / FntLineHeight();
  LstSetHeight(lst, lines);
  /*
    // This did not work:  ... maybe because I had objid instead of objIndex!
  FrmGetObjectBounds(frm, objIndex, &r);
  h = FntLineHeight();
  vis_items = r.extent.y / h; // calculate height INCLUDING blank slots thanks!
  vis_items = vis_items + y_diff / h; // add (or subtract) change
  // Hey - if subtracting - will int division work right, or fencepost?
  LstSetHeight(lst, vis_items);
  */
}
void PrvResizeField(FormPtr frm, UInt objid, Coord y_diff)
{
  // Wonder if I need to erase the old space.  Probably I DO.
  RectangleType r;
  Short h;
  FieldPtr fld;
  UInt objIndex = FrmGetObjectIndex(frm, objid);
  if (y_diff == 0) return;
  fld = FrmGetObjectPtr(frm, objIndex);
  FldGetBounds(fld, &r); // same as using FrmGetObjectBounds, I b'lieve
  h = FntLineHeight();
  r.extent.y = r.extent.y + h * (y_diff / h); // could just add y_diff...
  FldSetBounds(fld, &r);
  // Hmmm.  It says "don't change the WIDTH of the object while it's visible"
}
#endif

static void init_msglog_view(FormPtr frm,
          struct state_of_the_union *sotu) SEC_1;

Boolean MsgLog_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  FieldPtr fld;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* mine */
    /** place any form-initialization stuff here **/
    init_msglog_view(frm, /*inventory_not_messages,*/ sotu);
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_iv));
    update_field_scrollers(frm, fld, repeat_iv_up, repeat_iv_down);
    /*if (!inventory_not_messages)
      FrmCopyTitle(frm, "Message Log");*/
    /* end mine */  
#ifndef I_AM_OS_2
/*
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(false);
      // move buttons
      PrvMoveObject(frm, btn_iv_ok, y_diff, false);
      PrvMoveObject(frm, repeat_iv_up, y_diff, false);
      PrvMoveObject(frm, repeat_iv_down, y_diff,false);
      PrvResizeField(frm, field_iv, y_diff);
      update_field_scrollers(frm, fld, repeat_iv_up, repeat_iv_down);
    }
*/
#endif
    FrmDrawForm(frm);
    handled = true;
    break;


#ifndef I_AM_OS_2
/*
  case displayExtentChangedEvent:
    if (IsVGA) {
      Short y_diff;
      update_display_size();
      y_diff = adjust_form_size(true);
      // redraw the "new" screen too...  don't ask me whether this will work
      frm = FrmGetActiveForm();
      // move buttons
      PrvMoveObject(frm, btn_iv_ok, y_diff, true);
      PrvMoveObject(frm, repeat_iv_up, y_diff, true);
      PrvMoveObject(frm, repeat_iv_down, y_diff, true);
      PrvResizeField(frm, field_iv, y_diff);
      fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_iv));
      update_field_scrollers(frm, fld, repeat_iv_up, repeat_iv_down);
      FrmDrawForm(frm);
    }
    break;
*/
#endif

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_iv_ok:
      LeaveForm();
      handled = true;
      break;
    }
    break;

  case ctlRepeatEvent:
    /*     "Repeating controls don't repeat if handled is set true." */
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_iv));
    switch(e->data.ctlRepeat.controlID) {
    case repeat_iv_up:
      FldScrollField(fld, 1, winUp);
      update_field_scrollers(frm, fld, repeat_iv_up, repeat_iv_down);
      break;
    case repeat_iv_down:
      FldScrollField(fld, 1, winDown);
      update_field_scrollers(frm, fld, repeat_iv_up, repeat_iv_down);
      break;
    }
    break;

  case keyDownEvent:
    /* hardware button -- or else graffiti. */
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_iv));
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      page_scroll_field(frm, fld, winUp);
      update_field_scrollers(frm, fld, repeat_iv_up, repeat_iv_down);
      handled = true;
      break;
    case pageDownChr:
      page_scroll_field(frm, fld, winDown);
      update_field_scrollers(frm, fld, repeat_iv_up, repeat_iv_down);
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}

/**********************************************************************
                       INIT_msglog_VIEW
 IN:
 frm = a form that has a text-view widget
 various globals
 OUT:
 nothing
 PURPOSE:
 Initializes the displayed form 'frm' with
 a list of the last SAVED_MSGS messages.
 Called when frm is being initialized.
 **********************************************************************/
extern VoidHand view_TextHandle; // in inventory.c
/* possibly not large enough */
#define BIG_NUMBER 2048
static void init_msglog_view(FormPtr frm, struct state_of_the_union *sotu)
{
  FieldPtr fld;
  CharPtr txtP;
  Short i;

  /* Get the text field 'field_iv' */
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_iv));

  /* not sure whether to try to free old handles */
  if (view_TextHandle != NULL) {
    /* MemHandleFree(view_TextHandle); // bus error.. erk.  leak? */
  }

  /* Create a mem. handle and lock it */
  view_TextHandle = MemHandleNew(BIG_NUMBER);
  txtP = MemHandleLock(view_TextHandle);

  /* Use MemMove and/or MemSet to copy text to the mem. handle */
  for (i = 0 ; i < SAVED_MSGS ; i++) {
    MemMove(txtP, sotu->old_messages[i], StrLen(sotu->old_messages[i])+1);
    txtP += StrLen(sotu->old_messages[i]);
  }

  /* Unlock the handle.  Set the field to display the handle's text. */
  MemHandleUnlock(view_TextHandle);
  FldSetTextHandle(fld, (MemHandle) view_TextHandle);

  /* update scrollers... oo er. */
}
#undef BIG_NUMBER


// in my_object.c
Short wiz_form_object_type = -1;
Boolean Wiz_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Short whatkind = -1, lvis;
  ListPtr lst;
  ControlPtr btn;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    init_wiz_list(frm, list_wiza, -1);
    wiz_form_object_type = -1;
#ifndef I_AM_OS_2
/*
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(false);
      // move buttons
      PrvMoveObject(frm, btn_wiz_ok, y_diff, false);
      PrvMoveObject(frm, btn_wiz_cancel, y_diff,false);
      PrvResizeList(frm, list_wiza, btn_wiz_ok, y_diff);
    }
*/
#endif
    FrmDrawForm(frm);
    handled = true;
    break;


#ifndef I_AM_OS_2
/*
  case displayExtentChangedEvent:
    if (IsVGA) {
      Short y_diff;
      update_display_size();
      y_diff = adjust_form_size(true);
      // redraw the "new" screen too...  don't ask me whether this will work
      frm = FrmGetActiveForm();
      // move buttons
      PrvMoveObject(frm, btn_wiz_ok, y_diff, true);
      PrvMoveObject(frm, btn_wiz_cancel, y_diff, true);
      PrvResizeList(frm, list_wiza, btn_wiz_ok, y_diff);
      FrmDrawForm(frm);
    }
    break;
*/
#endif

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_wiz_ok:
      if (wiz_form_object_type != -1) {
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_wiza));
  whatkind = LstGetSelection(lst);
      }
      LeaveForm();
      /* It is important to do this after LeaveForm for msg to display */
      if (wiz_form_object_type != -1 && whatkind != -1) {
  if (wiz_form_object_type > 7) {
    whatkind += (wiz_form_object_type > 8 ? 1 : 27);
    create_monster(whatkind);
  } else {
    new_object_for_wizard(wiz_form_object_type, whatkind, sotu);
  }
      }
      handled = true;
      break;
    case btn_wiz_cancel:
      LeaveForm();
      handled = true;
      break;
    }
    break;

  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_wiza));
    lvis = LstGetVisibleItems(lst) - 1;
    if (lvis < 10) lvis = 10; // minimum of 11 items visible at a time
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      if (LstScrollList(lst, winUp, lvis))
  LstSetSelection(lst, -1);
      //  wiz_form_object_type = -1;
      handled = true;
      break;
    case pageDownChr:
      if (LstScrollList(lst, winDown, lvis))
  LstSetSelection(lst, -1);
      //  wiz_form_object_type = -1;
      handled = true;
      break;
    }
    break;

  case lstSelectEvent:
    if (e->data.lstSelect.listID == list_wiza) {
      if (wiz_form_object_type == -1) {
  wiz_form_object_type = e->data.lstSelect.selection;
  if (wiz_form_object_type != -1) {
    frm = FrmGetActiveForm();
    init_wiz_list(frm, list_wiza, wiz_form_object_type);
#ifndef I_AM_OS_2
/*
    if (IsVGA) // because number of items have changed...
      PrvResizeList(frm, list_wiza, btn_wiz_ok, 0);
*/
#endif
    FrmDrawForm(frm);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_wiz_ok));
    CtlShowControl(btn);    
    handled = true;
  }
      }
    }
    break;


  default:
    break;
  }

  return handled;
}

void sort_pack(object *pack);
extern Boolean have_read_memo; // in color.c
Boolean Prefs_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  ControlPtr checkbox;
  static Short rw = 0;
  static Short ws = 0;
  Boolean redraw = false;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    rw = max(1, my_prefs.run_walk_border - my_prefs.walk_search_border);
    ws = max(1, my_prefs.walk_search_border);
    /* and checkboxes */
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_1));
    CtlSetValue(checkbox, (my_prefs.run_on ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_2));
    CtlSetValue(checkbox, (my_prefs.autosave_on ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_3));
    CtlSetValue(checkbox, (my_prefs.dave_mode ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_4));
    CtlSetValue(checkbox, (my_prefs.scroll_mode ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_5));
    CtlSetValue(checkbox, (my_prefs.sound_on ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_6));
    CtlSetValue(checkbox, (my_prefs.stay_centered ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_7));
    CtlSetValue(checkbox, (my_prefs.font_small ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_8));
    CtlSetValue(checkbox, (my_prefs.rogue_relative ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_9));
    CtlSetValue(checkbox, (my_prefs.crosshairs_on ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_11));
    CtlSetValue(checkbox, (my_prefs.sort_pack ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_12));
    CtlSetValue(checkbox, (my_prefs.black_bg) ? 1 : 0);
    /* set player name */
    usr_form_init(frm, sotu);
#ifndef I_AM_OS_2
/*
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(false);
      // move buttons
      PrvMoveObject(frm, btn_bul_ok, y_diff, false);
      PrvMoveObject(frm, btn_bul_cancel, y_diff,false);
    }
*/
#endif
    FrmDrawForm(frm);
#ifdef I_AM_COLOR
    {
      DWord version;
      FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
      if (version >= 0x03503000L) {
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_10));
  CtlSetValue(checkbox, (my_prefs.color_on) ? 1 : 0);
  CtlShowControl(checkbox);
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_13));
  CtlSetValue(checkbox, (my_prefs.tiles_on) ? 1 : 0);
  CtlShowControl(checkbox);
      }
    }
#endif // COLOR
    /* set initial settings for lists */
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_bul_1));
    LstSetSelection(lst, ws-1);
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_bul_2));
    LstSetSelection(lst, rw-1);
    handled = true;
    break;


#ifndef I_AM_OS_2
/*
  case displayExtentChangedEvent:
    if (IsVGA) {
      Short y_diff;
      update_display_size();
      y_diff = adjust_form_size(true);
      // redraw the "new" screen too...  don't ask me whether this will work
      frm = FrmGetActiveForm();
      // move buttons
      PrvMoveObject(frm, btn_bul_ok, y_diff, true);
      PrvMoveObject(frm, btn_bul_cancel, y_diff, true);
      FrmDrawForm(frm);
    }
    break;
*/
#endif

  case lstSelectEvent:
    switch (e->data.lstSelect.listID) {
    case list_bul_1:
      ws = max(1, 1 + e->data.lstSelect.selection);      /* "hole" */
      break;
    case list_bul_2:
      rw = max(1, 1 + e->data.lstSelect.selection);      /* "donut" */
      break;
    }


    handled = true;
    break;
  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
      /*     case btn_bul_save: */
      /*       handled = true; */
      /*       break; */
    case btn_bul_ok:
      my_prefs.run_walk_border = rw+ws;
      my_prefs.walk_search_border = ws;
      /* read run/walk checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_1));
      my_prefs.run_on = (CtlGetValue(checkbox) != 0);  
      /* read save/don't checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_2));
      my_prefs.autosave_on = (CtlGetValue(checkbox) != 0);  
      /* read dave-mode checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_3));
      my_prefs.dave_mode = (CtlGetValue(checkbox) != 0);  
      /* read scroll-mode checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_4));
      my_prefs.scroll_mode = (CtlGetValue(checkbox) != 0);  
      /* read sound checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_5));
      my_prefs.sound_on = (CtlGetValue(checkbox) != 0);  
      /* read centered checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_6));
      my_prefs.stay_centered = (CtlGetValue(checkbox) != 0);  
      /* read font checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_7));
      my_prefs.font_small = (CtlGetValue(checkbox) != 0);  
      /* read rogue-relative checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_8));
      my_prefs.rogue_relative = (CtlGetValue(checkbox) != 0);  
      /* read crosshairs checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_9));
      my_prefs.crosshairs_on = (CtlGetValue(checkbox) != 0);  
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_11));
      if (!my_prefs.sort_pack && CtlGetValue(checkbox))
  sort_pack(&(sotu->roguep->pack));
      my_prefs.sort_pack = (CtlGetValue(checkbox) != 0);  
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm,check_bul_12));
      if ((CtlGetValue(checkbox) != 0) != my_prefs.black_bg)
  redraw = true;
      my_prefs.black_bg = (CtlGetValue(checkbox) != 0);
#ifdef I_AM_COLOR
      {
  /* read color checkbox... */
  DWord version;
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
  if (version >= 0x03503000L) {
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm,check_bul_10));
    if ((CtlGetValue(checkbox) != 0) != my_prefs.color_on)
      redraw = true;
    my_prefs.color_on = (CtlGetValue(checkbox) != 0);
    if (!have_read_color && my_prefs.color_on)
      // we'll need to read the memo.
      // I used to check that the old mp.color_on was false.. need to????
      look_for_memo(); // will set have_read_color true if found

    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm,check_bul_13));
    if ((CtlGetValue(checkbox) != 0) != my_prefs.tiles_on)
      redraw = true;
    my_prefs.tiles_on = (CtlGetValue(checkbox) != 0);
    if (!have_read_tiles && my_prefs.tiles_on) {
      look_for_tiles();
      if (!have_read_tiles) {
        FrmAlert(NoTiles);
        my_prefs.tiles_on = false;
      }

    }
  }
      }
#endif // COLOR

      /* read player name... */
      usr_form_update(frm, sotu);
      /* write bools and shorts to a PREFERENCES data thingy. */
      writeRoguePrefs();
      LeaveForm();
      if (redraw) { // Basically same as the MainForm re-center menu item...
  move_visible_window(sotu->roguep->col, sotu->roguep->row, true);
  print_stats(STAT_ALL, sotu); 
  check_message();  // clear the old message
      }
      /* It is important to do this after LeaveForm for msg to display */
      /* <anything with message> */
      handled = true;
      break;
    case btn_bul_cancel:
      LeaveForm();
      handled = true;
      break;
      /*
    case btn_bul_hw:
      FrmPopupForm(HwButtonsForm);
      handled = true;
      break;
      */
    case btn_bul_draw:
      draw_circle(ScrWidth/2, ScrHeight/2, (rw+ws)*WalkMagic, true);
      draw_circle(ScrWidth/2, ScrHeight/2, ws*WalkMagic, false);
      handled = true;
      break;
    case btn_bul_clear:
      //draw_circle(ScrWidth/2, ScrHeight/2, 160, false); // why?
      // let's try it without
      FrmDrawForm(frm);
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}

Boolean About_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
/*
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(true);
      // move buttons
      PrvMoveObject(frm, btn_about_ok, y_diff, false);
      PrvMoveObject(frm, btn_about_more, y_diff, false);
      PrvMoveObject(frm, btn_about_credits, y_diff, false);
    }
*/
#endif
    FrmDrawForm(frm);
    handled = true;
    break;


#ifndef I_AM_OS_2
/*
  case displayExtentChangedEvent:
    if (IsVGA) {
      Short y_diff;
      update_display_size();
      y_diff = adjust_form_size(true);
      // redraw the "new" screen too...  don't ask me whether this will work
      frm = FrmGetActiveForm();
      // move buttons
      PrvMoveObject(frm, btn_about_ok, y_diff, true);
      PrvMoveObject(frm, btn_about_more, y_diff, true);
      PrvMoveObject(frm, btn_about_credits, y_diff, true);
      FrmDrawForm(frm);
    }
    break;
*/
#endif

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_about_ok:
      LeaveForm();
      handled = true;
      break;
    case btn_about_more:
      FrmHelp(AboutStr);
      handled = true;
      break;
    case btn_about_credits:
      FrmHelp(CreditStr);
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}

Boolean Map_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
#ifndef I_AM_OS_2
/*
    if (IsVGA) {
      VgaFormModify(frm, vgaFormModify160To240);
      adjust_form_size(false);
    }
*/
#endif
    FrmDrawForm(frm);
    /* this must be AFTER draw form */
    show_dungeon(sotu);
    handled = true;
    break;


#ifndef I_AM_OS_2
/*
  case displayExtentChangedEvent:
    if (IsVGA) {
      update_display_size();
      adjust_form_size(true);
      // redraw the "new" screen too...  don't ask me whether this will work
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      show_dungeon(sotu);
    }
    break;
*/
#endif

  case penDownEvent:
    LeaveForm();
    handled = true;
    break;

  default:
    break;
  }

  return handled;
}


Boolean HwButtons_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;    // popup list
  ControlPtr ctl; // popup trigger
  CharPtr label;
  Short i;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    // my stuff
    ctl = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_hwb));
    CtlSetValue(ctl, (my_prefs.use_hardware ? 1 : 0));
    for (i = 0 ; i < 8 ; i++) {
      lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_hwb_1 + i));
      ctl = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, popup_hwb_1 + i));
      LstSetSelection(lst, my_prefs.hardware[i]);
      label = LstGetSelectionText(lst, my_prefs.hardware[i]);
      CtlSetLabel(ctl, label);
    }
    // end my stuff
#ifndef I_AM_OS_2
/*
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(false);
      // move buttons
      PrvMoveObject(frm, btn_hwb_ok, y_diff, false);
      PrvMoveObject(frm, btn_hwb_cancel, y_diff,false);
    }
*/
#endif
    FrmDrawForm(frm);
    handled = true;
    break;


#ifndef I_AM_OS_2
/*
  case displayExtentChangedEvent:
    if (IsVGA) {
      Short y_diff;
      update_display_size();
      y_diff = adjust_form_size(true);
      // redraw the "new" screen too...  don't ask me whether this will work
      frm = FrmGetActiveForm();
      // move buttons
      PrvMoveObject(frm, btn_hwb_ok, y_diff, true);
      PrvMoveObject(frm, btn_hwb_cancel, y_diff, true);
      FrmDrawForm(frm);
    }
    break;
*/
#endif

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_hwb_ok:
      frm = FrmGetActiveForm();
      ctl = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_hwb));
      my_prefs.use_hardware = (CtlGetValue(ctl) != 0);  
      for (i = 0 ; i < 8 ; i++) {
  lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_hwb_1 + i));
  my_prefs.hardware[i] = LstGetSelection(lst);
      }
      writeRoguePrefs();
      LeaveForm();
      handled = true;
      break;
    case btn_hwb_cancel:
      LeaveForm();
      handled = true;
      break;
    }
    break;
  default:
    break;
  }

  return handled;
}

Boolean topten_just_browsing = false; // quickfix to allow browsing while alive
Boolean TopTen_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  FieldPtr fld;
  ControlPtr btn;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_topten));
    if (!topten_just_browsing)
      save_me = false;
    init_topten_view(frm);
    update_field_scrollers(frm, fld, repeat_topten_up, repeat_topten_down);
    if (!topten_just_browsing) {
      kill_saved_rogue(STATUS_ISDEAD, 0); // make sure you are DEAD
      if (sotu->conduct & CONDUCT_WINNER) //if (sotu->score_status & STATUS_WINNER)
  topten_frob_btns(frm);
    }
    /* end mine */
#ifndef I_AM_OS_2
/*
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(false);
      // move buttons
      PrvMoveObject(frm, btn_dead_ok, y_diff, false);
      PrvMoveObject(frm, btn_dead_reviv, y_diff,false);
      PrvMoveObject(frm, btn_dead_done, y_diff, false);
      PrvMoveObject(frm, repeat_topten_up, y_diff, false);
      PrvMoveObject(frm, repeat_topten_down, y_diff, false);
      PrvResizeField(frm, field_topten, y_diff);
      update_field_scrollers(frm, fld, repeat_topten_up, repeat_topten_down);
    }
*/
#endif
    FrmDrawForm(frm);
    if (topten_just_browsing) {
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_dead_ok));
      CtlHideControl(btn);
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_dead_reviv));
      CtlHideControl(btn);
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_dead_done));
      CtlShowControl(btn);
      topten_just_browsing = false; // the view-scores menu item sets it True.
    }
    handled = true;
    break;


#ifndef I_AM_OS_2
/*
  case displayExtentChangedEvent:
    if (IsVGA) {
      Short y_diff;
      update_display_size();
      y_diff = adjust_form_size(true);
      // redraw the "new" screen too...  don't ask me whether this will work
      frm = FrmGetActiveForm();
      // move buttons
      PrvMoveObject(frm, btn_dead_ok, y_diff, true);
      PrvMoveObject(frm, btn_dead_reviv, y_diff, true);
      PrvMoveObject(frm, btn_dead_done, y_diff, true);
      PrvMoveObject(frm, repeat_topten_up, y_diff, true);
      PrvMoveObject(frm, repeat_topten_down, y_diff, true);
      PrvResizeField(frm, field_topten, y_diff);
      fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_topten));
      update_field_scrollers(frm, fld, repeat_topten_up, repeat_topten_down);
      FrmDrawForm(frm);
    }
    break;
*/
#endif

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_dead_done: // we're still alive, just browsing the score list..
      LeaveForm();
      handled = true;
      break;
    case btn_dead_reviv:
      FrmPopupForm(RevivifyForm);
      handled = true;
      break;
    case btn_dead_ok:
      free_old_level(sotu);
      init_sotu();
      clear_identifications();
      // that will:
      // init_id_potions to get rid of ids and 'call' titles
      // init_id_scrolls to get rid of ids and 'call' titles
      // also, might need wands and rings...
      mix_colors();
      get_wand_and_ring_materials();
      /* (does anything else need to be cleared???) */
      do_make_new_level = true; // don't try to load a saved character..
      save_me = true;
      i_am_dead = false;
      LeaveForm();
      FrmGotoForm(MainForm);
      check_message();
      /* Well.... it seems to work... */
      handled = true;
      break;
    }
    break;

  case ctlRepeatEvent:
    /*     "Repeating controls don't repeat if handled is set true." */
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_topten));
    switch(e->data.ctlRepeat.controlID) {
    case repeat_topten_up:
      FldScrollField(fld, 1, winUp);
      update_field_scrollers(frm, fld, repeat_topten_up, repeat_topten_down);
      break;
    case repeat_topten_down:
      FldScrollField(fld, 1, winDown);
      update_field_scrollers(frm, fld, repeat_topten_up, repeat_topten_down);
      break;
    }
    break;

  case keyDownEvent:
    /* hardware button -- or else graffiti. */
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_topten));
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      page_scroll_field(frm, fld, winUp);
      update_field_scrollers(frm, fld, repeat_topten_up, repeat_topten_down);
      handled = true;
      break;
    case pageDownChr:
      page_scroll_field(frm, fld, winDown);
      update_field_scrollers(frm, fld, repeat_topten_up, repeat_topten_down);
      handled = true;
      break;
    }
    break;


  default:
    break;
  }

  return handled;
}

// usr_form_* moved to chuvmey_nolib


/* Revivify 
   If you cancel, exit this form back to top-ten (dead) or dungeon (live).
   If you do not cancel, need to know which form you came from!!! argh.
   Well actually that should be apparent because if topten, sotu is DEAD. */
/* Undead or alive:
   50% The orange smoke slowly disappears.
   50% The cloud of orange smoke dissipates.
   make a nice piercing beep at this point.
   Undead:
   You feel.. strangely hungry.
   Alive:
   40% Someone has rifled your pockets!
   30% You don't recall how you got here.
   20% You feel shaky, weak in the knees.
   10% You feel distressingly light-headed.  */
Boolean Revivify_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Short filenum, new_i;
  ListPtr lst;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* end mine */
#ifndef I_AM_OS_2
/*
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(false);
      PrvMoveObject(frm, btn_reviv_alive, y_diff, false);
      PrvMoveObject(frm, btn_reviv_undead, y_diff, false);
      PrvMoveObject(frm, btn_reviv_cancel, y_diff, false);
    }
*/
#endif
    FrmDrawForm(frm);
    // Hey! Disable the "alive" button if the guy being restored already undead
    // If you can get to this form from not-top-ten-list, also,
    // disable the "undead" button if the guy being restored is alive.
    // I will do this LATER because I will have to READ THE DATABASE RECORD
    // 
    // Initialize the popup list with the available save files
    // (when deleting them: please PACK so there are no empty files.)
    // display "autosave", "<timestamp>" ...  have autosave slot selected.
    // initially and when a new one is chosen on list, print info on screen.
    // do this in a reusable function because snapshot_form needs it too
    //
    // Also do not allow more than MAX_SAVE_FILES.
    lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_reviv));
    snapshot_update_list(lst, false); // bool == whether to include 'new'
    display_saved_rogue(0);
    reviv_frob_btns(frm, 0);
    handled = true;
    break;


#ifndef I_AM_OS_2
/*
  case displayExtentChangedEvent:
    if (IsVGA) {
      Short y_diff;
      update_display_size();
      y_diff = adjust_form_size(true);
      // redraw the "new" screen too...  don't ask me whether this will work
      frm = FrmGetActiveForm();
      PrvMoveObject(frm, btn_reviv_alive, y_diff, true);
      PrvMoveObject(frm, btn_reviv_undead, y_diff, true);
      PrvMoveObject(frm, btn_reviv_cancel, y_diff, true);
      FrmDrawForm(frm);
      // also redraw this part of form
      lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_reviv));
      filenum = LstGetSelection(lst);
      display_saved_rogue(filenum);
    }
    break;
*/
#endif

  case popSelectEvent:
    filenum = e->data.popSelect.selection; // you also know priorSelection
    new_i = get_file_count();
    undisplay_saved_rogue();
    if (filenum < new_i) {
      display_saved_rogue(filenum);
      reviv_frob_btns(FrmGetActiveForm(), filenum);
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_reviv));
    switch(e->data.ctlSelect.controlID) {
    case btn_reviv_cancel:
      LstSetListChoices(lst, NULL, 0);
      LeaveForm(); // just go back to the topten form
      handled = true;
      break;
    case btn_reviv_undead:
    case btn_reviv_alive:
      revivify_which = LstGetSelection(lst);
      LstSetListChoices(lst, NULL, 0);
      LeaveForm();
      if (!save_me) LeaveForm(); // we came from top_ten, get rid of it too.
      FrmGotoForm(MainForm); // leave main form and goto main form
      check_message();
      /* \end{dangerous half-assed implementation} */
      /* Well.... it seems to work... */
      if (e->data.ctlSelect.controlID == btn_reviv_undead)
  revivify_p = STATUS_UNDEAD;
      else 
  revivify_p = STATUS_ISDEAD;
      do_make_new_level = false; // so that we load the saved character!
      save_me = true;
      i_am_dead = false;
      handled = true;
      break;
    }
    break;
  default:
    break;
  }

  return handled;
}


Boolean Snapshot_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Short filenum, new_i;
  ListPtr lst;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* end mine */
#ifndef I_AM_OS_2
/*
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(false);
      PrvMoveObject(frm, btn_snap_delete, y_diff, false);
      PrvMoveObject(frm, btn_snap_save, y_diff, false);
      PrvMoveObject(frm, btn_snap_cancel, y_diff, false);
    }
*/
#endif
    FrmDrawForm(frm);
    // Hey! Disable the "delete" button if there is only autosave.
    // (even though the button handler is smart enough to not do it).
    lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_snap));
    snapshot_update_list(lst, true); // true == whether to include 'new'
    display_saved_rogue(0);
    snapshot_frob_btns(frm, false);
    handled = true;
    break;


#ifndef I_AM_OS_2
/*
  case displayExtentChangedEvent:
    if (IsVGA) {
      Short y_diff;
      update_display_size();
      y_diff = adjust_form_size(true);
      frm = FrmGetActiveForm();
      // redraw the "new" screen too...  don't ask me whether this will work
      PrvMoveObject(frm, btn_snap_delete, y_diff, true);
      PrvMoveObject(frm, btn_snap_save, y_diff, true);
      PrvMoveObject(frm, btn_snap_cancel, y_diff, true);
      FrmDrawForm(frm);
      lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_snap));
      filenum = LstGetSelection(lst);
      display_saved_rogue(filenum);
    }
    break;
*/
#endif

  case popSelectEvent:
    filenum = e->data.popSelect.selection; // you also know priorSelection
    new_i = get_file_count();
    undisplay_saved_rogue();
    if (filenum < new_i) {
      display_saved_rogue(filenum);
      // if filenum == 0, disable 'delete' button..
      snapshot_frob_btns(FrmGetActiveForm(), (filenum!=0));      
    }
    ////    handled = true; // ?
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_snap));
    filenum = LstGetSelection(lst);
    new_i = get_file_count();
    switch(e->data.ctlSelect.controlID) {
    case btn_snap_cancel:
      LstSetListChoices(lst, NULL, 0);
      LeaveForm();
      handled = true;
      break;
    case btn_snap_delete:
      // figure out which is selected, if NEW or autosave then feep & ignore
      // ..or else have the got-selected code disable/enable delete..
      // Popup an "deleting foo, are you SURE?"
      // Do not leave the form.
      handled = true;
      if (filenum <= 0 || filenum >= new_i) break; // o.o.b.
      if (0 == FrmAlert(SaveDeleteP)) {
  delete_saved_rogue(filenum);
  snapshot_delete_listi(frm, filenum);
  snapshot_frob_btns(frm, false);
      }
      // then REDRAW the list!!
      // actually, just free the ith string and pack the array,
      // then tell it to redraw itself.
      break;
    case btn_snap_save:
      // if something other than autosave or NEW is selected,
      // Popup an "overwriting foo, are you SURE?"
      handled = true;
      if (filenum < 0 || filenum > new_i) break; // out of bounds
      if (filenum == 0 || filenum == new_i || 0 == FrmAlert(SaveOverP)) {
  save_character(sotu, filenum);
  LstSetListChoices(lst, NULL, 0);
  LeaveForm();
  message("saved.", sotu);
      }
      break;
    }
    break;
  default:
    break;
  }

  return handled;
}


/*****************************************************************************
 *                                                                           *
 *   The following stufff actually belongs in this file.                     *
 *                                                                           *
 *****************************************************************************/
/*****************************************************************************
 *                                                                           *
 *                      OpenDatabase                                         *
 *                                                                           *
 *****************************************************************************/
static Boolean OpenDatabase(void)
{
  do_make_new_level = false; // so that we will load the saved character
  /* Try to open the Rogue database.
     If it does not exist, create one. */

  RogueDB =  DmOpenDatabaseByTypeCreator(RogueDBType,
           RogueAppID,
           dmModeReadWrite);
  if (!RogueDB) {
    if (DmCreateDatabase(0, // card number 0
       RogueDBName,
       RogueAppID,
       RogueDBType,
       false))
      return 1;
    RogueDB = DmOpenDatabaseByTypeCreator(RogueDBType,
             RogueAppID,
             dmModeReadWrite);
    /* Success has crowned my efforts!  and I may consider myself
       engaged to Phyllis!  ok, maybe not. */
    do_make_new_level = true;    
    create_top_ten_record();
    create_filecount_record();
    return 0;
  }

  if (DmNumRecords(RogueDB) <= 11)
    do_make_new_level = true; /* the database doesn't really have stuff */    
  else if (get_saved_rogue_status(0) & STATUS_ISDEAD)
    do_make_new_level = true; /* the saved character is DEAD, ignore it */

  /* there was a Rogue DB and we opened it */
  return 0;
}


/*****************************************************************************
 *                      ApplicationHandleEvent                               *
 *****************************************************************************/
static Boolean ApplicationHandleEvent(EventPtr e)
{
    FormPtr frm;
    Word    formId;
    Boolean handled = false;

    if (e->eType == frmLoadEvent) {
  formId = e->data.frmLoad.formID;
  frm = FrmInitForm(formId);
  FrmSetActiveForm(frm);

  switch(formId) {
  case MainForm:
      FrmSetEventHandler(frm, Main_Form_HandleEvent);
      break;
  case DirectionForm:
      FrmSetEventHandler(frm, Direction_Form_HandleEvent);
      break;
  case MsgLogForm:
      FrmSetEventHandler(frm, MsgLog_Form_HandleEvent);
      break;
  case InvSelectForm:
      FrmSetEventHandler(frm, InvSelect_Form_HandleEvent);
      break;
  case WizForm:
      FrmSetEventHandler(frm, Wiz_Form_HandleEvent);
      break;
  case PrefsForm:
      FrmSetEventHandler(frm, Prefs_Form_HandleEvent);
      break;
  case AboutForm:
      FrmSetEventHandler(frm, About_Form_HandleEvent);
      break;
  case MapForm:
      FrmSetEventHandler(frm, Map_Form_HandleEvent);
      break;
  case TopTenForm:
      FrmSetEventHandler(frm, TopTen_Form_HandleEvent);
      break;
  case RevivifyForm:
      FrmSetEventHandler(frm, Revivify_Form_HandleEvent);
      break;
  case SnapshotForm:
      FrmSetEventHandler(frm, Snapshot_Form_HandleEvent);
      break;
  case HwButtonsForm:
      FrmSetEventHandler(frm, HwButtons_Form_HandleEvent);
      break;
  }
  handled = true;
    }

    return handled;
}

// dude - the 3.5ez (Vx) rom hassled me about this (I think it was about this).
// ...the III rom did not, and the color rom did not, and the 3.3's did not.
// wacky.
static void readRoguePrefs() {
  Word prefsSize;
  SWord prefsVersion;
  //  Short i;
  //  RoguePreferenceType prefs;
  prefsSize = sizeof(RoguePreferenceType);
  prefsVersion = PrefGetAppPreferences(RogueAppID, RogueAppPrefID, &my_prefs,
               &prefsSize, true);
  if (prefsVersion > noPreferenceFound) {
    // just to be sure.
    my_prefs.run_walk_border = max(my_prefs.run_walk_border, 1);
    my_prefs.walk_search_border = max(my_prefs.walk_search_border, 1);
    // also, check that it's not an old version; if it is,
    // fix the new stuff.
    if (prefsVersion < RogueAppPrefVersion) {
      my_prefs.tiles_on = false; // new in 1.1.1
    }
    if (prefsVersion < 0x03) {
      my_prefs.color_on = false; // new in 0.46
      my_prefs.crosshairs_on = false; // new in 0.46
    }
    if (prefsVersion < 0x02) {
      my_prefs.autosave_on = false; // new in 0.42 // nah, change to multiuser
      my_prefs.font_small = false; // new in 0.42
      my_prefs.rogue_relative = false; // new in 0.42
    }
    //    if (ancient) {
    //      my_prefs.sound_on = true;
    //      my_prefs.use_hardware = false;
    //      for (i = 0 ; i < 8 ; i++)
    //      my_prefs.hardware[i] = HWB_NOOP;
    //      my_prefs.stay_centered = false; // new in 0.40
    //    }
  }
}
static void writeRoguePrefs() {
  Word prefsSize;
  //  RoguePreferenceType prefs;
  prefsSize = sizeof(RoguePreferenceType);

  PrefSetAppPreferences(RogueAppID, RogueAppPrefID, RogueAppPrefVersion,
      &my_prefs, prefsSize, true);
}

/*****************************************************************************
 *                      StartApplication                                     *
 *****************************************************************************/
Boolean IsVGA = false; // like IsColor.  tells you if you're on a Handera 330.
#ifndef I_AM_OS_2
FontID SmallFont;
FontID BigFont;
#endif

/* moved to the head of the class */
#ifdef I_AM_OS_2
FontPtr oldFontSix = 0;
#endif
Boolean evil = false;
static Word StartApplication(void)
{
  Word error;
#ifdef I_AM_OS_2
  DWord version;
  void *font128 = 0;
#else
  DWord version;
  VoidHand fontHandle;
  FontType *fontPtr;
#endif

  error = OpenDatabase();
  if (error) { evil = true; return error; }
/*   error = checkAppInfo(); */
/*   if (error) return error; */

  /* DO NOT START if there is no rogue library loaded.  so there. */
  //  if (! DmFindDatabase(0, RogueLibraryName))
  //    return 1;

  /* load game and/or do all the ONE-TIME initialization */
  init_game();

  readRoguePrefs();

#ifdef I_AM_OS_2
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
  // evil font kludges!
  font128 = MemHandleLock(DmGetResource('NFNT', ItsyFont));
  if (version < 0x03000000L) {
    oldFontSix = ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6];
    ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6] = font128;
  }
#else
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
  if (version >= 0x03000000L) {
    //UInt32 version2;
    UInt32 defaultFont;
    //IsVGA = _TRGVGAFeaturePresent(&version2); // Are we on a Handera 330.
    FtrGet(sysFtrCreator, sysFtrDefaultFont, &defaultFont);
    if (IsVGA) {
/*
      if (VgaIsVgaFont(defaultFont)) { // vga fonts are LARGER
  SmallFont = VgaVgaToBaseFont(defaultFont);
  BigFont = defaultFont;
      } else {
  SmallFont = defaultFont;
  BigFont = VgaBaseToVgaFont(defaultFont);
      }
      FntSetFont(SmallFont);// So the forms OTHER than Main have non-VGA font!!
*/
    } else {
      BigFont = defaultFont;
      fontHandle = DmGetResource('NFNT',ItsyFont);
      fontPtr = MemHandleLock(fontHandle);
      // "user defined fonts start from 129"
      FntDefineFont(129, fontPtr);
      SmallFont = 129;
      // the custom font is uninstalled automatically when we leave iRogue;
      // however, fontHandle MUST remain locked until then.
    }
  }
  // set HasColor boolean to false
#ifdef I_AM_COLOR
  if (version >= 0x03503000L) {
    if (my_prefs.color_on & PREF_COLOR) {
      look_for_memo(); // find and grok 'iRogue-RGB' memo
    }
    if (my_prefs.tiles_on) {
      look_for_tiles(); // look for a database of 9x10 color icons.
    }
  }
#endif // COLOR
#endif // OS 2

  if (my_prefs.font_small) // user likes the small font.
    toggle_itsy(); // might return false, if OS is wrong version; ignore.

#ifndef I_AM_OS_2
/*
  if (IsVGA) {// If Handera, go to hi res mode
    VgaSetScreenMode(screenMode1To1, rotateModeNone);
    init_display_size();
  }
*/
#endif

  FrmGotoForm(MainForm);
  return 0;
}

/*****************************************************************************
 *                      StopApplication                                      *
 *****************************************************************************/
/* Save preferences, close forms, close app database */
extern Char * hit_message;
extern Boolean *rooms_visited;
#define Free_me(a)  h = MemPtrRecoverHandle((a)); if (h) MemHandleFree(h);
static void free_everything() {
  VoidHand h;
  Short i;

  free_old_level(sotu);
  Free_me(sotu->level_objects);
  Free_me(sotu->level_monsters);

  Free_me(hit_message);
  Free_me(rooms_visited);

  //  Free_me(sotu->fruit);
  Free_me(sotu->level_points);
  //  for (i = 0 ; i < TRAPS*2 ; i++) {
  //    Free_me(sotu->trap_strings[i]);
  //  }
  for (i = 0 ; i < DROWS ; i++) {
    Free_me(sotu->dungeon[i]);
  }
  Free_me(sotu->dungeon);
  Free_me(sotu->rooms);
  Free_me(sotu->traps);
  Free_me(sotu->id_potions);
  Free_me(sotu->id_scrolls);
  Free_me(sotu->id_weapons);
  Free_me(sotu->id_armors);
  Free_me(sotu->id_rings);
  Free_me(sotu->id_wands);
  Free_me(sotu->is_wood);

  Free_me(sotu->roguep);
  Free_me(sotu);

}

static void StopApplication(void)
{
#ifdef I_AM_OS_2
  DWord version;
  //void *font128 = 0;
#else
  DWord version;
  VoidHand fontHandle;
#endif
  if (evil) return;

#ifdef I_AM_OS_2
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
  if (version < 0x03000000L) {
    if (oldFontSix) {
      ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6] = oldFontSix;
      oldFontSix = 0;
      //      if ((font128 = ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6]))
      //  MemHandleUnlock(font128);
    }
  }
#else
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
  if (version >= 0x03000000L) {
    if (!IsVGA) {
      fontHandle = DmGetResource('NFNT',ItsyFont);
      MemHandleUnlock(fontHandle);
    }
  }  
#endif // OS 2

  //  if (save_me && my_prefs.save_on_exit)
  if (save_me) { // XXX DEBUGGING
    check_message(); // force clear
    sotu->last_old_message_shown = SAVED_MSGS-1; // no "--more--"
    message("Saving...", sotu);
    save_character(sotu, 0); /* save if not DEAD. */
  }
  
  FrmSaveAllForms();
  FrmCloseAllForms();

  /* it might be nice to CLOSE the DATABASE.  not necessary? */
  if (RogueDB!=NULL) {
    DmCloseDatabase(RogueDB);
    RogueDB = NULL;
  }
  if (RogueTileDB!=NULL) {
    DmCloseDatabase(RogueTileDB);
    RogueTileDB = NULL;
  }
  free_everything();
  /* it might be nice to free some crap, too */
}


/*****************************************************************************
 *                      EventLoop                                            *
 *****************************************************************************/

/* The main event loop */

static void EventLoop(void) {
  Word err;
  EventType e;
     
  do {
    EvtGetEvent(&e, evtWaitForever);

    // first see if it's a hardware button thing!!!
    // don't ask me what the poweredOnKeyMask is, though; cargo cult.
    // Do special hardware button things only if:
    // it's a hardware button event, [you're alive,] and in the main form.
    if ( (e.eType != keyDownEvent)
      || !my_prefs.use_hardware 
      || (e.data.keyDown.modifiers & poweredOnKeyMask)
      /*       || (FrmGetActiveFormID() != MainForm) */
      || !buttonsHandleEvent(&e) ) {
      // now proceed with usual handling

      if (!SysHandleEvent(&e)) {
        if (!MenuHandleEvent(NULL, &e, &err)) {
          if (!ApplicationHandleEvent(&e)) {
            FrmDispatchEvent(&e);
          }
        }
      }
    }
  } while (e.eType != appStopEvent);
}

/* Main entry point; it is unlikely you will need to change this except to
   handle other launch command codes */
/*****************************************************************************
 *                      PilotMain                                            *
 *****************************************************************************/
//#define ERROR_CHECK_LEVEL ERROR_CHECK_FULL
DWord PilotMain(Word cmd, MemPtr cmdPBP, Word launchFlags)
{
    Word err;

    /*
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
    Word crc = Crc16CalcBlock(NULL, 256, 0);
#endif
*/
/* #if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL */
/*     DWord startFreeDynamicMemory, endFreeDynamicMemory, junk; */
/*     MemHeapFreeBytes(0, &startFreeDynamicMemory, &junk); */
/* #endif */

    if (cmd == sysAppLaunchCmdNormalLaunch) {

      err = StartApplication();
      if (err) return err;

      EventLoop();
      StopApplication();

    } else {
      return sysErrParamErr;
    }
    /*
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
    ErrNonFatalDisplayIf (crc != Crc16CalcBlock(NULL, 256, 0), 
        "low Memory Trashed)");
#endif
*/
/* #if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL */
/*     MemHeapFreeBytes(0, &endFreeDynamicMemory, &junk); */
/*     ErrNonFatalDisplayIf (startFreeDynamicMemory != endFreeDynamicMemory, */
/*         "Memory Leak Occurred"); */
/* #endif */

    return 0;
}

/*******************************************************************
                    add new stuff below
*******************************************************************/

 
