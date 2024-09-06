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
extern DmOpenRef       RogueDB;

#ifndef I_AM_COLOR
#include <SystemMgr.rh> // for user name
#else /* I_AM_COLOR */
#include <SystemMgr.h> // needed? or not?
#endif /* I_AM_COLOR */
#include <DLServer.h> // for user name
#include <SoundMgr.h> // for feeping
#include <Preferences.h> // for feeping
#include "rogue.h"



extern Boolean i_am_dead;

VoidHand topten_handle = NULL;


// id_trap moved to trap.c where it belongs, eh


static void format_toptens(CharPtr topten_buf) SEC_2;
static void toptenify(Long score, Short level,
		      Char * name, Char * killed_by) SEC_2;
static void id_all(struct state_of_the_union *sotu) SEC_2;
static Short get_value(object *obj, struct state_of_the_union *sotu) SEC_1;

/**********************************************************************
                       KILLED_BY
 IN:
 monster = the monster that killed the rogue...
 other = ...or the non-monster thing that killed the rogue.
 sotu = various globals
 OUT:
 nothing
 PURPOSE:
 Calculate the rogue's final score and, using monter and other, write
 its epitaph.  If the score is high enough, record it in the "top ten".
 **********************************************************************/
void killed_by(object *monster, short other,
	       struct state_of_the_union *sotu)
{
  Char buf[80];
  Char mon_name[40];
  Long score, tmp;
  Char ch;

  //  if (i_am_dead) return;
  i_am_dead = true;
  if (other != QUIT) {
    //    sotu->roguep->gold = sotu->roguep->gold * .9;
    tmp = (sotu->roguep->gold * 9) / 10;
    if (tmp > 0) sotu->roguep->gold = tmp;
  }

  if (other) {
    switch(other) {
    case HYPOTHERMIA:
      (void) StrCopy(buf, "died of hypothermia");
      break;
    case STARVATION:
      (void) StrCopy(buf, "died of starvation");
      break;
    case POISON_DART:
      (void) StrCopy(buf, "killed by a dart");
      break;
    case QUIT:
      (void) StrCopy(buf, "quit");
      break;
    }
  } else {
    /*     (void) StrCopy(buf, "Killed by "); */
    (void) StrCopy(buf, "killed by ");

    if (monster->m_char > 'Z')
      ch = 26 + monster->m_char - 'a'; // yo ho!
    else
      ch = monster->m_char - 'A';
    copy_mon_name(ch, mon_name);
    if (is_vowel(mon_name[0]))
      (void) StrCat(buf, "an ");
    else
      (void) StrCat(buf, "a ");
    (void) StrCat(buf, mon_name);
  }

  /* add it to the "top ten" list if it's good enough */
  /* I guess I will be nice  XXXXXX */
  score = max(1, sotu->roguep->gold);
  /* XXXX I'm not sure how to calculate SCORE ... is it gold? */
  if (!(sotu->conduct & CONDUCT_NO_TOPTEN)) //  if (sotu->score_status == STATUS_NORMAL)
    toptenify(score, sotu->cur_level, sotu->username, buf);

  (void) StrCat(buf, " with ");
  StrPrintF(buf+StrLen(buf), "%ld gold", sotu->roguep->gold);
  /* here I would show the ascii "skull" */
  check_message(); // forced clear
  sotu->last_old_message_shown = SAVED_MSGS-1; // no "--more--"
  message(buf, sotu);
}

/**********************************************************************
                       INIT_TOPTEN_VIEW
 IN:
 frm = the top ten form
 OUT:
 nothing
 PURPOSE:
 Initialize the top ten form which will display stats of the ten
 highest scoring rogues.
 **********************************************************************/
void init_topten_view(FormPtr frm) {
  FieldPtr fld;
  CharPtr txtP;
  /*  VoidHand topten_handle = NULL; */

  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_topten));
  if (topten_handle != NULL) {
    FldSetTextHandle(fld, (MemHandle) 0);
    /* the following line is a fine way to crash the game,
       whenever someone kills two rogues in a single session */
    // MemHandleFree(topten_handle);
  }
  //   topten_handle = MemHandleNew(700); /* is this big enough? */ // Nope!
  topten_handle = MemHandleNew(1100); // 18 + 10*99 should be ok.
  txtP = MemHandleLock(topten_handle);
  format_toptens(txtP);
  MemHandleUnlock(topten_handle);
  FldSetTextHandle(fld, (MemHandle) topten_handle);
  /* uh, update scrollers?? */
}

