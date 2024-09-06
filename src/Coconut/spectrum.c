#include <PalmOS.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "machine.h"
#include "kbd.h"
#include "spectrum.h"
#include "misc.h"
#include "endian.h"
#include "gui.h"
#include "debug.h"

static UInt8 id;
static SpecPrefs prefs;
static Hardware *hardware;
static UInt8 *video = NULL;
static UInt16 key;

ColorType SpectrumColor[17] = {
  {0, 0x00, 0x00, 0x00},
  {1, 0x00, 0x00, 0xbf},
  {2, 0xbf, 0x00, 0x00},
  {3, 0xbf, 0x00, 0xbf},
  {4, 0x00, 0xbf, 0x00},
  {5, 0x00, 0xbf, 0xbf},
  {6, 0xbf, 0xbf, 0x00},
  {7, 0xbf, 0xbf, 0xbf},

  {8, 0x00, 0x00, 0x00},
  {9, 0x00, 0x00, 0xff},
  {10, 0xff, 0x00, 0x00},
  {11, 0xff, 0x00, 0xff},
  {12, 0x00, 0xff, 0x00},
  {13, 0x00, 0xff, 0xff},
  {14, 0xff, 0xff, 0x00},
  {15, 0xff, 0xff, 0xff},

  {-1, 0, 0, 0}
};

static UInt16 SpectrumVideoAddr[192] = {
  0x4000, 0x4100, 0x4200, 0x4300, 0x4400, 0x4500, 0x4600, 0x4700, 
  0x4020, 0x4120, 0x4220, 0x4320, 0x4420, 0x4520, 0x4620, 0x4720, 
  0x4040, 0x4140, 0x4240, 0x4340, 0x4440, 0x4540, 0x4640, 0x4740, 
  0x4060, 0x4160, 0x4260, 0x4360, 0x4460, 0x4560, 0x4660, 0x4760, 
  0x4080, 0x4180, 0x4280, 0x4380, 0x4480, 0x4580, 0x4680, 0x4780, 
  0x40A0, 0x41A0, 0x42A0, 0x43A0, 0x44A0, 0x45A0, 0x46A0, 0x47A0, 
  0x40C0, 0x41C0, 0x42C0, 0x43C0, 0x44C0, 0x45C0, 0x46C0, 0x47C0, 
  0x40E0, 0x41E0, 0x42E0, 0x43E0, 0x44E0, 0x45E0, 0x46E0, 0x47E0, 
  0x4800, 0x4900, 0x4A00, 0x4B00, 0x4C00, 0x4D00, 0x4E00, 0x4F00, 
  0x4820, 0x4920, 0x4A20, 0x4B20, 0x4C20, 0x4D20, 0x4E20, 0x4F20, 
  0x4840, 0x4940, 0x4A40, 0x4B40, 0x4C40, 0x4D40, 0x4E40, 0x4F40, 
  0x4860, 0x4960, 0x4A60, 0x4B60, 0x4C60, 0x4D60, 0x4E60, 0x4F60, 
  0x4880, 0x4980, 0x4A80, 0x4B80, 0x4C80, 0x4D80, 0x4E80, 0x4F80, 
  0x48A0, 0x49A0, 0x4AA0, 0x4BA0, 0x4CA0, 0x4DA0, 0x4EA0, 0x4FA0, 
  0x48C0, 0x49C0, 0x4AC0, 0x4BC0, 0x4CC0, 0x4DC0, 0x4EC0, 0x4FC0, 
  0x48E0, 0x49E0, 0x4AE0, 0x4BE0, 0x4CE0, 0x4DE0, 0x4EE0, 0x4FE0, 
  0x5000, 0x5100, 0x5200, 0x5300, 0x5400, 0x5500, 0x5600, 0x5700, 
  0x5020, 0x5120, 0x5220, 0x5320, 0x5420, 0x5520, 0x5620, 0x5720, 
  0x5040, 0x5140, 0x5240, 0x5340, 0x5440, 0x5540, 0x5640, 0x5740, 
  0x5060, 0x5160, 0x5260, 0x5360, 0x5460, 0x5560, 0x5660, 0x5760, 
  0x5080, 0x5180, 0x5280, 0x5380, 0x5480, 0x5580, 0x5680, 0x5780, 
  0x50A0, 0x51A0, 0x52A0, 0x53A0, 0x54A0, 0x55A0, 0x56A0, 0x57A0, 
  0x50C0, 0x51C0, 0x52C0, 0x53C0, 0x54C0, 0x55C0, 0x56C0, 0x57C0, 
  0x50E0, 0x51E0, 0x52E0, 0x53E0, 0x54E0, 0x55E0, 0x56E0, 0x57E0
};

