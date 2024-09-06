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

//#include <SystemMgr.rh> // for user name
//#include <DLServer.h> // for user name
//#include <SoundMgr.h> // for feeping
#include <Preferences.h> // for date formats

#include "rogue.h"
extern DmOpenRef       RogueDB;
extern UChar revivify_p;

static void set_file_len(Short filenum, Short qty);
static Short get_file_start(Short filenum, Short *qty);

static void save_character_id(struct id * kind, Short kindnum, UInt index);
static UInt save_character_objs(object * start, UInt *indexp); // no sec
static void load_character_id(struct id * idarray, Short num,
			      UInt index, struct state_of_the_union *sotu); //n
static void load_character_objs(object * start, UInt *indexp, Short num); // ns
static void fix_verbed_objs(object *pack, fighter *rogue);  // no sec


// see save_character.
// records 0-9 are top10 records
#define MOREMAGIC 10
// Record how many records the ith file takes, in position i+1.
// Also, in position 0, record how many files there are.
void create_filecount_record()
{
  //Err error;
  VoidHand fooRec;
  MemPtr p;
  UInt16 i = MOREMAGIC;

  fooRec = DmNewRecord(RogueDB, &i, (1+MAX_SAVE_FILES) * sizeof(Short));
  p = MemHandleLock(fooRec);
  /* zero it */
  /*error =*/ DmSet(p, 0, (1+MAX_SAVE_FILES) * sizeof(Short), 0); /* offset, val */
  MemPtrUnlock(p);
  DmReleaseRecord(RogueDB, i, true); /* rec num */
}
// Keep track of how many records there are in each file
// This inserts an element in the array or if qty==0 removes that element.
static void set_file_len(Short filenum, Short qty)
{
  //Err error;
  VoidHand vh;
  UInt index = MOREMAGIC;
  MemPtr p;
  Short num_files, tmp, i, *qtys;
  filenum++; // change from 0 based to 1 based because I'm using the 0 slot.
  vh = DmGetRecord(RogueDB, index);
  p = MemHandleLock(vh);
  if (!p) return; // error
  qtys = (Short *) p;
  num_files = qtys[0];
  if (qty > 0) {    // insert
    for (i = num_files ; i >= filenum ; i--) {
      tmp = qtys[i]; // move i to i+1 ...
      /*error =*/ DmWrite(p, (i+1) * sizeof(Short), &tmp, sizeof(Short));
    }
    /*error = */ DmWrite(p, (filenum) * sizeof(Short), &qty, sizeof(Short));
    num_files++;
    /*error = */ DmWrite(p, 0, &num_files, sizeof(Short));
  } else {    // remove
    // move i+1 to i ...
    for (i = filenum ; i < num_files ; i++) {
      tmp = qtys[i+1]; // move i+1 to i ...
      /*error = */ DmWrite(p, i * sizeof(Short), &tmp, sizeof(Short));
    }
    num_files--;
    /*error = */ DmWrite(p, 0, &num_files, sizeof(Short));
  }
  MemPtrUnlock(p);
  DmReleaseRecord(RogueDB, index, true);
}
// Get the number of save files in the database
Short get_file_count()
{
  VoidHand vh;
  UInt index = MOREMAGIC;
  MemPtr p;
  Short qty, *lengths;
  vh = DmQueryRecord(RogueDB, index);
  p = MemHandleLock(vh);
  if (!p) return -1;
  lengths = (Short *) p;
  qty = lengths[0];
  MemHandleUnlock(vh);
  return qty;
}
// Get the record index at which this file starts.
// qty also gets the number of records in this file.
static Short get_file_start(Short filenum, Short *qty)
{
  VoidHand vh;
  UInt index = MOREMAGIC;
  MemPtr p;
  Short i, len, *lengths;
  //  filenum++; // change from 0 based to 1 based because I'm using the 0 slot.
  vh = DmQueryRecord(RogueDB, index);
  p = MemHandleLock(vh);
  if (!p) return -1;
  lengths = (Short *) p;
  if (filenum >= lengths[0]) {
    filenum = lengths[0];
    *qty = 0;
  } else
    *qty = lengths[filenum+1];
  for (i = 1, len = 0 ; i <= filenum ; i++)
    len += lengths[i];
  MemHandleUnlock(vh);
  return len + 11;
}

