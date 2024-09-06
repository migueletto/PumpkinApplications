#include "palm.h"
#include "rogue.h" // for my EMDASH and FOODCHAR
Boolean have_read_color = false;
Boolean have_read_tiles = false;

#ifdef I_AM_COLOR
DmOpenRef MemoDB;
// this seems weird to me, but what the heck.
typedef struct {
  Char note;           // null terminated
} MemoDBRecordType;
typedef MemoDBRecordType * MemoDBRecordPtr;
#define memoDBType 'DATA'

// So.  what are the symbols.  From explain_it():
// A-Z a-z ? ! * FOODCHAR / ] ) = , . + # | EMDASH ^ % @   (and ' ')
// HERE IS WHAT WE DO to look up the color for Char c.
// If c == EMDASH then c = '-'
// If c == FOODCHAR then c = ':'
// Take the ASCII representation and subtract 33; that is your index.
/*  THese are things I'm not using at the moment after all:
  // I will also take the unused symbol '"' and use it for PIE SLICES.
  // I will also take the unused symbol '_' and use it for BACKGROUND.
  // I will also take the unused symbol '`' and use it for FOREGROUND secretly.
  //#define FG_COL_IX    ('`' - 33)
  //#define BG_COL_IX    ('_' - 33)
 */
// These are color symbols that get special cased in some way:
#define FOOD_COL_SYM ':'
#define DASH_COL_SYM '-'
#define NUM_SYMBOLS 92
IndexedColorType symbol_colors[NUM_SYMBOLS];
IndexedColorType black, white;
static Short grok_hex(Char *buf, Short len);
#endif // I_AM_COLOR


/* 
 * This is ONLY called if the OS version is at least 3.5.
 * If a memo entitled "iRogue-RGB" exists, read from it into symbol_colors
 * and set IsColor = true.  (Any symbol missing from the memo will be black.)
 * Otherwise IsColor is false (and symbol_colors remains black).
 */
#ifdef I_AM_COLOR
void look_for_memo()
{
  UInt recordNum;
  VoidHand recH;
  UInt mode = dmModeReadOnly;
  Char magic_title[20];
  Short title_len, memo_len, i, j, symbol;
  Char *memo/*, c*/;
  RGBColorType rgbcolor;
  IndexedColorType icolor;
  //  Boolean no_foreground = true;
  //  Boolean symbol_set[NUM_SYMBOLS];

  // initialize the symbol_colors array
  have_read_color = false;
  rgbcolor.r = rgbcolor.g = rgbcolor.b = 255; // white
  white = WinRGBToIndex(&rgbcolor);
  rgbcolor.r = rgbcolor.g = rgbcolor.b = 0; // black
  black = WinRGBToIndex(&rgbcolor);
  for (i = 0 ; i < NUM_SYMBOLS ; i++) {
    symbol_colors[i] = black;
    //    symbol_set[i] = false;
  }
  //  symbol_colors[BG_COL_IX] = white;
  //  symbol_set[BG_COL_IX] = true;

  // FIND KITTEN^H^H^H^H^H^HMEMO
  // first, what is our magic title:
  StrPrintF(magic_title, "iRogue-RGB%c", linefeedChr);
  title_len = StrLen(magic_title);
  // try to open the memo database
  if (!(MemoDB = DmOpenDatabaseByTypeCreator(memoDBType, sysFileCMemo, mode))){
    have_read_color = false;
    return;
  }
  recordNum = 0;
  // get a record (stop if we run out)
  while (!have_read_color &&
	 (recH = DmQueryNextInCategory(MemoDB, &recordNum, dmAllCategories))) {
    MemoDBRecordPtr memoRecP = MemHandleLock (recH);
    memo = &(memoRecP->note);
    // Does the title match our magic title?
    if (0 == StrNCompare(memo, magic_title, title_len)) {
      have_read_color = true;
      memo_len = StrLen(memo);
      i = title_len; // skip past the title!
      // Ok, we get this far.  But we forget to terminate somewhere below:
      while (i < memo_len - 1) {
	Char tmp;
	symbol = memo[i]; // this is the symbol whose color we'll define
	tmp = EMDASH;
	if (memo[i] == tmp) symbol = DASH_COL_SYM;
	tmp = FOODCHAR;
	if (memo[i] == tmp) symbol = FOOD_COL_SYM;
	symbol -= 33; // now it is an index into symbol_colors[]
	if (symbol >= 0 && symbol < NUM_SYMBOLS) {
	  // ok now skip some (probably) whitespace to get to the rrggbb
	  Boolean found = false;
	  for ( i = i+1 ; i < memo_len - 1 ; i++) {
	    //c = memo[i];
	    if (Is_Hex(memo[i])) { found = true; break; }
	    if (memo[i] == linefeedChr) { i++; break; }
	  }
	  if (!found) continue; // we've hit end of line or, maybe, EOF
	  if (memo_len < i + 6) break; // we'll hit EOF before end of rrggbb
	  // Read rrggbb color value.  First make sure they're all hexy.
	  for (j = 0 ; j < 6 ; j++)
	    if (!Is_Hex(memo[i+j]))
	      found = false;

	  // Ok, we get this far.  But we forget to terminate somewhere below:

	  // Ok if they're all hexy then understand it as a color and set it.
	  if (found) {
	    rgbcolor.r = grok_hex(&memo[i], 2);
	    rgbcolor.g = grok_hex(&memo[i+2], 2);
	    rgbcolor.b = grok_hex(&memo[i+4], 2);
	    icolor = WinRGBToIndex(&rgbcolor);
	    symbol_colors[symbol] = icolor;  // HOORAY!
	    //	    symbol_set[symbol] = true;
	    i += 6;
	    // A little extra stuff to make sure black bg gets white fg.
	    //	    if ((symbol == BG_COL_IX) && !symbol_set[FG_COL_IX])
	    //	      if ((rgbcolor.r + rgbcolor.g + rgbcolor.b < 128*3))
	    //		symbol_colors[FG_COL_IX] = white;
	  } // else the rrggbb is garbage and we skip to "Skip to the newline".
	} // else it's some freakish character we don't care about
	// We've sucked all the juice out of this line.  Skip to the newline
	while ((i < memo_len - 1) && (memo[i] != linefeedChr))
	  i++;
	if (memo[i] == linefeedChr) i++;
      } // end while(not EOF)
    } // else memo does not match, try the next memo
    MemHandleUnlock(recH);
    recordNum++;
  } // end while(records)
  DmCloseDatabase(MemoDB);
  /*
   * DONE READING MEMO
   * At this point either IsColor is true, meaning we found the memo and,
   * unless user is taunting us, set some entries of symbol_colors[];
   * or it is false, meaning we did not find a color definition memo.
   */
  // Oh yeah if we have a non-black fg, need to fix the defaulting symbols.
  //  for (i = 0 ; i < NUM_SYMBOLS ; i++)
  //    if (!symbol_set[i]) // symbol_set[BG] is always true to avoid a duh.
  //      symbol_colors[i] = symbol_colors[FG_COL_IX];
  /* Ok - since this is only called when color is turned ON (like,
   * either initially or when the user turns on that preference),
   * this is also a good place to change foreground-background color
   * maybe.  I might regret this in other forms, though.
   */

}
#endif // I_AM_COLOR