static UInt16 SpectrumVideoCAddr[192] = {
  0x5800, 0x5800, 0x5800, 0x5800, 0x5800, 0x5800, 0x5800, 0x5800, 
  0x5820, 0x5820, 0x5820, 0x5820, 0x5820, 0x5820, 0x5820, 0x5820, 
  0x5840, 0x5840, 0x5840, 0x5840, 0x5840, 0x5840, 0x5840, 0x5840, 
  0x5860, 0x5860, 0x5860, 0x5860, 0x5860, 0x5860, 0x5860, 0x5860, 
  0x5880, 0x5880, 0x5880, 0x5880, 0x5880, 0x5880, 0x5880, 0x5880, 
  0x58A0, 0x58A0, 0x58A0, 0x58A0, 0x58A0, 0x58A0, 0x58A0, 0x58A0, 
  0x58C0, 0x58C0, 0x58C0, 0x58C0, 0x58C0, 0x58C0, 0x58C0, 0x58C0, 
  0x58E0, 0x58E0, 0x58E0, 0x58E0, 0x58E0, 0x58E0, 0x58E0, 0x58E0, 
  0x5900, 0x5900, 0x5900, 0x5900, 0x5900, 0x5900, 0x5900, 0x5900, 
  0x5920, 0x5920, 0x5920, 0x5920, 0x5920, 0x5920, 0x5920, 0x5920, 
  0x5940, 0x5940, 0x5940, 0x5940, 0x5940, 0x5940, 0x5940, 0x5940, 
  0x5960, 0x5960, 0x5960, 0x5960, 0x5960, 0x5960, 0x5960, 0x5960, 
  0x5980, 0x5980, 0x5980, 0x5980, 0x5980, 0x5980, 0x5980, 0x5980, 
  0x59A0, 0x59A0, 0x59A0, 0x59A0, 0x59A0, 0x59A0, 0x59A0, 0x59A0, 
  0x59C0, 0x59C0, 0x59C0, 0x59C0, 0x59C0, 0x59C0, 0x59C0, 0x59C0, 
  0x59E0, 0x59E0, 0x59E0, 0x59E0, 0x59E0, 0x59E0, 0x59E0, 0x59E0, 
  0x5A00, 0x5A00, 0x5A00, 0x5A00, 0x5A00, 0x5A00, 0x5A00, 0x5A00, 
  0x5A20, 0x5A20, 0x5A20, 0x5A20, 0x5A20, 0x5A20, 0x5A20, 0x5A20, 
  0x5A40, 0x5A40, 0x5A40, 0x5A40, 0x5A40, 0x5A40, 0x5A40, 0x5A40, 
  0x5A60, 0x5A60, 0x5A60, 0x5A60, 0x5A60, 0x5A60, 0x5A60, 0x5A60, 
  0x5A80, 0x5A80, 0x5A80, 0x5A80, 0x5A80, 0x5A80, 0x5A80, 0x5A80, 
  0x5AA0, 0x5AA0, 0x5AA0, 0x5AA0, 0x5AA0, 0x5AA0, 0x5AA0, 0x5AA0, 
  0x5AC0, 0x5AC0, 0x5AC0, 0x5AC0, 0x5AC0, 0x5AC0, 0x5AC0, 0x5AC0, 
  0x5AE0, 0x5AE0, 0x5AE0, 0x5AE0, 0x5AE0, 0x5AE0, 0x5AE0, 0x5AE0
};

