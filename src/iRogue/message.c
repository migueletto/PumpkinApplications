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
#include "librogue.h"
#ifndef I_AM_OS_2
//#include "handera.h"
#endif

//Short screen_width = 160;
//Short screen_height = 160;

Boolean msg_cleared = true;
Boolean msg2_cleared = true;

// 160, 160, 11, 23, 23, 114.
extern Short ScrWidth;
extern Short ScrHeight;
extern Short LineHeight;
extern Short ScrMsgHt;
extern Short ScrStatsHt;
/*  extern Short ScrDungeonHt */
// how wide is "--more--" (160-"40"=120).

extern RoguePreferenceType my_prefs;
//#ifdef I_AM_COLOR
//void set_draw_color();
//extern IndexedColorType black, white;
//#endif // I_AM_COLOR

/***************************************************************
                   CLEAR_MSG
 IN:
 nothing
 OUT:
 nothing
 PURPOSE:
 Clear the message portion of the screen.
****************************************************************/
static void clear_msg() SEC_L;
static void clear_msg() {
  RectangleType r;
  // Hm, apparently I'm on crack:
  if (!IsVGA)
    RctSetRectangle(&r,0,0,ScrWidth,ScrMsgHt); /* top-left, width and height */
  else
    RctSetRectangle(&r,0,0,ScrWidth,ScrMsgHt); /* top-left, width and height */
  qWinEraseRectangle(&r, 0); /* 0 for square corners */
}
/***************************************************************
                   CHECK_MESSAGE
 IN:
 nothing (msg_cleared)
 OUT:
 nothing
 PURPOSE:
 If the message isn't cleared, clear it.
****************************************************************/
void check_message() {
#ifndef I_AM_COLOR
  if (msg_cleared) {
    return;
  }
#endif
  clear_msg();
  qWinDrawLine(0,ScrMsgHt-1,ScrWidth,ScrMsgHt-1);
  msg_cleared = true;
  msg2_cleared = true;
}

/***************************************************************
                   RECORD_MSG
 IN:
 msg = the message to display
 sotu = various globals (old_messages)
 OUT:
 nothing
 PURPOSE:
 Record the new message (there is sort of a circular array
 of the Last N messages to display in the message log.) 
****************************************************************/
static void record_msg(Char * msg, struct state_of_the_union * sotu) SEC_L;
static void record_msg(Char * msg, struct state_of_the_union * sotu) {
  Short i;
  Char * tmp;
  tmp = sotu->old_messages[0];
  for (i = 0 ; i < SAVED_MSGS-1 ; i++)
    sotu->old_messages[i] = sotu->old_messages[i+1];
  StrNCopy(tmp, msg, SAVED_MSG_LEN-2); /* leave room for \n.. */
  tmp[SAVED_MSG_LEN-2] = '\0'; /* making sure it's terminated.. */
  StrCat(tmp, "\n");
  sotu->old_messages[SAVED_MSGS-1] = tmp;

  sotu->last_old_message_shown--;
  if (sotu->last_old_message_shown < 0)
    sotu->last_old_message_shown = -1; // "none of the buffered msgs have been shown"
}

