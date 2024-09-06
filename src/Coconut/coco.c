#include <PalmOS.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "machine.h"
#include "kbd.h"
#include "coco.h"
#include "misc.h"
#include "gui.h"

static UInt8 id;
static CocoPrefs prefs;

ColorType CocoColor[14] = {
  {COLOR_BLACK, 0, 0, 0},
  {COLOR_GREEN, 0, 255, 0},
  {COLOR_YELLOW, 255, 255, 0},
  {COLOR_BLUE, 0, 0, 255},
  {COLOR_RED, 255, 0, 0},
  {COLOR_WHITE, 255, 255, 255},
  {COLOR_CYAN, 0, 255, 255},
  {COLOR_PURPLE, 255, 0, 255},
  {COLOR_ORANGE, 255, 128, 0},
  {COLOR_DARKGREEN, 0, 64, 0},
  {COLOR_DARKORANGE, 64, 16, 0},
  {COLOR_BRIGHTORANGE, 255, 196, 24},
  {COLOR_GRED, 255, 0, 0},
  {-1, 0, 0, 0} 
};

ColorType Cp400Color[14] = {
  {COLOR_BLACK, 0, 0, 0},
  {COLOR_GREEN, 0, 255, 0},
  {COLOR_YELLOW, 255, 255, 0},
  {COLOR_BLUE, 0, 0, 255},
  {COLOR_RED, 255, 0, 0},
  {COLOR_WHITE, 255, 255, 255},
  {COLOR_CYAN, 0, 255, 255},
  {COLOR_PURPLE, 255, 0, 255},
  {COLOR_ORANGE, 255, 128, 0},
  {COLOR_DARKGREEN, 0, 64, 0},
  {COLOR_DARKORANGE, 64, 16, 0},
  {COLOR_BRIGHTORANGE, 255, 196, 24},
  {COLOR_GRED, 100, 146, 10},
  {-1, 0, 0, 0} 
};

UInt16 CocoControl[9] = {stopCmd, restartCmd, configCmd,
                         loadSnapCmd, saveSnapCmd, loadCasCmd,
                         loadDsk1Cmd, loadBinCmd, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}
#define C4 {0, 0xFF, 0x20, 0x20}

ButtonDef CocoButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {KEY_UP,      internalFontID, "\x80",  C1, C2, C3},
  {KEY_DOWN,    internalFontID, "\x81", C1, C2, C3},
  {KEY_BREAK,   internalFontID, "BRK", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_BACK,    internalFontID, "\x82", C1, C2, C3},
  {KEY_RIGHT,   internalFontID, "\x83", C1, C2, C3},
  {KEY_CLEAR,   internalFontID, "CLR", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {KEY_RESET,   internalFontID, "RST", C1, C4, C3},
  {KEY_INVERSE, internalFontID, "INV", C1, C2, C3}
};

