#include <PalmOS.h>

#include "section.h"
#include "palm.h"
#include "console.h"
#include "auxp.h"
#include "lpt.h"
#include "machine.h"
#include "io.h"
#include "cpm.h"
#include "cpmdsk.h"
#include "screen.h"
#include "vt100.h"
#include "misc.h"
#include "gui.h"

#define CONSTA	0      // console status port
#define CONDAT	1      // console data port
#define PRTSTA	2      // printer status port
#define PRTDAT	3      // printer data port
#define AUXSTA	4      // auxiliary status port
#define AUXDAT	5      // auxiliary data port
#define FDCD	10     // fdc-port: # of drive
#define FDCT	11     // fdc-port: # of track
#define FDCS	12     // fdc-port: # of sector
#define FDCOP	13     // fdc-port: command
#define FDCST	14     // fdc-port: status
#define DMAL	15     // dma-port: dma address low
#define DMAH	16     // dma-port: dma address high

static UInt8 id;
static CpmPrefs prefs;
static Hardware *hardware;
static UInt8 drive = 0, track = 0, sector = 0;
static UInt16 addr = 0;
static UInt8 fdcst = 0;
static UInt8 buf[128];
static Boolean ctrl;
static UInt16 key;

ColorType CpmColor[5] = {
  {0, 0, 0, 0},
  {1, 0, 255, 0},
  {2, 0, 255, 255},
  {3, 255, 128, 0},
  {-1, 0, 0, 0}
};

UInt16 CpmControl[9] = {stopCmd, restartCmd, configCmd,
                        loadDsk1Cmd, loadDsk2Cmd, 0,
                        0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef CpmButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {KEY_UP,    internalFontID, "UP",  C1, C2, C3},
  {KEY_DOWN,  internalFontID, "DWN", C1, C2, C3},
  {KEY_BREAK, internalFontID, "BRK", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_LEFT,  internalFontID, "LFT", C1, C2, C3},
  {KEY_RIGHT, internalFontID, "RGT", C1, C2, C3},
  {KEY_CTRL,  internalFontID, "CTL", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {KEY_RESET, internalFontID, "RST", C1, C2, C3}
};

void cpm_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize)
{
  UInt16 len;

  id = machine->id;
  hardware = _hardware;

  len = sizeof(prefs);
  MemSet(&prefs, len, 0);

  if (PrefGetAppPreferences(AppID, 128 + id, &prefs, &len, true) ==
         noPreferenceFound) {
    prefs.crt = 0;
    prefs.font = Cpm6x10ID;
    PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, len, true);
  }

  ctrl = false;
  key = 0;
}

void cpm_finish(void)
{
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);
}

void cpm_key(UInt16 c)
{
  Boolean send = false;
  char *s = NULL;

  key = c;

  switch (key) {
    case 0:
      break;
    case KEY_UP:
      ctrl = false;
      s = GetKeySeq(upArrow);
      send = true;
      break;
    case KEY_DOWN:
      ctrl = false;
      s = GetKeySeq(downArrow);
      send = true;
    case KEY_LEFT:
      ctrl = false;
      s = GetKeySeq(leftArrow);
      send = true;
    case KEY_RIGHT:
      ctrl = false;
      s = GetKeySeq(rightArrow);
      send = true;
      break;
    case KEY_CTRL:
      ctrl = ~ctrl;
      break;
    case KEY_CLEAR:
      ctrl = false;
      key = DEL;
      send = true;
      break;
    case KEY_BREAK:
      ctrl = true;
      key = 'C';
      send = true;
      break;
    case '\n':
      ctrl = false;
      key = '\r';
      send = true;
      break;
    default:
      send = true;
  }

  if (send) {
    if (ctrl) {
      if (key >= 64 && key <= 90)
        key -= 64;
      else if (key >= 97 && key <= 122)
        key -= 96;
      ctrl = false;
    }

    if (s) {
      to_console(s[0]);
      to_console(s[1]);
      key = s[2];
    }
    to_console(key);

    io_run();
  }
}