static UInt8 SpectrumInk[256] = {
   0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7, 
   0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7, 
   0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7, 
   0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7, 
   8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15, 
   8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15, 
   8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15, 
   8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15, 
   0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7, 
   0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7, 
   0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7, 
   0,  1,  2,  3,  4,  5,  6,  7,  0,  1,  2,  3,  4,  5,  6,  7, 
   8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15, 
   8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15, 
   8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15, 
   8,  9, 10, 11, 12, 13, 14, 15,  8,  9, 10, 11, 12, 13, 14, 15
};

static UInt8 SpectrumPaper[256] = {
   8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9, 
  10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 
  12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 
  14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 
   8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9, 
  10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 
  12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 
  14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 
   8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9, 
    10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 
  12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 
  14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 
   8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9, 
  10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 
  12, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 
  14, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15
};

UInt16 SpectrumControl[9] = {stopCmd, restartCmd, configCmd,
                             loadSnapCmd, 0, 0,
                             0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef SpectrumButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {KEY_UP,    internalFontID, "\x80",  C1, C2, C3},
  {KEY_DOWN,  internalFontID, "\x81", C1, C2, C3},
  {KEY_BREAK, internalFontID, "BRK", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_LEFT,  internalFontID, "\x82", C1, C2, C3},
  {KEY_RIGHT, internalFontID, "\x83", C1, C2, C3},
  {KEY_EXT,   internalFontID, "EX",  C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {KEY_GRAPH, internalFontID, "GR",  C1, C2, C3}
};

// line
// FE   CAPS Z X C V
// FD   A S D F G
// FB   Q W E R T
// F7   1 2 3 4 5
// EF   0 9 8 7 6
// DF   P O I U Y
// BF   ENTER L K J H
// 7F   SPACE SYMBOL M N B

  //{KEY_CAPS, 0xFE, 0xFE, 0, 0},

static KeyMap SpectrumMap[] = {
  {'z', 0xFE, 0xFD, 0, 0},
  {'Z', 0xFE, 0xFD, 1, 0},
  {':', 0xFE, 0xFD, 0, 1},
  {'x', 0xFE, 0xFB, 0, 0},
  {'X', 0xFE, 0xFB, 1, 0},
  {'c', 0xFE, 0xF7, 0, 0},
  {'C', 0xFE, 0xF7, 1, 0},
  {'?', 0xFE, 0xF7, 0, 1},
  {'v', 0xFE, 0xEF, 0, 0},
  {'V', 0xFE, 0xEF, 1, 0},
  {'/', 0xFE, 0xEF, 0, 1},
  {'a', 0xFD, 0xFE, 0, 0},
  {'A', 0xFD, 0xFE, 1, 0},
  {'s', 0xFD, 0xFD, 0, 0},
  {'S', 0xFD, 0xFD, 1, 0},
  {'d', 0xFD, 0xFB, 0, 0},
  {'D', 0xFD, 0xFB, 1, 0},
  {'f', 0xFD, 0xF7, 0, 0},
  {'F', 0xFD, 0xF7, 1, 0},
  {'g', 0xFD, 0xEF, 0, 0},
  {'G', 0xFD, 0xEF, 1, 0},
  {'q', 0xFB, 0xFE, 0, 0},
  {'Q', 0xFB, 0xFE, 1, 0},
  {'w', 0xFB, 0xFD, 0, 0},
  {'W', 0xFB, 0xFD, 1, 0},
  {'e', 0xFB, 0xFB, 0, 0},
  {'E', 0xFB, 0xFB, 1, 0},
  {'r', 0xFB, 0xF7, 0, 0},
  {'R', 0xFB, 0xF7, 1, 0},
  {'<', 0xFB, 0xF7, 0, 1},
  {'t', 0xFB, 0xEF, 0, 0},
  {'T', 0xFB, 0xEF, 1, 0},
  {'>', 0xFB, 0xEF, 0, 1},
  {'1', 0xF7, 0xFE, 0, 0},
  {'!', 0xF7, 0xFE, 0, 1},
  {'2', 0xF7, 0xFD, 0, 0},
  {'@', 0xF7, 0xFD, 0, 1},
  {'3', 0xF7, 0xFB, 0, 0},
  {'#', 0xF7, 0xFB, 0, 1},
  {'4', 0xF7, 0xF7, 0, 0},
  {'$', 0xF7, 0xF7, 0, 1},
  {'5', 0xF7, 0xEF, 0, 0},
  {KEY_LEFT, 0xF7, 0xEF, 1, 0},
  {'%', 0xF7, 0xEF, 0, 1},
  {'0', 0xEF, 0xFE, 0, 0},
  {'_', 0xEF, 0xFE, 0, 1},
  {KEY_BACK, 0xEF, 0xFE, 1, 0},
  {'9', 0xEF, 0xFD, 0, 0},
  {KEY_GRAPH, 0xEF, 0xFD, 1, 0},
  {')', 0xEF, 0xFD, 0, 1},
  {'8', 0xEF, 0xFB, 0, 0},
  {KEY_RIGHT, 0xEF, 0xFB, 1, 0},
  {'(', 0xEF, 0xFB, 0, 1},
  {'7', 0xEF, 0xF7, 0, 0},
  {KEY_UP, 0xEF, 0xF7, 1, 0},
  {'\'', 0xEF, 0xF7, 0, 1},
  {'6', 0xEF, 0xEF, 0, 0},
  {KEY_DOWN, 0xEF, 0xEF, 1, 0},
  {'&', 0xEF, 0xEF, 0, 1},
  {'p', 0xDF, 0xFE, 0, 0},
  {'P', 0xDF, 0xFE, 1, 0},
  {'"', 0xDF, 0xFE, 0, 1},
  {'o', 0xDF, 0xFD, 0, 0},
  {'O', 0xDF, 0xFD, 1, 0},
  {';', 0xDF, 0xFD, 0, 1},
  {'i', 0xDF, 0xFB, 0, 0},
  {'I', 0xDF, 0xFB, 1, 0},
  {'u', 0xDF, 0xF7, 0, 0},
  {'U', 0xDF, 0xF7, 1, 0},
  {'y', 0xDF, 0xEF, 0, 0},
  {'Y', 0xDF, 0xEF, 1, 0},
  {KEY_ENTER, 0xBF, 0xFE, 0, 0},
  {'l', 0xBF, 0xFD, 0, 0},
  {'L', 0xBF, 0xFD, 1, 0},
  {'=', 0xBF, 0xFD, 0, 1},
  {'k', 0xBF, 0xFB, 0, 0},
  {'K', 0xBF, 0xFB, 1, 0},
  {'+', 0xBF, 0xFB, 0, 1},
  {'j', 0xBF, 0xF7, 0, 0},
  {'J', 0xBF, 0xF7, 1, 0},
  {'-', 0xBF, 0xF7, 0, 1},
  {'h', 0xBF, 0xEF, 0, 0},
  {'H', 0xBF, 0xEF, 1, 0},
  {'^', 0xBF, 0xEF, 0, 1},
  {' ', 0x7F, 0xFE, 0, 0},
  {KEY_BREAK, 0x7F, 0xFE, 1, 0},
  {'m', 0x7F, 0xFB, 0, 0},
  {'M', 0x7F, 0xFB, 1, 0},
  {'.', 0x7F, 0xFB, 0, 1},
  {'n', 0x7F, 0xF7, 0, 0},
  {'N', 0x7F, 0xF7, 1, 0},
  {',', 0x7F, 0xF7, 0, 1},
  {'b', 0x7F, 0xEF, 0, 0},
  {'B', 0x7F, 0xEF, 1, 0},
  {'*', 0x7F, 0xEF, 0, 1},
  {KEY_EXT, 0xFF, 0xFF, 1, 1},  // Caps Shift + Symbol Shift
  {0, 0, 0, 0, 0}
};

static void spectrum_video(void);

UInt32 spectrum_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_VSYNC:
      spectrum_video();
      spectrum_vsync(arg->a1);
      return 0;
    case IO_READB:
      return spectrum_readb(arg->a1);
    case IO_WRITEB:
      spectrum_writeb(arg->a1, arg->a2);
  }
  return 0;
}

