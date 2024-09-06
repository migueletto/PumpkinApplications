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
#include "endian.h"
#include "z80.h"
#include "m6845.h"
#include "cgenie.h"
#include "misc.h"
#include "gui.h"

#define VIDEO_SIZE	0x4000

#define CGENIE_FONT	'cgfn'
#define CGENIE_FONT_ID	1

static UInt8 id;
static CgeniePrefs prefs;
static Hardware *hardware;
static UInt16 key;
static MemHandle cgfn = NULL;

ColorType CgenieColor[22] = {
  { 0, 15*4, 15*4, 15*4},  // gray
  { 1,  0*4, 48*4, 48*4},  // cyan
  { 2, 60*4,  0*4,  0*4},  // red
  { 3, 47*4, 47*4, 47*4},  // white
  { 4, 55*4, 55*4,  0*4},  // yellow
  { 5,  0*4, 56*4,  0*4},  // green
  { 6, 42*4, 32*4,  0*4},  // orange
  { 7, 63*4, 63*4,  0*4},  // light yellow
  { 8,  0*4,  0*4, 48*4},  // blue
  { 9,  0*4, 24*4, 63*4},  // light blue
  {10, 60*4,  0*4, 38*4},  // pink
  {11, 38*4,  0*4, 60*4},  // purple
  {12, 31*4, 31*4, 31*4},  // light gray
  {13,  0*4, 63*4, 63*4},  // light cyan
  {14, 58*4,  0*4, 58*4},  // magenta
  {15, 63*4, 63*4, 63*4},  // bright white

  {16, 0, 0, 0},
  {17, 112, 0, 112},
  {18, 112, 40, 32},
  {19, 40, 112, 32},
  {20, 72, 72, 72},
  {-1, 0, 0, 0}
};

UInt16 CgenieControl[9] = {stopCmd, restartCmd, 0,
                         loadSnapCmd, loadDsk1Cmd, loadBinCmd,
                         0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef CgenieButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {KEY_UP,    internalFontID, "\x80",  C1, C2, C3},
  {KEY_DOWN,  internalFontID, "\x81", C1, C2, C3},
  {KEY_BREAK, internalFontID, "BRK",  C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_LEFT,  internalFontID, "\x82", C1, C2, C3},
  {KEY_RIGHT, internalFontID, "\x83", C1, C2, C3},
  {KEY_CTRL,  internalFontID, "CTL", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_BACK, internalFontID, "DEL", C1, C2, C3},
  {KEY_INSERT, internalFontID, "INS", C1, C2, C3},
  {KEY_END, internalFontID, "END", C1, C2, C3}
};

/*
   +-------------------------------+     +-------------------------------+
   | 0   1   2   3   4   5   6   7 |     | 0   1   2   3   4   5   6   7 |
+--+---+---+---+---+---+---+---+---+  +--+---+---+---+---+---+---+---+---+
|0 | @ | A | B | C | D | E | F | G |  |0 | ` | a | b | c | d | e | f | g |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|1 | H | I | J | K | L | M | N | O |  |1 | h | i | j | k | l | m | n | o |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|2 | P | Q | R | S | T | U | V | W |  |2 | p | q | r | s | t | u | v | w |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|3 | X | Y | Z | [ |F-1|F-2|F-3|F-4|  |3 | x | y | z | { |F-5|F-6|F-7|F-8|
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|4 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |  |4 | 0 | ! | " | # | $ | % | & | ' |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|5 | 8 | 9 | : | ; | , | - | . | / |  |5 | ( | ) | * | + | < | = | > | ? |
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|6 |ENT|CLR|BRK|UP |DN |LFT|RGT|SPC|  |6 |ENT|CLR|BRK|UP |DN |LFT|RGT|SPC|
|  +---+---+---+---+---+---+---+---+  |  +---+---+---+---+---+---+---+---+
|7 |SHF|ALT|PUP|PDN|INS|DEL|CTL|END|  |7 |SHF|ALT|PUP|PDN|INS|DEL|CTL|END|
+--+---+---+---+---+---+---+---+---+  +--+---+---+---+---+---+---+---+---+
*/

