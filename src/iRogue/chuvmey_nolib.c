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
 * chuvmey_nolib.c --- Leftovers that don't (directly) use the lib
 * If any.
 */

#include "palm.h"
#include "iRogueRsc.h"
#include "Globals.h"

#ifndef I_AM_COLOR
#include <SystemMgr.rh> // for user name
#else /* I_AM_COLOR */
#include <SystemMgr.h> // needed? or not?
#endif /* I_AM_COLOR */
#include <DLServer.h> // for user name
#include <SoundMgr.h> // for feeping
#include <Preferences.h> // for feeping

#include "rogue.h"
extern DmOpenRef       RogueDB;




// some dependencies:
// get_username: md_malloc
// show_dungeon: peek_me
// save_character: save_vbuffer
// I'll think about "load_character*" another time
// (and, hell, all the rest of that.)


/**********************************************************************
                       TURNS_P
 IN:
 row
 col
 dir = direction, defined by a number 1 to 8, see rogue_defines.h
 dungeon.. isn't this a global?
 various globals
 OUT:
 the new direction that the passage has taken, or 0 if dead-end/fork.
 PURPOSE:
 used only by multiple_move_rogue_Ctrl with go_speed_racer=true,
 to automatically follow twisty-but-not-forking passages.
 **********************************************************************/
Short turns_p(Short row, Short col, Short dir, UShort ** dungeon) {
  Short left = 0;
  Short right = 0;
  Short to_row, to_col;

  to_row = row;
  to_col = col;
  get_dir_rc(dir, &to_row, &to_col, true); /* updates with dirch. */
  if (can_move(row, col, to_row, to_col, dungeon))
    return 0;
  
  if (dir == NORTH || dir == SOUTH) {
    to_row = row;
    to_col = col;
    get_dir_rc(EAST, &to_row, &to_col, true); /* updates with dirch. */
    if (can_move(row, col, to_row, to_col, dungeon))
      left = EAST;
    to_row = row;
    to_col = col;
    get_dir_rc(WEST, &to_row, &to_col, true); /* updates with dirch. */
    if (can_move(row, col, to_row, to_col, dungeon))
      right = WEST;
  }
  if (dir == EAST || dir == WEST) {
    to_row = row;
    to_col = col;
    get_dir_rc(NORTH, &to_row, &to_col, true); /* updates with dirch. */
    if (can_move(row, col, to_row, to_col, dungeon))
      left = NORTH;
    to_row = row;
    to_col = col;
    get_dir_rc(SOUTH, &to_row, &to_col, true); /* updates with dirch. */
    if (can_move(row, col, to_row, to_col, dungeon))
      right = SOUTH;
  }
  if (left == 0 || right == 0)
    return (left | right);
  return 0;
}


// moved here from level.c
/***************************************************************
                   MAKE_ROOM
 IN:
 rn = room number
 r1, r2, r3 = the rooms the game has decided must appear
 sotu = various globals (rooms, dungeon)
 OUT:
 nothing
 PURPOSE:
 Make a room.  (If it is not a BIG_ROOM or one of the rooms that is
 required to appear, there is a 40% chance that it won't really be a
 "room".. it will be a passageway or something.)

 Was in lib_level.c... but the library got too big
****************************************************************/
void make_room(Short rn, Short r1, Short r2, Short r3,
	       struct state_of_the_union *sotu)
{
  Short left_col = 0, right_col = 0, top_row = 0, bottom_row = 0;
  Short width, height;
  Short row_offset, col_offset;
  Short i, j;
  Short ch;
  room * rooms = sotu->rooms;

