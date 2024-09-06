#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "palm.h"
#include "machine.h"
#include "kbd.h"
#include "ay8910.h"
#include "mc1000.h"
#include "endian.h"
#include "z80.h"
#include "misc.h"
#include "gui.h"

static UInt8 id;
static Mc1000Prefs prefs;
static Hardware *hardware;
static UInt8 key, vdg;
static UInt8 *buffer;

UInt16 MC1000Control[9] = {stopCmd, restartCmd, configCmd,
                           loadCasCmd, 0, 0,
                           0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef MC1000ButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {'^', internalFontID, "\x80", C1, C2, C3},
  {KEY_CTRL, internalFontID, "CTL", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {KEY_RESET, internalFontID, "RST", C1, C2, C3},
  {KEY_SHIFT, internalFontID, "SFT", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1}
};

static KeyMap MC1000Map[63] = {
  {'@', 0x01, 0xFE, 0, 0},
  {'H', 0x01, 0xFD, 0, 0},
  {'P', 0x01, 0xFB, 0, 0},
  {'X', 0x01, 0xF7, 0, 0},
  {'0', 0x01, 0xEF, 0, 0},
  {'8', 0x01, 0xDF, 0, 0},
  {'(', 0x01, 0xDF, 1, 0},
  {'A', 0x02, 0xFE, 0, 0},
  {'I', 0x02, 0xFD, 0, 0},
  {'Q', 0x02, 0xFB, 0, 0},
  {'Y', 0x02, 0xF7, 0, 0},
  {'1', 0x02, 0xEF, 0, 0},
  {'!', 0x02, 0xEF, 1, 0},
  {'9', 0x02, 0xDF, 0, 0},
  {')', 0x02, 0xDF, 1, 0},
  {'B', 0x04, 0xFE, 0, 0},
  {'J', 0x04, 0xFD, 0, 0},
  {'R', 0x04, 0xFB, 0, 0},
  {'Z', 0x04, 0xF7, 0, 0},
  {'2', 0x04, 0xEF, 0, 0},
  {'"', 0x04, 0xEF, 1, 0},
  {':', 0x04, 0xDF, 0, 0},
  {'*', 0x04, 0xDF, 1, 0},
  {'C', 0x08, 0xFE, 0, 0},
  {'K', 0x08, 0xFD, 0, 0},
  {'S', 0x08, 0xFB, 0, 0},
  {KEY_ENTER, 0x08, 0xF7, 0, 0},
  {'3', 0x08, 0xEF, 0, 0},
  {'#', 0x08, 0xEF, 1, 0},
  {';', 0x08, 0xDF, 0, 0},
  {'+', 0x08, 0xDF, 1, 0},
  {'D', 0x10, 0xFE, 0, 0},
  {'L', 0x10, 0xFD, 0, 0},
  {'T', 0x10, 0xFB, 0, 0},
  {' ', 0x10, 0xF7, 0, 0},
  {'4', 0x10, 0xEF, 0, 0},
  {'$', 0x10, 0xEF, 1, 0},
  {',', 0x10, 0xDF, 0, 0},
  {'<', 0x10, 0xDF, 1, 0},
  {'E', 0x20, 0xFE, 0, 0},
  {'M', 0x20, 0xFD, 0, 0},
  {'U', 0x20, 0xFB, 0, 0},
  {KEY_BACK, 0x20, 0xF7, 0, 0},
  {'5', 0x20, 0xEF, 0, 0},
  {'%', 0x20, 0xEF, 1, 0},
  {'-', 0x20, 0xDF, 0, 0},
  {'=', 0x20, 0xDF, 1, 0},
  {'F', 0x40, 0xFE, 0, 0},
  {'N', 0x40, 0xFD, 0, 0},
  {'V', 0x40, 0xFB, 0, 0},
  {'^', 0x40, 0xF7, 0, 0},
  {'6', 0x40, 0xEF, 0, 0},
  {'&', 0x40, 0xEF, 1, 0},
  {'.', 0x40, 0xDF, 0, 0},
  {'>', 0x40, 0xDF, 1, 0},
  {'G', 0x80, 0xFE, 0, 0},
  {'O', 0x80, 0xFD, 0, 0},
  {'W', 0x80, 0xFB, 0, 0},
  {'7', 0x80, 0xEF, 0, 0},
  {'\'', 0x80, 0xEF, 1, 0},
  {'/', 0x80, 0xDF, 0, 0},
  {'?', 0x80, 0xDF, 1, 0},
  {0, 0, 0, 0, 0}
};

UInt32 mc1000_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_VSYNC:
      mc1000_vsync(arg->a1);
      return 0;
    case IO_READB:
      return mc1000_readb(arg->a1);
    case IO_WRITEB:
      mc1000_writeb(arg->a1, arg->a2);
      return 0;
  }
  return 0;
}

