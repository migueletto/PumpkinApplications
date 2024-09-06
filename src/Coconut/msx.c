#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "machine.h"
#include "kbd.h"
#include "ay8910.h"
#include "ti9918.h"
#include "endian.h"
#include "z80.h"
#include "msx.h"
#include "misc.h"
#include "cdebug.h"
#include "gui.h"

static UInt8 id;
static MsxPrefs prefs;
static Hardware *hardware;
static UInt16 key;

UInt16 MsxControl[9] = {stopCmd, restartCmd, configCmd,
                        loadSnapCmd, 0, 0,
                        0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef MsxButtonDef[12] = {
  {KEY_F1,    internalFontID, "F1",  C1, C2, C3},
  {KEY_F2,    internalFontID, "F2",  C1, C2, C3},
  {KEY_F3,    internalFontID, "F3",  C1, C2, C3},
  {KEY_F4,    internalFontID, "F4",  C1, C2, C3},

  {KEY_UP,    internalFontID, "\x80",  C1, C2, C3},
  {KEY_DOWN,  internalFontID, "\x81", C1, C2, C3},
  {KEY_CODE,  internalFontID, "COD", C1, C2, C3},
  {KEY_GRAPH, internalFontID, "GR",  C1, C2, C3},

  {KEY_LEFT,  internalFontID, "\x82", C1, C2, C3},
  {KEY_RIGHT, internalFontID, "\x83", C1, C2, C3},
  {KEY_ESC,  internalFontID, "ESC", C1, C2, C3},
  {KEY_CTRL,  internalFontID, "CTL", C1, C2, C3},
};

/*
  Line  Bit_7 Bit_6 Bit_5 Bit_4 Bit_3 Bit_2 Bit_1 Bit_0
   0     7     6     5     4     3     2     1     0
   1     ;     ]     [     \     =     -     9     8
   2     B     A    ???    /     .     ,     '     `
   3     J     I     H     G     F     E     D     C
   4     R     Q     P     O     N     M     L     K
   5     Z     Y     X     W     V     U     T     S
   6     F3    F2    F1   CODE   CAP  GRAPH CTRL  SHIFT
   7     RET   SEL   BS   STOP   TAB   ESC   F5    F4
   8    RIGHT DOWN   UP   LEFT   DEL   INS  HOME  SPACE
 ( 9    NUM4  NUM3  NUM2  NUM1  NUM0  NUM/  NUM+  NUM*  )
 ( 10   NUM.  NUM,  NUM-  NUM9  NUM8  NUM7  NUM6  NUM5  )
*/

/*
static KeyMap MsxMap[] = {
  {'0', 0, 0xFE, 0, 0},
  {')', 0, 0xFE, 1, 0},
  {'1', 0, 0xFD, 0, 0},
  {'!', 0, 0xFD, 1, 0},
  {'2', 0, 0xFB, 0, 0},
  {'@', 0, 0xFB, 1, 0},
  {'3', 0, 0xF7, 0, 0},
  {'#', 0, 0xF7, 1, 0},
  {'4', 0, 0xEF, 0, 0},
  {'$', 0, 0xEF, 1, 0},
  {'5', 0, 0xDF, 0, 0},
  {'%', 0, 0xDF, 1, 0},
  {'6', 0, 0xBF, 0, 0},
  {'^', 0, 0xBF, 1, 0},
  {'7', 0, 0x7F, 0, 0},
  {'&', 0, 0x7F, 1, 0},

  {'8', 1, 0xFE, 0, 0},
  {'*', 1, 0xFE, 1, 0},
  {'9', 1, 0xFD, 0, 0},
  {'(', 1, 0xFD, 1, 0},
  {'-', 1, 0xFB, 0, 0},
  {'_', 1, 0xFB, 1, 0},
  {'=', 1, 0xF7, 0, 0},
  {'+', 1, 0xF7, 1, 0},
  {'\\', 1, 0xEF, 0, 0},
  {'|', 1, 0xEF, 1, 0},
  {'[', 1, 0xDF, 0, 0},
  {'{', 1, 0xDF, 1, 0},
  {']', 1, 0xBF, 0, 0},
  {'}', 1, 0xBF, 1, 0},
  {';', 1, 0x7F, 0, 0},
  {':', 1, 0x7F, 1, 0},

  {'\'', 2, 0xFE, 0, 0},
  {'"', 2, 0xFE, 1, 0},
  {'`', 2, 0xFD, 0, 0},
  {'~', 2, 0xFD, 1, 0},
  {',', 2, 0xFB, 0, 0},
  {'<', 2, 0xFB, 1, 0},
  {'.', 2, 0xF7, 0, 0},
  {'>', 2, 0xF7, 1, 0},
  {'/', 2, 0xEF, 0, 0},
  {'?', 2, 0xEF, 1, 0},
  //{0, 2, 0xDF, 0, 0},
  //{0, 2, 0xDF, 1, 0},
  {'a', 2, 0xBF, 0, 0},
  {'A', 2, 0xBF, 1, 0},
  {'b', 2, 0x7F, 0, 0},
  {'B', 2, 0x7F, 1, 0},

  {'c', 3, 0xFE, 0, 0},
  {'C', 3, 0xFE, 1, 0},
  {'d', 3, 0xFD, 0, 0},
  {'D', 3, 0xFD, 1, 0},
  {'e', 3, 0xFB, 0, 0},
  {'E', 3, 0xFB, 1, 0},
  {'f', 3, 0xF7, 0, 0},
  {'F', 3, 0xF7, 1, 0},
  {'g', 3, 0xEF, 0, 0},
  {'G', 3, 0xEF, 1, 0},
  {'h', 3, 0xDF, 0, 0},
  {'H', 3, 0xDF, 1, 0},
  {'i', 3, 0xBF, 0, 0},
  {'I', 3, 0xBF, 1, 0},
  {'j', 3, 0x7F, 0, 0},
  {'J', 3, 0x7F, 1, 0},

  {'k', 4, 0xFE, 0, 0},
  {'K', 4, 0xFE, 1, 0},
  {'l', 4, 0xFD, 0, 0},
  {'L', 4, 0xFD, 1, 0},
  {'m', 4, 0xFB, 0, 0},
  {'M', 4, 0xFB, 1, 0},
  {'n', 4, 0xF7, 0, 0},
  {'N', 4, 0xF7, 1, 0},
  {'o', 4, 0xEF, 0, 0},
  {'O', 4, 0xEF, 1, 0},
  {'p', 4, 0xDF, 0, 0},
  {'P', 4, 0xDF, 1, 0},
  {'q', 4, 0xBF, 0, 0},
  {'Q', 4, 0xBF, 1, 0},
  {'r', 4, 0x7F, 0, 0},
  {'R', 4, 0x7F, 1, 0},

  {'s', 5, 0xFE, 0, 0},
  {'S', 5, 0xFE, 1, 0},
  {'t', 5, 0xFD, 0, 0},
  {'T', 5, 0xFD, 1, 0},
  {'u', 5, 0xFB, 0, 0},
  {'U', 5, 0xFB, 1, 0},
  {'v', 5, 0xF7, 0, 0},
  {'V', 5, 0xF7, 1, 0},
  {'w', 5, 0xEF, 0, 0},
  {'W', 5, 0xEF, 1, 0},
  {'x', 5, 0xDF, 0, 0},
  {'X', 5, 0xDF, 1, 0},
  {'y', 5, 0xBF, 0, 0},
  {'Y', 5, 0xBF, 1, 0},
  {'z', 5, 0x7F, 0, 0},
  {'Z', 5, 0x7F, 1, 0},

  {KEY_SHIFT, 6, 0xFE, 0, 0},
  {KEY_CTRL, 6, 0xFD, 0, 0},
  {KEY_GRAPH, 6, 0xFB, 0, 0},
  {KEY_CAP, 6, 0xF7, 0, 0},
  {KEY_CODE, 6, 0xEF, 0, 0},
  {KEY_F1, 6, 0xDF, 0, 0},
  {KEY_F2, 6, 0xBF, 0, 0},
  {KEY_F3, 6, 0x7F, 0, 0},

  {KEY_F4, 7, 0xFE, 0, 0},
  {KEY_F5, 7, 0xFD, 0, 0},
  {KEY_ESC, 7, 0xFB, 0, 0},
  {'\t', 7, 0xF7, 0, 0},
  {KEY_STOP, 7, 0xEF, 0, 0},
  {KEY_BACK, 7, 0xDF, 0, 0},
  {KEY_SEL, 7, 0xBF, 0, 0},
  {KEY_ENTER, 7, 0x7F, 0, 0},

  {' ', 8, 0xFE, 0, 0},
  {KEY_HOME, 8, 0xFD, 0, 0},
  {KEY_INSERT, 8, 0xFB, 0, 0},
  {KEY_DEL, 8, 0xF7, 0, 0},
  {KEY_LEFT, 8, 0xEF, 0, 0},
  {KEY_UP, 8, 0xDF, 0, 0},
  {KEY_DOWN, 8, 0xBF, 0, 0},
  {KEY_RIGHT, 8, 0x7F, 0, 0},

  {0, 0, 0, 0, 0}
};

static KeyMap Expert10Map[] = {
  {'0', 0, 0xFE, 0, 0},
  {')', 0, 0xFE, 1, 0},
  {'1', 0, 0xFD, 0, 0},
  {'!', 0, 0xFD, 1, 0},
  {'2', 0, 0xFB, 0, 0},
  {'@', 0, 0xFB, 1, 0},
  {'3', 0, 0xF7, 0, 0},
  {'#', 0, 0xF7, 1, 0},
  {'4', 0, 0xEF, 0, 0},
  {'$', 0, 0xEF, 1, 0},
  {'5', 0, 0xDF, 0, 0},
  {'%', 0, 0xDF, 1, 0},
  {'6', 0, 0xBF, 0, 0},
  {'^', 0, 0xBF, 1, 0},
  {'7', 0, 0x7F, 0, 0},
  {'&', 0, 0x7F, 1, 0},

  {'8', 1, 0xFE, 0, 0},
  {'*', 1, 0xFE, 1, 0},
  {'9', 1, 0xFD, 0, 0},
  {'(', 1, 0xFD, 1, 0},
  {'-', 1, 0xFB, 0, 0},
  {'_', 1, 0xFB, 1, 0},
  {'=', 1, 0xF7, 0, 0},
  {'+', 1, 0xF7, 1, 0},
  {'\\', 1, 0xEF, 0, 0},
  {'|', 1, 0xEF, 1, 0},
  {'[', 1, 0xDF, 0, 0},
  {'{', 1, 0xDF, 1, 0},
  {']', 1, 0xBF, 0, 0},
  {'}', 1, 0xBF, 1, 0},
  {';', 1, 0x7F, 0, 0},
  {':', 1, 0x7F, 1, 0},

  {'\'', 2, 0xFE, 0, 0},
  {'"', 2, 0xFE, 1, 0},
  {231, 2, 0xFD, 0, 0},		// cedilha minisculo
  {199, 2, 0xFD, 1, 0},		// cedilha maiusculo
  {',', 2, 0xFB, 0, 0},
  {'<', 2, 0xFB, 1, 0},
  {'.', 2, 0xF7, 0, 0},
  {'>', 2, 0xF7, 1, 0},
  {'/', 2, 0xEF, 0, 0},
  {'?', 2, 0xEF, 1, 0},
  //{0, 2, 0xDF, 0, 0},		// crase (composicao)
  //{0, 2, 0xDF, 1, 0},		// acento agudo (composicao)
  {'a', 2, 0xBF, 0, 0},
  {'A', 2, 0xBF, 1, 0},
  {'b', 2, 0x7F, 0, 0},
  {'B', 2, 0x7F, 1, 0},

  {'c', 3, 0xFE, 0, 0},
  {'C', 3, 0xFE, 1, 0},
  {'d', 3, 0xFD, 0, 0},
  {'D', 3, 0xFD, 1, 0},
  {'e', 3, 0xFB, 0, 0},
  {'E', 3, 0xFB, 1, 0},
  {'f', 3, 0xF7, 0, 0},
  {'F', 3, 0xF7, 1, 0},
  {'g', 3, 0xEF, 0, 0},
  {'G', 3, 0xEF, 1, 0},
  {'h', 3, 0xDF, 0, 0},
  {'H', 3, 0xDF, 1, 0},
  {'i', 3, 0xBF, 0, 0},
  {'I', 3, 0xBF, 1, 0},
  {'j', 3, 0x7F, 0, 0},
  {'J', 3, 0x7F, 1, 0},

  {'k', 4, 0xFE, 0, 0},
  {'K', 4, 0xFE, 1, 0},
  {'l', 4, 0xFD, 0, 0},
  {'L', 4, 0xFD, 1, 0},
  {'m', 4, 0xFB, 0, 0},
  {'M', 4, 0xFB, 1, 0},
  {'n', 4, 0xF7, 0, 0},
  {'N', 4, 0xF7, 1, 0},
  {'o', 4, 0xEF, 0, 0},
  {'O', 4, 0xEF, 1, 0},
  {'p', 4, 0xDF, 0, 0},
  {'P', 4, 0xDF, 1, 0},
  {'q', 4, 0xBF, 0, 0},
  {'Q', 4, 0xBF, 1, 0},
  {'r', 4, 0x7F, 0, 0},
  {'R', 4, 0x7F, 1, 0},

  {'s', 5, 0xFE, 0, 0},
  {'S', 5, 0xFE, 1, 0},
  {'t', 5, 0xFD, 0, 0},
  {'T', 5, 0xFD, 1, 0},
  {'u', 5, 0xFB, 0, 0},
  {'U', 5, 0xFB, 1, 0},
  {'v', 5, 0xF7, 0, 0},
  {'V', 5, 0xF7, 1, 0},
  {'w', 5, 0xEF, 0, 0},
  {'W', 5, 0xEF, 1, 0},
  {'x', 5, 0xDF, 0, 0},
  {'X', 5, 0xDF, 1, 0},
  {'y', 5, 0xBF, 0, 0},
  {'Y', 5, 0xBF, 1, 0},
  {'z', 5, 0x7F, 0, 0},
  {'Z', 5, 0x7F, 1, 0},

  {KEY_SHIFT, 6, 0xFE, 0, 0},
  {KEY_CTRL, 6, 0xFD, 0, 0},
  {KEY_GRAPH, 6, 0xFB, 0, 0},
  {KEY_CAP, 6, 0xF7, 0, 0},
  {KEY_CODE, 6, 0xEF, 0, 0},
  {KEY_F1, 6, 0xDF, 0, 0},
  {KEY_F2, 6, 0xBF, 0, 0},
  {KEY_F3, 6, 0x7F, 0, 0},

  {KEY_F4, 7, 0xFE, 0, 0},
  {KEY_F5, 7, 0xFD, 0, 0},
  {KEY_ESC, 7, 0xFB, 0, 0},
  {'\t', 7, 0xF7, 0, 0},
  {KEY_STOP, 7, 0xEF, 0, 0},
  {KEY_BACK, 7, 0xDF, 0, 0},
  {KEY_SEL, 7, 0xBF, 0, 0},
  {KEY_ENTER, 7, 0x7F, 0, 0},

  {' ', 8, 0xFE, 0, 0},
  {KEY_HOME, 8, 0xFD, 0, 0},
  {KEY_INSERT, 8, 0xFB, 0, 0},
  {KEY_DEL, 8, 0xF7, 0, 0},
  {KEY_LEFT, 8, 0xEF, 0, 0},
  {KEY_UP, 8, 0xDF, 0, 0},
  {KEY_DOWN, 8, 0xBF, 0, 0},
  {KEY_RIGHT, 8, 0x7F, 0, 0},

  {0, 0, 0, 0, 0}
};
*/

static KeyMap Expert11Map[] = {
  {'0', 0, 0xFE, 0, 0},
  {')', 0, 0xFE, 1, 0},
  {'1', 0, 0xFD, 0, 0},
  {'!', 0, 0xFD, 1, 0},
  {'2', 0, 0xFB, 0, 0},
  {'"', 0, 0xFB, 1, 0},
  {'3', 0, 0xF7, 0, 0},
  {'#', 0, 0xF7, 1, 0},
  {'4', 0, 0xEF, 0, 0},
  {'$', 0, 0xEF, 1, 0},
  {'5', 0, 0xDF, 0, 0},
  {'%', 0, 0xDF, 1, 0},
  {'6', 0, 0xBF, 0, 0},
  {'^', 0, 0xBF, 1, 0},
  {'7', 0, 0x7F, 0, 0},
  {'&', 0, 0x7F, 1, 0},

  {'8', 1, 0xFE, 0, 0},
  {'\'', 1, 0xFE, 1, 0},
  {'9', 1, 0xFD, 0, 0},
  {'(', 1, 0xFD, 1, 0},
  {'-', 1, 0xFB, 0, 0},
  {'_', 1, 0xFB, 1, 0},
  {'=', 1, 0xF7, 0, 0},
  {'+', 1, 0xF7, 1, 0},
  {'{', 1, 0xEF, 0, 0},
  {'}', 1, 0xEF, 1, 0},
  //{'\'', 1, 0xDF, 0, 0},	// acento agudo (composicao)
  {'`', 1, 0xDF, 1, 0},		// acento crase (composicao)
  {'[', 1, 0xBF, 0, 0},
  {']', 1, 0xBF, 1, 0},
  {'~', 1, 0x7F, 0, 0},		// acento til (composicao)
  //{'^', 1, 0x7F, 1, 0},	// acento circunflexo (composicao)

  {'*', 2, 0xFE, 0, 0},
  {'@', 2, 0xFE, 1, 0},
  {231, 2, 0xFD, 0, 0},
  {199, 2, 0xFD, 1, 0},
  {',', 2, 0xFB, 0, 0},
  {'<', 2, 0xFB, 1, 0},
  {'.', 2, 0xF7, 0, 0},
  {'>', 2, 0xF7, 1, 0},
  {';', 2, 0xEF, 0, 0},
  {':', 2, 0xEF, 1, 0},
  {'/', 2, 0xDF, 0, 0},
  {'?', 2, 0xDF, 1, 0},
  {'a', 2, 0xBF, 0, 0},
  {'A', 2, 0xBF, 1, 0},
  {'b', 2, 0x7F, 0, 0},
  {'B', 2, 0x7F, 1, 0},

  {'c', 3, 0xFE, 0, 0},
  {'C', 3, 0xFE, 1, 0},
  {'d', 3, 0xFD, 0, 0},
  {'D', 3, 0xFD, 1, 0},
  {'e', 3, 0xFB, 0, 0},
  {'E', 3, 0xFB, 1, 0},
  {'f', 3, 0xF7, 0, 0},
  {'F', 3, 0xF7, 1, 0},
  {'g', 3, 0xEF, 0, 0},
  {'G', 3, 0xEF, 1, 0},
  {'h', 3, 0xDF, 0, 0},
  {'H', 3, 0xDF, 1, 0},
  {'i', 3, 0xBF, 0, 0},
  {'I', 3, 0xBF, 1, 0},
  {'j', 3, 0x7F, 0, 0},
  {'J', 3, 0x7F, 1, 0},

  {'k', 4, 0xFE, 0, 0},
  {'K', 4, 0xFE, 1, 0},
  {'l', 4, 0xFD, 0, 0},
  {'L', 4, 0xFD, 1, 0},
  {'m', 4, 0xFB, 0, 0},
  {'M', 4, 0xFB, 1, 0},
  {'n', 4, 0xF7, 0, 0},
  {'N', 4, 0xF7, 1, 0},
  {'o', 4, 0xEF, 0, 0},
  {'O', 4, 0xEF, 1, 0},
  {'p', 4, 0xDF, 0, 0},
  {'P', 4, 0xDF, 1, 0},
  {'q', 4, 0xBF, 0, 0},
  {'Q', 4, 0xBF, 1, 0},
  {'r', 4, 0x7F, 0, 0},
  {'R', 4, 0x7F, 1, 0},

  {'s', 5, 0xFE, 0, 0},
  {'S', 5, 0xFE, 1, 0},
  {'t', 5, 0xFD, 0, 0},
  {'T', 5, 0xFD, 1, 0},
  {'u', 5, 0xFB, 0, 0},
  {'U', 5, 0xFB, 1, 0},
  {'v', 5, 0xF7, 0, 0},
  {'V', 5, 0xF7, 1, 0},
  {'w', 5, 0xEF, 0, 0},
  {'W', 5, 0xEF, 1, 0},
  {'x', 5, 0xDF, 0, 0},
  {'X', 5, 0xDF, 1, 0},
  {'y', 5, 0xBF, 0, 0},
  {'Y', 5, 0xBF, 1, 0},
  {'z', 5, 0x7F, 0, 0},
  {'Z', 5, 0x7F, 1, 0},

  {KEY_SHIFT, 6, 0xFE, 0, 0},
  {KEY_CTRL, 6, 0xFD, 0, 0},
  {KEY_GRAPH, 6, 0xFB, 0, 0},	// LGRAPH
  {KEY_CAP, 6, 0xF7, 0, 0},
  {KEY_CODE, 6, 0xEF, 0, 0},	// RGRAPH
  {KEY_F1, 6, 0xDF, 0, 0},
  {KEY_F2, 6, 0xBF, 0, 0},
  {KEY_F3, 6, 0x7F, 0, 0},

  {KEY_F4, 7, 0xFE, 0, 0},
  {KEY_F5, 7, 0xFD, 0, 0},
  {KEY_ESC, 7, 0xFB, 0, 0},
  {'\t', 7, 0xF7, 0, 0},
  {KEY_STOP, 7, 0xEF, 0, 0},
  {KEY_BACK, 7, 0xDF, 0, 0},
  {KEY_SEL, 7, 0xBF, 0, 0},
  {KEY_ENTER, 7, 0x7F, 0, 0},

  {' ', 8, 0xFE, 0, 0},
  {KEY_HOME, 8, 0xFD, 0, 0},
  {KEY_INSERT, 8, 0xFB, 0, 0},
  {KEY_DEL, 8, 0xF7, 0, 0},
  {KEY_LEFT, 8, 0xEF, 0, 0},
  {KEY_UP, 8, 0xDF, 0, 0},
  {KEY_DOWN, 8, 0xBF, 0, 0},
  {KEY_RIGHT, 8, 0x7F, 0, 0},

  {0, 0, 0, 0, 0}
};

static KeyMap HotbitMap[] = {
  {'0', 0, 0xFE, 0, 0},
  {')', 0, 0xFE, 1, 0},
  {'1', 0, 0xFD, 0, 0},
  {'!', 0, 0xFD, 1, 0},
  {'2', 0, 0xFB, 0, 0},
  {'@', 0, 0xFB, 1, 0},
  {'3', 0, 0xF7, 0, 0},
  {'#', 0, 0xF7, 1, 0},
  {'4', 0, 0xEF, 0, 0},
  {'$', 0, 0xEF, 1, 0},
  {'5', 0, 0xDF, 0, 0},
  {'%', 0, 0xDF, 1, 0},
  {'6', 0, 0xBF, 0, 0},
  {'"', 0, 0xBF, 1, 0},
  {'7', 0, 0x7F, 0, 0},
  {'&', 0, 0x7F, 1, 0},

  {'8', 1, 0xFE, 0, 0},
  {'*', 1, 0xFE, 1, 0},
  {'9', 1, 0xFD, 0, 0},
  {'(', 1, 0xFD, 1, 0},
  {'-', 1, 0xFB, 0, 0},
  {'_', 1, 0xFB, 1, 0},
  {'=', 1, 0xF7, 0, 0},
  {'+', 1, 0xF7, 1, 0},
  {'\\', 1, 0xEF, 0, 0},
  {'^', 1, 0xEF, 1, 0},
  //{'\'', 1, 0xDF, 0, 0},	// acento aguto (composicao)
  {'`', 1, 0xDF, 1, 0},		// acento crase (composicao)
  {'"', 1, 0xBF, 0, 0},
  {'\'', 1, 0xBF, 1, 0},
  {231, 1, 0x7F, 0, 0},
  {199, 1, 0x7F, 1, 0},

  {'~', 2, 0xFE, 0, 0},		// acento til (composicao)
  //{'^', 2, 0xFE, 1, 0},	// acento circunflexo (composicao)
  {'[', 2, 0xFD, 0, 0},
  {']', 2, 0xFD, 1, 0},
  {',', 2, 0xFB, 0, 0},
  {';', 2, 0xFB, 1, 0},
  {'.', 2, 0xF7, 0, 0},
  {':', 2, 0xF7, 1, 0},
  {'/', 2, 0xEF, 0, 0},
  {'?', 2, 0xEF, 1, 0},
  {'<', 2, 0xDF, 0, 0},
  {'>', 2, 0xDF, 1, 0},
  {'a', 2, 0xBF, 0, 0},
  {'A', 2, 0xBF, 1, 0},
  {'b', 2, 0x7F, 0, 0},
  {'B', 2, 0x7F, 1, 0},

  {'c', 3, 0xFE, 0, 0},
  {'C', 3, 0xFE, 1, 0},
  {'d', 3, 0xFD, 0, 0},
  {'D', 3, 0xFD, 1, 0},
  {'e', 3, 0xFB, 0, 0},
  {'E', 3, 0xFB, 1, 0},
  {'f', 3, 0xF7, 0, 0},
  {'F', 3, 0xF7, 1, 0},
  {'g', 3, 0xEF, 0, 0},
  {'G', 3, 0xEF, 1, 0},
  {'h', 3, 0xDF, 0, 0},
  {'H', 3, 0xDF, 1, 0},
  {'i', 3, 0xBF, 0, 0},
  {'I', 3, 0xBF, 1, 0},
  {'j', 3, 0x7F, 0, 0},
  {'J', 3, 0x7F, 1, 0},

  {'k', 4, 0xFE, 0, 0},
  {'K', 4, 0xFE, 1, 0},
  {'l', 4, 0xFD, 0, 0},
  {'L', 4, 0xFD, 1, 0},
  {'m', 4, 0xFB, 0, 0},
  {'M', 4, 0xFB, 1, 0},
  {'n', 4, 0xF7, 0, 0},
  {'N', 4, 0xF7, 1, 0},
  {'o', 4, 0xEF, 0, 0},
  {'O', 4, 0xEF, 1, 0},
  {'p', 4, 0xDF, 0, 0},
  {'P', 4, 0xDF, 1, 0},
  {'q', 4, 0xBF, 0, 0},
  {'Q', 4, 0xBF, 1, 0},
  {'r', 4, 0x7F, 0, 0},
  {'R', 4, 0x7F, 1, 0},

  {'s', 5, 0xFE, 0, 0},
  {'S', 5, 0xFE, 1, 0},
  {'t', 5, 0xFD, 0, 0},
  {'T', 5, 0xFD, 1, 0},
  {'u', 5, 0xFB, 0, 0},
  {'U', 5, 0xFB, 1, 0},
  {'v', 5, 0xF7, 0, 0},
  {'V', 5, 0xF7, 1, 0},
  {'w', 5, 0xEF, 0, 0},
  {'W', 5, 0xEF, 1, 0},
  {'x', 5, 0xDF, 0, 0},
  {'X', 5, 0xDF, 1, 0},
  {'y', 5, 0xBF, 0, 0},
  {'Y', 5, 0xBF, 1, 0},
  {'z', 5, 0x7F, 0, 0},
  {'Z', 5, 0x7F, 1, 0},

  {KEY_SHIFT, 6, 0xFE, 0, 0},
  {KEY_CTRL, 6, 0xFD, 0, 0},
  {KEY_GRAPH, 6, 0xFB, 0, 0},
  {KEY_CAP, 6, 0xF7, 0, 0},
  {KEY_CODE, 6, 0xEF, 0, 0},
  {KEY_F1, 6, 0xDF, 0, 0},
  {KEY_F2, 6, 0xBF, 0, 0},
  {KEY_F3, 6, 0x7F, 0, 0},

  {KEY_F4, 7, 0xFE, 0, 0},
  {KEY_F5, 7, 0xFD, 0, 0},
  {KEY_ESC, 7, 0xFB, 0, 0},
  {'\t', 7, 0xF7, 0, 0},
  {KEY_STOP, 7, 0xEF, 0, 0},
  {KEY_BACK, 7, 0xDF, 0, 0},
  {KEY_SEL, 7, 0xBF, 0, 0},
  {KEY_ENTER, 7, 0x7F, 0, 0},

  {' ', 8, 0xFE, 0, 0},
  {KEY_HOME, 8, 0xFD, 0, 0},
  {KEY_INSERT, 8, 0xFB, 0, 0},
  {KEY_DEL, 8, 0xF7, 0, 0},
  {KEY_LEFT, 8, 0xEF, 0, 0},
  {KEY_UP, 8, 0xDF, 0, 0},
  {KEY_DOWN, 8, 0xBF, 0, 0},
  {KEY_RIGHT, 8, 0x7F, 0, 0},

  {0, 0, 0, 0, 0}
};

UInt32 msx_callback(ArmletCallbackArg *arg) {
  MsxGlobals *g;

  switch (arg->cmd) {
    case IO_VSYNC:
      g = hardware->globals;
      if (g->ti9918.intenable) {
        g->ti9918.interrupt = 1;
        ti9918_video(&g->ti9918);
      }
      msx_vsync(arg->a1);
      break;
    case IO_READB:
      return msx_readb(arg->a1);
    case IO_WRITEB:
      msx_writeb(arg->a1, arg->a2);
  }
  return 0;
}

UInt8 msx_readb(UInt16 a) {
  MsxGlobals *g;
  UInt32 key;
  UInt8 b;

  if (a == 0xFFFF)      // GAMBIARRA: CheckInput de console do CP/M
    return 1; 

  g = hardware->globals;

  switch (a & 0xFF) {
    case 0x98:	// VDP read RAM
      b = g->ti9918.vram[g->ti9918.readp];
      g->ti9918.readp = (g->ti9918.readp + 1) & 0x3FFF;
      return b;

    case 0x99:	// VDP status register
      return ti9918_status(&g->ti9918);

    case 0xA2:	// AY8910 value
      if (ay8910_getindex(&g->ay8910) == 0x0E) {
        // AY8910 port A
        // Bit 0: joystick Up       (0=Moved, 1=Not moved)
        // Bit 1: joystick Down     (0=Moved, 1=Not moved)
        // Bit 2: joystick Left     (0=Moved, 1=Not moved)
        // Bit 3: joystick Right    (0=Moved, 1=Not moved)
        // Bit 4: joystick button A (0=Pressed, 1=Not pressed)
        // Bit 5: joystick button B (0=Pressed, 1=Not pressed)

        b = 0xFF;

        if (g->joyselect == g->joystick) {
          if (hardware->joyy < 16)
            b &= 0xFE;
          else if (hardware->joyy > 48)
            b &= 0xFD;
          if (hardware->joyx < 16)
            b &= 0xFB;
          else if (hardware->joyx > 48)
            b &= 0xF7;

          if (hardware->button & g->button1)
            b &= 0xEF;
          if (hardware->button & g->button2)
            b &= 0xDF;
        }

        return b;
      }

      return ay8910_getvalue(&g->ay8910);

    case 0xA8:	// PPI port A (PSLOT)
      b = hardware->banksw[0] |
          (hardware->banksw[1] << 2) |
          (hardware->banksw[2] << 4) |
          (hardware->banksw[3] << 6);
      return b;

    case 0xA9:	// PPI port B
      key = hardware->key;
      b = 0xFF;

      if (key && g->kbselect <= 10) {

        if (g->keyMap[key].line == g->kbselect)
          b &= g->keyMap[key].column;

        if (g->keyMap[key].shift && (g->keyMap[KEY_SHIFT].line == g->kbselect))
          b &= g->keyMap[KEY_SHIFT].column;

        if (g->ctrl && (g->keyMap[KEY_CTRL].line == g->kbselect))
          b &= g->keyMap[KEY_CTRL].column;
      }

      return b;

    case 0xAA:	// PPI port C
      return g->kbselect;
  }

  return 0x00;
}

void msx_writeb(UInt16 a, UInt8 b) {
  MsxGlobals *g;
  UInt32 intenable;

  g = hardware->globals;

  switch (a & 0xFF) {
    case 0x98:	// VDP write RAM
      g->ti9918.vram[g->ti9918.writep] = b;
      g->ti9918.writep = (g->ti9918.writep + 1) & 0x3FFF;
      g->ti9918.dirty = 1;
      break;

    case 0x99:	// VDP set read/write pointer, register setup
      intenable = g->ti9918.intenable;
      ti9918_setb(&g->ti9918, b);
      if (intenable != g->ti9918.intenable)
        hardware->vsync_irq = g->ti9918.intenable ? 1 : 0;
      break;

    case 0xA0:	// AY8910 index
      ay8910_setindex(&g->ay8910, b);
      break;

    case 0xA1:	// AY8910 value
      if (ay8910_getindex(&g->ay8910) == 0x0F)
        // AY8910 port B
        // Bit 6: joystick select (0=Connector 1, 1=Connector 2)
        g->joyselect = (b & 0x40) >> 6;
      else
        ay8910_setvalue(&g->ay8910, b);
      break;

    case 0xA8:	// PPI port A (PSLOT)
      hardware->banksw[0] = b & 0x03;		// 0000-3FFF
      hardware->banksw[1] = (b >> 2) & 0x03;	// 4000-7FFF
      hardware->banksw[2] = (b >> 4) & 0x03;	// 8000-BFFF
      hardware->banksw[3] = (b >> 6) & 0x03;	// C000-FFFF
      break;

    case 0xAA:	// PPI port C
      // 0-3  KB0-3  Keyboard line               (0-8 on SV738 X'Press)
      // 4    CASON  Cassette motor relay        (0=On, 1=Off)
      // 5    CASW   Cassette audio out          (Pulse)
      // 6    CAPS   CAPS-LOCK lamp              (0=On, 1=Off)
      // 7    SOUND  Keyboard klick bit          (Pulse)

      g->kbselect = b & 0x0F;
      break;

    case 0xAB:	// PPI control register
      // 0    B      Set/reset the bit           (0=Reset, 1=Set)
      // 1-3  N0-N2  Bit number                  (0-7)
      // 4-6  0      Not used
      // 7    SF     Must be "0" for bit set/reset function.

      if (!(b & 0x80)) {	// port C set/reset function
        UInt32 bit = (b & 0x0E) >> 1;

        if (bit < 4) {
          if (b & 0x01)	// set
            g->kbselect |= 1 << bit;
          else		// reset
            g->kbselect &= (~(1 << bit)) & 0x0F;
        }
      }
  }
}

void msx_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  MsxGlobals *globals;
  KeyMap *keyMap, *machineKeyMap;
  UInt16 i, len;

  id = machine->id;
  hardware = _hardware;

  len = sizeof(prefs);
  MemSet(&prefs, len, 0);

  if (PrefGetAppPreferences(AppID, 128 + id, &prefs, &len, true) == noPreferenceFound) {
    prefs.joystick = 0;
    prefs.button1 = keyBitHard1;
    prefs.button2 = keyBitHard2;
    PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, len, true);
  }

  hardware->joyx = hardware->joyy = 32;
  hardware->key = 0;
  key = 0;

  if (!hardware->globals) {
    hardware->globals = MemPtrNew(sizeof(MsxGlobals));
    MemSet(hardware->globals, sizeof(MsxGlobals), 0);
  }
  globals = hardware->globals;

  globals->ctrl = 0;
  globals->kbselect = 0;
  globals->joyselect = 0;
  globals->joystick = prefs.joystick;
  globals->button1 = prefs.button1;
  globals->button2 = prefs.button2;

  keyMap = MemPtrNew(256 * sizeof(KeyMap));

  switch (machine->id) {
    case expert11:
      machineKeyMap = Expert11Map;
      break;
    case hotbit12:
      machineKeyMap = HotbitMap;
      break;
    default:
      machineKeyMap = NULL;
  }

  kbd_initmap(machineKeyMap, keyMap);

  for (i = 0; i < 256; i++) {
    globals->keyMap[i].key = (keyMap[i].key);
    globals->keyMap[i].line = (keyMap[i].line);
    globals->keyMap[i].column = (keyMap[i].column);
    globals->keyMap[i].shift = (keyMap[i].shift);
    globals->keyMap[i].ctrl = (keyMap[i].ctrl);
  }

  MemPtrFree(keyMap);

  ay8910_init(&globals->ay8910, hardware, AY_3_8910, 3579545L);
  ti9918_init(&globals->ti9918, hardware);

  hardware->banksw[0] = 0;	// BIOS  em 0000-3FFF
  hardware->banksw[1] = 0;	// BASIC em 4000-7FFF
  hardware->banksw[2] = 3;	// RAM   em 8000-BFFF
  hardware->banksw[3] = 3;	// RAM   em C000-FFFF
}