/**********************************************************************
                 DELETE_SAVED_ROGUE
 IN:
 filenum = the index of the series-of-records to delete
 0 is the autosave slot, 1 is the first snapshot, 2 second snapshot etc.
 PURPOSE:
 Called before overwriting a slot, or because the user wants to.
**********************************************************************/
void delete_saved_rogue(Short filenum)
{
  Short i, start, num; //, end;
  /* I could use 'index', but I'm distrustful. */
  //  end = DmNumRecords(RogueDB);
  start = get_file_start(filenum, &num);
  for (i = 0 ; i < num ; i++)
    DmRemoveRecord(RogueDB, start); // yes, this is what I mean to do
  set_file_len(filenum, 0);
}
/**********************************************************************
                 UNDISPLAY_SAVED_ROGUE
 PURPOSE:
 Clear an area on the screen
**********************************************************************/
extern Short LineHeight; // display.c.  "11" in 160x160 mode.
#define SHOWSAVE_TOP (LineHeight*4+1)
void undisplay_saved_rogue()
{
  RectangleType r;
#ifdef I_AM_OS_2
  RctSetRectangle(&r, 0, 45, 156, 77);  // 7 lines of text possible?
#else
  Coord         x, y;
  WinGetDisplayExtent(&x, &y);
  RctSetRectangle(&r, 0, SHOWSAVE_TOP, y, 7*LineHeight);
#endif
  WinEraseRectangle(&r, 0);

}
/**********************************************************************
                 DISPLAY_SAVED_ROGUE
 IN:
 Which save slot to display data from (warning, it is not bounds checked.)
 PURPOSE:
 Print some data from a save slot to the screen, so the user can
 decide whether to delete/load/whatever it
**********************************************************************/
void display_saved_rogue(Short filenum)
{
  // This can be used for Snapshot or Revivify
  VoidHand vh, vh2;
  UInt index, index2;
  Short ignore=0, x=2, y=SHOWSAVE_TOP; // was 45
  MemPtr p, p2;
  struct state_of_the_union *load_sotu;
  fighter * load_rogue;
  Char buf[80];
  DateTimeType datetime;
  //  DateFormatType datetype;  TimeFormatType timetype;
  Char datestr[dateStringLength], timestr[timeStringLength];

  index = (UInt) get_file_start(filenum, &ignore);
  index2 = index + 1;
  vh = DmQueryRecord(RogueDB, index);
  p = MemHandleLock(vh);
  if (!p) return;
  load_sotu = (struct state_of_the_union *) p;
  vh2 = DmQueryRecord(RogueDB, index2);
  p2 = MemHandleLock(vh2);
  if (!p2) { MemHandleUnlock(vh); return; }
  load_rogue = (fighter *) p2;

  StrPrintF(buf, "Name: %s", load_sotu->username);
  WinDrawChars(buf, StrLen(buf), x, y); // left, top
  y += LineHeight;

  TimSecondsToDateTime(load_sotu->birthdate, &datetime); // see DateTime.h
  DateToAscii(datetime.month, datetime.day, datetime.year,
	      PrefGetPreference(prefDateFormat), datestr);
  TimeToAscii(datetime.hour, datetime.minute,
	      PrefGetPreference(prefTimeFormat), timestr);
  StrPrintF(buf, "Birthdate: %s %s", datestr, timestr);
  WinDrawChars(buf, StrLen(buf), x, y); // left, top
  y += LineHeight;
  TimSecondsToDateTime(load_sotu->timestamp, &datetime); // see DateTime.h
  DateToAscii(datetime.month, datetime.day, datetime.year,
	      PrefGetPreference(prefDateFormat), datestr);
  TimeToAscii(datetime.hour, datetime.minute,
	      PrefGetPreference(prefTimeFormat), timestr);
  StrPrintF(buf, "Timestamp: %s %s", datestr, timestr);
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LineHeight;
  // Somewhere in here, I get a fatal error that is symptom of overwrite bug.
  //  StrPrintF(buf, "L %d HP %d", load_sotu->cur_level,
  //	    load_rogue->hp_current
  //	    ); 
  //  WinDrawChars(buf, StrLen(buf), x, y);
  //  y += LineHeight;
  //  MemHandleUnlock(vh);  return;
  StrPrintF(buf, "L %d HP %d(%d) Str %d(%d) %s", load_sotu->cur_level,
	    load_rogue->hp_current, load_rogue->hp_max,
	    load_rogue->str_current, load_rogue->str_max,
	    load_sotu->hunger_str); 
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LineHeight;
  //  StrPrintF(buf, "Arm: %d $ %ld Exp %d/%ld", get_armor_class(load_sotu),
  StrPrintF(buf, "$ %ld Exp %d/%ld",
	    load_rogue->gold, load_rogue->exp, load_rogue->exp_points); 
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LineHeight;
  // there are a couple more lines left if I can think of something
  StrPrintF(buf, "Status: ");
  /*  Revised to combine score_status and wizard..
  if (!(load_sotu->score_status & (STATUS_ISDEAD|STATUS_UNDEAD)))
    StrCat(buf, "live ");
  else if (load_sotu->score_status & STATUS_ISDEAD)
    StrCat(buf, "dead ");
  else if (load_sotu->score_status & STATUS_UNDEAD)
    StrCat(buf, "undead "); // yeah, you can be dead & undead but it sounds ODD
  if (load_sotu->wizard)
    StrCat(buf, "wizard ");
  else if (load_sotu->score_status & STATUS_WASWIZ)
    StrCat(buf, "former wizard ");
  */
  if (!(load_sotu->conduct & (CONDUCT_ISDEAD|CONDUCT_UNDEAD)))
    StrCat(buf, "live ");
  else if (load_sotu->conduct & CONDUCT_ISDEAD)
    StrCat(buf, "dead ");
  else if (load_sotu->conduct & CONDUCT_UNDEAD)
    StrCat(buf, "undead "); // yeah, you can be dead & undead but it sounds ODD
  if (load_sotu->conduct & CONDUCT_ISWIZ)
    StrCat(buf, "wizard ");
  else if (load_sotu->conduct & CONDUCT_WASWIZ)
    StrCat(buf, "former wizard ");

  WinDrawChars(buf, StrLen(buf), x, y);
  y += LineHeight;
  MemHandleUnlock(vh);
  MemHandleUnlock(vh2);
}
#undef SHOWSAVE_TOP

/**********************************************************************
                 KILL_SAVED_ROGUE
 IN:
 status = bits to turn on, probably dead
 filenum = which save slot to turn bits on in
 PURPOSE:
 Turn on desired status bits (this does not let you turn them OFF).
 Specifically we want to set DEAD when you enter the TOPTEN list.
 Not yet done: add a birthdate argument, instead of filenum, and kill
 any save slot that matches it..
**********************************************************************/
void kill_saved_rogue(UChar status, Short filenum)
{
  //Err error;
  VoidHand vh;
  Short ignore=0;
  UInt index;
  MemPtr p;
  struct state_of_the_union *load_sotu;
  index = get_file_start(filenum, &ignore);
  vh = DmGetRecord(RogueDB, index);
  p = MemHandleLock(vh);
  if (!p) return; // error
  load_sotu = (struct state_of_the_union *) p;
  //status |= load_sotu->score_status;
  status |= (load_sotu->conduct & CONDUCT_FILESTATUS) >> 8;
  /*error =*/ DmWrite(p, 2*sizeof(ULong), &status, sizeof(UChar));
  MemPtrUnlock(p);
  DmReleaseRecord(RogueDB, index, true);
}