void mc1000_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  Mc1000Globals *globals;
  KeyMap *keyMap;
  UInt16 i, len;

  id = machine->id;
  hardware = _hardware;

  len = sizeof(prefs);
  MemSet(&prefs, len, 0);

  if (PrefGetAppPreferences(AppID, 128 + id, &prefs, &len, true) ==
         noPreferenceFound) {
    prefs.joystick = 0;
    prefs.button = keyBitHard1;
    PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, len, true);
  }

  hardware->joyx = hardware->joyy = 32;
  hardware->key = 0;
  key = 0;
  vdg = 0;

  if (!hardware->globals) {
    hardware->globals = MemPtrNew(sizeof(Mc1000Globals));
    MemSet(hardware->globals, sizeof(Mc1000Globals), 0);
  }
  globals = hardware->globals;

  globals->shift = 0;
  globals->ctrl = 0;
  globals->kbline = 0;
  globals->bit = 1;
  globals->joystick = prefs.joystick;
  globals->button = prefs.button;

  buffer = MemPtrNew(CSW_BUFSIZE);
  globals->first = 1;
  globals->buffer = buffer;

  keyMap = MemPtrNew(256 * sizeof(KeyMap));

  kbd_initmap(MC1000Map, keyMap);

  for (i = 0; i < 256; i++) {
    globals->keyMap[i].key = (keyMap[i].key);
    globals->keyMap[i].line = (keyMap[i].line);
    globals->keyMap[i].column = (keyMap[i].column);
    globals->keyMap[i].shift = (keyMap[i].shift);
    globals->keyMap[i].ctrl = (keyMap[i].ctrl);
  }

  MemPtrFree(keyMap);

  ay8910_init(&globals->ay8910, hardware, AY_3_8910, 3540000L);

  // banksw[0]: tamanho da memoria RAM
  // 0: 16K
  // 1: 48K

  // banksw[1]: controla acesso a RAM de micros com 48K
  // 0: acessa VRAM de 8000 a 97FF
  // 1: acessa RAM  de 8000 a BFFF

  hardware->banksw[0] = ramsize == RAM_16K ? 0 : 1;
  hardware->banksw[1] = 0;

  hardware->globals = hardware->globals;
}

void mc1000_init(void) {
  io_setgp(0x8000, 0x9800);
  io_vdg(0);
  io_sam(0);
}

void mc1000_reset(void) {
  z80_Regs *z80;
  Mc1000Globals *globals;

  // No MC1000 o reset do Z80 aponta o PC para 0xC000
  z80 = hardware->cpu;
  z80->pc = 0xC000;
  hardware->banksw[1] = 0;

  MemSet(hardware->m0, 0x8000, 0);
  MemSet(hardware->m1, 0x4000, 0);
  MemSet(hardware->m2, 0x8000, 0);

  globals = hardware->globals;
  globals->first = 1;
  hardware->tape = 0;
}

void mc1000_finish(void) {
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);

  if (buffer);
    MemPtrFree(buffer);
  buffer = NULL;

  if (hardware->globals) {
    MemPtrFree(hardware->globals);
    hardware->globals = NULL;
  }
}

void mc1000_key(UInt16 c) {
  Mc1000Globals *g;

  if (c >= 'a' && c <= 'z')
    c &= 0xDF;
  else if (key == KEY_CTRL) {
    g = hardware->globals;
    g->ctrl = ~g->ctrl;
    c = 0;
  } else if (key == KEY_SHIFT) {
    g = hardware->globals;
    g->shift = ~g->shift;
    c = 0;
  } else if (c == 0) {
    g = hardware->globals;
    g->ctrl = g->shift = 0;
  }

  key = c;
  hardware->key = key ? key : 0;
}

void mc1000_joystick(UInt16 x, UInt16 y) {
  hardware->joyx = x;
  hardware->joyy = y;
}

static UInt32 csw_get(Hardware *hardware, UInt8 *b) {
  Mc1000Globals *g;
  UInt32 r;

  g = hardware->globals;

  if (g->index == g->size) {
    if (hardware->tape == 0)
      return 0;

    r = cas_buffer(CSW_BUFSIZE, g->buffer);
    if (r == 0) {
      g->index = g->size = 0;
      return 0;
    }

    g->index = 0;
    g->size = r;
  }

  *b = g->buffer[g->index++];
  return 1;
}

static UInt8 csw_input(Hardware *hardware) {
  Mc1000Globals *g;
  UInt8 b;
  UInt16 w;

  g = hardware->globals;

  if (g->first) {
    g->len = 0;
    g->first = 0;

    g->size = 0;
    g->index = 0;

    if (csw_get(hardware, &b) == 0)
      return 1;

    hardware->eventcount = 0;

    // invertido porque logo abaixo inverte de novo
    g->value = b > 128 ? 1 : 0;
  }

  if (g->len == 0) {
    g->value = g->value ? 0 : 1;

    if (csw_get(hardware, &b) == 0)
      return 1;

    if (b > 0)
      g->len = b;
    else {
      if (csw_get(hardware, &b) == 0)
        return 1;
      w = b;
      if (csw_get(hardware, &b) == 0)
        return 1;
      w |= (((UInt16)b) << 8);
      g->len = w;
    }
  }

  // 3572100 / 22050 = 162

  if (hardware->eventcount >= 162) {
    if (hardware->eventcount >= 324)
      hardware->eventcount = 0;
    else
      hardware->eventcount -= 162;

    g->len--;
  }

  return g->value;
}