void msx_reset(void) {
  MsxGlobals *g;

  g = hardware->globals;
  ti9918_reset(&g->ti9918);
}

void msx_finish(void) {
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);

  if (hardware->globals) {
    MemPtrFree(hardware->globals);
    hardware->globals = NULL;
  }
}

void msx_osd(UInt32 delay) {
}

void msx_debug(void) {
  MsxGlobals *g = hardware->globals;

  DebugInit("ti9918.txt", true);
  InfoDialog(D_FILE, "mode=%ld\n", (g->ti9918.mode));
  InfoDialog(D_FILE, "fontmap=%ld\n", (g->ti9918.fontmap));
  InfoDialog(D_FILE, "screenmap=%ld\n", (g->ti9918.screenmap));
  InfoDialog(D_FILE, "colormap=%ld\n", (g->ti9918.colormap));
  InfoDialog(D_FILE, "attrmap=%ld\n", (g->ti9918.attrmap));
  InfoDialog(D_FILE, "objmap=%ld\n", (g->ti9918.objmap));
  InfoDialog(D_FILE, "fgcolor=%ld\n", (g->ti9918.fgcolor));
  InfoDialog(D_FILE, "bgcolor=%ld\n", (g->ti9918.bgcolor));
  DebugFinish();

  DebugInit("ti9918.bin", true);
  DebugBuffer(g->ti9918.vram, 0x4000);
  DebugFinish();
}