/**********************************************************************
                 GET_SAVED_ROGUE_STATUS
 IN:
 filenum = which save slot to check
 PURPOSE:
 return status so caller can check for dead, undead.
**********************************************************************/
UChar get_saved_rogue_status(Short filenum)
{
  VoidHand vh;
  UInt index;
  Short ignore=0;
  MemPtr p;
  struct state_of_the_union *load_sotu;
  UChar val = STATUS_ISDEAD;
  index = get_file_start(filenum, &ignore);
  vh = DmQueryRecord(RogueDB, index);
  p = MemHandleLock(vh);
  if (!p) return val; // error, so may as well be dead
  load_sotu = (struct state_of_the_union *) p;
  //val = load_sotu->score_status;
  val = (load_sotu->conduct & CONDUCT_FILESTATUS) >> 8;
  MemHandleUnlock(vh);
  return val;
}


/**********************************************************************
                       SAVE_CHARACTER_ID
 IN:
 kind = one of the id_foo arrays
 kindnum = the number of items in that array
 index = the database index to pick up at
 OUT:
 nothing
 PURPOSE:
 Called when the character data is being saved to database records.
 This routine saves the various "id_foo" arrays.
 **********************************************************************/
static void save_character_id(struct id * kind, Short kindnum, UInt index)
{
  //Err error;
  VoidHand fooRec;
  MemPtr p;
  Int i, offset, size;
  Short len1, len2;

  size = 0; // how much space to allocate
  for (i = 0 ; i < kindnum ; i++) {
    size += sizeof(Short) * 2; /* the two string lengths (oh what the hell) */
    size += sizeof(Short) + sizeof(UShort); /* value, id_status */
    size += StrLen(kind[i].title) + 2; /* len, \0, and add one if odd */
    size += StrLen(kind[i].real) + 2; /* ditto (maintain alignment) */
  }

  fooRec = DmNewRecord(RogueDB, &index, size);
  p = MemHandleLock(fooRec);
  offset = 0;
  for (i = 0 ; i < kindnum ; i++) {
    len1 = StrLen(kind[i].title) + 1;
    if (len1 % 2 != 0) len1++; /* maintain alignment. */
    len2 = StrLen(kind[i].real) + 1;
    if (len2 % 2 != 0) len2++; /* any unused space is 0.. I think. */
    /*error = */ DmWrite(p, offset, &len1, sizeof(Short));
    offset += sizeof(Short);
    /*error = */ DmWrite(p, offset, &len2, sizeof(Short));
    offset += sizeof(Short);
    /*error = */ DmWrite(p, offset, &(kind[i].value), sizeof(Short));
    offset += sizeof(Short);
    /*error = */ DmWrite(p, offset, &(kind[i].id_status), sizeof(UShort));
    offset += sizeof(UShort);
    /*error = */ DmWrite(p, offset, kind[i].title, len1 * sizeof(Char));
    offset += len1 * sizeof(Char);
    /*error = */ DmWrite(p, offset, kind[i].real, len2 * sizeof(Char));
    offset += len2 * sizeof(Char);
  }
  MemPtrUnlock(p);

  DmReleaseRecord(RogueDB, index, true);
}
/**********************************************************************
                       SAVE_CHARACTER_OBJS
 IN:
 start = the start of a list of items to save
 indexp = the current database record index
 OUT:
 number of objects written to database (also, indexp is updated)
 PURPOSE:
 Save the items in the 'start' list (level_objects/level_monsters/pack)
 **********************************************************************/
/* for each object, write a record.  return number written.  */
/* BUG: items worn get that stuck on their name forever?! how? */
static UInt save_character_objs(object * start, UInt *indexp)
{
  //Err error;
  VoidHand fooRec;
  MemPtr p;
  object * tmp;
  object * obj = start->next_object;
  UInt x = 0;

  while (obj) {

    fooRec = DmNewRecord(RogueDB, indexp, sizeof(object));
    p = MemHandleLock(fooRec);
    tmp = obj->next_object;
    obj->next_object = (object*) 0;
    /*error =*/ DmWrite(p, 0, obj, sizeof(object));
    obj->next_object = tmp;
    MemPtrUnlock(p);
    DmReleaseRecord(RogueDB, *indexp, true);
    
    (*indexp)++;
    obj = obj->next_object;
    x++;
    
  }

  return x;
}
/**********************************************************************
                       LOAD_CHARACTER_ID
 IN:
 idarray = one of the id_foo arrays
 num = the number of items in that array
 index = the database index to pick up at
 OUT:
 nothing
 PURPOSE:
 Called when the character data is being loaded from database records.
 This routine loads the various "id_foo" arrays.
 **********************************************************************/
