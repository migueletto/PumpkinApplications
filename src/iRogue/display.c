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
#ifndef I_AM_OS_2
//#include "handera.h"
#endif

extern RoguePreferenceType my_prefs;

/* Unlike the original, this rogue port cannot show the whole screen
   at a time.  I think I need an x,y for the top,left visible
   character and a width,height in characters of the visible portion. 
   These should be updated as the rogue tries to walk off screen.  
   Required:
         visible_foo >= 0
	 visible_x + visible_w <= DCOLS
	 visible_y + visible_h <= DROWS
	 let's say visible_h, visible_w are constants
	 if w = DCOLS and h = DROWS, x = y = 0 as in normal rogue.
*/
/* The screen can have 15 rows, but I took 4 for messages and stats */
static Short visible_h = 11;
/* You can put about 19.9 m's on a memo line.. so.. we'll have 20 cols */
static Short visible_w = 20;
/* Height in pixels of one character ... in this case an 'M' */
static Short visible_char_h = 10;
/* Width in pixels of one character ... 160 pixels / 20 col = 8 pix/col */
static Short visible_char_w = 8;

// itsy is a smaller font
static Short visible_h_itsy = 18;
static Short visible_w_itsy = 32;
static Short visible_char_h_itsy = 6;
static Short visible_char_w_itsy = 5;
Boolean itsy_on = false;

// tile is ... well, a collection of 9x10 icons.
static Short visible_h_tile = 12;
static Short visible_w_tile = 16; /* grumble. */
static Short visible_char_h_tile = 9;
static Short visible_char_w_tile = 10;

//#define ITSY     void *font128 
//#define ITSYON   font128 = MemHandleLock(DmGetResource('NFNT', ItsyFont)); UICurrentFontPtr = font128
//#define ITSYOFF  MemHandleUnlock(font128)

// 160, 160, 11, 23, 23, 114.
Short ScrWidth = 160;
Short ScrHeight = 160;
Short LineHeight = 11;
Short ScrMsgHt = (11*2+1);
Short ScrStatsHt = (11*2+1);
Short ScrDungeonHt = (160-2*(11*2+1));
Short WalkMagic = 10;
// ScrDungeonHt = (ScrHeight-(ScrMsgHt+ScrStatsHt))

#ifndef I_AM_OS_2
void init_display_size()
{
  FntSetFont(BigFont);
  LineHeight = FntCharHeight();
  ScrMsgHt = 2 * LineHeight + 1; // "23" .. really 33 for handera???
  ScrStatsHt = ScrMsgHt; // "23"  .. (same)
  visible_char_h = FntCharHeight() - 1; // "10".. really 16-1 = 15 ???
  visible_char_w = FntCharWidth('M'); // "8".. really 11.  (not 12??)
  // note: perhaps I should change 'M' to '@' it seems wider in handera.

  FntSetFont(SmallFont);//  FntSetFont(stdFont);
  visible_char_h_itsy = FntCharHeight() - 1; // 10, for real
  visible_char_w_itsy = FntCharWidth('M');   //  8, for real

  update_display_size();
  FntSetFont(BigFont);
}

// Called initially and on event "vgaDisplayExtentChangedEvent" in
// main.c/iRogue.c.  Hey!  What if vgaDisplayExtentChangedEvent is
// received in some OTHER form?
// I think all forms need to call this ON ENTRY too.  gah.
//
// Call this AFTER form-dependent resizing.  yeah that's the ticket.
void update_display_size()
{
  Coord         x, y;
  if (!IsVGA) return;

  // Note: we must be at least OS 3.5 to call this function.
  // get the DISPLAY size
  WinGetDisplayExtent(&x, &y);

  // Update our own measurements
  ScrWidth = x; // "160"  (240 or more)
  ScrHeight = y; // "160"  (240 or more)
  WalkMagic = ( (x<y) ? x : y) / 16; // i.e. 10 if 160x160.
  ScrDungeonHt = (ScrHeight-(ScrMsgHt+ScrStatsHt));

  visible_h = ScrDungeonHt / visible_char_h; // 174? / 15? = "11" still! yeah.
  visible_h_itsy = ScrDungeonHt / visible_char_h_itsy; // 174? / 10 = 17.
  visible_h_tile = ScrDungeonHt / visible_char_h_tile;
  visible_w = ScrWidth / visible_char_w; // 240 / 11? = 21 (240/12 = 20).
  visible_w_itsy = ScrWidth / visible_char_w_itsy; // 240 / 8 = 30.
  visible_w_tile = ScrWidth / visible_char_w_tile;

  // go ahead and laugh...   (hm, should this be "D<FOO>S - 1" ?)
  if (visible_h_itsy > DROWS) visible_h_itsy = DROWS;
  if (visible_w_itsy > DCOLS) visible_w_itsy = DCOLS;
}