UInt8 spectrum_readb(UInt16 a) {
  SpecGlobals *g;
  UInt32 key;
  UInt8 b = 0;

  if (a == 0xFFFF)	// GAMBIARRA: CheckInput de console do CP/M
    return 1;

  switch (a & 0xFF) {
    case 0x1F:
      g = hardware->globals;
      b = 0x00;

      // kempston joystick (active high)
      // Bit 0: right
      // Bit 1: left
      // Bit 2: down
      // Bit 3: up
      // Bit 4: fire

      if (hardware->button & g->button)
        b |= 0x10;

      if (hardware->joyx < 16)
        b |= 0x02;
      else if (hardware->joyx > 48)
        b |= 0x01;
      if (hardware->joyy < 16)
        b |= 0x08;
      else if (hardware->joyy > 48)
        b |= 0x04;

      break;
    case 0x7F:
      g = hardware->globals;
      b = 0xFF;

      // fuller joystick (active low)
      // Bit 0: up
      // Bit 1: down
      // Bit 2: left
      // Bit 3: rigth
      // Bit 7: fire

      if (hardware->button & g->button)
        b &= 0x7F;

      if (hardware->joyx < 16)
        b &= 0xFB;
      else if (hardware->joyx > 48)
        b &= 0xF7;
      if (hardware->joyy < 16)
        b &= 0xFE;
      else if (hardware->joyy > 48)
        b &= 0xFD;

      break;
    case 0xDF:
      g = hardware->globals;
      b = 0xFF;

      // mikrogen joystick (active low)
      // Bit 0: up
      // Bit 1: down
      // Bit 2: right
      // Bit 3: left
      // Bit 4: fire

      if (hardware->button & g->button)
        b &= 0xEF;

      if (hardware->joyx < 16)
        b &= 0xF7;
      else if (hardware->joyx > 48)
        b &= 0xFB;
      if (hardware->joyy < 16)
        b &= 0xFE;
      else if (hardware->joyy > 48)
        b &= 0xFD;

      break;
    default:
      if (!(a & 1)) {
        b = 0xFF;
        switch (a) {
          case 0xFEFE:
          case 0xFDFE:
          case 0xFBFE:
          case 0xF7FE:
          case 0xEFFE:
          case 0xDFFE:
          case 0xBFFE:
          case 0x7FFE:
            key = hardware->key;
            if (key) {
              g = hardware->globals;
              if (g->keyMap[key].line == (a >> 8))
                b &= g->keyMap[key].column;
              if (g->keyMap[key].shift && SPECTRUM_SHIFT_LINE == (a >> 8))
                b &= SPECTRUM_SHIFT_COLUMN;
              if (g->keyMap[key].ctrl && SPECTRUM_SSHIFT_LINE == (a >> 8))
                b &= SPECTRUM_SSHIFT_COLUMN;
            }
        }
      }
  }

  return b;
}

