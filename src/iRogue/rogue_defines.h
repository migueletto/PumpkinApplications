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
#ifndef __ROGUEDEFS_H__
#define __ROGUEDEFS_H__

/* #define boolean char */

/* Use this instead of - to draw outlines. */
#define EMDASH 151
/* Use this instead of ':' (realllly hard to see.) */
#define FOODCHAR 176
/* other foodlike ones - 132, 133, 149, 166, 168, 170, 183, 184 */
/* 185: ' 168: " 166: | 133: ... 132: ,,  149: bullet */
/* some flamelike chars:     164, 167 */
#define FLAMECHAR 164


#define NOTHING		((UShort)     0)
#define OBJECT		((UShort)    01)
#define MONSTER		((UShort)    02)
#define STAIRS		((UShort)    04)
#define HORWALL		((UShort)   010)
#define VERTWALL	((UShort)   020)
#define DOOR		((UShort)   040)
#define FLOOR		((UShort)  0100)
#define TUNNEL		((UShort)  0200)
#define TRAP		((UShort)  0400)
#define HIDDEN		((UShort) 01000)

#define ARMOR		((UShort)   01)
#define WEAPON		((UShort)   02)
#define SCROLL		((UShort)   04)
#define POTION		((UShort)  010)
#define GOLD		((UShort)  020)
#define FOOD		((UShort)  040)
#define WAND		((UShort) 0100)
#define RING		((UShort) 0200)
#define AMULET		((UShort) 0400)
#define ALL_OBJECTS	((UShort) 0777)

#define	LEATHER	        0
#define	RING_MAIL       1
#define	STUDDED_LEATHER 2
#define	SCALE_MAIL      3
#define	PADDED_ARMOR    4
#define	CHAIN_MAIL      5
#define	SPLINT_MAIL     6
#define	BANDED_MAIL     7
#define	PLATE_MAIL      8
#define	PLATE_ARMOR     9
#define	MITHRIL	        10
#define	CRYSTAL_ARMOR   11
#define	ARMORS          12

#define BOW 0
#define DART 1
#define ARROW 2
#define DAGGER 3
#define SHURIKEN 4
#define MACE 5
#define LONG_SWORD 6
#define TWO_HANDED_SWORD 7
#define ROCK 8
#define SLING 9
#define CROSSBOW 10
#define CROSSBOW_BOLT 11
#define SPEAR 12
#define TRIDENT 13
#define SPETUM 14
#define BARDICHE 15
#define SHORT_PIKE 16
#define BASTARD_SWORD 17
#define HALBERD 18
#define BATTLE_AXE 19
#define HAND_AXE 20
#define CLUB 21
#define FLAIL 22
#define GLAIVE 23
#define GUISARME 24
#define HAMMER 25
#define JAVELIN 26
#define MORNING_STAR 27
#define PARTISAN 28
#define PICK 29
#define LONG_PIKE 30
#define SCIMITAR 31
#define SLING_BULLET 32
#define QUARTERSTAFF 33
#define BROADSWORD 34
#define SHORT_SWORD 35
#define CLAYMORE 36
#define FOOTBOW 37
#define FOOTBOW_BOLT 38
#define KATANA 39
#define LEUKU 40
#define TOMAHAWK 41
#define PERTUSKA 42
#define SABRE 43
#define CUTLASS 44
#define NO_LAUNCHER 99
#define WEAPONS 45

#define O_COLLECTION 01L
#define O_MISSILE    02L
#define O_METAL      04L
/* not used yet: */
#define O_POISON    010L
#define O_RETURNS   020L
#define O_BURNS     040L
#define O_ZAPPED   0100L

// use capital letters too: first a-z then A-X
#define MAX_PACK_COUNT 50
// how much an unaltered rogue can carry
#define BASE_ROGUE_CARRYABLE 1500