  /* Figure out the room's maximum dimensions based on its location:
     0 1 2      THE-
     3 4 5  or  BIG-
     6 7 8      ROOM  */
  switch(rn) {
  case 0:
    left_col = 0;
    right_col = COL1-1;
    top_row = MIN_ROW;
    bottom_row = ROW1-1;
    break;
  case 1:
    left_col = COL1+1;
    right_col = COL2-1;
    top_row = MIN_ROW;
    bottom_row = ROW1-1;
    break;
  case 2:
    left_col = COL2+1;
    right_col = DCOLS-1;
    top_row = MIN_ROW;
    bottom_row = ROW1-1;
    break;
  case 3:
    left_col = 0;
    right_col = COL1-1;
    top_row = ROW1+1;
    bottom_row = ROW2-1;
    break;
  case 4:
    left_col = COL1+1;
    right_col = COL2-1;
    top_row = ROW1+1;
    bottom_row = ROW2-1;
    break;
  case 5:
    left_col = COL2+1;
    right_col = DCOLS-1;
    top_row = ROW1+1;
    bottom_row = ROW2-1;
    break;
  case 6:
    left_col = 0;
    right_col = COL1-1;
    top_row = ROW2+1;
    bottom_row = DROWS - 2;
    break;
  case 7:
    left_col = COL1+1;
    right_col = COL2-1;
    top_row = ROW2+1;
    bottom_row = DROWS - 2;
    break;
  case 8:
    left_col = COL2+1;
    right_col = DCOLS-1;
    top_row = ROW2+1;
    bottom_row = DROWS - 2;
    break;
  case BIG_ROOM:
    /* Figure big room's random actual dimensions. */
    top_row = get_rand(MIN_ROW, MIN_ROW+5);
    bottom_row = get_rand(DROWS-7, DROWS-2);
    left_col = get_rand(0, 10);;
    right_col = get_rand(DCOLS-11, DCOLS-1);
    rn = 0;
/*      goto B; */
  }

  /* Figure non-big room's random actual dimensions. */
  if (rn != BIG_ROOM) {
    height = get_rand(4, (bottom_row-top_row+1));
    width = get_rand(7, (right_col-left_col-2));

    row_offset = get_rand(0, ((bottom_row-top_row)-height+1));
    col_offset = get_rand(0, ((right_col-left_col)-width+1));
    
    top_row += row_offset;
    bottom_row = top_row + height - 1;
    
    left_col += col_offset;
    right_col = left_col + width - 1;
  }

  if ( (rn == BIG_ROOM) ||
       (rn == r1) || (rn == r2) || (rn == r3) ||
       rand_percent(60) ) {

    /* This room will be an R_ROOM, i.e. it really will be a room. */ 

    rooms[rn].is_room = R_ROOM;
    
    for (i = top_row; i <= bottom_row; i++) {
      for (j = left_col; j <= right_col; j++) {
      if ((i == top_row) || (i == bottom_row)) {
	ch = HORWALL;
      } else if (((i != top_row) && (i != bottom_row)) &&
		 ((j == left_col) || (j == right_col))) {
	ch = VERTWALL;
      } else {
	ch = FLOOR;
      }
      sotu->dungeon[i][j] = ch;
      }
    }
  }
  /* (Otherwise, the room will not be an R_ROOM but something else.) */

  rooms[rn].top_row = top_row;
  rooms[rn].bottom_row = bottom_row;
  rooms[rn].left_col = left_col;
  rooms[rn].right_col = right_col;
}

/* written late at night, needs sanity check */
void usr_form_init(FormPtr frm, struct state_of_the_union *sotu) {
  FieldPtr fld;
  CharPtr p;
  VoidHand vh;
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_usr));
  vh = (VoidHand) FldGetTextHandle(fld);
  if (!vh) {
    vh = MemHandleNew(32 * sizeof(Char));
  }
  FldSetTextHandle(fld, (MemHandle) 0);
  p = MemHandleLock(vh);
  StrNCopy(p, sotu->username, 30);
  MemHandleUnlock(vh);
  FldSetTextHandle(fld, (MemHandle) vh);
}
/* written late at night, needs sanity check */
void usr_form_update(FormPtr frm, struct state_of_the_union *sotu) {
  FieldPtr fld;
  CharPtr p;
  VoidHand vh;
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_usr));
  vh = (VoidHand) FldGetTextHandle(fld);
  if (vh) {
    p = MemHandleLock(vh);
    StrNCopy(sotu->username, p, 30);
    MemHandleUnlock(vh);
  }
}