// draw should be true if form has been drawn before this is called
Short adjust_form_size(Boolean draw)
{
  Coord         x, y, x_diff, y_diff;
  
  FormPtr frm = FrmGetActiveForm();
  RectangleType r, erase_rect;
  if (!IsVGA) return 0;

  WinGetDisplayExtent(&x, &y); /* DISPLAY is like 160x160 (or 240xmumble)
				  regardless of FORM or WINDOW bounds */

  // get the current FORM size
  FrmGetFormBounds(frm, &r);
  // Warning:  we're comparing eggs to apples here or something.
  // WinGetDisplayExtent is "available screen size"
  // Not sure this will do the right thing for a modal bordered form!!
  //
  // for iLarn's QuestionForm,                     for an AboutForm it would be
  // in iLarn.rcp it's "AT (2 115 156 32)"            2 2 156 156
  // FrmGetFormBounds returns 0,113; 160,36           0 0 160 160
  // and WinGetWindowBounds returns 2,115; 156,32     2 2 156 156
  // GAAAAHHH!
  // (and for QuestionForm, y_diff would be totally wrong since the edge of
  // form is not at bottom of screen.)

  x_diff = x - (r.topLeft.x + r.extent.x); // i added this x_diff
  y_diff = y - (r.topLeft.y + r.extent.y);
  // if we made the thing smaller, erase what was under silkscreen
  if (draw && (y_diff < 0)) {
    // perhaps I should be using the later 'r' from WinGetWindowBounds.
    erase_rect           = r;
    erase_rect.topLeft.y = r.extent.y + y_diff;
    erase_rect.extent.y  = -y_diff;
      WinEraseRectangle(&erase_rect, 0);
  }
  // Set the WINDOW size which the form is drawn on top of
  // I've added a WinGetWindowBounds in the hopes of not fscking-up small frms.
  WinGetWindowBounds(&r);
  r.extent.x += x_diff;
  r.extent.y += y_diff; // so far we expect only y to change, really
  //r.extent.y = y;
  //r.extent.x = x;
  WinSetWindowBounds(FrmGetWindowHandle(frm), &r);
  // the form that called this function must manually REDRAW itself too.
  
  // we may need to move some objects - this will be FORM SPECIFIC.
  
  // hey, what if we are on top of another form.  eh?
  // sigh.
  return y_diff;
}


// draw should be true if form has been drawn before this is called
// We expect this to be called only for small modal forms that have a frame.
Short adjust_form_position(Boolean draw)
{
  Coord         x, y, /*x_diff,*/ y_diff=0;
  
  FormPtr frm = FrmGetActiveForm();
  RectangleType r;//, erase_rect;
  if (!IsVGA) return 0;

  WinGetDisplayExtent(&x, &y); /* DISPLAY is like 160x160 (or 240xmumble)
				  regardless of FORM or WINDOW bounds */

  // get the current form location (and size, which we won't change)
  FrmGetFormBounds(frm, &r);
  // We want r.topLeft.y + r.extent.y to add up to "y" (160 or 240 or whatever)
  // Find the value of r.topLeft.y that will do that...
  r.topLeft.y = y - r.extent.y;
  RctInsetRectangle(&r, 2); // adjust for frame width!  cool.
  WinSetWindowBounds(FrmGetWindowHandle(frm), &r);
  // well, I hope that works.
  return y_diff;
}
#endif