static KeyMap CgenieMap[] = {
  {'@', 0x01, 0x01, 0, 0},
  {'`', 0x01, 0x01, 1, 0},
  {'A', 0x01, 0x02, 0, 0},
  {'a', 0x01, 0x02, 1, 0},
  {'B', 0x01, 0x04, 0, 0},
  {'b', 0x01, 0x04, 1, 0},
  {'C', 0x01, 0x08, 0, 0},
  {'c', 0x01, 0x08, 1, 0},
  {'D', 0x01, 0x10, 0, 0},
  {'d', 0x01, 0x10, 1, 0},
  {'E', 0x01, 0x20, 0, 0},
  {'e', 0x01, 0x20, 1, 0},
  {'F', 0x01, 0x40, 0, 0},
  {'f', 0x01, 0x40, 1, 0},
  {'G', 0x01, 0x80, 0, 0},
  {'g', 0x01, 0x80, 1, 0},

  {'H', 0x02, 0x01, 0, 0},
  {'h', 0x02, 0x01, 1, 0},
  {'I', 0x02, 0x02, 0, 0},
  {'i', 0x02, 0x02, 1, 0},
  {'J', 0x02, 0x04, 0, 0},
  {'j', 0x02, 0x04, 1, 0},
  {'K', 0x02, 0x08, 0, 0},
  {'k', 0x02, 0x08, 1, 0},
  {'L', 0x02, 0x10, 0, 0},
  {'l', 0x02, 0x10, 1, 0},
  {'M', 0x02, 0x20, 0, 0},
  {'n', 0x02, 0x20, 1, 0},
  {'N', 0x02, 0x40, 0, 0},
  {'n', 0x02, 0x40, 1, 0},
  {'O', 0x02, 0x80, 0, 0},
  {'o', 0x02, 0x80, 1, 0},

  {'P', 0x04, 0x01, 0, 0},
  {'p', 0x04, 0x01, 1, 0},
  {'Q', 0x04, 0x02, 0, 0},
  {'q', 0x04, 0x02, 1, 0},
  {'R', 0x04, 0x04, 0, 0},
  {'r', 0x04, 0x04, 1, 0},
  {'S', 0x04, 0x08, 0, 0},
  {'s', 0x04, 0x08, 1, 0},
  {'T', 0x04, 0x10, 0, 0},
  {'t', 0x04, 0x10, 1, 0},
  {'U', 0x04, 0x20, 0, 0},
  {'u', 0x04, 0x20, 1, 0},
  {'V', 0x04, 0x40, 0, 0},
  {'v', 0x04, 0x40, 1, 0},
  {'W', 0x04, 0x80, 0, 0},
  {'w', 0x04, 0x80, 1, 0},

  {'X', 0x08, 0x01, 0, 0},
  {'x', 0x08, 0x01, 1, 0},
  {'Y', 0x08, 0x02, 0, 0},
  {'y', 0x08, 0x02, 1, 0},
  {'Z', 0x08, 0x04, 0, 0},
  {'z', 0x08, 0x04, 1, 0},
  {'[', 0x08, 0x08, 0, 0},
  {'{', 0x08, 0x08, 1, 0},
  {KEY_F1, 0x08, 0x10, 0, 0},
  {KEY_F5, 0x08, 0x10, 1, 0},
  {KEY_F2, 0x08, 0x20, 0, 0},
  {KEY_F6, 0x08, 0x20, 1, 0},
  {KEY_F3, 0x08, 0x40, 0, 0},
  {KEY_F7, 0x08, 0x40, 1, 0},
  {KEY_F4, 0x08, 0x80, 0, 0},
  {KEY_F8, 0x08, 0x80, 1, 0},

  {'0', 0x10, 0x01, 0, 0},
  {'1', 0x10, 0x02, 0, 0},
  {'!', 0x10, 0x02, 1, 0},
  {'2', 0x10, 0x04, 0, 0},
  {'"', 0x10, 0x04, 1, 0},
  {'3', 0x10, 0x08, 0, 0},
  {'#', 0x10, 0x08, 1, 0},
  {'4', 0x10, 0x10, 0, 0},
  {'$', 0x10, 0x10, 1, 0},
  {'5', 0x10, 0x20, 0, 0},
  {'%', 0x10, 0x20, 1, 0},
  {'6', 0x10, 0x40, 0, 0},
  {'&', 0x10, 0x40, 1, 0},
  {'7', 0x10, 0x80, 0, 0},
  {'\'', 0x10, 0x80, 1, 0},

  {'8', 0x20, 0x01, 0, 0},
  {'(', 0x20, 0x01, 1, 0},
  {'9', 0x20, 0x02, 0, 0},
  {')', 0x20, 0x02, 1, 0},
  {':', 0x20, 0x04, 0, 0},
  {'*', 0x20, 0x04, 1, 0},
  {';', 0x20, 0x08, 0, 0},
  {'+', 0x20, 0x08, 1, 0},
  {',', 0x20, 0x10, 0, 0},
  {'<', 0x20, 0x10, 1, 0},
  {'-', 0x20, 0x20, 0, 0},
  {'=', 0x20, 0x20, 1, 0},
  {'.', 0x20, 0x40, 0, 0},
  {'>', 0x20, 0x40, 1, 0},
  {'/', 0x20, 0x80, 0, 0},
  {'?', 0x20, 0x80, 1, 0},

  {KEY_ENTER, 0x40, 0x01, 0, 0},
  {KEY_CLEAR, 0x40, 0x02, 0, 0},
  {KEY_BREAK, 0x40, 0x04, 0, 0},
  {KEY_UP, 0x40, 0x08, 0, 0},
  {KEY_DOWN, 0x40, 0x10, 0, 0},
  {KEY_LEFT, 0x40, 0x20, 0, 0},
  {KEY_RIGHT, 0x40, 0x40, 0, 0},
  {' ', 0x40, 0x80, 0, 0},

  {KEY_SHIFT, 0x80, 0x01, 0, 0},
  {KEY_BACK, 0x80, 0x20, 0, 0},
  {KEY_CTRL, 0x80, 0x40, 0, 0},
  {KEY_END, 0x80, 0x80, 0, 0},

  {0, 0, 0, 0, 0}
};