#define PROTECT_ARMOR 0
#define HOLD_MONSTER 1
#define ENCH_WEAPON 2
#define ENCH_ARMOR 3
#define IDENTIFY 4
#define TELEPORT 5
#define SLEEP 6
#define SCARE_MONSTER 7
#define REMOVE_CURSE 8
#define CREATE_MONSTER 9
#define AGGRAVATE_MONSTER 10
#define MAGIC_MAPPING 11
#define CREATE_OBJ 12
#define SCROLLS 13

#define INCREASE_STRENGTH 0
#define RESTORE_STRENGTH 1
#define HEALING 2
#define EXTRA_HEALING 3
#define POISON 4
#define RAISE_LEVEL 5
#define BLINDNESS 6
#define HALLUCINATION 7
#define DETECT_MONSTER 8
#define DETECT_OBJECTS 9
#define CONFUSION 10
#define LEVITATION 11
#define HASTE_SELF 12
#define SEE_INVISIBLE 13
#define POTIONS 14

#define TELE_AWAY 0
#define SLOW_MONSTER 1
#define CONFUSE_MONSTER 2
#define INVISIBILITY 3
#define POLYMORPH 4
#define HASTE_MONSTER 5
#define PUT_TO_SLEEP 6
#define MAGIC_MISSILE 7
#define CANCELLATION 8
#define DO_NOTHING 9
#define LIGHTNING 10
#define W_SCARE_MONSTER 11
#define WANDS 12

/* these should be kept in same order as the ring_flag #define's. */
/* and as ring_actions in my_object.c of course, which I forgot last time. */
#define SUSTAIN_STRENGTH 0
#define R_SEE_INVISIBLE  1
#define MAINTAIN_ARMOR   2
#define R_LEVITATE       3
#define R_FREE_ACTION    4
#define R_BREATHING      5
#define R_ALERTNESS      6
#define R_PROTECT_FIRE   7
#define R_PROTECT_COLD   8
#define R_PROTECT_SHOCK  9
#define R_LIFESAVING    10
#define R_ESP           11
#define R_WARNING       12
#define R_MOOD          13
#define R_TELEPORT      14
#define R_AGGRAVATE     15
#define R_DELUSION      16
#define R_BURDEN        18
#define ADORNMENT       17
#define STEALTH         19
#define REGENERATION    20
#define SLOW_DIGEST     21
#define ADD_STRENGTH    22
#define DEXTERITY       23
#define SEARCHING       24
#define R_PROTECTION    25
#define RINGS 26
/* ring of carrying?  if I add 'weight' to everything.
// ring of teleport control?  if I run out of cool things to do.
// make sure that RINGS <= GEMS always.
// some ring flags
// these should be kept in same order as the type_of_ring #define's. */
#define NO_RING_EFFECTS           00L

#define RING_SUSTAIN_STR          01L
#define RING_SEE_INVISIBLE        02L
#define RING_MAINTAIN_ARMOR       04L
#define RING_LEVITATE            010L
#define RING_FREE_ACTION         020L
#define RING_BREATHING           040L
#define RING_ALERTNESS          0100L
#define RING_PROTECT_FIRE       0200L
#define RING_PROTECT_COLD       0400L
#define RING_PROTECT_SHOCK     01000L
#define RING_LIFESAVING        02000L
#define RING_ESP               04000L
#define RING_WARNING          010000L
#define RING_MOOD             020000L
#define RING_TELEPORT         040000L
#define RING_AGGRAVATE       0100000L
#define RING_DELUSION        0200000L
/* 'burden' has no effect except great weight, if I implement weight.
// 'adornment' has no effect
// these are cumulative: stealth, regen, digest, add_str, dex, search, protec'n
*/

#define RATION 0
#define FRUIT 1
#define CARCASE 2
// have his...

#define NOT_USED                  00L
#define BEING_WIELDED             01L
#define BEING_WORN                02L

#define ON_LEFT_HAND_1            04L
#define ON_LEFT_HAND_2           010L
#define ON_LEFT_HAND_3           020L
#define ON_LEFT_HAND_4           040L