void cpm_writeb(UInt8 *r1, UInt8 *r2, UInt16 a, UInt8 b)
{
  UInt16 p;
  Int32 i;

  switch (a & 0xFF) {
    case CONDAT:
      write_console(b);
      break;
    case PRTDAT:
      write_lpt(b);
      break;
    case AUXDAT:
      write_aux(b);
      break;
    case FDCD:
      drive = b;
      break;
    case FDCT:
      track = b;
      break;
    case FDCS:
      sector = b;
      break;
    case FDCOP:
      switch (b) {
        case 0:
          fdcst = read_sector(drive, track, sector, buf);
          for (i = 0, p = addr; i < 128; i++, p++)
            if (p < 0x8000)
              r1[p] = buf[i];
            else
              r2[p - 0x8000] = buf[i];
          break;
        case 1:
          for (i = 0, p = addr; i < 128; i++, p++)
            if (p < 0x8000)
              buf[i] = r1[p];
            else
              buf[i] = r2[p - 0x8000];
          fdcst = write_sector(drive, track, sector, buf);
          break;
        default:
          fdcst = 1;
      }
      break;
    case DMAL:
      addr = (addr & 0xff00) | b;
      break;
    case DMAH:
      addr = (addr & 0x00ff) | ((UInt16)b << 8);
  }
}

UInt8 cpm_readb(UInt8 *r1, UInt8 *r2, UInt16 a)
{
  UInt8 b;

  if (a == 0xFFFF)	// GAMBIARRA: CheckInput de console do CP/M
    return stat_console() ? 1 : 0;

  switch (a & 0xFF) {
    case CONSTA:
      // console status, return 0ffh if character ready, 00h if not
      return stat_console();
    case CONDAT:
      return read_console();
    case PRTSTA:
      // return list status (0 if not ready, 1 if ready)
      return stat_lpt();
    case AUXSTA:
      return stat_aux();
    case AUXDAT:
      return read_aux();
    case FDCST:
      // status of i/o operation: 00h success, 01h error
      b = fdcst;
      fdcst = 0;
      return b;
  }

  return 0;
}

void cpm_cls(void)
{
  UInt16 i, y0;
  UInt8 *s;

  y0 = (hardware->display_height - 192 - lowBorder * 2) / 2;

  WinSetCoordinateSystem(kCoordinatesDouble);
  s = WinScreenLock(winLockCopy);

  for (i = 0; i < 192 + 2 * y0; i++, s += hardware->display_width) {
    MemSet(s, hardware->display_width, hardware->color[0]);
  }

  WinScreenUnlock();
  WinSetCoordinateSystem(kCoordinatesStandard);
}

CpmPrefs *CpmGetPrefs(void)
{
  return &prefs;
}

Boolean CpmFormHandler(EventPtr event, Boolean *close)
{
  FormPtr frm;
  ListPtr lst;
  CpmPrefs *prefs;
  MachineType *machine;
  Boolean handled;

  frm = FrmGetActiveForm();
  prefs = CpmGetPrefs();
  handled = false;
  *close = false;

  switch (event->eType) {
    case frmOpenEvent:
      switch (prefs->font) {
        case Cpm6x10ID:
          FrmSetControlValue(frm, FrmGetObjectIndex(frm, c64Ctl), 1);
          break;
        case Cpm4x6ID:
          FrmSetControlValue(frm, FrmGetObjectIndex(frm, c80Ctl), 1);
      }

      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, crtList));
      LstSetSelection(lst, prefs->crt);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
        FrmGetObjectIndex(frm, crtCtl)),
        LstGetSelectionText(lst, prefs->crt));

      FrmDrawForm(frm);
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case c64Ctl:
          prefs->font = Cpm6x10ID;
          machine = mch_getmachine(id);
          machine->font = prefs->font;
          InitScreen(25, 64, 0, 0, hardware->color[prefs->crt + 1], hardware->color[0]);
          InitTerminal(25, 64);
          break;
        case c80Ctl:
          prefs->font = Cpm4x6ID;
          machine = mch_getmachine(id);
          machine->font = prefs->font;
          InitScreen(25, 80, 0, 0, hardware->color[prefs->crt + 1], hardware->color[0]);
          InitTerminal(25, 80);
          break;
        case okBtn:
          *close = true;
          handled = true;
      }
      break;

    case popSelectEvent:
      switch (event->data.popSelect.listID) {
        case crtList:
          prefs->crt = event->data.popSelect.selection;
          InitScreen(25, prefs->font == Cpm6x10ID ? 64 : 80,
                     0, 0, hardware->color[prefs->crt + 1], hardware->color[0]);
          InitTerminal(25, prefs->font == Cpm6x10ID ? 64 : 80);
      }

    default:
      break;
  }

  return handled;
}