/***************************************************************
                   MESSAGE
 IN:
 msg = the message to display
 sotu = various globals...
 OUT:
 nothingn
 PURPOSE:
 Display (and record for the message log) the given message.
 (Note: If msg_cleared is false, (and I should probably make it a
 counter instead of a boolean), at least one other message has
 occurred since the start of the turn.  I have not thought of
 a good way to do "--more--", so after the first message, a
 message will be displayed on the second line (probably
 overwriting each other.)
    // Revised:
    // Figure out whether the message is 'clear'
    // If so, print it and advance the sotu->last_old_msg counter.
    // If not, print "--more--" in the lower right of msg window and return.
****************************************************************/
void message(Char * msg, struct state_of_the_union * sotu) {

  FontID fntid;
  Boolean fits, tiny;
  Int msgwidth, msglength;
  RectangleType r;
  Char buf[SAVED_MSG_LEN];
  Short moreWidth;

  //  StrPrintF(buf, "--more--");
#ifdef I_AM_OS_2
  fntid = FntSetFont(stdFont);
#else
  fntid = FntSetFont(BigFont);
#endif
  tiny = false;
  moreWidth = FntCharsWidth(" --more--", 9); // "40" in 160x160 default font.

  if (msg) {
    // it's a new message; add it to the old-messages buffer
    record_msg(msg, sotu);
  } else {
    // find an unseen buffered message to show
    if (sotu->last_old_message_shown >= SAVED_MSGS-1)
      return; // all of the buffered messages have been shown
    if (sotu->last_old_message_shown < 0)
      sotu->last_old_message_shown = -1;
    msg = sotu->old_messages[sotu->last_old_message_shown + 1];
    msglength = StrLen(msg)-1;
    StrNCopy(buf, msg, msglength); // get rid of the \n !!!
    buf[msglength] = '\0';
    msg = buf;
  }

  /*
#ifdef I_AM_COLOR
    if (IsColor) {
      WinPushDrawState();
      set_draw_color();
    }
#endif I_AM_COLOR
  */

  if (!msg_cleared) {
    // maybe the second line is still clear
    if (msg2_cleared) {
      // maybe the message will fit entirely on the second line
      msgwidth = ScrWidth - moreWidth; //"120" Leave room for the "--more--" !!
      msglength = StrLen(msg);
      FntCharsInWidth(msg, &msgwidth, &msglength, &tiny);
      if (tiny) {
	// yes, it will fit...
	RctSetRectangle(&r,0,LineHeight,ScrWidth,LineHeight);
	qWinEraseRectangle(&r, 0); // 0 for square corners
	qWinDrawChars(msg, msglength, 1, LineHeight); // left, top
	msg2_cleared = false;	  // (leave msg_cleared false.)
	sotu->last_old_message_shown++;
      }
    }
    // If there are unseen messages in the buffer, (re)print --more--
    if (sotu->last_old_message_shown < SAVED_MSGS-1)
      qWinDrawChars(" --more--", 9, ScrWidth - moreWidth, LineHeight);
#ifndef I_AM_OS_2
    FntSetFont(fntid);
#endif
    /*
#ifdef I_AM_COLOR
    if (IsColor) {
      WinPopDrawState();
    }
#endif I_AM_COLOR
    */
    return;
  } // end if !msg_cleared

  // Ok, if we haven't returned yet, we're going to print the darn thing.
  sotu->last_old_message_shown++;

  msgwidth = ScrWidth;
  msglength = StrLen(msg);
  fits = false;
  FntCharsInWidth(msg, &msgwidth, &msglength, &fits);

  if (fits) {
    qWinDrawChars(msg, StrLen(msg), 1, 0); /* left, top */
    qWinDrawLine(0,ScrMsgHt-1,ScrWidth,ScrMsgHt-1); // do I need this?
    // if there are still unseen messages, try to fit another one on the second line!
    if (sotu->last_old_message_shown < SAVED_MSGS-1) {
      msg = sotu->old_messages[sotu->last_old_message_shown + 1];
      msglength = StrLen(msg)-1;
      StrNCopy(buf, msg, msglength); // get rid of the \n
      buf[msglength] = '\0';
      msg = buf;
      msgwidth = ScrWidth - moreWidth; // leave room for the "--more--" !!
      //      msglength = StrLen(msg);
      FntCharsInWidth(msg, &msgwidth, &msglength, &tiny);
      if (tiny) {
	// yes, it will fit...
	qWinDrawChars(msg, msglength, 1, LineHeight); // left, top
	msg2_cleared = false;	  // (leave msg_cleared false.)
	sotu->last_old_message_shown++;
      }
    }
  } else {
    //    StrPrintF(buf, "%d %d", StrLen(msg), fits);
    qWinDrawChars(msg, msglength, 1, 0); /* left, top */    
    msg += msglength;
    msglength = StrLen(msg);
    FntCharsInWidth(msg, &msgwidth, &msglength, &fits);
    qWinDrawChars(msg, msglength, 1, LineHeight);
    qWinDrawLine(0,ScrMsgHt-1,ScrWidth,ScrMsgHt-1);
    msg2_cleared = false;
    /* And if it doesn't fit in two lines, that's just too bad. */
  }

  // If there are (still) unseen messages in the buffer, (re)print --more--
  if (sotu->last_old_message_shown < SAVED_MSGS-1) {
    //    WinDrawChars(buf, 8, 120, 11);
    qWinDrawChars("--more--",  9, ScrWidth - moreWidth, LineHeight);
  }
  
#ifndef I_AM_OS_2
  FntSetFont(fntid);
#endif
  msg_cleared = false;
  /*
#ifdef I_AM_COLOR
    if (IsColor) {
      WinPopDrawState();
    }
#endif I_AM_COLOR
  */
}