#define ON_RIGHT_HAND_1         0100L
#define ON_RIGHT_HAND_2         0200L
#define ON_RIGHT_HAND_3         0400L
#define ON_RIGHT_HAND_4        01000L

#define ON_LEFT_HAND             074L
#define ON_RIGHT_HAND          01700L
#define ON_EITHER_HAND         01774L
#define BEING_USED             01777L


#define NO_TRAP -1
#define TRAP_DOOR 0
#define BEAR_TRAP 1
#define TELE_TRAP 2
#define DART_TRAP 3
#define SLEEPING_GAS_TRAP 4
#define RUST_TRAP 5
#define TRAPS 6

#define STEALTH_FACTOR 3
#define R_TELE_PERCENT 8

#define UNIDENTIFIED ((UShort) 00)	/* MUST BE ZERO! */
#define IDENTIFIED ((UShort) 01)
#define CALLED ((UShort) 02)

#define DROWS 24
#define DCOLS 80
#define MAX_TITLE_LENGTH 30
//#define MORE "-more-"
#define MAXSYLLABLES 40
#define MAX_METAL 22
#define WAND_MATERIALS 47
#define GEMS 37
#define POTION_COLORS 32

#define GOLD_PERCENT 46

struct id {
	Short value;
	Char title[40];
	Char real[40];
	UShort id_status;
};

/* The following #defines provide more meaningful names for some of the
 * struct object fields that are used for monsters.  This, since each monster
 * and object (scrolls, potions, etc) are represented by a struct object.
 * Ideally, this should be handled by some kind of union structure.
 */

#define m_damage damage
#define hp_to_kill quantity
#define m_char ichar
#define kill_exp launcher
#define first_level is_protected
#define last_level is_cursed
#define m_hit_chance class
#define stationary_damage identified
#define breath_type identified
#define carcase_type identified
/* nothing stationary breaths;  nothing edible should breath;
   nothing stationary does stationary-type damage if it's edible. */
#define drop_percent which_kind
#define trail_char d_enchant
#define slowed_toggle quiver
#define moves_confused hit_enchant
#define nap_length picked_up
#define disguise what_is
#define next_monster next_object
#define m_flags1 o_flags
#define m_flags2 in_use_flags
#define trow weight
#define tcol o_unused
/* unused is used for food (to indicate which string in an array of names)
   and might be used for wands (to indicate the zap character to use) */

struct obj {				/* comment is monster meaning */
	ULong o_flags;	        /* monster flags 1 */
	ULong in_use_flags;	/* monster flags 2 */
  /*	char *damage;	*/		/* damage it does */
  	Char damage[4];                 /* Using char as very-short int. */
	Short quantity;			/* hit points to kill */
	Short ichar;			/* 'A' is for aquatar */
	Short launcher;			/* exp for killing it */
	Short is_protected;		/* level starts */
	Short is_cursed;		/* level ends */
	Short class;			/* chance of hitting you */
	Short identified;		/* 'F' damage, 1,2,3... */
	UShort which_kind; /* item carry/drop % */
	Short o_row, o_col, o;	/* o is how many times stuck at o_row, o_col */
	Short row, col;			/* current row, col */
	Short d_enchant;		/* room char when detect_monster */
	Short quiver;			/* monster slowed toggle */
	Short weight, o_unused;	/* target row, col */
	Short hit_enchant;		/* how many moves is confused */
	UShort what_is;	/* imitator's charactor (?!%: */
	Short picked_up;		/* sleep from wand of sleep */
  /*	UShort in_use_flags; */
	struct obj *next_object;	/* next monster */
};
/* I think I'll use o_unused for fruit name... */

typedef struct obj object;

#define INIT_HP 12

struct fight {
	object *armor;
	object *weapon;
        object *rings[8]; /* 0-3 on left hand, 4-7 on right hand */
	Short hp_current;
	Short hp_max;
	Short str_current;
	Short str_max;
	object pack;
	Long gold;
	Short exp;
	Long exp_points;
	Short row, col;
	Short fchar;
	Short moves_left;
};