UInt32 cgenie_callback(ArmletCallbackArg *arg) {
  CgenieGlobals *g;

  switch (arg->cmd) {
    case IO_VSYNC:
      snd_calcnoise();
      g = hardware->globals;
      m6845_video(&g->m6845, hardware);
      cgenie_vsync(arg->a1);
      return 0;
    case IO_READB:
      return cgenie_readb(arg->a1);
    case IO_WRITEB:
      cgenie_writeb(arg->a1, arg->a2);
      return 0;
  }
  return 0;
}

UInt8 cgenie_readb(UInt16 a) {
  CgenieGlobals *g;

  if (a == 0xFFFF)      // GAMBIARRA: CheckInput de console do CP/M
    return 1; 

  g = hardware->globals;

  if (a >= 0xF800 && a < 0xF900) {	// portas artificiais (teclado)
    UInt32 key = hardware->key;
    UInt8 b = 0x00;

    if (key) {
      UInt32 kbselect = a & 0xFF;

      if (g->keyMap[key].line & kbselect)
        b |= g->keyMap[key].column;

      if (g->keyMap[key].shift && (g->keyMap[KEY_SHIFT].line & kbselect))
        b |= g->keyMap[KEY_SHIFT].column;

      if (g->ctrl && (g->keyMap[KEY_CTRL].line & kbselect))
        b |= g->keyMap[KEY_CTRL].column;
    }

    return b;
  }

  switch (a & 0xFF) {
    case 0xF8:	// AY8910 control
      return ay8910_getindex(&g->ay8910);
    case 0xF9:	// AY8910 data
      return ay8910_getvalue(&g->ay8910);
    case 0xFA:
      return m6845_getindex(&g->m6845);
    case 0xFB:
      return m6845_getvalue(&g->m6845);
    case 0xFF:
      return g->port_ff;
  }

  return 0x00;
}

