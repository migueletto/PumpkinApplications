#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "machine.h"
#include "kbd.h"
#include "sn76489.h"
#include "ti9918.h"
#include "endian.h"
#include "z80.h"
#include "coleco.h"
#include "misc.h"
#include "gui.h"

static UInt8 id;
static ColecoPrefs prefs;
static Hardware *hardware;
static UInt16 key;

UInt16 ColecoControl[9] = {stopCmd, restartCmd, configCmd,
                           loadSnapCmd, 0, 0,
                           0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef ColecoButtonDef[12] = {
  {'1', internalFontID, "1",  C1, C2, C3},
  {'2', internalFontID, "2",  C1, C2, C3},
  {'3', internalFontID, "3",  C1, C2, C3},
  {'*', internalFontID, "*",  C1, C2, C3},

  {'4', internalFontID, "4",  C1, C2, C3},
  {'5', internalFontID, "5",  C1, C2, C3},
  {'6', internalFontID, "6",  C1, C2, C3},
  {'0', internalFontID, "0",  C1, C2, C3},

  {'7', internalFontID, "7",  C1, C2, C3},
  {'8', internalFontID, "8",  C1, C2, C3},
  {'9', internalFontID, "9",  C1, C2, C3},
  {'#', internalFontID, "#",  C1, C2, C3},
};

static KeyMap ColecoMap[] = {
  {'0', 0, 0x0A, 0, 0},
  {'1', 0, 0x0D, 0, 0},
  {'2', 0, 0x07, 0, 0},
  {'3', 0, 0x0C, 0, 0},
  {'4', 0, 0x02, 0, 0},
  {'5', 0, 0x03, 0, 0},
  {'6', 0, 0x0E, 0, 0},
  {'7', 0, 0x05, 0, 0},
  {'8', 0, 0x01, 0, 0},
  {'9', 0, 0x0B, 0, 0},
  {'#', 0, 0x06, 0, 0},
  {'*', 0, 0x09, 0, 0},
  {0, 0, 0, 0, 0}
};

UInt32 coleco_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_VSYNC:
      coleco_vsync(arg->a1);
      break;
    case IO_HALT:
      io_halt();
  }
  return 0;
}

void coleco_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  ColecoGlobals *globals;
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

  if (!hardware->globals) {
    hardware->globals = MemPtrNew(sizeof(ColecoGlobals));
    MemSet(hardware->globals, sizeof(ColecoGlobals), 0);
  }
  globals = hardware->globals;

  globals->joymode = 0;
  globals->joystick = (prefs.joystick);
  globals->button = prefs.button;

  keyMap = MemPtrNew(256 * sizeof(KeyMap));
  kbd_initmap(ColecoMap, keyMap);

  for (i = 0; i < 256; i++) {
    globals->keyMap[i].key = (keyMap[i].key);
    globals->keyMap[i].line = (keyMap[i].line);
    globals->keyMap[i].column = (keyMap[i].column);
    globals->keyMap[i].shift = (keyMap[i].shift);
    globals->keyMap[i].ctrl = (keyMap[i].ctrl);
  }

  MemPtrFree(keyMap);

  sn76489_init(&globals->sn76489, hardware, 3579545L);
  ti9918_init(&globals->ti9918, hardware);
}

void coleco_init(void) {
  hardware->vsync_irq = 0;
  MemSet(&hardware->m1[0x6000], 0x400, 0xFF);
}

void coleco_reset(void) {
  ColecoGlobals *g;

  g = hardware->globals;
  ti9918_reset(&g->ti9918);
}

void coleco_finish(void) {
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);

  if (hardware->globals) {
    MemPtrFree(hardware->globals);
    hardware->globals = NULL;
  }
}

void coleco_key(UInt16 c) {
  key = c;
  hardware->key = key;
}

void coleco_joystick(UInt16 x, UInt16 y) {
  hardware->joyx = x;
  hardware->joyy = y;
}

Err coleco_readcart(FileRef f, z80_Regs *z80, Hardware *hardware) {
  UInt32 size, r;
  Err err;

  if ((err = VFSFileSize(f, &size)) != 0)
    return -1;

  if (size > 0x8000)
    return -2;

  if ((err = VFSFileRead(f, size, hardware->m0, &r)) != 0) {
    MemSet(hardware->m0, 0x8000, 0);
    return -3;
  }

  if (size != r) {
    MemSet(hardware->m0, 0x8000, 0);
    return -4;
  }

  if (hardware->m0[0] == 0x55 && hardware->m0[1] == 0xAA)
    return 0;

  if (hardware->m0[0] == 0xAA && hardware->m0[1] == 0x55)
    return 0;

  MemSet(hardware->m0, 0x8000, 0);
  return -5;
}

ColecoPrefs *ColecoGetPrefs(void) {
  return &prefs;
}

Boolean ColecoFormHandler(EventPtr event, Boolean *close) {
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
            case 0: prefs.button = keyBitHard1; break;
            case 1: prefs.button = keyBitHard2; break;
            case 2: prefs.button = keyBitHard3; break;
            case 3: prefs.button = keyBitHard4; break;
            //case 4: prefs.button = keyBitJogPress; break;
            //case 5: prefs.button = keyBitRockerCenter; break;
            default: prefs.button = keyBitHard1;
          }
      }

    default:
      break;
  }

  return handled;
}
