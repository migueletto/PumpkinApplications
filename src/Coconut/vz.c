#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "endian.h"
#include "palm.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "machine.h"
#include "kbd.h"
#include "z80.h"
#include "vz.h"
#include "misc.h"
#include "gui.h"

typedef struct {
  UInt8 magic[4];
  char  name[17];
  UInt8 type;
  UInt8 addrl;
  UInt8 addrh;
} VZFILE;

static UInt8 id;
static VzPrefs prefs;
static Hardware *hardware;
static KeyMap keyMap[256];
static Boolean ctrl;
static UInt16 key, joyx, joyy;
static UInt8 vdg;

UInt16 VZ300Control[9] = {stopCmd, restartCmd, configCmd,
                          loadSnapCmd, 0, 0,
                          0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef VZ300ButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {KEY_UP,      internalFontID, "\x80",  C1, C2, C3},
  {KEY_DOWN,    internalFontID, "\x81", C1, C2, C3},
  {KEY_BREAK,   internalFontID, "BRK", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_LEFT,    internalFontID, "\x82", C1, C2, C3},
  {KEY_RIGHT,   internalFontID, "\x83", C1, C2, C3},
  {KEY_CTRL,    internalFontID, "CTL", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {KEY_RESET,   internalFontID, "RST", C1, C2, C3},
  {KEY_INVERSE, internalFontID, "INV", C1, C2, C3}
};

static KeyMap VZ300Map[] = {
  {'R', 0xFE, 0x1F, 0, 0},
  {'Q', 0xFE, 0x2F, 0, 0},
  {'E', 0xFE, 0x37, 0, 0},
  {'W', 0xFE, 0x3D, 0, 0},
  {'T', 0xFE, 0x3E, 0, 0},
  {'F', 0xFD, 0x1F, 0, 0},
  {'A', 0xFD, 0x2F, 0, 0},
  {'D', 0xFD, 0x37, 0, 0},
  {KEY_CTRL, 0xFD, 0x3B, 0, 0},
  {'S', 0xFD, 0x3D, 0, 0},
  {'G', 0xFD, 0x3E, 0, 0},
  {'V', 0xFB, 0x1F, 0, 0},
  {'Z', 0xFB, 0x2F, 0, 0},
  {'C', 0xFB, 0x37, 0, 0},
  {KEY_SHIFT, 0xFB, 0x3B, 0, 0},
  {'X', 0xFB, 0x3D, 0, 0},
  {'B', 0xFB, 0x3E, 0, 0},
  {'4', 0xF7, 0x1F, 0, 0},
  {'$', 0xF7, 0x1F, 1, 0},
  {'1', 0xF7, 0x2F, 0, 0},
  {'!', 0xF7, 0x2F, 1, 0},
  {'3', 0xF7, 0x37, 0, 0},
  {'#', 0xF7, 0x37, 1, 0},
  {'2', 0xF7, 0x3D, 0, 0},
  {'\\', 0xF7, 0x3D, 1, 0},
  {'5', 0xF7, 0x3E, 0, 0},
  {'%', 0xF7, 0x3E, 1, 0},
  {'M', 0xEF, 0x1F, 0, 0},
  {KEY_LEFT, 0xEF, 0x1F, 0, 1},
  {' ', 0xEF, 0x2F, 0, 0},
  {KEY_DOWN, 0xEF, 0x2F, 0, 1},
  {',', 0xEF, 0x37, 0, 0},
  {KEY_RIGHT, 0xEF, 0x37, 0, 1},
  {'.', 0xEF, 0x3D, 0, 0},
  {KEY_UP, 0xEF, 0x3D, 0, 1},
  {'N', 0xEF, 0x3E, 0, 0},
  {'7', 0xDF, 0x1F, 0, 0},
  {'\'', 0xDF, 0x1F, 1, 0},
  {'0', 0xDF, 0x2F, 0, 0},
  {'@', 0xDF, 0x2F, 1, 0},
  {'8', 0xDF, 0x37, 0, 0},
  {'(', 0xDF, 0x37, 1, 0},
  {'-', 0xDF, 0x3B, 0, 0},
  {'=', 0xDF, 0x3B, 1, 0},
  {KEY_BREAK, 0xDF, 0x3B, 0, 1},
  {'9', 0xDF, 0x3D, 0, 0},
  {')', 0xDF, 0x3D, 1, 0},
  {'6', 0xDF, 0x3E, 0, 0},
  {'&', 0xDF, 0x3E, 1, 0},
  {'U', 0xBF, 0x1F, 0, 0},
  {'P', 0xBF, 0x2F, 0, 0},
  {'I', 0xBF, 0x37, 0, 0},
  {KEY_ENTER, 0xBF, 0x3B, 0, 0},
  {'O', 0xBF, 0x3D, 0, 0},
  {'Y', 0xBF, 0x3E, 0, 0},
  {'J', 0x7F, 0x1F, 0, 0},
  {';', 0x7F, 0x2F, 0, 0},
  {'+', 0x7F, 0x2F, 1, 0},
  {KEY_BACK, 0x7F, 0x2F, 0, 1},
  {'K', 0x7F, 0x37, 0, 0},
  {':', 0x7F, 0x3B, 0, 0},
  {'*', 0x7F, 0x3B, 1, 0},
  {KEY_INVERSE, 0x7F, 0x3B, 0, 1},
  {'L', 0x7F, 0x3D, 0, 0},
  {KEY_INSERT, 0x7F, 0x3D, 0, 1},
  {'H', 0x7F, 0x3E, 0, 0},
  {0, 0, 0, 0, 0}
};

UInt32 vz_callback(ArmletCallbackArg *arg)
{
  switch (arg->cmd) {
    case IO_VSYNC:
      vz_vsync(arg->a1);
      return 0;
    case IO_READB:
      return vz_readb(arg->a1);
    case IO_WRITEB:
      vz_writeb(arg->a1, arg->a2);
      return 0;
  }
  return 0;
}

void vz_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize)
{
  UInt16 len;

  id = machine->id;
  hardware = _hardware;

  len = sizeof(prefs);
  MemSet(&prefs, len, 0);

  if (PrefGetAppPreferences(AppID, 128 + id, &prefs, &len, true) == noPreferenceFound) {
    prefs.joystick = JOY_LEFT;
    prefs.button1 = keyBitHard1;
    prefs.button2 = keyBitHard2;
    PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, len, true);
  }

  kbd_initmap(VZ300Map, keyMap);

  ctrl = false;
  joyx = joyy = 32;
  key = 0;
  vdg = 0;

  hardware->ramsize = ramsize * 4096 - 1;
}