static void load_character_id(struct id * idarray, Short num, UInt index, struct state_of_the_union *sotu) 
{
  VoidHand vh;
  VoidPtr p;
  Short i;
  Short * short_p;
  UShort * ushort_p;
  Char * char_p;
  
  Short len1, len2;
  Char buf[40];

  vh = DmQueryRecord(RogueDB, index); /* read-only, promise */
  p = MemHandleLock(vh);
  if (p) {
    short_p = (Short *) p;
    for (i = 0 ; i < num ; i++) {
      len1 = short_p[0];
      len2 = short_p[1];
      idarray[i].value = short_p[2];
      short_p += 3;
      ushort_p = (UShort *) short_p;
      //      message("chomp chomp", sotu);
      idarray[i].id_status = ushort_p[0];
      ushort_p++;
      if (0) {
	// 0014 0012 505 0
	// 2764 7275 26732 8291
	StrPrintF(buf, "%x %x  %d %d", len1, len2, 
		  idarray[i].value, idarray[i].id_status);
	message(buf, sotu);
      }
      char_p = (Char *) ushort_p;
      //    message("chomp", sotu);
      MemMove(idarray[i].title, char_p, sizeof(Char) * len1);
      char_p += len1;
      MemMove(idarray[i].real, char_p, sizeof(Char) * len2);
      char_p += len2;
      short_p = (Short *) char_p;
    }
  }
  MemHandleUnlock(vh);

}


/**********************************************************************
                       LOAD_CHARACTER_OBJS
 IN:
 start = the start of a list of items to load
 indexp = the current database record index
 num = the number of items to load
 OUT:
 nothing (well, indexp is updated)
 PURPOSE:
 Load the items into the 'start' list (level_objects/level_monsters/pack)
 **********************************************************************/
/* for each object, write a record.  return number written.  */
static void load_character_objs(object * start, UInt *indexp, Short num)
{
  VoidHand vh;
  VoidPtr p;
  Short i;
  object *obj, *prev_obj;

  prev_obj = start;
  for (i = 0 ; i < num ; i++) {
    vh = DmQueryRecord(RogueDB, (*indexp)); /* read-only, promise */
    p = MemHandleLock(vh);
    if (p) {
      obj = (object *) md_malloc(sizeof(object));
      prev_obj->next_object = obj;
      MemMove(obj, (object *) p, sizeof(object));
      obj->next_object = (object *) 0; /* need to re-zero pointers */
      prev_obj = obj;
    }
    MemHandleUnlock(vh);
    (*indexp)++;
  }
}


/**********************************************************************
                       FIX_VERBED_OBJS
 IN:
 pack = the list of objects the rogue is holding
 rogue
 OUT:
 nothing
 PURPOSE:
 Called when the rogue data is loaded from database records;
 this updates some pointers (from the rogue into the pack list.)
 Hooray for redundancy.
 **********************************************************************/
static void fix_verbed_objs(object *pack, fighter *rogue) {
  /* for each item in pack,
     if worn, wielded, or put on ... */
  object *obj;
  ULong ring_flag;
  Short i;
  obj = pack->next_object;
  while (obj) {
    switch(obj->what_is) {
    case RING:
      if (obj->in_use_flags & ON_EITHER_HAND) {
	ring_flag = obj->in_use_flags >> 2; // =~ divide by 4
	// Right-shift it into nothing; the index is #shifts - 1
	// (I think!  XXXX)
	for (i = 0 ; (i < 8) && ring_flag ; i++) {
	  ring_flag = ring_flag >> 1;
	  if (!ring_flag)
	    rogue->rings[i] = obj;
	}
      } // else not being worn.
      break;
    case WEAPON:
      if (obj->in_use_flags & BEING_WIELDED) {
	rogue->weapon = obj;
      }
      break;
    case ARMOR:
      if (obj->in_use_flags & BEING_WORN) {
	rogue->armor = obj;
      }
      break;
    }
    obj = obj->next_object;
  }
}

/**********************************************************************
                       SAVE_CHARACTER
 IN:
 sotu = various globals (all of 'em)
 OUT:
 nothing
 PURPOSE:
 Called when a game is in progress, i.e. the rogue is currently alive,
 and player 'preferences' are to save the game between sessions.
 This will basically write all the interesting data from the heap to a
 bunch of database records, which will be loaded into the heap again,
 the next time the player starts the Rogue application.
 It is very long, icky, and inefficient.
 **********************************************************************/