KeyMap CocoMap[70] = {
  {'@', 0xFE, 0xFE, 0, 0},
  {'A', 0xFE, 0xFD, 0, 0},
  {'B', 0xFE, 0xFB, 0, 0},
  {'C', 0xFE, 0xF7, 0, 0},
  {'D', 0xFE, 0xEF, 0, 0},
  {'E', 0xFE, 0xDF, 0, 0},
  {'F', 0xFE, 0xBF, 0, 0},
  {'G', 0xFE, 0x7F, 0, 0},
  {'H', 0xFD, 0xFE, 0, 0},
  {'I', 0xFD, 0xFD, 0, 0},
  {'J', 0xFD, 0xFB, 0, 0},
  {'K', 0xFD, 0xF7, 0, 0},
  {'L', 0xFD, 0xEF, 0, 0},
  {'M', 0xFD, 0xDF, 0, 0},
  {'N', 0xFD, 0xBF, 0, 0},
  {'O', 0xFD, 0x7F, 0, 0},
  {'P', 0xFB, 0xFE, 0, 0},
  {'Q', 0xFB, 0xFD, 0, 0},
  {'R', 0xFB, 0xFB, 0, 0},
  {'S', 0xFB, 0xF7, 0, 0},
  {'T', 0xFB, 0xEF, 0, 0},
  {'U', 0xFB, 0xDF, 0, 0},
  {'V', 0xFB, 0xBF, 0, 0},
  {'W', 0xFB, 0x7F, 0, 0},
  {'X', 0xF7, 0xFE, 0, 0},
  {'Y', 0xF7, 0xFD, 0, 0},
  {'Z', 0xF7, 0xFB, 0, 0},
  {KEY_UP, 0xF7, 0xF7, 0, 0},
  {KEY_DOWN, 0xF7, 0xEF, 0, 0},
  {'[', 0xF7, 0xEF, 1, 0},
  {KEY_BACK, 0xF7, 0xDF, 0, 0},
  {KEY_RIGHT, 0xF7, 0xBF, 0, 0},
  {']', 0xF7, 0xBF, 1, 0},
  {' ', 0xF7, 0x7F, 0, 0},
  {'0', 0xEF, 0xFE, 0, 0},
  {KEY_INVERSE, 0xEF, 0xFE, 1, 0},
  {'1', 0xEF, 0xFD, 0, 0},
  {'!', 0xEF, 0xFD, 1, 0},
  {'2', 0xEF, 0xFB, 0, 0},
  {'"', 0xEF, 0xFB, 1, 0},
  {'3', 0xEF, 0xF7, 0, 0},
  {'#', 0xEF, 0xF7, 1, 0},
  {'4', 0xEF, 0xEF, 0, 0},
  {'$', 0xEF, 0xEF, 1, 0},
  {'5', 0xEF, 0xDF, 0, 0},
  {'%', 0xEF, 0xDF, 1, 0},
  {'6', 0xEF, 0xBF, 0, 0},
  {'&', 0xEF, 0xBF, 1, 0},
  {'7', 0xEF, 0x7F, 0, 0},
  {'\'', 0xEF, 0x7F, 1, 0},  // nao funciona
  {'8', 0xDF, 0xFE, 0, 0},
  {'(', 0xDF, 0xFE, 1, 0},
  {'9', 0xDF, 0xFD, 0, 0},
  {')', 0xDF, 0xFD, 1, 0},
  {':', 0xDF, 0xFB, 0, 0},
  {'*', 0xDF, 0xFB, 1, 0},
  {';', 0xDF, 0xF7, 0, 0},
  {'+', 0xDF, 0xF7, 1, 0},
  {',', 0xDF, 0xEF, 0, 0},
  {'<', 0xDF, 0xEF, 1, 0},
  {'-', 0xDF, 0xDF, 0, 0},
  {'=', 0xDF, 0xDF, 1, 0},
  {'.', 0xDF, 0xBF, 0, 0},
  {'>', 0xDF, 0xBF, 1, 0},
  {'/', 0xDF, 0x7F, 0, 0},
  {'?', 0xDF, 0x7F, 1, 0},   // nao funciona
  {KEY_ENTER, 0xBF, 0xFE, 0, 0},
  {KEY_CLEAR, 0xBF, 0xFD, 0, 0},
  {KEY_BREAK, 0xBF, 0xFB, 0, 0},
  {0, 0, 0, 0, 0}
};

