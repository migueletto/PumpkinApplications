#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "machine.h"
#include "kbd.h"
#include "endian.h"
#include "z80.h"
#include "aquarius.h"
#include "misc.h"
#include "gui.h"

#define AQUARIUS_FONT		'aqfn'
#define AQUARIUS_FONT_ID	1

static UInt8 id;
static AquariusPrefs prefs;
static Hardware *hardware;
static UInt16 key;
static MemHandle aqfn = NULL;

ColorType AquariusColor[17] = {
  { 0, 0x00, 0x00, 0x00},       // Black
  { 1, 0xff, 0x00, 0x00},       // Red
  { 2, 0x00, 0xff, 0x00},       // Green
  { 3, 0xff, 0xff, 0x00},       // Yellow
  { 4, 0x00, 0x00, 0xff},       // Blue
  { 5, 0x7f, 0x00, 0x7f},       // Violet
  { 6, 0x7f, 0xff, 0xff},       // Light Blue-Green
  { 7, 0xff, 0xff, 0xff},       // White
  { 8, 0xc0, 0xc0, 0xc0},       // Light Gray
  { 9, 0x00, 0xff, 0xff},       // Blue-Green
  {10, 0xff, 0x00, 0xff},       // Magenta
  {11, 0x00, 0x00, 0x7f},       // Dark Blue
  {12, 0xff, 0xff, 0x7f},       // Light Yellow
  {13, 0x7f, 0xff, 0x7f},       // Light Green
  {14, 0xff, 0x7f, 0x00},       // Orange
  {15, 0x7f, 0x7f, 0x7f},       // Dark Gray
  {-1, 0, 0, 0}
};

UInt16 AquariusControl[9] = {stopCmd, restartCmd, 0,
                             loadSnapCmd, 0, 0,
                             0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef AquariusButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {KEY_CTRL,  internalFontID, "CTL", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {KEY_RESET,  internalFontID, "RST", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
};

static KeyMap AquariusMap[] = {
  {'=', 0x01, 0xFE, 0, 0},
  {'+', 0x01, 0xFE, 1, 0},
  {KEY_BACK, 0x01, 0xFD, 0, 0},
  {'\\', 0x01, 0xFD, 1, 0},
  {':', 0x01, 0xFB, 0, 0},
  {'*', 0x01, 0xFB, 1, 0},
  {KEY_ENTER, 0x01, 0xF7, 0, 0},
  {';', 0x01, 0xEF, 0, 0},
  {'@', 0x01, 0xEF, 1, 0},
  {'.', 0x01, 0xDF, 0, 0},
  {'>', 0x01, 0xDF, 1, 0},

  {'-', 0x02, 0xFE, 0, 0},
  {'_', 0x02, 0xFE, 1, 0},
  {'/', 0x02, 0xFD, 0, 0},
  {'^', 0x02, 0xFD, 1, 0},
  {'0', 0x02, 0xFB, 0, 0},
  {'?', 0x02, 0xFB, 1, 0},
  {'p', 0x02, 0xF7, 0, 0},
  {'P', 0x02, 0xF7, 1, 0},
  {'l', 0x02, 0xEF, 0, 0},
  {'L', 0x02, 0xEF, 1, 0},
  {',', 0x02, 0xDF, 0, 0},
  {'<', 0x02, 0xDF, 1, 0},

  {'9', 0x04, 0xFE, 0, 0},
  {')', 0x04, 0xFE, 1, 0},
  {'o', 0x04, 0xFD, 0, 0},
  {'O', 0x04, 0xFD, 1, 0},
  {'k', 0x04, 0xFB, 0, 0},
  {'K', 0x04, 0xFB, 1, 0},
  {'m', 0x04, 0xF7, 0, 0},
  {'M', 0x04, 0xF7, 1, 0},
  {'n', 0x04, 0xEF, 0, 0},
  {'N', 0x04, 0xEF, 1, 0},
  {'j', 0x04, 0xDF, 0, 0},
  {'J', 0x04, 0xDF, 1, 0},

  {'8', 0x08, 0xFE, 0, 0},
  {'(', 0x08, 0xFE, 1, 0},
  {'i', 0x08, 0xFD, 0, 0},
  {'I', 0x08, 0xFD, 1, 0},
  {'7', 0x08, 0xFB, 0, 0},
  {'\'', 0x08, 0xFB, 1, 0},
  {'u', 0x08, 0xF7, 0, 0},
  {'U', 0x08, 0xF7, 1, 0},
  {'h', 0x08, 0xEF, 0, 0},
  {'H', 0x08, 0xEF, 1, 0},
  {'b', 0x08, 0xDF, 0, 0},
  {'B', 0x08, 0xDF, 1, 0},

  {'6', 0x10, 0xFE, 0, 0},
  {'&', 0x10, 0xFE, 1, 0},
  {'y', 0x10, 0xFD, 0, 0},
  {'Y', 0x10, 0xFD, 1, 0},
  {'g', 0x10, 0xFB, 0, 0},
  {'G', 0x10, 0xFB, 1, 0},
  {'v', 0x10, 0xF7, 0, 0},
  {'V', 0x10, 0xF7, 1, 0},
  {'c', 0x10, 0xEF, 0, 0},
  {'C', 0x10, 0xEF, 1, 0},
  {'f', 0x10, 0xDF, 0, 0},
  {'F', 0x10, 0xDF, 1, 0},

  {'5', 0x20, 0xFE, 0, 0},
  {'%', 0x20, 0xFE, 1, 0},
  {'t', 0x20, 0xFD, 0, 0},
  {'T', 0x20, 0xFD, 1, 0},
  {'4', 0x20, 0xFB, 0, 0},
  {'$', 0x20, 0xFB, 1, 0},
  {'r', 0x20, 0xF7, 0, 0},
  {'R', 0x20, 0xF7, 1, 0},
  {'d', 0x20, 0xEF, 0, 0},
  {'D', 0x20, 0xEF, 1, 0},
  {'x', 0x20, 0xDF, 0, 0},
  {'X', 0x20, 0xDF, 1, 0},

  {'3', 0x40, 0xFE, 0, 0},
  {'#', 0x40, 0xFE, 1, 0},
  {'e', 0x40, 0xFD, 0, 0},
  {'E', 0x40, 0xFD, 1, 0},
  {'s', 0x40, 0xFB, 0, 0},
  {'S', 0x40, 0xFB, 1, 0},
  {'z', 0x40, 0xF7, 0, 0},
  {'Z', 0x40, 0xF7, 1, 0},
  {' ', 0x40, 0xEF, 0, 0},
  {'a', 0x40, 0xDF, 0, 0},
  {'A', 0x40, 0xDF, 1, 0},

  {'2', 0x80, 0xFE, 0, 0},
  {'"', 0x80, 0xFE, 1, 0},
  {'w', 0x80, 0xFD, 0, 0},
  {'W', 0x80, 0xFD, 1, 0},
  {'1', 0x80, 0xFB, 0, 0},
  {'!', 0x80, 0xFB, 1, 0},
  {'q', 0x80, 0xF7, 0, 0},
  {'Q', 0x80, 0xF7, 1, 0},
  {KEY_SHIFT, 0x80, 0xEF, 0, 0},
  {KEY_CTRL, 0x80, 0xDF, 0, 0},

  {0, 0, 0, 0, 0}
};

static void aquarius_video(void);

UInt32 aquarius_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_VSYNC:
      aquarius_video();
      aquarius_vsync(arg->a1);
      return 0;
    case IO_READB:
      return aquarius_readb(arg->a1);
    case IO_WRITEB:
      aquarius_writeb(arg->a1, arg->a2);
      return 0;
  }
  return 0;
}