void save_character(struct state_of_the_union * sotu, Short filenum)
{
  
  /* 5 Boolean, 8 Char, 14 Short */
  /*
  Boolean being_held   see_invisible   detect_monster   wizard   score_only
  Char hunger_str[8]
  Short cur_level max_level cur_room party_room r_de m_moves foods
        party_counter bear_trap halluc blind confused levitate haste_self
  */
  /* Plus all the stuff that's pointed to. */

  Err error;
  VoidHand fooRec;
  UInt index;
  MemPtr p;
  //  UInt attr; /* category */
  UInt object_count[4];
  //  Int i, j, offset;
  //  Char tmpc;
  Int i, offset;
  Short ignore=0, numfiles, myindex, mystart;
  //  Char buf[40];

  /* 
   *   0th:
   *   I ought to nuke anything that's in the database already....
   */
  numfiles = get_file_count();
  if (filenum < numfiles)
    delete_saved_rogue(filenum);

  /* 
   * FIRST:
   *   Save the 'sotu' record.
   *   Including pointers - we'll zero those on reload.
   */

  index = mystart = get_file_start(filenum, &ignore);
  sotu->timestamp = TimGetSeconds(); // record what time this was saved

  fooRec = DmNewRecord(RogueDB, &index, sizeof(struct state_of_the_union));
  p = MemHandleLock(fooRec);
  error = DmWrite(p, 0, sotu, sizeof(struct state_of_the_union));
  /* I expected this not to work, but it does, I think. */
  MemPtrUnlock(p);

  /* Set the category of the record, for jollies.
  DmRecordInfo (RogueDB, index, &attr, NULL, NULL);
  attr |= dmUnfiledCategory;
  DmSetRecordInfo (RogueDB, index, &attr, NULL);   */

  /* Release the record, dirty=true. */
  DmReleaseRecord(RogueDB, index, true);

  /* 
   * SECOND:
   *   Save the 'rogue' record.
   *   Including pointers - we'll zero those on reload.
   */
  index += 1;

  fooRec = DmNewRecord(RogueDB, &index, sizeof(fighter));
  p = MemHandleLock(fooRec);
  error = DmWrite(p, 0, sotu->roguep, sizeof(fighter));
  MemPtrUnlock(p);

  DmReleaseRecord(RogueDB, index, true);

  /* 
   * THIRD:
   *   Save the 'dungeon' array.
   */
  index += 1;

  fooRec = DmNewRecord(RogueDB, &index, sizeof(UShort) * DROWS * DCOLS);
  p = MemHandleLock(fooRec);
  offset = 0;
  //  for (i = 0 ; i < DROWS ; i++) {
  //    for (j = 0 ; j < DCOLS ; j++) {
  //      /* there ought to be a better way */
  //      error = DmWrite(p, offset, &(sotu->dungeon[i][j]), sizeof(UShort));
  //      offset += sizeof(UShort);
  //    }
  for (i = 0 ; i < DROWS ; i++) {
    error = DmWrite(p, offset, sotu->dungeon[i], DCOLS * sizeof(UShort) );
    offset += DCOLS * sizeof(UShort);
  }
  MemPtrUnlock(p);

  DmReleaseRecord(RogueDB, index, true);

  /* 
   * THIRD AND A HALF:
   *   Save the 'buffer' array - this will be colossally slow
   */
  index += 1;

  fooRec = DmNewRecord(RogueDB, &index, sizeof(Char) * DROWS * DCOLS);
  p = MemHandleLock(fooRec);
  offset = 0;
  //    for (i = 0 ; i < DROWS ; i++) {
  //      for (j = 0 ; j < DCOLS ; j++) {
  //        tmpc = peek_me(i, j);
  //        /* maybe I could just have the library write it */
  //        error = DmWrite(p, offset, &(tmpc), sizeof(Char));
  //        offset += sizeof(Char);
  //      }
  //    }
  save_vbuffer(p, &offset, &error);
  MemPtrUnlock(p);

  DmReleaseRecord(RogueDB, index, true);

  /* 
   * FOURTH:
   *   Save the 'rooms' array.
   */
  index += 1;

  fooRec = DmNewRecord(RogueDB, &index, sizeof(room) * MAXROOMS);
  p = MemHandleLock(fooRec);
  offset = 0;
  for (i = 0 ; i < MAXROOMS ; i++) {
    error = DmWrite(p, offset, &(sotu->rooms[i]), sizeof(room));
    offset += sizeof(room);
  }
  MemPtrUnlock(p);

  DmReleaseRecord(RogueDB, index, true);

  /* 
   * FIFTH:
   *   Save the 'traps' array (teeny tiny)
   */
  index += 1;

  fooRec = DmNewRecord(RogueDB, &index, sizeof(trap) * MAX_TRAPS);
  p = MemHandleLock(fooRec);
  offset = 0;
  for (i = 0 ; i < MAX_TRAPS ; i++) {
    error = DmWrite(p, offset, &(sotu->traps[i]), sizeof(trap));
    offset += sizeof(trap);
  }
  MemPtrUnlock(p);

  DmReleaseRecord(RogueDB, index, true);

  /* 
   * SIXTH:
   *   Save the 'id_FOO' arrays - they're all the same struct type
   *   scrolls, potions, wands, rings, weapons, armors
   *      better have a record for each... size limitation...
   */
  save_character_id(sotu->id_scrolls, SCROLLS, ++index);
  save_character_id(sotu->id_potions, POTIONS, ++index);
  save_character_id(sotu->id_wands, WANDS, ++index);
  save_character_id(sotu->id_rings, RINGS, ++index);
  //  save_character_id(sotu->id_weapons, WEAPONS, ++index);
  //  save_character_id(sotu->id_armors, ARMORS, ++index);
  /* seems to be working so far.... */

  /* 
   * SEVENTH:
   *   Save the 'rogue->pack', 'level_monsters', and 'level_objects'
   */
  index += 1;
  offset = index; /* yes, I'm skipping a record on purpose. */
  //  index += 1;
  object_count[0] = save_character_objs(&(sotu->roguep->pack), &index);
  object_count[1] = save_character_objs(sotu->level_monsters, &index);
  object_count[2] = save_character_objs(sotu->level_objects, &index);
  object_count[3] = 0;
  // Ok, here 'index' is as high as it will ever be.(?)  Write it down.
  myindex = index - (mystart-1); // Ha, this is what I forgot to subtract
  set_file_len(filenum, myindex);
  //  StrPrintF(buf, "files %d len %d", numfiles, myindex);
  //  message(buf, sotu);

  /* 
   * SIXTH and a half
   *   Save the 'is_wood' array... along with object-count info ...
   */
  index = offset; /* (restore skipped index from temporary) */

  fooRec = DmNewRecord(RogueDB, &index, 
		       sizeof(UInt) * 4 + sizeof(Boolean) * WANDS);
  p = MemHandleLock(fooRec);
  offset = 0;
  /* now write the number of pack items, level monsters, level items,
     and a 0 just to keep things tidy */
  for (i = 0 ; i < 4 ; i++) {
    error = DmWrite(p, offset, &(object_count[i]), sizeof(UInt));
    offset += sizeof(UInt);
  }
  /* ok, go ahead with the is_wood */
  for (i = 0 ; i < WANDS ; i++) {
    error = DmWrite(p, offset, &(sotu->is_wood[i]), sizeof(Boolean));
    offset += sizeof(Boolean);
  }
  MemPtrUnlock(p);

  /* THAT IS ALL.  I think. */

}