void cgenie_writeb(UInt16 a, UInt8 b) {
  CgenieGlobals *g;
  UInt8 bg;

  g = hardware->globals;

  switch (a & 0xFF) {
    case 0xF8:	// AY8910 control
      ay8910_setindex(&g->ay8910, b);
      break;
    case 0xF9:	// AY8910 data
      ay8910_setvalue(&g->ay8910, b);
      break;
    case 0xFA:
      m6845_setindex(&g->m6845, b);
      break;
    case 0xFB:
      m6845_setvalue(&g->m6845, b);
      break;
    case 0xFF:
      // CAS  0x01  tape output signal
      // BGD0 0x04  background color enable
      // CHR1 0x08  charset 0xc0 - 0xff 1:fixed 0:defined
      // CHR0 0x10  charset 0x80 - 0xbf 1:fixed 0:defined
      // FGR  0x20  1: graphic mode, 0: text mode
      // BGD1 0x40  background color select 1
      // BGD2 0x80  background color select 2

      if ((g->port_ff & 0x08) != (b & 0x08))
        m6845_setfs(&g->m6845, 1, b & 0x08 ? 1 : 0);

      if ((g->port_ff & 0x10) != (b & 0x10))
        m6845_setfs(&g->m6845, 0, b & 0x10 ? 1 : 0);

      if ((g->port_ff & 0x20) != (b & 0x20))
        m6845_setmode(&g->m6845, b & 0x20 ? 1 : 0);

      if ((g->port_ff & 0xC4) != (b & 0xC4)) {
        if (b & 0x04)
          bg = 17;
        else switch (b & 0xC0) {
          case 0x40: bg = 18; break;
          case 0x80: bg = 19; break;
          case 0xC0: bg = 20; break;
          default: bg = 16;
        }
        m6845_setbg(&g->m6845, bg);
      }

      g->port_ff = b;
  }
}

void cgenie_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  CgenieGlobals *globals;
  KeyMap *keyMap;
  UInt16 i, len;
  UInt8 *font;

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

  if (!hardware->globals) {
    hardware->globals = MemPtrNew(sizeof(CgenieGlobals));
    MemSet(hardware->globals, sizeof(CgenieGlobals), 0);
  }
  globals = hardware->globals;

  globals->init = 1;
  globals->ctrl = 0;
  globals->port_ff = 0x18;

  cgfn = DmGet1Resource(CGENIE_FONT, CGENIE_FONT_ID);
  font = MemHandleLock(cgfn);

  m6845_init(&globals->m6845, &hardware->m1[0x4000], &hardware->m0[0xF000 & 0x7FFF], font);

  keyMap = MemPtrNew(256 * sizeof(KeyMap));
  kbd_initmap(CgenieMap, keyMap);

  for (i = 0; i < 256; i++) {
    globals->keyMap[i].key = (keyMap[i].key);
    globals->keyMap[i].line = (keyMap[i].line);
    globals->keyMap[i].column = (keyMap[i].column);
    globals->keyMap[i].shift = (keyMap[i].shift);
    globals->keyMap[i].ctrl = (keyMap[i].ctrl);
  }
  MemPtrFree(keyMap);

  ay8910_init(&globals->ay8910, hardware, AY_3_8912, 1000000L);

  hardware->ramsize = (ramsize * 4096 - 1);
}

void cgenie_init(void) {
  hardware->vsync_irq = 0;
  MemSet(&hardware->m0[0xF400 & 0x7FFF], 0x400, 0xFF);
}

void cgenie_finish(void) {
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);

  if (cgfn) {
    MemHandleUnlock(cgfn);
    DmReleaseResource(cgfn);
    cgfn = NULL;
  }

  if (hardware->globals) {
    MemPtrFree(hardware->globals);
    hardware->globals = NULL;
  }
}

void cgenie_key(UInt16 c) {
  CgenieGlobals *g;

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

void cgenie_joystick(UInt16 x, UInt16 y) {
  hardware->joyx = x;
  hardware->joyy = y;
}

CgeniePrefs *CgenieGetPrefs(void) {
  return &prefs;
}

/*
Boolean CgenieFormHandler(EventPtr event, Boolean *close)
{
  FormPtr frm;
  Boolean handled;

  frm = FrmGetActiveForm();
  handled = false;
  *close = false;

  switch (event->eType) {
    case frmOpenEvent:
      FrmDrawForm(frm);
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case okBtn:
          *close = true;
          handled = true;
      }
  }

  return handled;
}
*/