/**********************************************************************
                       EXPLAIN_IT
 IN:
 x,y that the player tapped on
 various globals
 OUT:
 nothing
 PURPOSE:
 display what the thing is
**********************************************************************/
extern Short ScrHeight;
extern Short ScrMsgHt;
extern Short ScrStatsHt;
void explain_it(Short x, Short y, struct state_of_the_union *sotu) {
  Short row, col, mch, ch;
  //  Char buf[10];

  if (y < ScrMsgHt) // 22 or less, for 160x160
    message("message window", sotu);
  else if (y >= ScrHeight - ScrStatsHt) // 137 or more, for 160x160
    message("vital statistics of your rogue", sotu);
  else {
    ch = what_row_col(x, y, &row, &col);
    //    ch = mvinch(row, col);
    if (Is_Alpha(ch)) {
      if (ch > 'Z')
	mch = 26 + (ch - 'a'); // 97=='a' 65=='A'
      else
	mch = ch - 'A';
      print_mon_name(mch, sotu);
    } else {
      switch(ch) {
      case ' ':
	message("nothing is visible there", sotu);
	return;
      case '?':
	message("scroll", sotu);
	return;
      case '!':
	message("potion", sotu);
	return;
      case '*':
	message("some gold", sotu);
	return;
      case -80:
	// FOODCHAR
	message("some food", sotu);
	return;
      case '/':
	message("wand", sotu);
	return;
      case ']':
	message("armor", sotu);
	return;
      case ')':
	message("weapon", sotu);
	return;
      case '=':
	message("ring", sotu);
	return;
      case ',':
	message("amulet", sotu);
	return;
      case '.':
	message("the floor", sotu);
	return;
      case '+':
	message("door", sotu);
	return;
      case '#':
	message("tunnel", sotu);
	return;
      case '|':
      case -105:
	// EMDASH
	message("wall", sotu);
	return;
      case '^':
	message("trap", sotu);
	return;
      case '%':
	message("stairs", sotu);
	return;
      case '@':
	message("that's you", sotu);
	return;
      default:
	//	StrPrintF(buf, "%d", ch);
	message("...beats me", sotu);
	//      message(buf, sotu);
	return;
      }
    }
  }
}
/**********************************************************************
                       CALCULATE_WARNING
 IN:
 various globals
 OUT:
 warning level: 0-5.
 PURPOSE:
 Called when the character has a ring of warning, to determine the
 level of a$$-kicking-ness nearby.
 **********************************************************************/
#define WARN_AGAIN 75
static const Char warn_colors[6][7] = {
  "white", "pink", "red", "ruby", "purple", "black"
};
void calculate_warning(struct state_of_the_union *sotu) {
  object *monster;
  Short r, c, dist_sq;
  Short warn_lev = 0;
  Char glowbuf[30];

  monster = level_monsters->next_monster;
  while (monster) {
    // if the monster is within 10 squares of the rogue, it's of interest.
    // add something proportional to its 'level'.
    r = monster->row - sotu->roguep->row;
    c = monster->col - sotu->roguep->col;
    dist_sq = r * r + c * c; // square of the distance to the rogue
    if (dist_sq <= 100) {
      r = monster->first_level / 4;
      if (r > warn_lev)
	warn_lev += r;
    }
    monster = monster->next_monster;
  }
  // Maybe I should also divide as the rogue gains experience.
  if (warn_lev > 5)
    warn_lev = 5;

  // Done calculating the warning level.  Now see if we need to warn.

  if ((sotu->warning_moves++ > WARN_AGAIN)
      || (warn_lev > sotu->warning_level)) {
    sotu->warning_moves = 0;
    sotu->warning_level = (Char) warn_lev;
    StrPrintF(glowbuf, "your ring glows %s",
	      warn_colors[warn_lev]);
    message(glowbuf, sotu);
    if (warn_lev > 0)
      sotu->interrupted = true;
  }

}



/**********************************************************************
                       UPDATE_FIELD_SCROLLERS
 IN:
 frm, fld, up_scroller, down_scroller = various UI doodads
 OUT:
 nothing
 PURPOSE:
 Update the given scroller widgets (for the given field 
 (in the given form)), according to whether the field is scrollable
 in the "up" and "down" directions.
 **********************************************************************/
void update_field_scrollers(FormPtr frm, FieldPtr fld,
			    Word up_scroller, Word down_scroller) 
{
  Boolean u, d;
  u = FldScrollable(fld, winUp);
  d = FldScrollable(fld, winDown);
  FrmUpdateScrollers(frm, 
		     FrmGetObjectIndex(frm, up_scroller),
		     FrmGetObjectIndex(frm, down_scroller),
		     u, d);
  return;
}


/**********************************************************************
                       PAGE_SCROLL_FIELD
 IN:
 frm, fld = various UI doodads
 dir = whether to scroll 'fld' up or down
 OUT:
 PURPOSE:
 Call this to scroll the field 'fld' up/down by one "page".
 (The caller should call update_field_scrollers immediately afterward.)
 **********************************************************************/