typedef struct fight fighter;

struct dr {
  Short oth_room;
  Short oth_row,
        oth_col;
  Short door_row,
        door_col;
};

typedef struct dr door;

struct rm {
  Char bottom_row, right_col, left_col, top_row;
  door doors[4];
  UShort is_room;
};

typedef struct rm room;

#define MAXROOMS 9
#define BIG_ROOM 10

#define NO_ROOM -1

#define PASSAGE -3		/* cur_room value */

//#define AMULET_LEVEL 26
#define AMULET_LEVEL 27

#define R_NOTHING	((UShort) 01)
#define R_ROOM		((UShort) 02)
#define R_MAZE		((UShort) 04)
#define R_DEADEND	((UShort) 010)
#define R_CROSS		((UShort) 020)

#define MAX_EXP_LEVEL 21
#define MAX_EXP 10000000L
#define MAX_GOLD 900000
#define MAX_ARMOR 99
#define MAX_HP 800
#define MAX_STRENGTH 99
#define LAST_DUNGEON 99

#define STAT_LEVEL 01
#define STAT_GOLD 02
#define STAT_HP 04
#define STAT_STRENGTH 010
#define STAT_ARMOR 020
#define STAT_EXP 040
#define STAT_HUNGER 0100
#define STAT_LABEL 0200
#define STAT_ALL 0377

//#define PARTY_TIME 10	/* one party somewhere in each 10 level span */
#define PARTY_TIME 3	/* one party somewhere in each 3 level span */

#define MAX_TRAPS 10	/* maximum traps per level */

#define HIDE_PERCENT 12

struct tr {
	Short trap_type;
	Short trap_row, trap_col;
};

typedef struct tr trap;

/* extern fighter rogue; */
//extern fighter *rogue;
/* extern room * rooms; */
extern trap * traps;
extern UShort ** dungeon;
extern object * level_objects;
/* extern object level_objects; */

/* extern struct id id_scrolls[]; */
/* extern struct id id_potions[]; */
/* extern struct id id_wands[]; */
/* extern struct id id_rings[]; */
/* extern struct id id_weapons[]; */
/* extern struct id id_armors[]; */
extern struct id * id_scrolls;
extern struct id * id_potions;
extern struct id * id_wands;
extern struct id * id_rings;
extern struct id * id_weapons;
extern struct id * id_armors;

extern object mon_tab[];
extern object * level_monsters;
/* extern object level_monsters; */

#define MONSTERS 52

/*
// keep together: napping, asleep
// keep together: asleep, imitates, wakens
// keep together: wakens, wanders, seeks gold
// keep together: flits, CONFUSED, can flit
// keep together: flies, flits, "special_hit", invisible, flames,
//                imitates, confuses, seeks gold, holds. (cancellation)
// special hit:
//   rusts holds freezes steals_gold steals_item stings drains_life drops_level
// what doesn't this include?
// hasted, slowed, freezing_rogue, rust_vanished, stationary, already_moved.
// let's add: napping, asleep, confused, can_flit.

// m_flags1
//  #define foo                  0200000L
//  #define foo                  0400000L  bad smell?  stink?
//  #define foo                 01000000L  paralyze? like ice monster..
//  #define foo                 02000000L  suffocate?
//  #define foo                 04000000L  cause rot?  petrify?
//  #define foo                010000000L  blink?  summon?
//  #define foo                020000000L  blind?  fear?
//  #define foo                040000000L  itch?
// change FLAMES to BREATHES, and use some of m_flags2 to determine type.
// stop before you get to 0100000000L !!!
*/
#define INVISIBLE                 01L
#define FLIES                     02L
#define FLITS                     04L
#define RUSTS                    010L
#define HOLDS                    020L
#define FREEZES                  040L
#define STEALS_GOLD             0100L
#define STEALS_ITEM             0200L
#define STINGS                  0400L
#define DRAINS_LIFE            01000L
#define DROPS_LEVEL            02000L
#define SEEKS_GOLD             04000L
#define CONFUSES              010000L
#define IMITATES              020000L
#define FLAMES                040000L
#define SHRIEKS              0100000L
#define HUGS                 0200000L
#define SUCKER               0400000L
#define PARALYZES           01000000L
// Note - "paralyzes" is not currently implemented!
/* m_flags2 */
#define HASTED            0000000001L
#define SLOWED            0000000002L
#define NAPPING           0000000004L  /* can't wake up for a while */
#define ASLEEP            0000000010L
#define WAKENS            0000000020L
#define WANDERS           0000000040L
#define CAN_FLIT          0000000100L  /* can, but usually doesn't */
#define CONFUSED          0000000200L
#define STATIONARY        0000000400L  /* damage will be 1,2,3,... */
#define FREEZING_ROGUE    0000001000L
#define RUST_VANISHED     0000002000L
#define ALREADY_MOVED     0000004000L
#define TASTY             0000010000L
#define FLEEING           0000020000L