static void aquarius_video(void) {
  RectangleType rect;
  WinHandle prev;
  UInt32 x, y, i, j, k, caddr;
  UInt8 code, color, c1, c2, mask;
  AquariusGlobals *g;
  Err err;

  g = hardware->globals;
  g->frame++;

  if (!hardware->dirty)
    return;

  if (g->frame & 1)
    return;

  WinSetCoordinateSystem(kCoordinatesDouble);

  if (hardware->screen_wh == NULL) {
    hardware->screen_wh = WinCreateOffscreenWindow(hardware->dx, hardware->dy, nativeFormat, &err);
  }

  prev = WinSetDrawWindow(hardware->screen_wh);

  if (hardware->dirty) {
    // 29 linhas: 2 (border) + 1 (header) + 24 (texto) + 2 (border)
    // 28 * 8 = 232

    code = hardware->m1[0x3000];
    caddr = code * 8;

    color = hardware->m1[0x3400];
    c1 = hardware->color[color >> 4];
    c2 = hardware->color[color & 0x0F];

    // linhas 1 e 2 da borda
    for (i = 0; i < 80; i++) {
      x = (i % 40) * 8;
      y = (i / 40) * 8;

      for (k = 0; k < 8; k++) {
        mask = g->font[caddr + k];
        for (j = 0; j < 8; j++, mask <<= 1) {
          WinSetForeColor((mask & 0x80) ? c1 : c2);
          WinDrawPixel(x+j, y+k);
        }
      }
    }

    // linhas 27 e 28 da borda (27 * 40 = 1080)
    for (i = 1080; i < 1160; i++) {
      x = (i % 40) * 8;
      y = (i / 40) * 8;

      for (k = 0; k < 8; k++) {
        mask = g->font[caddr + k];
        for (j = 0; j < 8; j++, mask <<= 1) {
          WinSetForeColor((mask & 0x80) ? c1 : c2);
          WinDrawPixel(x+j, y+k);
        }
      }
    }

    for (i = 0; i < 1000; i++) {
      x = (i % 40) * 8;
      y = (i / 40 + 2) * 8;

      code = hardware->m1[0x3000 + i];
      caddr = code * 8;

      color = hardware->m1[0x3400 + i];
      c1 = hardware->color[color >> 4];
      c2 = hardware->color[color & 0x0F];

      for (k = 0; k < 8; k++) {
        mask = g->font[caddr + k];
        for (j = 0; j < 8; j++, mask <<= 1) {
          WinSetForeColor((mask & 0x80) ? c1 : c2);
          WinDrawPixel(x+j, y+k);
        }
      }
    }

    hardware->dirty = 0;
  }

  WinSetDrawWindow(prev);
  RctSetRectangle(&rect, 0, 0, hardware->dx, hardware->dy);
  WinCopyRectangle(hardware->screen_wh, NULL, &rect, hardware->x0, hardware->y0, winPaint);
  WinSetCoordinateSystem(kCoordinatesStandard);
}


