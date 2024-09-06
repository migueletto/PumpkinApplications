#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "machine.h"
#include "kbd.h"
#include "cas.h"
#include "mc10.h"
#include "misc.h"
#include "endian.h"
#include "gui.h"

static UInt8 id;
static Mc10Prefs prefs;
static Hardware *hardware;
static Boolean ctrl, shift;
static UInt16 key, joyx, joyy;
static UInt8 kbcolumn;
static KeyMap keyMap[256];
static UInt8 vdg;

UInt16 MC10Control[9] = {stopCmd, restartCmd, configCmd,
                         loadCasCmd, 0, 0,
                         0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef MC10ButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {KEY_UP,      internalFontID, "\x80",  C1, C2, C3},
  {KEY_DOWN,    internalFontID, "\x81", C1, C2, C3},
  {KEY_BREAK,   internalFontID, "BRK", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_BACK,    internalFontID, "\x82", C1, C2, C3},
  {KEY_RIGHT,   internalFontID, "\x83", C1, C2, C3},
  {KEY_CTRL,    internalFontID, "CTL", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {KEY_RESET,   internalFontID, "RST", C1, C2, C3},
  {KEY_INVERSE, internalFontID, "INV", C1, C2, C3}
};

static KeyMap MC10Map[69] = {
  {'@', 0x01, 0x01, 0, 0},
  {'A', 0x01, 0x02, 0, 0},
  {'B', 0x01, 0x04, 0, 0},
  {'C', 0x01, 0x08, 0, 0},
  {'D', 0x01, 0x10, 0, 0},
  {'E', 0x01, 0x20, 0, 0},
  {'F', 0x01, 0x40, 0, 0},
  {'G', 0x01, 0x80, 0, 0},
  {KEY_BACK, 0x01, 0x02, 0, 1},
  {'H', 0x02, 0x01, 0, 0},
  {'I', 0x02, 0x02, 0, 0},
  {'J', 0x02, 0x04, 0, 0},
  {'K', 0x02, 0x08, 0, 0},
  {'L', 0x02, 0x10, 0, 0},
  {'M', 0x02, 0x20, 0, 0},
  {'N', 0x02, 0x40, 0, 0},
  {'O', 0x02, 0x80, 0, 0},
  {'P', 0x04, 0x01, 0, 0},
  {'Q', 0x04, 0x02, 0, 0},
  {'R', 0x04, 0x04, 0, 0},
  {'S', 0x04, 0x08, 0, 0},
  {'T', 0x04, 0x10, 0, 0},
  {'U', 0x04, 0x20, 0, 0},
  {'V', 0x04, 0x40, 0, 0},
  {'W', 0x04, 0x80, 0, 0},
  {KEY_RIGHT, 0x04, 0x08, 0, 1},
  {KEY_UP,    0x04, 0x80, 0, 1},
  {'X', 0x08, 0x01, 0, 0},
  {'Y', 0x08, 0x02, 0, 0},
  {'Z', 0x08, 0x04, 0, 0},
  {' ', 0x08, 0x08, 0, 0},
  {KEY_ENTER, 0x08, 0x40, 0, 0},
  {KEY_DOWN,  0x08, 0x04, 0, 1},
  {'0', 0x10, 0x01, 0, 0},
  {'1', 0x10, 0x02, 0, 0},
  {'2', 0x10, 0x04, 0, 0},
  {'3', 0x10, 0x08, 0, 0},
  {'4', 0x10, 0x10, 0, 0},
  {'5', 0x10, 0x20, 0, 0},
  {'6', 0x10, 0x40, 0, 0},
  {'7', 0x10, 0x80, 0, 0},
  {'!', 0x10, 0x02, 1, 0},
  {'"', 0x10, 0x04, 1, 0},
  {'#', 0x10, 0x08, 1, 0},
  {'$', 0x10, 0x10, 1, 0},
  {'%', 0x10, 0x20, 1, 0},
  {'&', 0x10, 0x40, 1, 0},
  {'\'',0x10, 0x80, 1, 0},  // nao funciona
  {KEY_INVERSE, 0x10, 0x01, 1, 0},
  {'8', 0x20, 0x01, 0, 0},
  {'9', 0x20, 0x02, 0, 0},
  {':', 0x20, 0x04, 0, 0},
  {';', 0x20, 0x08, 0, 0},
  {',', 0x20, 0x10, 0, 0},
  {'-', 0x20, 0x20, 0, 0},
  {'.', 0x20, 0x40, 0, 0},
  {'/', 0x20, 0x80, 0, 0},
  {'(', 0x20, 0x01, 1, 0},
  {')', 0x20, 0x02, 1, 0},
  {'*', 0x20, 0x04, 1, 0},
  {'+', 0x20, 0x08, 1, 0},
  {'<', 0x20, 0x10, 1, 0},
  {'=', 0x20, 0x20, 1, 0},
  {'>', 0x20, 0x40, 1, 0},
  {'?', 0x20, 0x80, 1, 0},   // nao funciona
  {KEY_CTRL,  0x40, 0x01, 0, 0},
  {KEY_BREAK, 0x40, 0x04, 0, 0},
  {KEY_SHIFT, 0x40, 0x80, 0, 0},
  {0, 0, 0, 0, 0}
};

UInt32 mc10_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_VSYNC:
      mc10_vsync(arg->a1);
      return 0;
    case IO_READB:
      return mc10_readb(arg->a1);
    case IO_WRITEB:
      mc10_writeb(arg->a1, arg->a2);
      return 0;
  }
  return 0;
}