#ifdef I_AM_COLOR
/*
void set_char_color(Char c);
void set_draw_color();
*/
IndexedColorType get_color(Char c);
extern IndexedColorType black, white;
#endif // I_AM_COLOR

/* This is the offset to convert character-positions in
   the "visible" part of the screen represented by 'terminal'
   to the larger "real" dungeon represented by 'buffer'. */
Short visible_x = 0, visible_y = 0;

/*
typedef struct wheres_cursor_s {
  Short _cury, _curx; // absolute, I guess
  Short _maxy, _maxx;
} wheres_cursor;
wheres_cursor scr_buf;
wheres_cursor *curscr = &scr_buf;
*/
//Char terminal[visible_h_itsy][visible_w_itsy]; /* relative -
//						  a model of the screen */
Char terminal[DROWS][320/5]; /* relative - a model of the screen */
Char buffer[DROWS][DCOLS]; /* absolute - a model of the dungeon */
Boolean screen_dirty;
Boolean lines_dirty[DROWS]; /* absolute */


static void clear_visible() SEC_L;
static void maybe_move_visible_window(Short left_x, Short top_y) SEC_L;
static void put_char_at(Short row, Short col, Char ch) SEC_L;
static void clear_buffers() SEC_L;


/*
 * Switch between the small font and the regular font.
 * (Do NOT allow the small font if the SDK that I compiled with
 * is inappropriate for your operating system version.)
 * Returns true on success, false on failure (in which case the caller
 * may wish to pop up an Alert form to bitch to the user about this.)
 * It is up to the caller to redraw the screen.
 */
Boolean toggle_itsy()
{
  DWord version;
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
#ifdef I_AM_OS_2
  if (version < 0x03000000l) {
    itsy_on = !itsy_on;
    return true;
  } else itsy_on = false;
#else
  if (version >= 0x03000000l) {
    itsy_on = !itsy_on;
    return true;
  } else itsy_on = false;
#endif
  return false;
}



/* If the rogue is off the visible screen, redraw centered on rogue.
   If the rogue is on the edge of the visible screen but NOT the virtual
   screen, scroll the visible screen (observing boundary conditions.) */

/* 
 *  This will clear the part of the display that belongs to
 *  "the dungeon".  It also clears 'terminal' since that is
 *  a representation of the display.
 *  This should be called before the offset for 'terminal' is
 *  moved to show a different part of 'buffer'.
 */
extern Boolean crosshairs_dirty;
static void clear_visible() 
{
  Short i, j;
  //  Short v_h = itsy_on ? visible_h_itsy : visible_h;
  //  Short v_w = itsy_on ? visible_w_itsy : visible_w;
  Short v_h = visible_h_itsy;
  Short v_w = visible_w_itsy;

  /* Clear the physical screen. */
  RectangleType r;
  RctSetRectangle(&r, 0, ScrMsgHt, ScrWidth, ScrDungeonHt);
  qWinEraseRectangle(&r, 0);

  /* Update 'terminal' to represent the cleared screen. */
  // hm - in future (for large screens) may need to check v_h & v_w size.
  screen_dirty = true;
  for (i = 0 ; i < v_h; i++) {
    lines_dirty[i+visible_y] = true;
    for (j = 0; j < v_w; j++) {
      /*       terminal[i][j] = buffer[i+visible_y][j+visible_x]; */
      terminal[i][j] = ' ';
    }
  }
  // Make a note that the crosshairs are no longer on the screen!
  crosshairs_dirty = false;  
}

/*
 *  This will ensure that the rogue remains "on screen",
 *  i.e. it scrolls 'terminal' so that it includes the rogue.
 * (I've increased too_small_margin so that it will scroll the screen
 * BEFORE your donut hole meets the edge of the screen.. I hope.
 * Note that this is necessary & desirable only for the x-axis, not for y.)
 */