void spectrum_writeb(UInt16 a, UInt8 b) {
  if (!(a & 1)) {
    SpecGlobals *g = hardware->globals;

    if (g->border_color != (b & 0x07)) {
      g->border_color = b & 0x07;
      g->border = 1;
    }
    snd_output((b & 0x10) ? 0xFF : 0x00);
  }
}

static void spectrum_video(void) {
  SpecGlobals *g = hardware->globals;
  RectangleType rect;
  WinHandle prev;
  UInt32 addr, caddr, i, j, q;
  UInt8 p, c, ink, paper, c1, c2, aux;
  Boolean dirty = false;
  Err err;

  WinSetCoordinateSystem(kCoordinatesDouble);

  if (hardware->screen_wh == NULL) {
    hardware->screen_wh = WinCreateOffscreenWindow(256, 192, nativeFormat, &err);
  }

  if (g->flash_count & 1) {
    if (g->flash_count == 15) {
      for (addr = 0x5800; !dirty && addr < 0x5B00; addr++) {
        if ((hardware->m1[addr] & 0x80) || hardware->m1[addr] != g->video[addr - 0x4000]) dirty = true;
      }
      for (addr = 0x4000; !dirty && addr < 0x5800; addr++) {
        if (hardware->m1[addr] != g->video[addr - 0x4000]) dirty = true;
      }
    } else {
      for (addr = 0x4000; !dirty && addr < 0x5B00; addr++) {
        if (hardware->m1[addr] != g->video[addr - 0x4000]) dirty = true;
      }
    }
  }

  if (g->border || dirty) {
    prev = WinSetDrawWindow(hardware->screen_wh);

    if (g->border) {
      c = hardware->color[g->border_color | 0x08];
      WinSetBackColor(c);
      RctSetRectangle(&rect, 0, 0, hardware->display_width, hardware->y0+192+hardware->y0);
      WinEraseRectangle(&rect, 0);
      dirty = true;
      g->border = 0;
    }

    if (dirty) {
      for (i = 0; i < 192; i++) {
        addr = g->SpectrumVideoAddr[i];
        caddr = g->SpectrumVideoCAddr[i];

        for (j = 0; j < 32; j++, addr++) {
          p = hardware->m1[addr]; 
          c = hardware->m1[caddr + j];

          if (!(c & 0x80) && p == g->video[addr - 0x4000] && c == g->video[caddr + j - 0x4000]) {
            continue;
          }

          ink = g->SpectrumInk[c];
          paper = g->SpectrumPaper[c];

          if ((c & 0x80) && g->flash_status) {
            aux = ink;
            ink = paper; 
            paper = aux | 0x08;
          }
          c1 = hardware->color[ink];
          c2 = hardware->color[paper];

          for (q = 0; q < 8; q++, p <<= 1) {
            WinSetForeColor((p & 0x80) ? c1 : c2);
            WinDrawPixel(j*8+q, i);
          }
        }
      }
      MemMove(g->video, &hardware->m1[0x4000], 0x1B00);
    }

    WinSetDrawWindow(prev);
    RctSetRectangle(&rect, 0, 0, 256, 192);
    WinCopyRectangle(hardware->screen_wh, NULL, &rect, hardware->x0, hardware->y0, winPaint);
  }

  g->flash_count++;

  if (g->flash_count == 16) {
    g->flash_count = 0;
    g->flash_status = !g->flash_status;
  }

  WinSetCoordinateSystem(kCoordinatesStandard);
}