void msx_key(UInt16 c) {
  MsxGlobals *g;

  if (c == KEY_CTRL) {
    g = hardware->globals;
    g->ctrl = 1;
    return;
  }

  if (c == 0 && key) {
    g = hardware->globals;
    g->ctrl = 0;
  }

  key = c;
  hardware->key = key;
}

void msx_joystick(UInt16 x, UInt16 y) {
  hardware->joyx = x;
  hardware->joyy = y;
}

Err msx_readcart(FileRef f, z80_Regs *z80, Hardware *hardware) {
  UInt32 size, r;
  Err err;

  if ((err = VFSFileSize(f, &size)) != 0)
    return -1;

  if (size > 0x8000)
    return -2;

  if (hardware->bank3 == NULL) {
    if ((hardware->bank3 = sys_calloc(0x8000, 1)) == NULL)
      return -3;

    hardware->m3 = hardware->bank3;
  }

  if ((err = VFSFileRead(f, size, hardware->m3, &r)) != 0) {
    MemSet(hardware->m3, 0x8000, 0);
    return -4;
  }

  if (size != r) {
    MemSet(hardware->m3, 0x8000, 0);
    return -5;
  }

  if (hardware->m3[0] != 0x41 || hardware->m3[1] != 0x42) {
    MemSet(hardware->m3, 0x8000, 0);
    return -6;
  }

  return 0;
}