/* check out that database, dude! */
/**********************************************************************
                       LOAD_CHARACTER
 IN:
 sotu = various globals (all of 'em)
 OUT:
 nothing
 PURPOSE:
 As promised, this will load stuff from the database records (which
 will then be deleted) into the heap.  (It is called when such records
 exist and the application has just been started.)
 Long and tedious.
 **********************************************************************/
// So, like, if you notice that you're dead, what should you do?
void load_character(struct state_of_the_union * sotu, Short filenum)
{
  VoidHand vh;
  VoidPtr p;

  struct state_of_the_union *load_sotu;
  fighter * load_rogue;
  UShort * load_dungeon;
  room * load_room;
  trap * load_trap;
  Char * load_buf;
  UInt * uint_p;
  Boolean * boolean_p;

  Int i, j;
  UInt object_count[4];
  UInt index;
  Short ignore=0;


/*   if (1 < DmNumRecords(RecipeDB)) */

  /* 
   * FIRST:
   *   Load the 'sotu' record.
   *   Skip pointers.
   */

  index = get_file_start(filenum, &ignore);

  vh = DmQueryRecord(RogueDB, index); /* read-only, promise */
  p = MemHandleLock(vh);
  if (p) {
    load_sotu = (struct state_of_the_union *) p;

    if (false) // if (my_prefs.multiuser)
      if (revivify_p == STATUS_ISDEAD) {
	// enable multiple users!  don't penalize a very different save file
	if ((sotu->birthdate != load_sotu->birthdate) &&
	    (0 != StrCompare(sotu->username, load_sotu->username))) {
	  revivify_p = STATUS_NORMAL;
	  message("Welcome to multi-user rogue", sotu);
	}
      }

    sotu->being_held = load_sotu->being_held;
    sotu->see_invisible = load_sotu->see_invisible;
    sotu->detect_monster = load_sotu->detect_monster;
    //    sotu->wizard = load_sotu->wizard;
    //    sotu->score_status = load_sotu->score_status;
    sotu->conduct = load_sotu->conduct;
    sotu->birthdate = load_sotu->birthdate;
    sotu->timestamp = load_sotu->timestamp; // for no good reason

    StrNCopy(sotu->hunger_str, load_sotu->hunger_str, 8);
    StrNCopy(sotu->username, load_sotu->username, 32);

    sotu->cur_level = load_sotu->cur_level;
    sotu->max_level = load_sotu->max_level;
    sotu->cur_room = load_sotu->cur_room;
    sotu->party_room = load_sotu->party_room;
    sotu->r_de = load_sotu->r_de;
    sotu->m_moves = load_sotu->m_moves;
    sotu->foods = load_sotu->foods;
    sotu->party_counter = load_sotu->party_counter;
    sotu->bear_trap = load_sotu->bear_trap;
    sotu->halluc = load_sotu->halluc;
    sotu->blind = load_sotu->blind;
    sotu->confused = load_sotu->confused;
    sotu->levitate = load_sotu->levitate;
    sotu->haste_self = load_sotu->haste_self;
    //    sotu->fruit_number = load_sotu->fruit_number;

    sotu->warning_level = load_sotu->warning_level;
    sotu->warning_moves = load_sotu->warning_moves;

    /*
      // instead, we're recalculating these by calling ring_stats
    sotu->ring_flags = load_sotu->ring_flags;
    sotu->r_rings = load_sotu->r_rings;
    sotu->e_rings = load_sotu->e_rings;
    sotu->stealthy = load_sotu->stealthy;
    sotu->add_strength = load_sotu->add_strength;
    sotu->regeneration = load_sotu->regeneration;
    sotu->ring_dex = load_sotu->ring_dex;
    sotu->auto_search = load_sotu->auto_search;
    */
    //    init_fruit();
  }
  MemHandleUnlock(vh);

  /* 
   * SECOND:
   *   Load the 'rogue' record.
   *   Skip pointers.
   */

  index += 1;
  vh = DmQueryRecord(RogueDB, index); /* read-only, promise */
  p = MemHandleLock(vh);
  if (p) {
    load_rogue = (fighter *) p;
    
    sotu->roguep->hp_current = load_rogue->hp_current;
    sotu->roguep->hp_max = load_rogue->hp_max;
    sotu->roguep->str_current = load_rogue->str_current;
    sotu->roguep->str_max = load_rogue->str_max;
    sotu->roguep->gold = load_rogue->gold;
    sotu->roguep->exp = load_rogue->exp;
    sotu->roguep->exp_points = load_rogue->exp_points;
    sotu->roguep->row = load_rogue->row;
    sotu->roguep->col = load_rogue->col;
    sotu->roguep->fchar = load_rogue->fchar;
    sotu->roguep->moves_left = load_rogue->moves_left;
    /* go through inventory to find armor, weapon, left/right rings */
  }
  MemHandleUnlock(vh);

  /* 
   * THIRD:
   *   Load the 'dungeon' array.
   */

  index += 1;
  vh = DmQueryRecord(RogueDB, index); /* read-only, promise */
  p = MemHandleLock(vh);
  if (p) {
    load_dungeon = (UShort *) p;
    for (i = 0 ; i < DROWS ; i++) {
      for (j = 0 ; j < DCOLS ; j++) {
	sotu->dungeon[i][j] = *load_dungeon;
	load_dungeon++;
      }
    }
  }
  MemHandleUnlock(vh);

  /* 
   * THIRD AND A HALF:
   *   Load the 'buffer' array - this will be colossally slow
   */

  index += 1;
  vh = DmQueryRecord(RogueDB, index); /* read-only, promise */
  p = MemHandleLock(vh);
  if (p) {
    load_buf = (Char *) p;
    for (i = 0 ; i < DROWS ; i++) {
      for (j = 0 ; j < DCOLS ; j++) {
	mvaddch(i, j, *load_buf);
	load_buf++;
      }
    }
  }
  MemHandleUnlock(vh);

  /* 
   * FOURTH:
   *   Load the 'rooms' array.
   */

  index += 1;
  vh = DmQueryRecord(RogueDB, index); /* read-only, promise */
  p = MemHandleLock(vh);
  if (p) {
    load_room = (room *) p;
    for (i = 0 ; i < MAXROOMS ; i++) {
      MemMove(&(sotu->rooms[i]), load_room, sizeof(room)); /* works? */
      load_room++;
    }
  }
  MemHandleUnlock(vh);

  /* 
   * FIFTH:
   *   Load the 'traps' array.
   */

  index += 1;
  vh = DmQueryRecord(RogueDB, index); /* read-only, promise */
  p = MemHandleLock(vh);
  if (p) {
    load_trap = (trap *) p;
    for (i = 0 ; i < MAX_TRAPS ; i++) {
      MemMove(&(sotu->traps[i]), load_trap, sizeof(trap)); /* works! */
      load_trap++;
    }
  }
  MemHandleUnlock(vh);

  /* 
   * SIXTH:
   *   Load the 'id_' arrays.
   */
  load_character_id(sotu->id_scrolls, SCROLLS, ++index, sotu);
  load_character_id(sotu->id_potions, POTIONS, ++index, sotu);
  load_character_id(sotu->id_wands,   WANDS,   ++index, sotu);
  load_character_id(sotu->id_rings,   RINGS,   ++index, sotu);
  //  load_character_id(sotu->id_weapons, WEAPONS, ++index);
  //  load_character_id(sotu->id_armors,  ARMORS,  ++index);

  /* 
   * SIXTH and a half
   *   Load the 'is_wood' array... along with object-count info ...
   */
  index += 1;
  vh = DmQueryRecord(RogueDB, index); /* read-only, promise */
  p = MemHandleLock(vh);
  if (p) {
    uint_p = (UInt *) p;
    for (i = 0 ; i < 4 ; i++) {
      object_count[i] = uint_p[i];
    }
    uint_p +=4 ;
    boolean_p = (Boolean *) uint_p;
    for (i = 0 ; i < WANDS ; i++) {
      sotu->is_wood[i] = boolean_p[i];
    }
  }
  MemHandleUnlock(vh);

  /* 
   * SEVENTH:
   *   Load the 'rogue->pack', 'level_monsters', and 'level_objects'
   *   Yowza!  If this works I will be SHOCKED.
   */

  index += 1; /* it was up to 12 */
  load_character_objs(&(sotu->roguep->pack), &index, object_count[0]);
  fix_verbed_objs(&(sotu->roguep->pack), sotu->roguep);
  load_character_objs(sotu->level_monsters,  &index, object_count[1]);
  load_character_objs(sotu->level_objects,   &index, object_count[2]);

  //  delete_saved_rogue(); // This is moved up to before-saving-over-it

  /* let's see if this will fly..  not bad. */
  // the following has moved to iRogue.c:
  /*
  move_visible_window(sotu->roguep->col, sotu->roguep->row, true);
  if (sotu->cur_room != PASSAGE)
    light_up_room(sotu->cur_room, sotu);
  else {
    light_passage(sotu->roguep->row, sotu->roguep->col, sotu);
    mvaddch(sotu->roguep->row, sotu->roguep->col, sotu->roguep->fchar);
  }
  print_stats(STAT_ALL, sotu);
  */
}