#ifndef I_AM_COLOR
void page_scroll_field(FormPtr frm, FieldPtr fld, DirectionType dir)
#else /* I_AM_COLOR */
void page_scroll_field(FormPtr frm, FieldPtr fld, WinDirectionType dir)
#endif /* I_AM_COLOR */
{
  Word linesToScroll;

  /* how many lines can we scroll? */
  if (FldScrollable(fld, dir)) {
    linesToScroll = FldGetVisibleLines(fld) - 1;
    FldScrollField(fld, linesToScroll, dir);
  }

  return;
}

/**********************************************************************
                       GET_USERNAME
 IN:
 sotu = various globals (username)
 OUT:
 nothing
 PURPOSE:
 Get a default username, truncating it if necessary.
 (originally from Reptoids, modified some.)
 **********************************************************************/
void get_username(struct state_of_the_union *sotu) {
  Char *tmp, *first_wspace;
  VoidHand h;
  tmp = md_malloc(sizeof(Char) * (dlkMaxUserNameLength + 1));
  DlkGetSyncInfo(NULL, NULL, NULL, tmp, NULL, NULL);
  /* if it's too long, use the first name only */
  if (StrLen(tmp) > 30) {
    first_wspace = StrChr(tmp, spaceChr);
    if (first_wspace)
      *(first_wspace) = '\0';
    else
      tmp[30] = '\0';
  }
  /* username is 32 chars allocated */
  if (StrLen(tmp))
    StrNCopy(sotu->username, tmp, 31);
  else
    StrPrintF(sotu->username, "rogue");

  h = MemPtrRecoverHandle(tmp);
  if (h) MemHandleFree(h);  
}


/*
enum patterns { blackPattern, whitePattern, grayPattern, customPattern };
typedef enum patterns PatternType;
typedef Word CustomPatternType [4];
*/
/*static CustomPatternType bombPattern = { 0x8855, 0x2255, 0x8855, 0x2255 };*/
/*static CustomPatternType graePattern = { 0xaa55, 0xaa55, 0xaa55, 0xaa55 };*/
/*static CustomPatternType diagPattern = { 0x8844, 0x2211, 0x8844, 0x2211 };*/

/**********************************************************************
                       SHOW_DUNGEON
 IN:
 sotu = various globals (dungeon, rooms, wizard, cur_level, rogue)
 OUT:
 nothing
 PURPOSE:
 The "real" dungeon is 24 wide x 80 high, and doesn't all fit on the
 (156^2 pixel) screen at once when it's "full size".  So I have a
 form to display a tiny version of the map (just rooms/mazes/tunnels,
 the rogue, and the stairs), where each square is 2 x 3 pixels.
 Maybe I could put flyspecks for monsters/objects too.  Maybe.
 **********************************************************************/