void vz_init(void)
{
  io_setgp(0x7000, 0x7800);
  io_vdg(0);
  io_sam(0);
}

void vz_finish(void)
{
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);
}

void vz_key(UInt16 c)
{
  if (c >= 'a' && c <= 'z')
    c &= 0xDF;
  else if (c == KEY_CTRL) {
    ctrl = ~ctrl;
    c = 0;
  }

  key = c;
}

void vz_joystick(UInt16 x, UInt16 y)
{
  joyx = x;
  joyy = y;
}

UInt8 vz_readb(UInt16 a)
{
  UInt8 b = 0xFF;

  if (a == 0xFFFF)
    return 1;

  if (a >= 0x6800) {
    UInt8 line = (a & 0xFF) ^ 0xFF;

    // I/O latch

    if (key) {
      if ((keyMap[key].line^0xFF) & line)
        b = keyMap[key].column;

      if (keyMap[key].shift && ((keyMap[KEY_SHIFT].line^0xFF) & line))
        b &= keyMap[KEY_SHIFT].column;

      if ((ctrl || keyMap[key].ctrl) && ((keyMap[KEY_CTRL].line^0xFF) & line))
        b &= keyMap[KEY_CTRL].column;
    }
  } else {
    a &= 0xFF;

    if (a >= 0x20 && a < 0x30) {
      UInt8 kbit, jbit;
      UInt32 mask = KeyCurrentState();

      if (prefs.joystick == JOY_LEFT) {
        kbit = 0x01;
        jbit = 0x02;
      } else {
        kbit = 0x04;
        jbit = 0x08;
      }

      // joystick

      if (!(a & kbit)) {
        if (joyx < 16)		// left
          b &= 0xFB;
        else if (joyx > 48)	// right
          b &= 0xF7;

        if (joyy < 16)		// up
          b &= 0xFE;
        else if (joyy > 48)	// down
          b &= 0xFD;

        if (mask & prefs.button1)		// button 1
          b &= 0xEF;
      }

      if (!(a & jbit)) {
        if (mask & prefs.button2)		// button 2
          b &= 0xEF;
      }
    }
  }

  return b;
}