/****************************************************************************/

Char ** snapshot_select_items;
void snapshot_update_list(ListPtr lst, Boolean include_new)
{
  // set the list stuff.
  // note that this might be called multiple times by snapshot form
  // (only once by revivify form) but only to *delete* an item.
  Word s_items; 
  Short i, s, len, ignore=0;
  VoidHand vh;
  UInt index;
  MemPtr p;
  struct state_of_the_union *load_sotu;
  DateTimeType datetime;
  Char datestr[dateStringLength], timestr[timeStringLength], name9[10];

  s = get_file_count();
  s_items = (include_new) ? s + 1 : s;

  /* Hey!  Who's freeing this puppy? */
  snapshot_select_items = (Char **) md_malloc(sizeof(Char *) * s_items);
  len = dateStringLength + 1 + timeStringLength + 1 + 10;
  if (len < 9) len = 9; // make sure to fit "autosave\0"
  for (i = 0 ; i < s_items ; i++)
    snapshot_select_items[i] = md_malloc(sizeof(Char) * len);
  StrCat(snapshot_select_items[0], "Autosave");
  if (include_new)
    StrCat(snapshot_select_items[s], "New Slot");

  for (i = 1 ; i < s ; i++) {
    index = (UInt) get_file_start(i, &ignore);
    vh = DmQueryRecord(RogueDB, index);
    p = MemHandleLock(vh);
    if (!p) continue; // ideally this should not happen
    load_sotu = (struct state_of_the_union *) p;
    TimSecondsToDateTime(load_sotu->timestamp, &datetime); // see DateTime.h
    StrNCopy(name9, load_sotu->username, 9);
    name9[9] = '\0';
    MemHandleUnlock(vh);    
    DateToAscii(datetime.month, datetime.day, datetime.year,
		PrefGetPreference(prefDateFormat), datestr);
    TimeToAscii(datetime.hour, datetime.minute,
		PrefGetPreference(prefTimeFormat), timestr);
    if (false) // if (my_prefs.multiuser)
      StrPrintF(snapshot_select_items[i], "%s %s %s", datestr, timestr, name9);
    else
      StrPrintF(snapshot_select_items[i], "%s %s", datestr, timestr);
    //    StrPrintF(snapshot_select_items[i], "%d %d %s",
    //	      (Short) index, ignore, timestr);
  }

  LstSetListChoices(lst, snapshot_select_items, s_items);  
  LstSetHeight(lst, (s_items > 10) ? 10 : s_items);
  return;
}
// Free the ith string in snapshot_select_items
// Move the i+1th, etc, each back a slot
// (Use LstGetNumberOfItems to remember how many are in the list.)
//  LstSetListChoices(lst, snapshot_select_items, numitems-1);
void snapshot_delete_listi(FormPtr frm, Short i)
{
  Word numitems;
  Short j;
  VoidHand h;
  CharPtr label;
  ListPtr lst;
  ControlPtr ctl;

  h = MemPtrRecoverHandle(snapshot_select_items[i]);
  if (h) MemHandleFree(h);
  lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_snap));
  numitems = LstGetNumberOfItems(lst);
  for (j = i+1 ; j < numitems ; j++)
    snapshot_select_items[j-1] = snapshot_select_items[j];
  numitems--;

  LstSetListChoices(lst, snapshot_select_items, numitems);  
  LstSetHeight(lst, (numitems > 10) ? 10 : numitems);
  // select 0
  LstSetSelection (lst, 0); // do I also need to fix ctl?
  label = LstGetSelectionText(lst, 0);
  ctl = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, popup_snap));
  CtlSetLabel(ctl, label);
  display_saved_rogue(0);
}