void spectrum_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  SpecGlobals *globals;
  KeyMap *keyMap;
  UInt16 i, len;

  id = machine->id;
  hardware = _hardware;

  len = sizeof(prefs);
  MemSet(&prefs, len, 0);

  if (PrefGetAppPreferences(AppID, 128 + id, &prefs, &len, true) == noPreferenceFound) {
    prefs.joystick = 0;
    prefs.button = keyBitHard1;
    PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, len, true);
  }

  hardware->joyx = hardware->joyy = 32;
  hardware->key = 0;
  key = 0;

  if (!video) {
    video = MemPtrNew(0x1B00);
    MemSet(video, 0x1B00, 0);
  }

  if (!hardware->globals) {
    hardware->globals = MemPtrNew(sizeof(SpecGlobals));
    MemSet(hardware->globals, sizeof(SpecGlobals), 0);
  }
  globals = hardware->globals;

  globals->flash_count = 0;
  globals->flash_status = 0;
  globals->border = 1;
  globals->border_color = 7;
  globals->video = video;
  globals->SpectrumInk = SpectrumInk;
  globals->SpectrumPaper = SpectrumPaper;
  globals->button = prefs.button;

  keyMap = MemPtrNew(256 * sizeof(KeyMap));
  kbd_initmap(SpectrumMap, keyMap);

  for (i = 0; i < 256; i++) {
    globals->keyMap[i].key = keyMap[i].key;
    globals->keyMap[i].line = keyMap[i].line;
    globals->keyMap[i].column = keyMap[i].column;
    globals->keyMap[i].shift = keyMap[i].shift;
    globals->keyMap[i].ctrl = keyMap[i].ctrl;
  }
  MemPtrFree(keyMap);

  for (i = 0; i < 192; i++) {
    globals->SpectrumVideoAddr[i] = SpectrumVideoAddr[i];
    globals->SpectrumVideoCAddr[i] = SpectrumVideoCAddr[i];
  }

  hardware->x0 = (hardware->display_width - 256) / 2;
  hardware->y0 = (hardware->display_height - 192 - lowBorder*2) / 2;
  hardware->dx = 256;
  hardware->dy = 192;
}