#ifdef I_AM_COLOR
// lacking a scanf function, and being uncertain whether StrAToI likes hex,..
// Requires:  There are at least len characters in buf, and those len
//            characters satisfy Is_Hex (that is they are 0-9a-fA-F)
static Short grok_hex(Char *buf, Short len)
{
  Short i, val;
  Short total = 0;
  for (i = 0 ; i < len ; i++) {
    total *= 16;
    if (buf[i] >= '0' && buf[i] <= '9')
      val = buf[i] - '0';
    else if (buf[i] >= 'a' && buf[i] <= 'f')
      val = 10 + (buf[i] - 'a');
    else if (buf[i] >= 'A' && buf[i] <= 'F')
      val = 10 + (buf[i] - 'A');
    else // silly rabbit, this is not hex
      val = 0;
    total += val;
  }
  return total;
}
#endif // I_AM_COLOR
// This might be pretty slow.  Maybe should inline it,
// Also compare how long it takes w/o bounds check.
// If inlined this would go in put_char_at() in lib_curses.c
// put_char_at is called only by refresh() so the Push and Pop should go there.
#ifdef I_AM_COLOR
IndexedColorType get_color(Char c)
{
  IndexedColorType ict = black;
  Short i;
  Char tmp;
  tmp = EMDASH;
  if (c == tmp) c = DASH_COL_SYM;
  tmp = FOODCHAR;
  if (c == tmp) c = FOOD_COL_SYM;
  i = c - 33;
  // Let's have some bounds checking here.
  if (i >= 0 && i < NUM_SYMBOLS)
    ict = symbol_colors[i];
  if (my_prefs.black_bg && ict == black)
    return white;
  else
    return ict;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//                           Tiles
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
#include "Globals.h" /* for roguetiledbname */
extern DmOpenRef RogueTileDB;

// This will set RogueTileDB and have_read_tiles,
// if it can find a database that looks like the right thing.
void look_for_tiles()
{
  LocalID dbID;
  ULong supported_depths, depth;
  Short i;

  if (RogueTileDB!=NULL) {
    DmCloseDatabase(RogueTileDB);
    RogueTileDB = NULL;
    have_read_tiles = false;
  }
  // Find the tiles DB, if it exists
  if (0 == (dbID = DmFindDatabase(0, RogueTileDBName))) return;
  if (0 == (RogueTileDB = DmOpenDatabase(0,dbID,dmModeReadOnly))) return;
  // Make sure there are "enough" tiles in it..
  if (DmNumResources(RogueTileDB) < END_TILE - START_TILE) {
    DmCloseDatabase(RogueTileDB);
    RogueTileDB = NULL;
    return;
  }    
  have_read_tiles = true; // yay

  // try setting the screen depth

  WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL,
		&supported_depths, NULL); // all allowed settings.
  // supported_depths :==  2^(depth-1)   for each supported value of 'depth' 
  // e.g. 2^(4-1) + 2^(2-1) + 2^(1-1) = 11, depth == { 4 or 2 or 1 }, 3.5-en-ez
  depth = supported_depths << 1;
  i = 0; 
  while (depth > 1) {
    depth = depth >> 1;
    i++;
  }
  depth = i;
  WinScreenMode(winScreenModeSet, 0, 0, &depth, 0);

}


#endif // I_AM_COLOR