MsxPrefs *MsxGetPrefs(void) {
  return &prefs;
}

Boolean MsxFormHandler(EventPtr event, Boolean *close) {
  FormPtr frm;
  ListPtr lst;
  UInt32 button;
  Boolean handled;

  frm = FrmGetActiveForm();
  handled = false;
  *close = false;

  switch (event->eType) {
    case frmOpenEvent:
      if (prefs.joystick == 0)
        FrmSetControlValue(frm, FrmGetObjectIndex(frm, leftCtl), 1);
      else
        FrmSetControlValue(frm, FrmGetObjectIndex(frm, rightCtl), 1);

      switch (prefs.button1) {
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

      switch (prefs.button2) {
        case keyBitHard1: button = 0; break;
        case keyBitHard2: button = 1; break;
        case keyBitHard3: button = 2; break;
        case keyBitHard4: button = 3; break;
        //case keyBitJogPress: button = 4; break;
        //case keyBitRockerCenter: button = 5; break;
        default: button = 0;
      }

      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, button2List));
      LstSetSelection(lst, button);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
         FrmGetObjectIndex(frm, button2Ctl)),
         LstGetSelectionText(lst, button));

      FrmDrawForm(frm);
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case leftCtl:
          prefs.joystick = 0;
          handled = true;
          break;
        case rightCtl:
          prefs.joystick = 1;
          handled = true;
          break;
        case okBtn:
          *close = true;
          handled = true;
      }
      break;

    case popSelectEvent:
      switch (event->data.popSelect.listID) {
        case buttonList:
          switch (event->data.popSelect.selection) {
            case 0: prefs.button1 = keyBitHard1; break;
            case 1: prefs.button1 = keyBitHard2; break;
            case 2: prefs.button1 = keyBitHard3; break;
            case 3: prefs.button1 = keyBitHard4; break;
            //case 4: prefs.button1 = keyBitJogPress; break;
            //case 5: prefs.button1 = keyBitRockerCenter; break;
            default: prefs.button1 = keyBitHard1;
          }
          break;
        case button2List:
          switch (event->data.popSelect.selection) {
            case 0: prefs.button2 = keyBitHard1; break;
            case 1: prefs.button2 = keyBitHard2; break;
            case 2: prefs.button2 = keyBitHard3; break;
            case 3: prefs.button2 = keyBitHard4; break;
            //case 4: prefs.button2 = keyBitJogPress; break;
            //case 5: prefs.button2 = keyBitRockerCenter; break;
            default: prefs.button2 = keyBitHard1;
          }
      }

    default:
      break;
  }

  return handled;
}