//static CustomPatternType yourPattern = { 0xaaaa, 0x5555, 0xaaaa, 0x5555 };
// The passage pattern got messed up so I moved it inside.
extern Short ScrWidth;
extern Short ScrHeight;
extern Short LineHeight;
extern Short WalkMagic;
static const Char* map_foo[6] = {
  "You Can't Get There From Here",
  "(Tap  Anywhere  When  Done)",
  "Strange, This Doesn't Look Like ...",
  "Oops, I Was Holding It Upside Down.",
  "Here There Be Dragons",
  "(Nothing You Can't Handle, Eh?)",
};
void show_dungeon(struct state_of_the_union * sotu) {
  RectangleType r;
  Word col, row, i, w, h, len;
  UShort mask;
  Short unit, hunit, map_top;
  Char c;
  /*
#ifndef I_AM_COLOR
  CustomPatternType origPattern;
  CustomPatternType yourPattern = { 0xaaaa, 0x5555, 0xaaaa, 0x5555 };
#else // I_AM_COLOR
  // XXXXX
  // const CustomPatternType yourPattern = { 0xaa, 0xaa, 0x55, 0x55,
  //					  0xaa, 0xaa, 0x55, 0x55 };
#endif // I_AM_COLOR
  */
  UShort ** dungeon = sotu->dungeon;
  room * rooms = sotu->rooms;

  RctSetRectangle(&r, 0,0, ScrWidth, ScrHeight);
  WinDrawRectangle(&r, 6);
  unit = ScrWidth/DCOLS;
  hunit = unit+1;
  // width of map will be DCOLS units.  height of map will be DROWS hunits.
  // that's 80 cols, 24 rows

  /* This is just some fluff to put in the extra space above and below. 
     I could probably think of some more-useful fluff to put there. */
  if (!IS_WIZARD)
    i = (sotu->cur_level % 2 == 1) ? 0 : 1; // alternate the fluff
  else
    i = 2;
  len = StrLen(map_foo[i]);
  w = (ScrWidth - FntCharsWidth(map_foo[i],len) )/2;
  WinDrawInvertedChars(map_foo[i], len, w, 1);
  len = StrLen(map_foo[i+1]);
  w = (ScrWidth - FntCharsWidth(map_foo[i+1],len) )/2;
  WinDrawInvertedChars(map_foo[i+1], len, w, ScrHeight-(LineHeight+2));

  /* Each tile gets UNIT pixels/col and HUNIT pixels/row. 
     for 160x16,      =2                   =3              */
  map_top = (ScrHeight-DROWS*hunit)/2;

  /* This is just some decoration. three white horizontal lines above map. */
  for (i = 2 ; i <= 4 ; i++)
    WinEraseLine(i*unit, map_top-i*unit, ScrWidth-(i+2)*unit, map_top-i*unit);
  
  RctSetRectangle(&r, 0, map_top, ScrWidth, DROWS*hunit);
  WinEraseRectangle(&r, 0);
  WinDrawRectangleFrame(simpleFrame, &r); /* simple,round,popup,boldRound */

  /* Draw the rooms of the dungeon as rectangles */
  for (i = 0 ; i < MAXROOMS ; i++) {
    if (rooms[i].is_room & R_ROOM) {
      col = rooms[i].left_col;
      row = rooms[i].top_row;
      c = peek_me(row, col); /* this is a gross hack */
      if ( IS_WIZARD || (c && c != ' ') ) {
	w = rooms[i].right_col - col;
	h = rooms[i].bottom_row - row;
	/* col * 2, row * 3 + 42, w * 2, h * 3... -1 all round. */
	RctSetRectangle(&r, col * unit + 2, row * hunit + map_top + 2, 
			    w * unit - 2,   h * hunit - 1);
	WinDrawRectangleFrame(simpleFrame, &r);
      }
    }
  }
  /* Draw the rest of the dungeon square by square */
  for (col = 0 ; col < DCOLS ; col++) {
    for (row = 0 ; row < DROWS ; row++) {
      RctSetRectangle(&r, col*unit, row*hunit + map_top, unit,hunit);
      c = peek_me(row, col); /* this is a gross hack */
      if ( IS_WIZARD || (c && c != ' ') ) {
	mask = dungeon[row][col];
	if ( (mask & STAIRS) || (mask & TUNNEL) ) {
	  /*
#ifndef I_AM_COLOR
	  WinGetPattern(origPattern);
	  WinSetPattern(yourPattern);
	  WinFillRectangle(&r, 0);
	  WinSetPattern(origPattern);
#else // I_AM_COLOR //
	  //	  WinPushDrawState();
	  //	  WinSetPattern(&yourPattern);
	  WinFillRectangle(&r, 0);
	  //	  WinPopDrawState();
#endif // I_AM_COLOR
	  */
	  // Forget it!!, just do this:
	  /*
	  WinDrawLine(col*unit + 1, row*hunit + map_top + 1,
		      col*unit + 2, row*hunit + map_top + 2);
	  WinDrawLine(col*unit + 2, row*hunit + map_top + 2,
		      col*unit + 1, row*hunit + map_top + 3);
	  */
	  WinDrawLine(col*unit, row*hunit + map_top,
		      col*unit + unit, row*hunit + map_top + hunit);
	} else if (mask & FLOOR) {
	  /* 	WinDrawRectangle(&r, 0); */
	} else if (0 && (mask & HORWALL) ) {
	  WinDrawLine(col*unit + 1, row*hunit + map_top + 1,
		      col*unit + unit+1, row*hunit + map_top + 1);
	} else if (0 && (mask & VERTWALL) ) {
	  WinDrawLine(col*unit + 1, row*hunit + map_top + 1,
		      col*unit + 1, row*hunit + map_top + hunit+1);
	} else if (mask & DOOR) {
	  if (! (mask & HIDDEN)) {
	    WinInvertRectangle(&r, 0);
	  }
	}
      }
      /* used to have "else" (peekme==' ') "erase rectangle" */
    } /* end for row */
  } /* end for col */

  /* don't forget to put the rogue somewhere */
  col = sotu->roguep->col * unit - 1;
  if (col < 0) col = 0;
  row = sotu->roguep->row * hunit + map_top - 1;
  RctSetRectangle(&r, col, row, 
		      unit+1, hunit+1);
  WinDrawRectangle(&r, 0);
  return;
}


