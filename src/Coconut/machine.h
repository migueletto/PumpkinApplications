#ifndef _MACHINE_H
#define _MACHINE_H

#include "cpu.h"
#include "armlet.h"
#include "video.h"
#include "kbd.h"

typedef enum {
  fcoco = 1, fdragon, fmc10, fspec, fcpm, fmc1000, fapple, fvz, fvic20,
  foric, fatom, fcgenie, fjupiter, faquarius, fmsx, fcoleco, fnes
} MachineFamily;

typedef enum {
  coco = 1, cocoe, coco2, coco2b, cp400, dragon32, dragon64, mc10,
  spectrum, cpm, mc1000, apple2p, vz300, vic20, orica, atom, cgenie,
  jupiter, aquarius, expert11, hotbit12, coleco, nes
} MachineID;

#define RAM_4K	1
#define RAM_16K	4
#define RAM_32K	8
#define RAM_64K	16

#define DIR_ROM		1
#define DIR_DISK	2
#define DIR_CASSETTE	3
#define DIR_SNAPSHOT	4
#define DIR_SCREENSHOT	5

typedef struct MachineType {
  MachineFamily family;
  MachineID id;
  UInt16 cpu;
  UInt32 clock;
  UInt8 ramsizes;
  UInt16 memmode;
  UInt16 vsync;
  ColorType *colortable;
  UInt16 *control;
  ButtonDef *buttonDef;
  KeyMap *map;
  UInt16 font;
  UInt16 form;
  UInt16 dac;
  void (*selectFunction)(struct MachineType *machine, Hardware *hardware, UInt8 ramsize);
  void (*initFunction)(void);
  void (*resetFunction)(void);
  void (*finishFunction)(void);
  void (*osdFunction)(UInt32 delay);
  void (*debugFunction)(void);
  void (*keyHandler)(UInt16 c);
  void (*joystickHandler)(UInt16 x, UInt16 y);
  Boolean (*formHandler)(EventPtr event, Boolean *close);
  UInt32 (*callback)(ArmletCallbackArg *arg);
  char *year;
  char *name;
  char *description;
  char *company;
  char *fname;
} MachineType;

typedef struct {
  MachineID id;
  UInt16 start;
  UInt16 size;
  UInt32 crc;
  UInt8 optional;
  UInt8 preload;
  char *name;
} RomType;

void mch_init(void) SECTION("aux");
void mch_finish(void) SECTION("aux");
MachineType *mch_getmachine(Int16 index) SECTION("aux");
char **mch_getfamilies(void);
Int16 mch_getnummachines(void) SECTION("aux");
char **mch_getnames(void) SECTION("aux");
Int16 mch_getnumroms(Int16 index) SECTION("aux");
RomType *mch_getrom(Int16 index, Int16 r) SECTION("aux");
void mch_createdirs(void) SECTION("aux");
void mch_mkdir(Int16 family, Int16 dir) SECTION("aux");

UInt8 getramsize(UInt8 ramsizes, UInt16 index) SECTION("aux");
UInt8 getmaxramsize(UInt8 ramsizes) SECTION("aux");
char **buildramsizes(UInt16 id, UInt8 ramsizes, UInt8 ramsize, UInt16 *num, UInt16 *index) SECTION("aux");

#endif