UInt8 aquarius_readb(UInt16 a) {
  AquariusGlobals *g;
  UInt32 key, kbselect;
  UInt8 b;

  if (a == 0xFFFF)      // GAMBIARRA: CheckInput de console do CP/M
    return 1; 

  g = hardware->globals;

  switch (a & 0xFF) {
    case 0xFF:
      key = hardware->key;
      b = 0xFF;

      if (key) {
        kbselect = (a >> 8) ^ 0xFF;

        if (g->keyMap[key].line & kbselect)
          b &= g->keyMap[key].column;

        if (g->keyMap[key].shift && (g->keyMap[KEY_SHIFT].line & kbselect))
          b &= g->keyMap[KEY_SHIFT].column;

        if (g->ctrl && (g->keyMap[KEY_CTRL].line & kbselect))
          b &= g->keyMap[KEY_CTRL].column;
      }

      return b;
  }

  return 0x00;
}

void aquarius_writeb(UInt16 a, UInt8 b) {
  switch (a & 0xFF) {
    case 0xFC:
      snd_output(b & 0x01 ? 0xFF : 0x00);
      break;
    case 0xFF:	// scrambler
      hardware->m0[0xFF] = b;
  }
}

void aquarius_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  AquariusGlobals *globals;
  KeyMap *keyMap;
  UInt16 i, len;

  id = machine->id;
  hardware = _hardware;

  len = sizeof(prefs);
  MemSet(&prefs, len, 0);

  if (PrefGetAppPreferences(AppID, 128 + id, &prefs, &len, true) == noPreferenceFound) {
    prefs.pad = 0;
    PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, len, true);
  }

  hardware->key = 0;
  key = 0;

  if (!hardware->globals) {
    hardware->globals = MemPtrNew(sizeof(AquariusGlobals));
    MemSet(hardware->globals, sizeof(AquariusGlobals), 0);
  }
  globals = hardware->globals;

  globals->frame = 0;
  globals->ctrl = 0;

  aqfn = DmGet1Resource(AQUARIUS_FONT, AQUARIUS_FONT_ID);
  if (aqfn) {
    UInt8 *p = MemHandleLock(aqfn);
    globals->font = p;
  }

  keyMap = MemPtrNew(256 * sizeof(KeyMap));
  kbd_initmap(AquariusMap, keyMap);

  for (i = 0; i < 256; i++) {
    globals->keyMap[i].key = (keyMap[i].key);
    globals->keyMap[i].line = (keyMap[i].line);
    globals->keyMap[i].column = (keyMap[i].column);
    globals->keyMap[i].shift = (keyMap[i].shift);
    globals->keyMap[i].ctrl = (keyMap[i].ctrl);
  }
  MemPtrFree(keyMap);

  hardware->dx = 320;
  hardware->dy = 232;
  hardware->x0 = (hardware->display_width - hardware->dx) / 2;
  hardware->y0 = (hardware->display_height - hardware->dy - lowBorder*2) / 2;

  if (ramsize == RAM_4K)
    hardware->ramsize = (0x3000LU + RAM_4K * 4096 - 1);
  else
    hardware->ramsize = (0x3000LU + (RAM_4K|RAM_16K) * 4096 - 1);
}

void aquarius_init(void) {
  hardware->vsync_irq = 0;
}

void aquarius_finish(void) {
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);

  if (aqfn) {
    MemHandleUnlock(aqfn);
    DmReleaseResource(aqfn);
    aqfn = NULL;
  }

  if (hardware->globals) {
    MemPtrFree(hardware->globals);
    hardware->globals = NULL;
  }
}

void aquarius_key(UInt16 c) {
  AquariusGlobals *g;

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

Err aquarius_readbin(FileRef f, z80_Regs *z80, Hardware *hardware) {
  UInt32 size, r;
  Err err;

  if ((err = VFSFileSize(f, &size)) != 0)
    return -1;

  if (size != 0x4000)
    return -2;

  if ((err = VFSFileRead(f, size, &hardware->m0[0x4000], &r)) != 0)
    return -3;

  if (size != r)
    return -4;

  return 0;
}

AquariusPrefs *AquariusGetPrefs(void) {
  return &prefs;
}

/*
Boolean AquariusFormHandler(EventPtr event, Boolean *close)
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