KeyMap DragonMap[70] = {
  {'0', 0xFE, 0xFE, 0, 0},
  {KEY_INVERSE, 0xFE, 0xFE, 1, 0},
  {'1', 0xFE, 0xFD, 0, 0},
  {'!', 0xFE, 0xFD, 1, 0},
  {'2', 0xFE, 0xFB, 0, 0},
  {'"', 0xFE, 0xFB, 1, 0},
  {'3', 0xFE, 0xF7, 0, 0},
  {'#', 0xFE, 0xF7, 1, 0},
  {'4', 0xFE, 0xEF, 0, 0},
  {'$', 0xFE, 0xEF, 1, 0},
  {'5', 0xFE, 0xDF, 0, 0},
  {'%', 0xFE, 0xDF, 1, 0},
  {'6', 0xFE, 0xBF, 0, 0},
  {'&', 0xFE, 0xBF, 1, 0},
  {'7', 0xFE, 0x7F, 0, 0},
  {'\'', 0xFE, 0x7F, 1, 0},
  {'8', 0xFD, 0xFE, 0, 0},
  {'(', 0xFD, 0xFE, 1, 0},
  {'9', 0xFD, 0xFD, 0, 0},
  {')', 0xFD, 0xFD, 1, 0},
  {':', 0xFD, 0xFB, 0, 0},
  {'*', 0xFD, 0xFB, 1, 0},
  {';', 0xFD, 0xF7, 0, 0},
  {'+', 0xFD, 0xF7, 1, 0},
  {',', 0xFD, 0xEF, 0, 0},
  {'<', 0xFD, 0xEF, 1, 0},
  {'-', 0xFD, 0xDF, 0, 0},
  {'=', 0xFD, 0xDF, 1, 0},
  {'.', 0xFD, 0xBF, 0, 0},
  {'>', 0xFD, 0xBF, 1, 0},
  {'/', 0xFD, 0x7F, 0, 0},
  {'?', 0xFD, 0x7F, 1, 0},
  {'@', 0xFB, 0xFE, 0, 0},
  {'A', 0xFB, 0xFD, 0, 0},
  {'B', 0xFB, 0xFB, 0, 0},
  {'C', 0xFB, 0xF7, 0, 0},
  {'D', 0xFB, 0xEF, 0, 0},
  {'E', 0xFB, 0xDF, 0, 0},
  {'F', 0xFB, 0xBF, 0, 0},
  {'G', 0xFB, 0x7F, 0, 0},
  {'H', 0xF7, 0xFE, 0, 0},
  {'I', 0xF7, 0xFD, 0, 0},
  {'J', 0xF7, 0xFB, 0, 0},
  {'K', 0xF7, 0xF7, 0, 0},
  {'L', 0xF7, 0xEF, 0, 0},
  {'M', 0xF7, 0xDF, 0, 0},
  {'N', 0xF7, 0xBF, 0, 0},
  {'O', 0xF7, 0x7F, 0, 0},
  {'P', 0xEF, 0xFE, 0, 0},
  {'Q', 0xEF, 0xFD, 0, 0},
  {'R', 0xEF, 0xFB, 0, 0},
  {'S', 0xEF, 0xF7, 0, 0},
  {'T', 0xEF, 0xEF, 0, 0},
  {'U', 0xEF, 0xDF, 0, 0},
  {'V', 0xEF, 0xBF, 0, 0},
  {'W', 0xEF, 0x7F, 0, 0},
  {'X', 0xDF, 0xFE, 0, 0},
  {'Y', 0xDF, 0xFD, 0, 0},
  {'Z', 0xDF, 0xFB, 0, 0},
  {KEY_UP, 0xDF, 0xF7, 0, 0},
  {KEY_DOWN, 0xDF, 0xEF, 0, 0},
  {'[', 0xDF, 0xEF, 1, 0},
  {KEY_BACK, 0xDF, 0xDF, 0, 0},
  {KEY_RIGHT, 0xDF, 0xBF, 0, 0},
  {']', 0xDF, 0xBF, 1, 0},
  {' ', 0xDF, 0x7F, 0, 0},
  {KEY_ENTER, 0xBF, 0xFE, 0, 0},
  {KEY_CLEAR, 0xBF, 0xFD, 0, 0},
  {KEY_BREAK, 0xBF, 0xFB, 0, 0},
  {0, 0, 0, 0, 0}
};

void coco_select(MachineType *machine, Hardware *hardware, UInt8 ramsize)
{
  UInt16 len;

  id = machine->id;
  len = sizeof(prefs);
  MemSet(&prefs, len, 0);

  if (PrefGetAppPreferences(AppID, 128 + id, &prefs, &len, true) ==
         noPreferenceFound) {
    prefs.id = machine->id;
    prefs.ramsize = ramsize;
    prefs.joystick = JOY_LEFT;
    prefs.button = keyBitHard1;
    prefs.artifacting = 0;
    PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, len, true);
  }
}

void coco_finish(void)
{
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);
}

CocoPrefs *CocoGetPrefs(void)
{
  return &prefs;
}

Boolean CocoFormHandler(EventPtr event, Boolean *close)
{
  FormPtr frm;
  ListPtr lst;
  CocoPrefs *prefs;
  UInt32 button;
  Boolean handled;

  frm = FrmGetActiveForm();
  prefs = CocoGetPrefs();
  handled = false;
  *close = false;

  switch (event->eType) {
    case frmOpenEvent:
      switch (prefs->artifacting) {
        case 0: FrmSetControlValue(frm, FrmGetObjectIndex(frm, noneCtl), 1);
                break;
        case 1: FrmSetControlValue(frm, FrmGetObjectIndex(frm, blueCtl), 1);
                break;
        case 2: FrmSetControlValue(frm, FrmGetObjectIndex(frm, redCtl), 1);
      }

      if (prefs->joystick == JOY_LEFT)
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
        case noneCtl:
          prefs->artifacting = 0;
          io_setvdg();
          break;
        case blueCtl:
          prefs->artifacting = 1;
          io_setvdg();
          break;
        case redCtl:
          prefs->artifacting = 2;
          io_setvdg();
          break;
        case leftCtl:
          prefs->joystick = JOY_LEFT;
          handled = true;
          break;
        case rightCtl:
          prefs->joystick = JOY_RIGHT;
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