void check_rogue_position(fighter * rogue, Boolean centered) {
  Short v_h, v_w, x_margin;
  Short too_small_margin = 1;
  if (!IsTiles) {
    if (itsy_on) {
      v_h = visible_h_itsy;
      v_w = visible_w_itsy;
      x_margin = (my_prefs.walk_search_border*WalkMagic) / visible_char_w_itsy;
    } else {
      v_h = visible_h;
      v_w = visible_w;
      x_margin = (my_prefs.walk_search_border*WalkMagic) / visible_char_w;
    }
  } else {
    v_h = visible_h_tile;
    v_w = visible_w_tile;
    x_margin = (my_prefs.walk_search_border*WalkMagic) / visible_char_w_tile;
  }


  if (x_margin > 7) x_margin = 7;

  if ((rogue->col < visible_x) || (rogue->col >= visible_x + v_w) ||
      (rogue->row < visible_y) || (rogue->row >= visible_y + v_h)) {
    /* I guess the rogue teleported or something */
    //    clear_visible(); // moved to move_visible_window
    move_visible_window(rogue->col, rogue->row, true);
    return;
  } else if (centered) {
    maybe_move_visible_window(rogue->col, rogue->row);
    return;
  }
  /* The rogue is able to move off the screen from here, so scroll it. */
  // I'm getting TOO MUCH redrawing.  WHY.
  if ((0 != visible_x) &&
      (rogue->col <= visible_x + too_small_margin + x_margin)) {
    clear_visible();
    visible_x = max(visible_x - v_w / 2, 0);
  } else if ((DCOLS - v_w != visible_x) &&
	     (rogue->col >= visible_x+(v_w-1) - (too_small_margin+x_margin))){
    clear_visible();
    visible_x = min(visible_x + v_w / 2, DCOLS - v_w);
  }
  if ((0 != visible_y) &&
      (rogue->row <= visible_y + too_small_margin)) {
    clear_visible();
    visible_y = max(visible_y - v_h / 2, 0);
  } else if ((DROWS - v_h != visible_y) && 
	     (rogue->row >= visible_y + (v_h-1) - too_small_margin)) {
    clear_visible();
    visible_y = min(visible_y + v_h / 2, DROWS - v_h);
  }
}

/*
 *  This will move 'terminal' either so that it's centered on the
 *  buffer-coordinate x,y, or so that the buffer-coordinate x,y is the
 *  left top position of 'terminal'.
 */
void move_visible_window(Short left_x, Short top_y, Boolean centered) {
  Short v_h = itsy_on ? visible_h_itsy : visible_h;
  Short v_w = itsy_on ? visible_w_itsy : visible_w;
  if (IsTiles) {
    v_h = visible_h_tile;
    v_w = visible_w_tile;
  }

  clear_visible();
  if (centered) {
    left_x -= v_w / 2;
    top_y  -= v_h / 2;
  }
  if (left_x + v_w > DCOLS) left_x = DCOLS - v_w;
  if ( top_y + v_h > DROWS)  top_y = DROWS - v_h;
  // for a huge display, v_w or v_h may exceed DROWS or DCOLS,
  // so let's check for being negative *last*.
  if (left_x < 0) left_x = 0;
  if (top_y < 0) top_y = 0;
  visible_x = left_x;
  visible_y = top_y;
}
// added so that 'centered' won't redrew screen as much, I hope
static void maybe_move_visible_window(Short left_x, Short top_y) {
  Short v_h = itsy_on ? visible_h_itsy : visible_h;
  Short v_w = itsy_on ? visible_w_itsy : visible_w;
  if (IsTiles) {
    v_h = visible_h_tile;
    v_w = visible_w_tile;
  }

  left_x -= v_w / 2; // alas, v_w is odd if itsy is on.
  top_y  -= v_h / 2;
  if (left_x < 0) left_x = 0;
  if (top_y < 0) top_y = 0;
  if (left_x + v_w > DCOLS) left_x = DCOLS - v_w;
  if ( top_y + v_h > DROWS)  top_y = DROWS - v_h;
  if (visible_x != left_x || visible_y != top_y) {
    clear_visible();
    visible_x = left_x;
    visible_y = top_y;
  }
}