void mc10_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  UInt16 len;

  id = machine->id;
  hardware = _hardware;

  len = sizeof(prefs);
  MemSet(&prefs, len, 0);

  if (PrefGetAppPreferences(AppID, 128 + id, &prefs, &len, true) ==
         noPreferenceFound) {
    prefs.joystick = 0;
    prefs.button = keyBitHard1;
    prefs.artifacting = 0;
    PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, len, true);
  }

  if (ramsize == RAM_4K)
    hardware->ramsize = RAM_4K * 4096 - 1;
  else
    hardware->ramsize = (RAM_4K|RAM_16K) * 4096 - 1;

  kbd_initmap(MC10Map, keyMap);

  ctrl = shift = false;
  joyx = joyy = 32;
  key = 0;
  vdg = 0;
}

void mc10_init(void) {
  io_setgp(0x4000 - 0x3000, 0x4200 - 0x3000); // ajuste para caber na ram
  io_vdg(0);
  io_sam(0);
  hardware->vsync_irq = 0;
}

void mc10_finish(void) {
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);
}

void mc10_key(UInt16 c) {
  if (c >= 'a' && c <= 'z')
    c &= 0xDF;
  else if (c == KEY_CTRL)
    ctrl = ~ctrl;

  if (keyMap[c].ctrl)
    ctrl = true;

  if (keyMap[c].shift)
    shift = true;

  key = c;
}

void mc10_joystick(UInt16 x, UInt16 y) {
  joyx = x;
  joyy = y;
}

UInt8 mc10_readb(UInt16 a) {
  UInt8 b;

  switch (a & 0xFF) {
    case 0x81:	// M6803_PORT1
      return kbcolumn;

    case 0x82:	// M6803_PORT2
      //   BIT 1 KEYBOARD SHIFT/CONTROL KEYS INPUT
      // ! BIT 2 PRINTER OTS INPUT
      // ! BIT 3 RS232 INPUT DATA
      //   BIT 4 CASSETTE TAPE INPUT

      b = 0xEF;

      if (ctrl && (keyMap[KEY_CTRL].column | kbcolumn) == 0xFF)
        b &= 0xED, ctrl = false;
      else if (shift && (keyMap[KEY_SHIFT].column | kbcolumn) == 0xFF)
        b &= 0xED, shift = false;
      else if (key == KEY_BREAK && (keyMap[key].column | kbcolumn) == 0xFF)
        b &= 0xED;

      if (cas_input())
        b |= 0x10;

      return b;

    case 0xFF:	// 0xBFFF
      b = 0xC0;

      if (key && (keyMap[key].column | kbcolumn) == 0xFF &&
          keyMap[key].line < 0x40)
        b |= keyMap[key].line;

      return b;
  }

  return 0;
}

void mc10_writeb(UInt16 a, UInt8 b) {
  UInt8 b1;

  switch (a & 0xFF) {
    case 0x81:	// M6803_PORT1
      kbcolumn = b;
      break;

    case 0x82:	// M6803_PORT2
      // BIT 0 PRINTER OUTPUT & CASS OUTPUT
      break;

    case 0xFF:	// 0xBFFF
      // 0x80 BIT 7 SOUND OUTPUT BIT
      // 0x40 BIT 6 CSS 6847 CONTROL
      // 0x20 BIT 5 A/G 684? CONTROL
      // 0x10 BIT 4 GM0 6847 CONTROL
      // 0x08 BIT 3 GM1 6847 CONTROL
      // 0x04 BIT 2 GM2 6847 CONTROL & INT/EXT CONTROL

      io_snd1bit(b & 0x80);

      b1 = 0;
      if (b & 0x40) b1 |= 0x08;
      if (b & 0x20) b1 |= 0x80;
      if (b & 0x10) b1 |= 0x10;
      if (b & 0x08) b1 |= 0x20;
      if (b & 0x04) b1 |= 0x40;

      if (b1 != vdg) {
        vdg = b1;
        io_vdg(vdg);

        switch (vdg & 0xF0) {
          case 0x80: b1 = 1; break;
          case 0x90: b1 = 1; break;
          case 0xA0: b1 = 2; break;
          case 0xB0: b1 = 3; break;
          case 0xC0: b1 = 4; break;
          case 0xD0: b1 = 5; break;
          case 0xE0: b1 = 6; break;
          case 0xF0: b1 = 6; break;
          default:   b1 = 0;
        }
        io_sam(b1);
      }
  }
}

Mc10Prefs *Mc10GetPrefs(void) {
  return &prefs;
}

Boolean Mc10FormHandler(EventPtr event, Boolean *close) {
  FormPtr frm;
  Boolean handled;

  frm = FrmGetActiveForm();
  handled = false;
  *close = false;

  switch (event->eType) {
    case frmOpenEvent:
      switch (prefs.artifacting) {
        case 0: FrmSetControlValue(frm, FrmGetObjectIndex(frm, noneCtl), 1);
                break;
        case 1: FrmSetControlValue(frm, FrmGetObjectIndex(frm, blueCtl), 1);
                break;
        case 2: FrmSetControlValue(frm, FrmGetObjectIndex(frm, redCtl), 1);
      }

      FrmDrawForm(frm);
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case noneCtl:
          prefs.artifacting = 0;
          io_setvdg();
          break;
        case blueCtl:
          prefs.artifacting = 1;
          io_setvdg();
          break;
        case redCtl:
          prefs.artifacting = 2;
          io_setvdg();
          break;
        case okBtn:
          *close = true;
          handled = true;
      }

    default:
      break;
  }

  return handled;
}