// moved do_feep to iRogue.c
/**********************************************************************
                       MAKE_IT_FIT
 IN:
 buf = the string that might be too long
 avail_space = the width (in .rcp terms) of the space it goes in
 maxbuflen = the space allocated to buf
 OUT:
 nothing (may modify buf)
 PURPOSE:
 If necessary, this truncates 'buf' and adds "..." to the end.
 **********************************************************************/
void make_it_fit(CharPtr buf, Short avail_space, Short maxbuflen) {
  /* let's not overflow the space available */
  Boolean fits;
  Int length_msg, width_msg;
  length_msg = StrLen(buf);
  width_msg = avail_space;
  FntCharsInWidth(buf, &width_msg, &length_msg, &fits);
  if (!fits) {
    width_msg -= 14; /* make room for ducklings, er, "..." */
    FntCharsInWidth(buf, &width_msg, &length_msg, &fits);
    if (length_msg+5 > maxbuflen)
      length_msg -= 5; /* to be sure.  actually this will never happen? */
    buf[length_msg+1] = '.';
    buf[length_msg+2] = '.';
    buf[length_msg+3] = '.';
    buf[length_msg+4] = '\0'; /* check for FENCEPOST error */
  }
}


/**********************************************************************
                       DRAW_CIRCLE
 IN:
 x,y = center (pixels)
 radius = radius (pixels)
 b = whether to draw it in black (or erase it in "white")
 various globals
 OUT:
 nothing
 PURPOSE:
 Draw a circle on the screen...
 **********************************************************************/
void draw_circle(Short x, Short y, Short radius, Boolean b) {
  RectangleType r;
  RctSetRectangle(&r, x-radius, y-radius, 2*radius, 2*radius);
  if (b)
    WinDrawRectangle(&r, radius);
  else 
    WinEraseRectangle(&r, radius);
}



extern struct state_of_the_union * sotu; // megaSIGH.
extern RoguePreferenceType my_prefs;

Short rogue_xor_x0, rogue_xor_y0;
static void draw_open_circle(Short radius, Short x0, Short y0,Short gap) SEC_2;
static void truncate(Short *x, Short *y, Short i) SEC_2;
static Boolean test_truncate(Short *x, Short *y) SEC_2;
#ifdef I_AM_COLOR
//IndexedColorType get_color(Char c); // in color.c
#endif // I_AM_COLOR
static const Short xfactor[8] = { 1, 2,-1,-2, 1, 2,-1,-2};
static const Short yfactor[8] = { 2, 1, 2, 1,-2,-1,-2,-1};
Boolean crosshairs_dirty = false;
void draw_crosshairs()
{
  Short ox = ScrWidth/2, oy = ScrHeight/2; // the center of screen; was 80,80
  if (crosshairs_dirty) return; // Dude!  They're already drawn.
  if (my_prefs.rogue_relative)
    what_x_y(sotu->roguep->row, sotu->roguep->col, &ox, &oy);
  rogue_xor_x0 = ox;
  rogue_xor_y0 = oy;
  invert_crosshairs(); // This looks really wacky in color...
  crosshairs_dirty = true;
}
// one_move_rogue calls invert_crosshairs and sets crosshairs_dirty to false.
void invert_crosshairs()
{
  Short ox = rogue_xor_x0, oy = rogue_xor_y0;
  Short x1, y1, x2, y2, i;
  // delta1 = 10 * walk_search / sqrt(3) = 5.7735 * walk_search ~= 23/4 * w_s
  // delta1 = magic * walk_search / 1.73.   1/1.73 ~= 37/64.
  Short delta1 = (WalkMagic*my_prefs.walk_search_border*37)/64;
  Short delta2 = (WalkMagic*my_prefs.run_walk_border*37)/64;
  // XXXX Replace 10 above with WalkMagic
#ifdef I_AM_COLOR
  if (IsColor) {
    WinPushDrawState();
    //    WinSetForeColor(get_color('"'));
  }
#endif // I_AM_COLOR
  for (i = 0 ; i < 8 ; i++) {
    x1 = ox + xfactor[i] * delta1;
    y1 = oy + yfactor[i] * delta1;
    if (test_truncate(&x1,&y1)) continue;// segment is entirely out of view..
    x2 = ox + xfactor[i] * delta2;
    y2 = oy + yfactor[i] * delta2;
    truncate(&x2,&y2,i);
    //    WinDrawGrayLine(x1,y1,x2,y2);
    WinInvertLine(x1,y1,x2,y2);
  }
  draw_open_circle(WalkMagic*my_prefs.walk_search_border, ox, oy, 3);
  draw_open_circle(WalkMagic*my_prefs.run_walk_border, ox, oy, 6);
  // really I shouldn't draw the part that falls into the message/stats areas
  // or any part that runs off the screen
#ifdef I_AM_COLOR
  if (IsColor) {
    WinPopDrawState();
  }
#endif // I_AM_COLOR
}