/**********************************************************************
                       FORMAT_TOPTENS
 IN:
 topten_buf = space to write the top-ten stats to
 OUT:
 nothing
 PURPOSE:
 Format the "top ten" data from the database, for display, when the
 top ten form is initialized.
 **********************************************************************/
static void format_toptens(CharPtr topten_buf) {
  Char buf[100], tmp[20];
  Short i, j, k;
  VoidHand vh;
  VoidPtr p;
  Long * scorep;
  Short * level_and_len;
  Char * name_and_killedby;

  StrPrintF(topten_buf, "rank  score  name\n");
  for (i = 0 ; i <= 9 ; i++) {
    vh = DmQueryRecord(RogueDB, i); /* read-only, promise */
    p = MemHandleLock(vh);
    if (p) {
      scorep = (Long *) p;
      if (*scorep == 0) {
	/* gee.  we don't even have ten scorers. */
	MemHandleUnlock(vh);	
/* 	StrCat(topten_buf, "(that's all, folks!)\n"); */
	break;
      }
      /* Ok, it works up to here; *scorep is the right thing. */

      level_and_len = (Short *) (scorep + 1);
      /*      j = 0; */
      if (i == 9) {
	StrPrintF(buf, "%d. ", i+1);
      } else {
	StrPrintF(buf, "%d.  ", i+1);
      }
      /*      j = 3; */
      StrPrintF(tmp, "%ld", *scorep); /* 10 numbers == 25 spaces. */
      k = StrLen(tmp);
      if (k > 7) j = 0;
      else       j = 7 - k;
      //      j *= 2.5;
      j = j*2 + j/2;
      for (k = 0 ; k < j ; k++) {
	buf[k+3] = ' ';
      }
      /* numbers + spaces = 10 */
      /* spaces : numbers - 10, j++ */
      j = k + 3;
      StrPrintF(buf + j, "%s   ", tmp);
      j = StrLen(buf);

      name_and_killedby = (Char *) p;
      name_and_killedby += sizeof(Long) + (2 * sizeof(Short)); /*scor,lev,len*/
      StrNCopy(buf + j, name_and_killedby, 32); /* name */
      /* XXX If TopTen throws a fatal error, maybe it's misalignment here */
      name_and_killedby += level_and_len[1]; /* actual length of name */
      if (name_and_killedby[0] == 'q' || name_and_killedby[0] == 'w')
	StrNCat(buf, ": ", 80);
      else 
	StrNCat(buf, "\n  ...", 80);
      StrNCat(buf, name_and_killedby, 80);   /* killed by */
      if (level_and_len[0]) {
	StrPrintF(tmp, " on L%d.", level_and_len[0]); /* level */
	StrNCat(buf, tmp, 80);
      }
      StrCat(buf, "\n"); /* don't truncate-away the newline... */
      StrCat(topten_buf, buf);
    }
    MemHandleUnlock(vh);
  }
  /* done! */
  /*  StrPrintF(tmp, "%d", StrLen(topten_buf));
      StrCat(topten_buf, tmp); */
  return;
}


/**********************************************************************
                       CREATE_TOP_TEN_RECORD
 IN:
 nothing
 OUT:
 nothing
 PURPOSE:
 Called by OpenDatabase - if there isn't a database, it has to create
 one and also create this (null) top-ten record to put in it...
 **********************************************************************/
void create_top_ten_record()
{
  //Err error;
  VoidHand fooRec;
  MemPtr p;
  UInt16 i;

  for (i = 0 ; i < 10 ; i++) {
    fooRec = DmNewRecord(RogueDB, &i, 2 * sizeof(Long));
    p = MemHandleLock(fooRec);
    /* zero it */
    /*error =*/ DmSet(p, 0, sizeof(Long) * 2, 0); /* offset, val */
    MemPtrUnlock(p);
    DmReleaseRecord(RogueDB, i, true); /* rec num */
  }
}