/* these are all m_flags1 and should stay that way! */
#define SPECIAL_HIT   (RUSTS|HOLDS|FREEZES|STEALS_GOLD|STEALS_ITEM|STINGS|DRAINS_LIFE|DROPS_LEVEL|HUGS|SUCKER|PARALYZES)

#define BREATH_ATTACKS 8
#define B_FLAME 1
#define B_ACID 2
#define B_LGHTN 3
#define B_CL2 4
#define B_ICE 5
#define B_NERVE 6
#define B_SLEEP 7
#define B_SLOW 8
//#define B_FEAR 9
#define B_RANDOM 0

#define WAKE_PERCENT 45
#define FLIT_PERCENT 33
#define PARTY_WAKE_PERCENT 75

#define HYPOTHERMIA 1
#define STARVATION 2
#define POISON_DART 3
#define QUIT 4
#define WIN 5

#define UP 0
#define UPRIGHT 1
#define RIGHT 2
#define RIGHTDOWN 3
#define DOWN 4
#define DOWNLEFT 5
#define LEFT 6
#define LEFTUP 7
#define DIRS 8

#define ROW1 7
#define ROW2 15

#define COL1 26
#define COL2 52

#define MOVED 0
#define MOVE_FAILED -1
#define STOPPED_ON_SOMETHING -2
#define CANCEL '\033'
#define LIST '*'

#define HUNGRY 300
#define WEAK 150
#define FAINT 20
#define STARVE 0

#define MIN_ROW 1

/* external routine declarations.
 */
/* These are commented out for now -- later I'll fix and uncomment.
char *strcpy();
char *strncpy();
char *strcat();

char *mon_name();
char *get_ench_color();
char *name_of();
char *md_gln();
char *md_getenv();
char *md_malloc();
Boolean is_direction();
Boolean mon_sees();
Boolean mask_pack();
Boolean mask_room();
Boolean is_digit();
Boolean check_hunger();
Boolean reg_move();
Boolean md_df();
Boolean has_been_touched();
object *add_to_pack();
object *alloc_object();
object *get_letter_object();
object *gr_monster();
object *get_thrown_at_monster();
object *get_zapped_monster();
object *check_duplicate();
object *gr_object();
object *object_at();
object *pick_up();
struct id *get_id_table();
UShort gr_what_is();
Long rrandom();
Long lget_number();
Long xxx();
Int byebye(), onintr(), error_save();

*/









/* This is where I am keeping ALLL the small globals. */
#define SAVED_MSGS 10 
#define SAVED_MSG_LEN 70

// still need these STATUS_foo for save.c which records savefile statuses.
#define STATUS_NORMAL 0x0
#define STATUS_WASWIZ 0x01
#define STATUS_UNDEAD 0x02
#define STATUS_ISDEAD 0x04
#define STATUS_WINNER 0x08