/* Is there room for this? */
// REturns true if it was able to scroll the window, false if at edge.
Boolean scroll_window(Short direction, Short steps) {
  Short v_h = itsy_on ? visible_h_itsy : visible_h;
  Short v_w = itsy_on ? visible_w_itsy : visible_w;
  Short left_x, top_y;
  if (IsTiles) {
    v_h = visible_h_tile;
    v_w = visible_w_tile;
  }

  left_x = visible_x;
  top_y = visible_y;
  if (steps <= 0) return false; // avoid extreme silliness
  steps = min(steps, max(DCOLS, DROWS)); // avoid extreme silliness
  if (direction == NORTH || direction == NORTHWEST || direction == NORTHEAST)
    top_y = max(top_y - steps, 0); // bounds checking: must be at least 0
  if (direction == WEST  || direction == NORTHWEST || direction == SOUTHWEST)
    left_x = max(left_x - steps, 0);
  if (direction == SOUTH || direction == SOUTHWEST || direction == SOUTHEAST)
    top_y = min(top_y + steps, DROWS - v_h); // bounds checking..
  if (direction == EAST  || direction == NORTHEAST || direction == SOUTHEAST)
    left_x = min(left_x + steps, DCOLS - v_w);
  if (visible_x == left_x && visible_y == top_y) return false; // no change
  clear_visible();
  visible_x = left_x;
  visible_y = top_y;
  return true;
}

/* first see if this works, then eliminate the j-loop like chuvmey.c #3 */
void save_vbuffer(MemPtr p, Int *offsetp, Err *errorp)
{
  Int i;
  *offsetp = 0;
  for (i = 0 ; i < DROWS ; i++) {
    *errorp = DmWrite(p, *offsetp, buffer[i], DCOLS * sizeof(Char));
      *offsetp += DCOLS * sizeof(Char);
  }
}

void mvaddch(Short row, Short col, Short ch)
{
  buffer[row][col] = ch;
  lines_dirty[row] = true;
  screen_dirty = true;
}


/* row, col are RELATIVE here */
#define VCHEAT_ION 14
#define VCHEAT_IOFF 5
static void put_char_at(Short row, Short col, Char ch)
{
 // center the dungeon vertically
  Short cheat, vcheat = itsy_on ? VCHEAT_ION : VCHEAT_IOFF;
  /* the space has already been cleared with clear_line */
  /* on second thought, no it hasn't. */
  RectangleType r;
  Short vc_w = itsy_on ? visible_char_w_itsy : visible_char_w;
  Short vc_h = itsy_on ? visible_char_h_itsy : visible_char_h;

  // change to user's favorite font
#ifdef I_AM_OS_2
    if (itsy_on)
      FntSetFont(ledFont);
#else
    if (itsy_on)
      FntSetFont(SmallFont);
#endif

  RctSetRectangle(&r, col * vc_w, (row+2) * vc_h + vcheat,
		  vc_w, vc_h); /* Left, Top, W, H */

  if (!my_prefs.black_bg || IsColor)
    WinEraseRectangle(&r, 0);
  else
    WinDrawRectangle(&r, 0);

  /* calculate pixel position of "row, col" and put char there */
  cheat = vc_w - FntCharWidth(ch);

  if (cheat <= 1)  cheat = 0;
  else             cheat /= 2;
  if (ch != ' ') {
    /* unfortunately, lower-case letters are a pain in the normal font. */
#ifdef I_AM_COLOR
    if (IsColor) {
      //      set_char_color(ch);
      WinSetTextColor(get_color(ch));
    }
#endif // I_AM_COLOR
    if (!itsy_on &&
	(ch == 'g' || ch == 'j' || ch == 'p' || ch == 'q' || ch == 'y'))
      vcheat--;
    if (!my_prefs.black_bg || IsColor)
      WinDrawChars(&ch, 1, col * vc_w + cheat, (row+2)*vc_h + vcheat);
    else
      WinDrawInvertedChars(&ch, 1, col * vc_w + cheat, (row+2)*vc_h + vcheat);
  }
  terminal[row][col] = ch;

  // change back to normal font
#ifdef I_AM_OS_2
    if (itsy_on)
      FntSetFont(stdFont);
#else
    if (itsy_on)
      FntSetFont(BigFont);
#endif
}