/**********************************************************************
                       TOPTENIFY
 IN:
 score = final score of the rogue (calculated from gold)
 level = dungeon level that the rogue was killed on
 name = name supplied by player (in preferences form)
 killed_by = what killed the rogue (monster, hypothermia, etc)
 OUT:
 nothing
 PURPOSE:
 Figure out where, if at all, this dead rogue falls in the top-ten
 list, and record it in the database appropriately.
 This is NOT called for characters that flirted with wizard-mode.
 **********************************************************************/
static void toptenify(Long score, Short level,
		      Char * name, Char * killed_by)
{
  /* Look through the ten records, from 9 to 0, and note
     the index which last fails to beat this score.
     (If the tenth beats this score, just return() right now.)
     REDACTED{Also, see if any record has a name string-equal to 'name'.
              If it does, remove it; otherwise}
     remove the 10th record
     Create a new record, with the noted index.
  */
  VoidHand vh;
  VoidPtr p;
  UInt16 i_beat_you, my_old_self, qty;
  Int16 i;
  Long beatp_score;
  /*  Char * some_winner_name; */

  //Err error;
  MemPtr ptr;


  i_beat_you = 10;
  my_old_self = 9; /* the record to delete */
  for (i = 9 ; i >= 0 ; i--) {
    vh = DmQueryRecord(RogueDB, i); /* read-only, promise */
    p = MemHandleLock(vh);
    if (p) {
      /* read the first part, as a Long */
      beatp_score = *((Long *) p);
      if (beatp_score < score)
	i_beat_you = i;
      /* do not replace your own save records
      some_winner_name = (Char *) p;
      some_winner_name += sizeof(Long) + (2 * sizeof(Short)); //score,lev,len
      if (0 == StrCaselessCompare(some_winner_name, name))
	my_old_self = i;
      */
    }
    MemHandleUnlock(vh);
  }
  if (i_beat_you >= 10)
    return;  /* sorry LOOOOSER */
  /*  do not replace your own save records
  if (i_beat_you > my_old_self)
    return; // you didn't beat yourself!
  */
  
  DmRemoveRecord(RogueDB, my_old_self);
  
  /* Now, write a new record! */
  qty = sizeof(Long) + sizeof(Short) + sizeof(Short);
  i = StrLen(name) + 1;
  if (i % 2 != 0) i++;
  qty += i + StrLen(killed_by) + 1;
  vh = DmNewRecord(RogueDB, &i_beat_you, qty);
  ptr = MemHandleLock(vh);
  /* zero it!  rah.  */
  /*error = */ DmSet(ptr, 0, qty, 0); /* offset, quantity, value */

  qty = 0;
  /*error = */ DmWrite(ptr, qty, &score, sizeof(Long)); /* ptr offset src size */
  qty += sizeof(Long);
  /*error = */ DmWrite(ptr, qty, &level, sizeof(Short));
  qty += sizeof(Short);
  /*error = */ DmWrite(ptr, qty, &i, sizeof(Short));
  qty += sizeof(Short);
  /*error = */ DmWrite(ptr, qty, name, i);
  qty += i;
  /*error = */ DmWrite(ptr, qty, killed_by, StrLen(killed_by) + 1);

  MemPtrUnlock(ptr);
  DmReleaseRecord(RogueDB, i_beat_you, true);
  
}


/**********************************************************************/
/*       The following static functions are called if you WIN.        */
/**********************************************************************/

/**********************************************************************
                       WIN
 IN:
 sotu = various globals (rogue)
 OUT:
 nothing
 PURPOSE:
 Called when the rogue reaches level 0 with the amulet of yendor.
 The rogue's possessions are valued and added to the gold score
 and the rogue, if put in the top ten list, is commemorated as "won".
 Whee.
 **********************************************************************/