#define IS_WIZARD (sotu->conduct & CONDUCT_ISWIZ)
#define CONDUCT_ISWIZ     0x0001
#define CONDUCT_ATE_VEG   0x0002
#define CONDUCT_ATE_MEAT  0x0004
#define CONDUCT_ATE_BLOOD 0x0008
#define CONDUCT_WORE      0x0010
#define CONDUCT_WIELDED   0x0020
//                        0x0040
//                        0x0080
#define CONDUCT_WASWIZ    0x0100
#define CONDUCT_UNDEAD    0x0200
#define CONDUCT_ISDEAD    0x0400
#define CONDUCT_WINNER    0x0800
#define CONDUCT_NO_TOPTEN (CONDUCT_ISWIZ | CONDUCT_WASWIZ | CONDUCT_UNDEAD | CONDUCT_ISDEAD) /* xxx not sure about the ISDEAD.  and the ISWIZ is redundant. */
#define CONDUCT_FILESTATUS (CONDUCT_WASWIZ | CONDUCT_UNDEAD | CONDUCT_ISDEAD | CONDUCT_WINNER)

struct sotu_Before_Conducts {
  ULong birthdate;
  ULong timestamp;
  UChar score_status; /* !=0: you don't go in the topten 
			      0x1: you've been a wizard
			      0x2: you're undead 
			      0x4: you're CURRENTLY dead */
  Boolean wizard; /* true if currently a wizard */
  Char hunger_str[8];
  Char username[32];
/*   object *fight_monster; */
/*   Char hit_message[80]; */
/*   Boolean is_wood[WANDS]; */
/*   Char *wand_materials[WAND_MATERIALS]; */
/*   Char *gems[GEMS]; */
/*   Char *syllables[MAXSYLLABLES]; */
  Short cur_level;
  Short max_level;
  Short cur_room;
/*   Char *new_level_message; */
  Short party_room;
  Short r_de;
/*   Long level_points[MAX_EXP_LEVEL]; */
/*   Char random_rooms[MAX_ROOMS+1]; */
/*   Boolean msg_cleared; */
/*   Object level_monsters; */
  Boolean mon_disappeared;
/*   Char *m_names[MONSTERS]; */
/*   Object mon_tab[MONSTERS]; */
  Short m_moves;
  Boolean jump;
/*   Char *you_can_move_again; */
/*   object level_objects; */
/*   UShort dungeon[DROWS][DCOLS]; */
  Short foods;
  Short party_counter;
/*   object *free_list; */

/*   fighter rogue; */ /* ... will be at end. */
/*   struct id ... */
/*   Char *curse_message; */
  Boolean interrupted;
/*   Char *unknown_command; */
/*   Char *left_or_right; */
/*   Char *no_ring; */
  Short r_rings;
  Short e_rings;
  Short stealthy; /* this is cumulative */
  Short regeneration;
  Short auto_search;
  Short add_strength;
  Short ring_dex;
  Short ring_ac;
/*   room rooms[MAXROOMS]; */
/*   Boolean rooms_visited[MAXROOMS]; */
  Short less_hp;
/*   Char *flame_name; */
  Boolean being_held;
/*   trap traps[MAXTRAPS]; */
  Boolean trap_door;
  ULong ring_flags;
  Short bear_trap;
  Short halluc;
  Short blind;
  Short confused;
  Short levitate;
  Short haste_self;
  Boolean see_invisible;
  Short extra_hp;
  Boolean detect_monster;
/*   Char *strange_feeling; */
  /* POINTERS to other MALLOC'd things */
  fighter *roguep;
  UShort **dungeon;
  room * rooms;
  trap * traps;
  struct id * id_potions;
  struct id * id_scrolls;
  struct id * id_wands;
  struct id * id_rings;
  struct id * id_weapons;
  struct id * id_armors;
  Boolean * is_wood; /* goes with id_wands */
  object * level_objects;
  object * level_monsters;
  object * fight_monster;
  object * free_list;
  Long * level_points;

  Char *new_level_message;
  Char *fruit;
  Char *old_messages[SAVED_MSGS];
  Short last_old_message_shown;
  Short fruit_number;