void vz_writeb(UInt16 a, UInt8 b)
{
  UInt8 b1;

  switch (a) {
    case 0x6800:	// 0x6800 - 0x6FFF
      // bit 5: Speaker B
      // bit 4: VDC Background 0=green, 1=orange (text) / buff (graphics)
      // bit 3: VDC Display 0=text, 1=graphics
      // bit 2: Cassette out (MSB)
      // bit 1: Cassette out (LSB)
      // bit 0: Speaker A

      if (b & 0x08)
        b1 = 0xA0;	// graphic mode, 128x64 pixels, 4 colors
      else
        b1 = 0x00;	// text mode

      if (b & 0x10)
        b1 |= 0x08;	// CSS

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

      snd_output((b & 0x01) ? 0xFF : 0x00);
      snd_output((b & 0x20) ? 0xFF : 0x00);
  }
}

Err vz_readvz(FileRef f, z80_Regs *z80, Hardware *hardware)
{
  VZFILE vz;
  UInt32 addr, size, hsize, aux, r;
  Err err;

  hsize = 24;

  if ((err = VFSFileSize(f, &size)) != 0)
    return -1;

  if (size < hsize)
    return -2;

  if ((err = VFSFileRead(f, hsize, &vz, &r)) != 0)
    return -3;

  if (r != hsize)
    return -4;

  if (vz.type != 0xF0 && vz.type != 0xF1)
    return -5;

  addr = ((vz.addrh << 8) | vz.addrl) & 0xFFFF;
  size -= hsize;

  if (addr < 0x7000 || (addr + size) >= 0xB800)
    return -6;

  if (addr >= 0x8000) {

    aux = size;
    if ((err = VFSFileRead(f, aux, &hardware->m0[addr & 0x7FFF], &r)) != 0)
      return -7;
    if (r != aux)
      return -8;

  } else if ((addr + size) < 0x8000) {

    aux = size;
    if ((err = VFSFileRead(f, aux, &hardware->m1[addr], &r)) != 0)
      return -9;
    if (r != aux)
      return -10;

  } else {

    aux = 0x8000 - addr;
    if ((err = VFSFileRead(f, aux, &hardware->m1[addr], &r)) != 0)
      return -11;
    if (r != aux)
      return -12;

    aux = size - aux;
    if ((err = VFSFileRead(f, aux, hardware->m0, &r)) != 0)
      return -13;
    if (r != aux)
      return -14;
  }

  if (vz.type == 0xF1)
    z80->pc = LITTLE_ENDIANIZE_INT32(addr);
  else {
    hardware->m1[0x78A4] = vz.addrl;
    hardware->m1[0x78A5] = vz.addrh;
    hardware->m1[0x78F9] = ((addr + size) & 0x00FF);
    hardware->m1[0x78FA] = (((addr + size) & 0xFF00) >> 8);
  }

  return 0;
}

VzPrefs *VzGetPrefs(void)
{
  return &prefs;
}

Boolean VzFormHandler(EventPtr event, Boolean *close)
{
  FormPtr frm;
  ListPtr lst;
  UInt32 button;
  Boolean handled;

  frm = FrmGetActiveForm();
  handled = false;
  *close = false;

  switch (event->eType) {
    case frmOpenEvent:
      if (prefs.joystick == JOY_LEFT)
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
          prefs.joystick = JOY_LEFT;
          handled = true;
          break;
        case rightCtl:
          prefs.joystick = JOY_RIGHT;
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