void win(struct state_of_the_union *sotu) {
  Short i;
  unwield(sotu->roguep->weapon, sotu->roguep); /* disarm and relax */
  unwear(sotu->roguep->armor, sotu->roguep);
  for (i = 0 ; i < 8 ; i++) {
    un_put_on(sotu->roguep->rings[i], sotu);
  }
  id_all(sotu);
  sell_pack(sotu);
/* 	put_scores((object *) 0, WIN); */
  if (!(sotu->conduct & CONDUCT_NO_TOPTEN))// if (sotu->score_status == STATUS_NORMAL) 
    toptenify(sotu->roguep->gold, 0, sotu->username, "won");

  if (sotu->conduct & CONDUCT_UNDEAD)//  if (sotu->score_status & STATUS_UNDEAD)
    message("You crumble to dust in the sunlight!", sotu); // dam' vampires..
  else
    message("You win!!", sotu);
  sotu->conduct |= CONDUCT_WINNER; //  sotu->score_status = STATUS_WINNER;
}

/**********************************************************************
                       ID_ALL
 IN:
 sotu = various globals (id_foo)
 OUT:
 nothing
 PURPOSE:
 Set status of all identifiable possessions to "identified"
 **********************************************************************/
static void id_all(struct state_of_the_union *sotu)
{
  Short i;

  for (i = 0; i < SCROLLS; i++) {
    sotu->id_scrolls[i].id_status = IDENTIFIED;
  }
  /*
  for (i = 0; i < WEAPONS; i++) {
    sotu->id_weapons[i].id_status = IDENTIFIED;
  }
  for (i = 0; i < ARMORS; i++) {
    sotu->id_armors[i].id_status = IDENTIFIED;
  }
  */
  for (i = 0; i < WANDS; i++) {
    sotu->id_wands[i].id_status = IDENTIFIED;
  }
  for (i = 0; i < POTIONS; i++) {
    sotu->id_potions[i].id_status = IDENTIFIED;
  }
}



/*
static UInt count_objects(object * start) {
  object * obj;
  UInt cnt = 0;
  obj = start->next_object;
  while (obj && cnt <= 1000) {
    cnt++;
    obj = obj->next_object;
  }
  if (cnt >= 1000) cnt = 0; // not that we won't have crashed by now 
  return cnt;
}
*/




/**********************************************************************
                       GET_VALUE
 IN:
 obj = the object to evaluate
 sotu = various globals (id_foo)
 OUT:
 value of the given object
 PURPOSE:
 Calculate the value of the given object; used to supplement the
 gold accumulated by a rogue who has won the game (determins score.)
 **********************************************************************/
static Short get_value(object *obj, struct state_of_the_union *sotu) {
  Short wc;
  Int val = 0;

  wc = obj->which_kind;

  switch(obj->what_is) {
  case WEAPON:
    val = sotu->id_weapons[wc].value;
    if (obj->o_flags & O_COLLECTION) {
      val *= obj->quantity;
    }
    val += (obj->d_enchant * 85);
    val += (obj->hit_enchant * 85);
    break;
  case ARMOR:
    val = sotu->id_armors[wc].value;
    val += (obj->d_enchant * 75);
    if (obj->is_protected) {
      val += 200;
    }
    break;
  case WAND:
    val = sotu->id_wands[wc].value * (obj->class + 1);
    break;
  case SCROLL:
    val = sotu->id_scrolls[wc].value * obj->quantity;
    break;
  case POTION:
    val = sotu->id_potions[wc].value * obj->quantity;
    break;
  case AMULET:
    val = 5000;
    break;
  case RING:
    val = sotu->id_rings[wc].value * (obj->class + 1);
    break;
  }
  if (val <= 0) {
    val = 10;
  }
  return(val);
}

/**********************************************************************
                       SELL_PACK
 IN:
 sotu = various globals (rogue)
 OUT:
 nothing
 PURPOSE:
 Called when the rogue wins the game.  Appraises and sells all the
 rogue's possessions, adding to 'gold' before score is calculated.
 (The appraisals are NOT currently displayed to the player.)
 **********************************************************************/
void sell_pack(struct state_of_the_union *sotu) {
  object *obj;
  Short val;

  obj = sotu->roguep->pack.next_object;

  while (obj) {
    if (obj->what_is != FOOD) {
      obj->identified = 1;
      val = get_value(obj, sotu);
      sotu->roguep->gold += val;
    }
    obj = obj->next_object;
  }
  if (sotu->roguep->gold > MAX_GOLD) {
    sotu->roguep->gold = MAX_GOLD;
  }
}