/***************************************************************
                   PRINT_STATS
 IN:
 stat_mask = which stats to update
 sotu = various globals (rogue, cur_level, add_strength, hunger_string)
 OUT:
 nothing
 PURPOSE:
 Update the stats display on the lower part of the screen.
 (Hm. Maybe it would be cool to make the hit points go reverse-video
 when they get Too Low.)
****************************************************************/
/* Width of various Strings in stats: in 160x160 stdFont,
   L=5, 14, HP =13, 42, Str =15, 32, hungry=31,
   Arm: =21, 12, $=6, 34, Exp: =19, 55 (59 if you add "  ")*/
static Short stats_w[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static void init_print_stats()
{
  stats_w[0] = 1;
  stats_w[1] = FntCharsWidth("L", 1);
  stats_w[2] = FntCharsWidth("00  ", 4); // STAT_LEVEL
  stats_w[3] = FntCharsWidth("HP ", 3);
  stats_w[4] = FntCharsWidth("000(000)  ", 10); // STAT_HP
  stats_w[5] = FntCharsWidth("Str ", 4);
  stats_w[6] = FntCharsWidth("00(00)  ", 8); // STAT_STRENGTH
  stats_w[7] = FntCharsWidth("hungry", 8); // {"" hungry weak faint}
  stats_w[8] = FntCharsWidth("Arm: ", 5);
  stats_w[9] = FntCharsWidth("00 ", 3); // STAT_ARMOR
  stats_w[10] = FntCharsWidth("$", 1);
  stats_w[11] = FntCharsWidth("000000  ", 8); // STAT_GOLD
  stats_w[12] = FntCharsWidth("Exp: ", 5);
  stats_w[13] = FntCharsWidth("00/00000000", 11); // STAT_EXP
}
void print_stats(UInt stat_mask, struct state_of_the_union * sotu)
{
  RectangleType r;
  Char buf[20];
  Short w, hi;
  fighter * rogue = sotu->roguep;
  if (stats_w[0] == 0) init_print_stats();

  if (stat_mask & STAT_LABEL) {
    RctSetRectangle(&r,0,ScrHeight-(ScrStatsHt-1),ScrWidth,ScrStatsHt-1);
    qWinEraseRectangle(&r, 0); /* 0 for square corners */

    qWinDrawLine(0,ScrHeight-ScrStatsHt,ScrWidth,ScrHeight-ScrStatsHt);
    hi = ScrHeight - 2*LineHeight;
    w = stats_w[0];
    StrCopy(buf, "L");
    qWinDrawChars(buf, 1, w, hi);
    w += stats_w[1] + stats_w[2];
    StrCopy(buf, "HP");
    qWinDrawChars(buf, 2, w, hi);
    w += stats_w[3] + stats_w[4];
    StrCopy(buf, "Str");
    qWinDrawChars(buf, 3, w, hi);
    hi = ScrHeight - LineHeight;
    w = stats_w[0];
    StrCopy(buf, "Arm:");
    qWinDrawChars(buf, 4, w, hi);
    w += stats_w[8] + stats_w[9];
    StrCopy(buf, "$");
    qWinDrawChars(buf, 1, w, hi);
    w += stats_w[10] + stats_w[11];
    StrCopy(buf, "Exp:");
    qWinDrawChars(buf, 4, w, hi);
  }

  hi = ScrHeight - 2*LineHeight;
  w = stats_w[0] + stats_w[1]; // just after the "L".
  if (stat_mask & STAT_LEVEL) {
    /* LEVEL */
    /* "L" takes 5; each (of 2) digit takes 5; right-justify the number. */
    /* "  " takes 4. */
    RctSetRectangle(&r,w,hi,stats_w[2],LineHeight-1);
    qWinEraseRectangle(&r, 0);
    StrPrintF(buf, "%d", sotu->cur_level);
    qWinDrawChars(buf, StrLen(buf), w, hi);
    /* add 19 */
  }
  w += stats_w[2] + stats_w[3]; // just after the "HP ".
  if (stat_mask & STAT_HP) {
    /* enforce uppper bound */
    if (rogue->hp_max > MAX_HP) {
      rogue->hp_current -= (rogue->hp_max - MAX_HP);
      rogue->hp_max = MAX_HP;
    }
    /* HIT POINTS */
    /* "HP " takes 13; each digit takes 5; "(" or ")" takes 4. */
    //    w = 33;
    RctSetRectangle(&r,w,hi,stats_w[4],LineHeight-1);
    qWinEraseRectangle(&r, 0);
    StrPrintF(buf, "%d(%d)", rogue->hp_current, rogue->hp_max);
    qWinDrawChars(buf, StrLen(buf), w, hi);  
    /* add 55 */
  }

  w += stats_w[4] + stats_w[5]; // just after the "Str ".
  if (stat_mask & STAT_STRENGTH) {
    /* enforce upper bound */
    if (rogue->str_max > MAX_STRENGTH) {
      rogue->str_current -= (rogue->str_max - MAX_STRENGTH);
      rogue->str_max = MAX_STRENGTH;
    }
    /* STRENGTH */
    /* "Str " takes 15; each digit takes 5; "()" takes 8. */
    //    w = 88;
    RctSetRectangle(&r,w,hi,stats_w[6],LineHeight-1);
    qWinEraseRectangle(&r, 0);
    StrPrintF(buf, "%d(%d)", (rogue->str_current + sotu->add_strength),
	      rogue->str_max);
    qWinDrawChars(buf, StrLen(buf), w, hi);  
    /* add 47 */
  }

  w += stats_w[6]; // just after the "Str 00(00)  ".
  if (stat_mask & STAT_HUNGER) {
    /* FOOD-P */
    /* note: "Hungry" takes 31 pixels. */
    //    w = 124;
    RctSetRectangle(&r,w,hi,stats_w[7],LineHeight); /* 10 wasn't enough */
    qWinEraseRectangle(&r, 0);
    StrPrintF(buf, "      ");
    StrPrintF(buf, sotu->hunger_str);
    /*     StrPrintF(buf, "hungry"); */
    qWinDrawChars(buf, StrLen(buf), w, hi);
  }

  /* LINE TWO */
  hi = ScrHeight - LineHeight;
  w = stats_w[0] + stats_w[8]; // just after the "Arm: ".
  if (stat_mask & STAT_ARMOR) {
    /* enforce upper bound */
    if (rogue->armor && (rogue->armor->d_enchant > MAX_ARMOR)) {
      rogue->armor->d_enchant = MAX_ARMOR;
    }
    /* ARMOR */
    /* "Arm: " takes 21; digits take 5+5; " " takes 2 */
    //    w = 22;
    RctSetRectangle(&r,w,hi,stats_w[9],LineHeight-1);
    qWinEraseRectangle(&r, 0);
    StrPrintF(buf, "%d", get_armor_class(sotu));
    qWinDrawChars(buf, StrLen(buf), w, hi);
    /* add 33 (35) */
  }  
  w += stats_w[9] + stats_w[10]; // just after the "$".
  if (stat_mask & STAT_GOLD) {
    /* enforce upper bound */
    if (rogue->gold > MAX_GOLD) {
      rogue->gold = MAX_GOLD;
    }
    /* GOLD */
    /* "$" takes 6; each of 6 digits takes 5. */
    //    w = 40;
    RctSetRectangle(&r,w,hi,stats_w[11],LineHeight-1);
    qWinEraseRectangle(&r, 0);
    StrPrintF(buf, "%ld", rogue->gold);
    /*     StrPrintF(buf, "999999"); */
    qWinDrawChars(buf, StrLen(buf), w, hi);
    /* add 40 */
  }
  w += stats_w[11] + stats_w[12]; // just after the "Exp: ".
  if (stat_mask & STAT_EXP) {
    /* EXPERIENCE */
    /* "Exp: " takes 19; 10 digits take 50;"/" takes 5. */
    //    w = 93;
    RctSetRectangle(&r,w,hi,stats_w[13],LineHeight-1);
    qWinEraseRectangle(&r, 0);
    StrPrintF(buf, "%d/%ld", rogue->exp, rogue->exp_points);
    /*     StrPrintF(buf, "21/10000000"); */
    qWinDrawChars(buf, StrLen(buf), w, hi);
    /* add 78 */
  }
  return;
}