static void put_tile_at(Short row, Short col, Char ch)
{
#ifdef I_AM_COLOR
#define FOOD_TILE_SYM ':'
#define DASH_TILE_SYM '-'
#define FLAME_TILE_SYM '~'
  extern DmOpenRef RogueTileDB;
  Short cheat = 0, vcheat = VCHEAT_IOFF;
  UChar tile_id = ch;
  Short x, y;
  RectangleType r;
  Short vc_w = visible_char_w_tile;
  Short vc_h = visible_char_h_tile;
  //  Short v_h = visible_h_tile;
  //  Short v_w = visible_w_tile;
  //  cheat = (ScrWidth - visible_w_tile * visible_char_w_tile) / 2;

  x = col * vc_w + cheat;
  y = (row+2)*vc_h + vcheat;
  RctSetRectangle(&r, x, y, vc_w, vc_h);

  if (!my_prefs.black_bg/* || IsColor */)
    WinEraseRectangle(&r, 0);
  else
    WinDrawRectangle(&r, 0);
  if (ch != ' ') {
    VoidHand vh;
    BitmapPtr bmp_p;

    switch(tile_id) {
    case FOODCHAR: tile_id = FOOD_TILE_SYM; break;
    case EMDASH: tile_id = DASH_TILE_SYM; break;
    case FLAMECHAR: tile_id = FLAME_TILE_SYM; break;
    }
    tile_id -= START_TILE;

    //    WinDrawChars(&ch, 1, col * vc_w + cheat, (row+2)*vc_h + vcheat);
    // Find the "ch - START_TILE"th icon in the RogueTileDB !

    //    bitmap_rsc = DmGetResource('Tbmp', FooBitmapFamily); 
    vh = DmGetResourceIndex(RogueTileDB, tile_id); // xxx
    if (vh != NULL) {
      bmp_p = (BitmapPtr) MemHandleLock(vh);
      WinDrawBitmap(bmp_p, x, y);
      MemHandleUnlock(vh);
      DmReleaseResource(vh);
    }

  }
  terminal[row][col] = ch;
#endif // I_AM_COLOR
}






void refresh()
{
  Short col, line, absline;
  Short v_h = itsy_on ? visible_h_itsy : visible_h;
  Short v_w = itsy_on ? visible_w_itsy : visible_w;
  if (IsTiles) {
    v_h = visible_h_tile;
    v_w = visible_w_tile;
  }

  if (screen_dirty) {

#ifdef I_AM_COLOR
    if (IsColor) {
      WinPushDrawState();
      if (my_prefs.black_bg) WinSetBackColor(black);
      //      set_draw_color();
    }
#endif // I_AM_COLOR
    for (line = v_h - 1; line >= 0; line--) {
      /* line is RELATIVE, line+visible_y is ABSOLUTE. */
      absline = line + visible_y;
      if (lines_dirty[absline]) {
	/* clear_row(line); */ /* XXXXXX */
	for (col = 0; col < v_w; col++) {
	  /* col is RELATIVE, col+visible_x is ABSOLUTE. */
	  if (buffer[absline][col+visible_x] != terminal[line][col]) {
	    if (!IsTiles)
	      put_char_at(line, col, buffer[absline][col+visible_x]);
	    else
	      put_tile_at(line, col, buffer[absline][col+visible_x]);
	  }
	}
	lines_dirty[absline] = false;
      }
    }
#ifdef I_AM_COLOR
    if (IsColor) {
      WinPopDrawState();
    }
#endif // I_AM_COLOR
    screen_dirty = false;
  }
  // This is NOT a good place in the code to draw_crosshairs....
}