UInt8 mc1000_readb(UInt16 a) {
  Mc1000Globals *g;
  UInt32 key;
  UInt8 b = 0xFF;

  if (a == 0xFFFF) {
    return 1;
  }

  switch (a & 0xFF) {
    case 0x40:	// readRegister
      g = hardware->globals;

      if (ay8910_getindex(&g->ay8910) == 0x0F) {

        if (hardware->button & g->button)
          b &= g->keyMap[g->joystick == 0 ? 'A' : '@'].column;

        if (hardware->joyx < 16)	// left
          b &= g->keyMap[g->joystick == 0 ? 'Y' : 'X'].column;
        else if (hardware->joyx > 48)	// right
          b &= g->keyMap[g->joystick == 0 ? '1' : '0'].column;

        if (hardware->joyy < 16)	// up
          b &= g->keyMap[g->joystick == 0 ? 'I' : 'H'].column;
        else if (hardware->joyy > 48)	// down
          b &= g->keyMap[g->joystick == 0 ? 'Q' : 'P'].column;

        key = hardware->key;

        if (key) {
          if (g->keyMap[key].line & g->kbline)
            b &= g->keyMap[key].column;

          if (g->keyMap[key].shift)
            b &= 0xBF;

          if (g->keyMap[key].ctrl)
            b &= 0x7F;
        }

        if (g->shift)
          b &= 0xBF;

        if (g->ctrl)
          b &= 0x7F;

        if (csw_input(hardware) == 0)
          b &= 0x7F;

        return b;
      }

      return ay8910_getvalue(&g->ay8910);
  }

  return b;
}

static void mc1000_writeb_internal(UInt16 a, UInt8 b) {
  UInt8 b1;

  switch (a & 0xFF) {
    case 0x80:
      b1 = 0;

      // bits de b:
      // TEXT: 0XXX XXXX
      // GR  : 1000 10XX
      // HGR : 1001 11XX

      b1 = ((b & 0x1C) << 2) |	/* GM0,GM1,GM2 */
           ((b & 0x02) << 2) |	/* CSS */
           (b & 0x80);		      /* A/G */

      if (b1 != vdg) {
        vdg = b1;
        io_vdg(vdg);

        if (b & 0x80) {
          switch (vdg & 0x70) {
            case 0x00: b1 = 1; break;
            case 0x10: b1 = 1; break;
            case 0x20: b1 = 2; break;
            case 0x30: b1 = 3; break;
            case 0x40: b1 = 4; break;
            case 0x50: b1 = 5; break;
            case 0x60: b1 = 6; break;
            case 0x70: b1 = 6; break;
            default  : b1 = 1;
          }
          io_sam(b1);
        } else
          io_sam(0);
      }
  }
}

void mc1000_writeb(UInt16 a, UInt8 b) {
  Mc1000Globals *g;

  switch (a & 0xFF) {
    case 0x20:  // setRegister
      g = hardware->globals;
      ay8910_setindex(&g->ay8910, b);
      return;

    case 0x60:  // writeRegister
      g = hardware->globals;

      if (ay8910_getindex(&g->ay8910) == 0x0E) {
        g->kbline = (b ^ 0xFF) & 0xFF;
      } else {
        ay8910_setvalue(&g->ay8910, b);
      }
      return;

    case 0x80:
      if (hardware->banksw[0]) {
        hardware->banksw[1] = b & 0x01;
      }
      mc1000_writeb_internal(a, b);
      break;

    default:
      return;
  }
}

Mc1000Prefs *Mc1000GetPrefs(void)
{
  return &prefs;
}

Boolean Mc1000FormHandler(EventPtr event, Boolean *close)
{
  FormPtr frm;
  ListPtr lst;
  Mc1000Prefs *prefs;
  UInt32 button;
  Boolean handled;

  frm = FrmGetActiveForm();
  prefs = Mc1000GetPrefs();
  handled = false;
  *close = false;

  switch (event->eType) {
    case frmOpenEvent:
      if (prefs->joystick == 0)
        FrmSetControlValue(frm, FrmGetObjectIndex(frm, leftCtl), 1);
      else
        FrmSetControlValue(frm, FrmGetObjectIndex(frm, rightCtl), 1);

      switch (prefs->button) {
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
        case leftCtl:
          prefs->joystick = 0;
          handled = true;
          break;
        case rightCtl:
          prefs->joystick = 1;
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
            case 0: prefs->button = keyBitHard1; break;
            case 1: prefs->button = keyBitHard2; break;
            case 2: prefs->button = keyBitHard3; break;
            case 3: prefs->button = keyBitHard4; break;
            //case 4: prefs->button = keyBitJogPress; break;
            //case 5: prefs->button = keyBitRockerCenter; break;
            default: prefs->button = keyBitHard1;
          }
      }

    default:
      break;
  }

  return handled;
}