  Char warning_level;
  Char warning_moves;
};


struct state_of_the_union {
  ULong birthdate;
  ULong timestamp;
  UInt conduct; /*  0x0001: you are a wizard RIGHT NOW
                    0x0100: (256) you've ever been a wizard
                    0x0200: (512) you're undead
                    0x0400: you're CURRENTLY dead
                       If (conduct & 0x0700) YOU DON'T GET IN THE TOPTEN. */
  //  UChar score_status; /* !=0: you don't go in the topten 
  //			      0x1: you've been a wizard
  //			      0x2: you're undead 
  //			      0x4: you're CURRENTLY dead */
  //  Boolean wizard; /* true if currently a wizard */
  Char hunger_str[8];
  Char username[32];
/*   object *fight_monster; */
/*   Char hit_message[80]; */
/*   Boolean is_wood[WANDS]; */
/*   Char *wand_materials[WAND_MATERIALS]; */
/*   Char *gems[GEMS]; */
/*   Char *syllables[MAXSYLLABLES]; */
  Short cur_level;
  Short max_level;
  Short cur_room;
/*   Char *new_level_message; */
  Short party_room;
  Short r_de;
/*   Long level_points[MAX_EXP_LEVEL]; */
/*   Char random_rooms[MAX_ROOMS+1]; */
/*   Boolean msg_cleared; */
/*   Object level_monsters; */
  Boolean mon_disappeared;
/*   Char *m_names[MONSTERS]; */
/*   Object mon_tab[MONSTERS]; */
  Short m_moves;
  Boolean jump;
/*   Char *you_can_move_again; */
/*   object level_objects; */
/*   UShort dungeon[DROWS][DCOLS]; */
  Short foods;
  Short party_counter;
/*   object *free_list; */

/*   fighter rogue; */ /* ... will be at end. */
/*   struct id ... */
/*   Char *curse_message; */
  Boolean interrupted;
/*   Char *unknown_command; */
/*   Char *left_or_right; */
/*   Char *no_ring; */
  Short r_rings;
  Short e_rings;
  Short stealthy; /* this is cumulative */
  Short regeneration;
  Short auto_search;
  Short add_strength;
  Short ring_dex;
  Short ring_ac;
/*   room rooms[MAXROOMS]; */
/*   Boolean rooms_visited[MAXROOMS]; */
  Short less_hp;
/*   Char *flame_name; */
  Boolean being_held;
/*   trap traps[MAXTRAPS]; */
  Boolean trap_door;
  ULong ring_flags;
  Short bear_trap;
  Short halluc;
  Short blind;
  Short confused;
  Short levitate;
  Short haste_self;
  Boolean see_invisible;
  Short extra_hp;
  Boolean detect_monster;
/*   Char *strange_feeling; */
  /* POINTERS to other MALLOC'd things */
  fighter *roguep;
  UShort **dungeon;
  room * rooms;
  trap * traps;
  struct id * id_potions;
  struct id * id_scrolls;
  struct id * id_wands;
  struct id * id_rings;
  struct id * id_weapons;
  struct id * id_armors;
  Boolean * is_wood; /* goes with id_wands */
  object * level_objects;
  object * level_monsters;
  object * fight_monster;
  object * free_list;
  Long * level_points;

  Char *new_level_message;
  Char *fruit;
  Char *old_messages[SAVED_MSGS];
  Short last_old_message_shown;
  Short fruit_number;

  Char warning_level;
  Char warning_moves;
};


/* I'm currently relying on NORTH and NORTHWEST being min/max. */
#define NORTH     1
#define NORTHEAST 2
#define EAST      3
#define SOUTHEAST 4
#define SOUTH     5
#define SOUTHWEST 6
#define WEST      7
#define NORTHWEST 8
#define NO_DIRECTION 0