void reviv_frob_btns(FormPtr frm, Short filenum)
{
  ControlPtr btn;
  UChar stat;
  stat = get_saved_rogue_status(filenum);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_reviv_undead));
  if (stat & (STATUS_ISDEAD | STATUS_UNDEAD)) // can come back undead
    CtlShowControl(btn);
  else CtlHideControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_reviv_alive));
  if (stat & STATUS_UNDEAD) // can't come back live
    CtlHideControl(btn);
  else CtlShowControl(btn);
}

void snapshot_frob_btns(FormPtr frm, Boolean del_ok)
{
  ControlPtr btn;
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_snap_delete));
  if (del_ok) // ok to delete
    CtlShowControl(btn);
  else CtlHideControl(btn);
}

void topten_frob_btns(FormPtr frm)
{
  ControlPtr btn;
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_dead_reviv));
  CtlHideControl(btn);
}



// Call AFTER loading autosave image
// This should work for reviving dead OR alive save-images,
// as long as you don't screw up revivify_p.
void do_revivify(struct state_of_the_union * sotu)
{
  fighter *rogue;
  if (revivify_p == STATUS_NORMAL)
    return; // in case caller screws up
  // First we need some orange smoke!
  if (rand_percent(50)) message("The orange smoke slowly clears.", sotu);
  else            message("The cloud of orange smoke dissipates.", sotu);
  // Figure out whether we are "alive" and penalized, or "undead".
  // ("Undead" will not have been a choice if the save-image is alive,
  // and alive will not have been a choice if the save-image is undead.)
  rogue = sotu->roguep;
  if (revivify_p & STATUS_UNDEAD) {
    // Hello, you are now a fairly incompetent vampire.
    sotu->conduct |= CONDUCT_UNDEAD;//    sotu->score_status |= STATUS_UNDEAD;
    rogue->moves_left = (HUNGRY + WEAK) / 2;
    if (sotu->conduct & CONDUCT_ISDEAD)//if(sotu->score_status & STATUS_ISDEAD)
      message("You are consumed by a dark hunger", sotu);
    else {
      message("You feel.. strangely hungry", sotu);
      // actually I should do something like hp_current - old_hp_current
      rogue->moves_left += (WEAK/2) * (rogue->hp_current/rogue->hp_max);
    }
  } else { // survey results were 5:3:2:2, gold/xp/str/maxhp.
    if (rand_percent(42)) {
      message("Someone has rifled your pockets!", sotu);
      rogue->gold %= 100;
      // leave 'em some pocket change
      if (rand_percent(10))
	rogue->gold /= 2;
    } else if (rogue->exp > 1 && rand_percent(43)) {
      message("You don't recall how you got here.", sotu);
      rogue->exp_points = sotu->level_points[rogue->exp-2] + 1;
      // just short of dropping a level
      // maybe have 10% chance of dropping a level instead
    } else if (rand_percent(50)) {
      message("You feel shaky, weak in the knees.", sotu);
      // decrease str
      rogue->str_current = max(3, rogue->str_current-1);
      if (rand_percent(10))
	rogue->str_max = rogue->str_current; // this might be real cruel
    } else {
      message("You feel distressingly light-headed.", sotu);
      // decrease max hp
      if (rand_percent(10)) rogue->hp_max--;
      rogue->hp_max = max(5, rogue->hp_max-1);
      rogue->hp_current = min(rogue->hp_max, rogue->hp_current);
    }
  }
  //  if (sotu->score_status & STATUS_ISDEAD) {
  //    sotu->score_status &= ~STATUS_ISDEAD; // make sure we are alive :)
  if (sotu->conduct & CONDUCT_ISDEAD) {
    sotu->conduct &= ~CONDUCT_ISDEAD; // make sure we are alive :)
    rogue->hp_current = rogue->hp_max; // heal yourself
  }
  revivify_p = STATUS_NORMAL;
}