void spectrum_init(void) {
}

void spectrum_finish(void) {
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);

  if (video) {
    MemPtrFree(video);
    video = NULL;
  }

  if (hardware->globals) {
    MemPtrFree(hardware->globals);
    hardware->globals = NULL;
  }
}

void spectrum_key(UInt16 c) {
  key = c;
  hardware->key = c;
}

void spectrum_joystick(UInt16 x, UInt16 y) {
  hardware->joyx = x;
  hardware->joyy = y;
}

SpecPrefs *SpecGetPrefs(void) {
  return &prefs;
}

Boolean SpecFormHandler(EventPtr event, Boolean *close) {
  FormPtr frm;
  ListPtr lst;
  UInt32 button;
  SpecGlobals *g;
  Boolean handled;

  frm = FrmGetActiveForm();
  handled = false;
  *close = false;

  switch (event->eType) {
    case frmOpenEvent:
      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, joyList));
      LstSetSelection(lst, prefs.joystick);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
         FrmGetObjectIndex(frm, joyCtl)),
         LstGetSelectionText(lst, prefs.joystick));

      switch (prefs.button) {
        case keyBitHard1: button = 0; break;
        case keyBitHard2: button = 1; break;
        case keyBitHard3: button = 2; break;
        case keyBitHard4: button = 3; break;
        //case keyBitJogPress: button = 4; break;
        //case keyBitRockerCenter: button = 5; break;
        default: button = 0;
      }

      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, buttonList));
      LstSetSelection(lst, button);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
        FrmGetObjectIndex(frm, buttonCtl)),
        LstGetSelectionText(lst, button));

      FrmDrawForm(frm);
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case okBtn:
          *close = true;
          handled = true;
      }
      break;

    case popSelectEvent:
      switch (event->data.popSelect.listID) {
        case buttonList:
          switch (event->data.popSelect.selection) {
            case 0: prefs.button = keyBitHard1; break;
            case 1: prefs.button = keyBitHard2; break;
            case 2: prefs.button = keyBitHard3; break;
            case 3: prefs.button = keyBitHard4; break;
            //case 4: prefs.button = keyBitJogPress; break;
            //case 5: prefs.button = keyBitRockerCenter; break;
            default: prefs.button = keyBitHard1;
          }
          // para o arm_callback
          g = hardware->globals;
          g->button = prefs.button;
          break;
        case joyList:
          prefs.joystick = event->data.popSelect.selection;
      }

    default:
      break;
  }

  return handled;
}