/* Actions that are assignable to hardware buttons */
// pg-u, pg-d, calc, find
// date, addr, todo, memo
#define HWB_NOOP   0
// n, e, s, w order is important: x*2-1 = direction as above
#define HWB_N      1
#define HWB_E      2
#define HWB_S      3
#define HWB_W      4
#define HWB_UP     5
#define HWB_DOWN   6
#define HWB_SEARCH 7
#define HWB_REST   8
#define HWB_MAP    9
#define HWB_SCROLL 10
#define HWB_THROW  11
#define HWB_ZAP    12
#define HWB_INV    13
/* ...that's enough for now, surely. */

/* Most of these use inventory-select-item form */

/* #define INV_SEL 0 */
#define INV_DROP       1
#define INV_EAT        2
#define INV_READ       3
#define INV_PUTON      4
#define INV_WIELD      5
#define INV_ZAP        6
#define INV_QUAFF      7
#define INV_WEAR       8
#define INV_REMOVE     9
#define INV_THROW     10
#define INV_UNWIELD   11
#define INV_TAKEOFF   12
#define INV_IDENTIFY  13
#define INV_CALL      14
#define INV_MOVE      15
#define INV_FIGHT     16
#define INV_IDTRAP    17
#define SCROLL_WINDOW 20
/* (search, fight) eat read put-on (prev-msg) [up] (list-armor id-trap)
   take-off wield zap [version] (avg-hp) [inv] (fight2death) quaff
   [drop] remove-ring [down] (list-weapons list-rings short-inv)
   wear-armor (call-name) throw (quit save move-onto)
*/


#define PREF_COLOR 0x1
#define PREF_BLACKBG 0x2
/* whats AbtShowAbout ? */
struct RoguePreferenceType_s {
  /* ... */
  Boolean run_on;
  Boolean autosave_on; // actually I think I should replace with "multiuser"
  //Boolean save_on_exit; // changed this to autosave in 0.42 !!!
  Short run_walk_border;
  Short walk_search_border;
  Boolean dave_mode;
  Boolean scroll_mode;
  Boolean sound_on;
  Boolean use_hardware;
  Short hardware[8];
  // RogueAppPrefVersion = 0x01 (?)
  Boolean stay_centered; // new in 0.40
  // RogueAppPrefVersion = 0x02 (?)
  Boolean font_small; // new in 0.42
  Boolean rogue_relative; // new in 0.42
  // RogueAppPrefVersion = 0x03  ----  These are added in iRogue Version 0.45:
  Boolean color_on;
  Boolean crosshairs_on;
  // RogueAppPrefVersion = 0x03  ----  These are added in iRogue Version 0.46:
  Boolean sort_pack;
  Boolean black_bg;
  // RogueAppPrefVersion = 0x04  ----  These are added in iRogue 1.1.1
  Boolean tiles_on;
};
typedef struct RoguePreferenceType_s RoguePreferenceType;


#define Is_Alpha(x) ((('A' <= (x)) && ((x) <= 'Z')) || (('a' <= (x)) && ((x) <= 'z')))
#define Not_Alpha(x) (((x) < 'A') || ((x) > 'z') || (('Z' < (x)) && ((x) < 'a')))
#define Is_Hex(x) ((('0' <= (x)) && ((x) <= '9')) || (('A' <= (x)) && ((x) <= 'F')) || (('a' <= (x)) && ((x) <= 'f')))

// number of records in a save "file"
// #define SAVEFILE_RECS 12
#define MAX_SAVE_FILES 15

extern RoguePreferenceType my_prefs;

extern Boolean have_read_color;
extern Boolean have_read_tiles;
#define IsColor (have_read_color && my_prefs.color_on)
#define IsTiles (have_read_tiles && my_prefs.tiles_on)
extern Boolean IsVGA; // are we on a handera 330.
#ifndef I_AM_OS_2
extern FontID SmallFont;
extern FontID BigFont;
#endif

#ifdef I_AM_COLOR
extern IndexedColorType symbol_colors[];

#define START_TILE 32 /* ' ' */
#define END_TILE 126  /* '~' */

#endif // I_AM_COLOR



#endif