static void truncate(Short *x, Short *y, Short i)
{
  Short maxx = 155, maxy = 137-1, minx = 0, miny = 22+1;
  if (*x > maxx) {
    *y += (yfactor[i] * (maxx - *x))/xfactor[i];
    *x = maxx;
  }
  if (*y > maxy) {
    *x += (xfactor[i] * (maxy - *y))/yfactor[i];
    *y = maxy;
  }
  if (*x < minx) {
    *y += (yfactor[i] * (minx - *x))/xfactor[i];
    *x = minx;
  }
  if (*y < miny) {
    *x += (xfactor[i] * (miny - *y))/yfactor[i];
    *y = miny;
  }
}
static Boolean test_truncate(Short *x, Short *y)
{
  Short maxx = 159, maxy = 137-1, minx = 0, miny = 22+1, d=5;
  if ((*x > maxx-d) || (*y > maxy-d) || (*x < minx+d) || (*y < miny+d))
    return true;
  return false;
}


// Draw a circle of radius 'radius' pixels.
// this is a sad sad little function but it doesn't look all that bad really.
// so, maybe it would be good to check for being within screen boundary
static void draw_open_circle(Short radius, Short x0, Short y0, Short gap)
{
  Short x, y;
  Short maxy = 137-1, miny = 22+1;
  if (radius >= 80) return;

  for (x = 0, y = radius ; x <= y ; x += gap) {
    while (x*x + y*y > radius*radius)
      y--;
    // draw horizontalish parts
    if (y+y0 < maxy) {
      WinInvertLine( x + x0, y + y0, x + x0, y + y0);
      WinInvertLine(-x + x0, y + y0,-x + x0, y + y0);
    }
    if (-y+y0 > miny) {
      WinInvertLine( x + x0,-y + y0, x + x0,-y + y0);
      WinInvertLine(-x + x0,-y + y0,-x + x0,-y + y0);
    }
    // draw verticalish parts
    if (x+y0 < maxy) {
      WinInvertLine( y + x0, x + y0, y + x0, x + y0);
      WinInvertLine(-y + x0, x + y0,-y + x0, x + y0);
    }
    if (-x+y0 > miny) {
      WinInvertLine( y + x0,-x + y0, y + x0,-x + y0);
      WinInvertLine(-y + x0,-x + y0,-y + x0,-x + y0);
    }
    /*
    // draw horizontalish parts
    WinInvertLine( x + x0, y + y0, x + x0, y + y0);
    WinInvertLine( x + x0,-y + y0, x + x0,-y + y0);
    WinInvertLine(-x + x0, y + y0,-x + x0, y + y0);
    WinInvertLine(-x + x0,-y + y0,-x + x0,-y + y0);
    // draw verticalish parts
    WinInvertLine( y + x0, x + y0, y + x0, x + y0);
    WinInvertLine( y + x0,-x + y0, y + x0,-x + y0);
    WinInvertLine(-y + x0, x + y0,-y + x0, x + y0);
    WinInvertLine(-y + x0,-x + y0,-y + x0,-x + y0);
    */
  }
  // really I shouldn't draw the part that falls into the message area
  // or any part that runs off the screen
}