/* row, col are ABSOLUTE */
Short mvinch(Short row, Short col)
{
  return((Short) buffer[row][col]);
}

static void clear_buffers()
{
  Short i, j;

  screen_dirty = false;

  for (i = 0; i < DROWS; i++) {
    lines_dirty[i] = false;
    for (j = 0; j < DCOLS; j++) {
      buffer[i][j] = ' ';
    }
  }
  for (i = 0 ; i < visible_h_itsy; i++) {
    for (j = 0; j < visible_w_itsy; j++) {
      terminal[i][j] = ' ';
    }
  }
}

void clear(Boolean all) 
{
  RectangleType r;
  /* I draw lines at y=22 and y=137 */
  if (all)
    RctSetRectangle(&r, 0, 0, ScrWidth, ScrHeight);
  else
    RctSetRectangle(&r, 0, ScrMsgHt, ScrWidth, ScrDungeonHt);/* L, Top, W, H */
  qWinEraseRectangle(&r, 0);
  /*   cur_row = cur_col = 0; */
  //  move(0, 0);
  clear_buffers(); /* sets dirty = false */
  visible_x = visible_y = 0;
}

/* was used for dragon flame, not any more
void standout()
{
  buf_stand_out = true;
}
void standend()
{
  buf_stand_out = false;
} */

// this is now essentially identical to mvinch
Char peek_me(Short row, Short col) {
  if (0 <= row && 0 <= col && row < DROWS && col < DCOLS)
    return buffer[row][col];
  else
    return 0;
}

// Given a location that the player tapped at, return its (row,col)
// (The y is guaranteed to be within 23-136.)
// ((no, now we guarantee that: ScrMsgHt < y < ScrHeight - ScrStatsHt.))
Short what_row_col(Short x, Short y, Short *rowp, Short *colp) {
  Short col, row;
  Short vc_w = itsy_on ? visible_char_w_itsy : visible_char_w;
  Short vc_h = itsy_on ? visible_char_h_itsy : visible_char_h;
  Short vcheat = itsy_on ? VCHEAT_ION : VCHEAT_IOFF;

  if (IsTiles) {
    vc_h = visible_char_h_tile;
    vc_w = visible_char_w_tile;
    vcheat = VCHEAT_IOFF;
  }

  col = x / vc_w;
  row = ((y - vcheat) / vc_h) - 2;
  col += visible_x;
  row += visible_y;
  if (col < 0) col = 0;
  if (row < 0) row = 0;
  if (col >= DCOLS) col = DCOLS-1;
  if (row >= DROWS) row = DROWS-1;
  *rowp = row;
  *colp = col;
  return buffer[row][col];
}


// Given a (row, col) that is promised to be on the screen,
// tell what screen coordinate it was drawn at
void what_x_y(Short row, Short col, Short *x, Short *y) {
  Short vc_w = itsy_on ? visible_char_w_itsy : visible_char_w;
  Short vc_h = itsy_on ? visible_char_h_itsy : visible_char_h;
  Short vcheat = itsy_on ? VCHEAT_ION : VCHEAT_IOFF;
  Short cheat = vc_w - FntCharWidth('@');
  if (cheat <= 1)  cheat = 0;
  else             cheat /= 2;

  if (IsTiles) {
    vc_h = visible_char_h_tile;
    vc_w = visible_char_w_tile;
    vcheat = VCHEAT_IOFF;
    cheat = 0; // XXX make sure this matches put_tile_at.
  }

  row -= visible_y;
  col -= visible_x;
  *x = col * vc_w + cheat;
  *y = (row+2) * vc_h + vcheat;
  // HOWEVER, that is one corner of the cell, and we want the "center",
  // whatever that means.  I should fix what_x_y to do that.
}

