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
#include "jupiter.h"
#include "misc.h"
#include "gui.h"

typedef struct {
  UInt8 b1[3];
  char  name[10];
  UInt8 lenl, lenh;
  UInt8 startl, starth;
  UInt8 b2[11];
  UInt16 len1;
} DICFILE;

static UInt8 id;
static JupiterPrefs prefs;
static Hardware *hardware;
static UInt16 key;
static UInt8 buf[256];

ColorType JupiterColor[3] = {
  {0, 0x00, 0x00, 0x00},
  {1, 0xFF, 0xFF, 0xFF},
  {-1, 0, 0, 0}
};

UInt16 JupiterControl[9] = {stopCmd, restartCmd, 0,
                            loadSnapCmd, 0, 0,
                            0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef JupiterButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {KEY_UP,      internalFontID, "\x80",  C1, C2, C3},
  {KEY_DOWN,    internalFontID, "\x81", C1, C2, C3},
  {KEY_BREAK,   internalFontID, "BRK", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_LEFT,    internalFontID, "\x82", C1, C2, C3},
  {KEY_RIGHT,   internalFontID, "\x83", C1, C2, C3},
  {KEY_BACK,    internalFontID, "DEL", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_CLEAR,   internalFontID, "DLI", C1, C2, C3},
  {KEY_GRAPH,   internalFontID, "GR", C1, C2, C3},
  {KEY_INVERSE, internalFontID, "INV", C1, C2, C3}
};

static KeyMap JupiterMap[] = {
  {KEY_SHIFT, 0x01, 0xFE, 0, 0},
  {KEY_CTRL,  0x01, 0xFD, 0, 0},	// symbol shift
  {'z', 0x01, 0xFB, 0, 0},
  {'Z', 0x01, 0xFB, 1, 0},
  {':', 0x01, 0xFB, 0, 1},
  {'x', 0x01, 0xF7, 0, 0},
  {'X', 0x01, 0xF7, 1, 0},
  {163, 0x01, 0xF7, 0, 1},	// pound
  {'c', 0x01, 0xEF, 0, 0},
  {'C', 0x01, 0xEF, 1, 0},
  {'?', 0x01, 0xEF, 0, 1},

  {'a', 0x02, 0xFE, 0, 0},
  {'A', 0x02, 0xFE, 1, 0},
  {'~', 0x02, 0xFE, 0, 1},
  {'s', 0x02, 0xFD, 0, 0},
  {'S', 0x02, 0xFD, 1, 0},
  {'|', 0x02, 0xFD, 0, 1},
  {'d', 0x02, 0xFB, 0, 0},
  {'D', 0x02, 0xFB, 1, 0},
  {'\\', 0x02, 0xFB, 0, 1},
  {'f', 0x02, 0xF7, 0, 0},
  {'F', 0x02, 0xF7, 1, 0},
  {'{', 0x02, 0xF7, 0, 1},
  {'g', 0x02, 0xEF, 0, 0},
  {'G', 0x02, 0xEF, 1, 0},
  {'}', 0x02, 0xEF, 0, 1},

  {'q', 0x04, 0xFE, 0, 0},
  {'Q', 0x04, 0xFE, 1, 0},
  {'w', 0x04, 0xFD, 0, 0},
  {'W', 0x04, 0xFD, 1, 0},
  {'e', 0x04, 0xFB, 0, 0},
  {'E', 0x04, 0xFB, 1, 0},
  {'r', 0x04, 0xF7, 0, 0},
  {'R', 0x04, 0xF7, 1, 0},
  {'<', 0x04, 0xF7, 0, 1},
  {'t', 0x04, 0xEF, 0, 0},
  {'T', 0x04, 0xEF, 1, 0},
  {'>', 0x04, 0xEF, 0, 1},

  {'1', 0x08, 0xFE, 0, 0},
  {KEY_CLEAR, 0x08, 0xFE, 1, 0},	// delete line
  {'!', 0x08, 0xFE, 0, 1},
  {'2', 0x08, 0xFD, 0, 0},
  {'@', 0x08, 0xFD, 0, 1},
  {'3', 0x08, 0xFB, 0, 0},
  {'#', 0x08, 0xFB, 0, 1},
  {'4', 0x08, 0xF7, 0, 0},
  {KEY_INVERSE, 0x08, 0xF7, 1, 0},
  {'$', 0x08, 0xF7, 0, 1},
  {'5', 0x08, 0xEF, 0, 0},
  {KEY_LEFT, 0x08, 0xEF, 1, 0},
  {'%', 0x08, 0xEF, 0, 1},

  {'0', 0x10, 0xFE, 0, 0},
  {KEY_BACK, 0x10, 0xFE, 1, 0},
  {'_', 0x10, 0xFE, 0, 1},
  {'9', 0x10, 0xFD, 0, 0},
  {KEY_GRAPH, 0x10, 0xFD, 1, 0},
  {')', 0x10, 0xFD, 0, 1},
  {'8', 0x10, 0xFB, 0, 0},
  {KEY_RIGHT, 0x10, 0xFB, 1, 0},
  {'(', 0x10, 0xFB, 0, 1},
  {'7', 0x10, 0xF7, 0, 0},
  {KEY_DOWN, 0x10, 0xF7, 1, 0},
  {'\'', 0x10, 0xF7, 0, 1},
  {'6', 0x10, 0xEF, 0, 0},
  {KEY_UP, 0x10, 0xEF, 1, 0},
  {'&', 0x10, 0xEF, 0, 1},

  {'p', 0x20, 0xFE, 0, 0},
  {'P', 0x20, 0xFE, 1, 0},
  {'"', 0x20, 0xFE, 0, 1},
  {'o', 0x20, 0xFD, 0, 0},
  {'O', 0x20, 0xFD, 1, 0},
  {';', 0x20, 0xFD, 0, 1},
  {'i', 0x20, 0xFB, 0, 0},
  {169, 0x20, 0xFB, 1, 0},	// copyright
  {'I', 0x20, 0xFB, 1, 0},
  {'u', 0x20, 0xF7, 0, 0},
  {'U', 0x20, 0xF7, 1, 0},
  {']', 0x20, 0xF7, 0, 1},
  {'y', 0x20, 0xEF, 0, 0},
  {'Y', 0x20, 0xEF, 1, 0},
  {'[', 0x20, 0xEF, 0, 1},

  {KEY_ENTER, 0x40, 0xFE, 0, 0},
  {'l', 0x40, 0xFD, 0, 0},
  {'L', 0x40, 0xFD, 1, 0},
  {'=', 0x40, 0xFD, 0, 1},
  {'k', 0x40, 0xFB, 0, 0},
  {'K', 0x40, 0xFB, 1, 0},
  {'+', 0x40, 0xFB, 0, 1},
  {'j', 0x40, 0xF7, 0, 0},
  {'J', 0x40, 0xF7, 1, 0},
  {'-', 0x40, 0xF7, 0, 1},
  {'h', 0x40, 0xEF, 0, 0},
  {'H', 0x40, 0xEF, 1, 0},
  {'^', 0x40, 0xEF, 1, 1},

  {' ', 0x80, 0xFE, 0, 0},
  {KEY_BREAK, 0x80, 0xFE, 0, 1},
  {'m', 0x80, 0xFD, 0, 0},
  {'M', 0x80, 0xFD, 1, 0},
  {'.', 0x80, 0xFD, 0, 1},
  {'n', 0x80, 0xFB, 0, 0},
  {'N', 0x80, 0xFB, 1, 0},
  {',', 0x80, 0xFB, 0, 1},
  {'b', 0x80, 0xF7, 0, 0},
  {'B', 0x80, 0xF7, 1, 0},
  {'*', 0x80, 0xF7, 0, 1},
  {'v', 0x80, 0xEF, 0, 0},
  {'V', 0x80, 0xEF, 1, 0},
  {'/', 0x80, 0xEF, 0, 1},

  {0, 0, 0, 0, 0}
};

static void jupiter_video(void);

UInt32 jupiter_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_VSYNC:
      jupiter_video();
      jupiter_vsync(arg->a1);
      return 0;
    case IO_READB:
      return jupiter_readb(arg->a1);
    case IO_WRITEB:
      jupiter_writeb(arg->a1, arg->a2);
      return 0;
  }
  return 0;
}

UInt8 jupiter_readb(UInt16 a) {
  JupiterGlobals *g;
  UInt32 key, kbselect;
  UInt8 b;

  if (a == 0xFFFF)      // GAMBIARRA: CheckInput de console do CP/M
    return 1;

  g = hardware->globals;

  switch (a & 0xFF) {
    case 0xFE:
      key = hardware->key;
      b = 0xFF;

      if (key) {
        kbselect = (a >> 8) ^ 0xFF;

        if (g->keyMap[key].line & kbselect) {
          b &= g->keyMap[key].column;
        }

        if (g->keyMap[key].shift && (g->keyMap[KEY_SHIFT].line & kbselect)) {
          b &= g->keyMap[KEY_SHIFT].column;
        }

        // symbol shift
        if (g->keyMap[key].ctrl && (g->keyMap[KEY_CTRL].line & kbselect)) {
          b &= g->keyMap[KEY_CTRL].column;
        }
      }

      if (!(a & 0x8000)) {
        snd_output(0x00);
      }

      return b;
  }

  return 0x00;
}

void jupiter_writeb(UInt16 a, UInt8 b) {
  switch (a & 0xFF) {
    case 0xFE:
      snd_output(0xFF);
  }
}

static void jupiter_video(void) {
  UInt32 height, x, y, i, j, k, caddr;
  UInt8 code, c1, c2, mask;
  RectangleType rect;
  WinHandle prev;
  JupiterGlobals *g;
  Err err;

  g = hardware->globals;
  g->frame++;

  if (!hardware->dirty && !g->border)
    return;

  if (g->frame & 1)
    return;

  if (hardware->screen_wh == NULL) {
    hardware->screen_wh = WinCreateOffscreenWindow(256, 192, nativeFormat, &err);
  }

  WinSetCoordinateSystem(kCoordinatesDouble);
  prev = WinSetDrawWindow(hardware->screen_wh);
  height = hardware->display_height - lowBorder * 2;

  if (g->border) {
    c1 = hardware->color[0];
    WinSetBackColor(c1);
    RctSetRectangle(&rect, 0, 0, hardware->display_width, height);
    WinEraseRectangle(&rect, 0);
    g->border = 0;
  }

  if (hardware->dirty) {
    for (i = 0; i < 768; i++) {
      x = (i % 32) * 8;
      y = (i / 32) * 8;

      code = hardware->m1[0x2400 + i];
      caddr = 0x2C00 + (code & 0x7F) * 8;

      if (code & 0x80) {
        c1 = hardware->color[0];
        c2 = hardware->color[1];
      } else {
        c1 = hardware->color[1];
        c2 = hardware->color[0];
      }

      for (k = 0; k < 8; k++) {
        mask = hardware->m1[caddr + k];
        for (j = 0; j < 8; j++, mask <<= 1) {
          WinSetForeColor((mask & 0x80) ? c1 : c2);
          WinDrawPixel(x+j, y+k);
        }
      }
    }

    WinSetDrawWindow(prev);
    RctSetRectangle(&rect, 0, 0, 256, 192);
    WinCopyRectangle(hardware->screen_wh, NULL, &rect, hardware->x0, hardware->y0, winPaint);
    WinSetCoordinateSystem(kCoordinatesStandard);

    hardware->dirty = 0;
  }
}

void jupiter_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  JupiterGlobals *globals;
  KeyMap *keyMap;
  UInt16 len, i;

  id = machine->id;
  hardware = _hardware;

  len = sizeof(prefs);
  MemSet(&prefs, len, 0);

  if (PrefGetAppPreferences(AppID, 128 + id, &prefs, &len, true) ==
         noPreferenceFound) {
    prefs.pad = 0;
    PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, len, true);
  }

  hardware->key = 0;
  key = 0;

  if (!hardware->globals) {
    hardware->globals = MemPtrNew(sizeof(JupiterGlobals));
    MemSet(hardware->globals, sizeof(JupiterGlobals), 0);
  }
  globals = hardware->globals;

  globals->frame = 0;
  globals->border = 1;

  keyMap = MemPtrNew(256 * sizeof(KeyMap));
  kbd_initmap(JupiterMap, keyMap);

  for (i = 0; i < 256; i++) {
    globals->keyMap[i].key = (keyMap[i].key);
    globals->keyMap[i].line = (keyMap[i].line);
    globals->keyMap[i].column = (keyMap[i].column);
    globals->keyMap[i].shift = (keyMap[i].shift);
    globals->keyMap[i].ctrl = (keyMap[i].ctrl);
  }
  MemPtrFree(keyMap);

  hardware->x0 = (hardware->display_width - 256) / 2;
  hardware->y0 = (hardware->display_height - 192 - lowBorder*2) / 2;
  hardware->dx = 256;
  hardware->dy = 192;
}

void jupiter_finish(void) {
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);

  if (hardware->globals) {
    MemPtrFree(hardware->globals);
    hardware->globals = NULL;
  }
}

void jupiter_key(UInt16 c) {
  key = c;
  hardware->key = key;
}

/*
STKBOT $3C37 (15415)  The address of the next byte into
                      which anything will be enclosed in the
                      dictionary, i.e. one byte past the present end
                      of the dictionary.
                      *'HERE'* is equivalent to 15415 @.
DICT   $3C39 (15417)  The address of the length field in the
                      newest word in the dictionary. If that length
                      field is correctly filled in then DICT may be 0.
SPARE  $3C3B (15419)  2 bytes. The address of the first byte past the
                      top of the stack.
*/

Err jupiter_readdic(FileRef f, z80_Regs *z80, Hardware *hardware) {
  DICFILE dic;
  UInt32 addr, size, aux, hsize, r;
  Err err;

  hsize = 30;

  if ((err = VFSFileSize(f, &size)) != 0)
    return -1;

  if (size < hsize)
    return -2;

  if ((err = VFSFileRead(f, hsize, &dic, &r)) != 0)
    return -3;

  if (r != hsize)
    return -4;

  if (dic.b1[0] != 0x1A || dic.b1[1] != 0x00)
    return -5;

  addr = (dic.starth * 256 + dic.startl) & 0xFFFF;
  size = (dic.lenh * 256 + dic.lenl) & 0xFFFF;

  if ((addr + size) >= 0x8000)
    return -6;

  if ((err = VFSFileRead(f, size, &hardware->m1[addr], &r)) != 0)
    return -7;
  if (r != size)
    return -8;

  // here
  aux  = (addr + size) & 0xFFFF;
  hardware->m1[0x3C37] = aux & 0xFF;
  hardware->m1[0x3C38] = (aux & 0xFF00) >> 8;

  hardware->m1[0x3C39] = 0;
  hardware->m1[0x3C3A] = 0;

  // stack top
  aux = (hardware->m1[0x3C3B] + hardware->m1[0x3C3C] * 256) & 0xFFFF;
  aux = (aux + size) & 0xFFFF;
  hardware->m1[0x3C3B] = aux & 0xFF;
  hardware->m1[0x3C3C] = (aux & 0xFF00) >> 8;

  return 0;
}

Err jupiter_readace(FileRef f, z80_Regs *z80, Hardware *hardware) {
  UInt32 i, j, n;
  UInt16 addr, count;
  Int16 s;
  UInt8 *p, b;
  Err err;

  p = hardware->m1;
  i = n = 0;
  count = 0;

  for (s = 0, addr = 0x2000; s >= 0 && addr < 0x8000;) {
    if (i == n) {
      if ((err = VFSFileRead(f, sizeof(buf), buf, &n)) != 0)
        return -2;
      if (n == 0)
        break;
      i = 0;
    }
    b = buf[i++];

    // ED 00           : End marker
    // ED 01 ED        : 0xED
    // ED <cnt> <byte> : repeat <byte> count <cnt:3-240> times
    // <byte>          : <byte>

    switch (s) {
      case 0:	// procurando 0xED
        if (b == 0xED)
          s = 1;
        else
          p[addr++] = b;
        break;
      case 1:	// apos 0xED
        switch (b) {
          case 0x00:
            s = -1;
            break;
          case 0x01:
            s = 2;
            break;
          case 0x02:
            return -3;
          default:
            if (b > 240)  // ???
              return -4;
            count = b;
            s = 3;
            break;
        }
        break;
      case 2:	// apos 0xED 0x01
        if (b != 0xED)
          return -5;
        p[addr++] = b;
        s = 0;
        break;
      case 3:	// apos 0xED <cnt>
        for (j = 0; j < count && addr < 0x8000; j++)
          p[addr++] = b;
        s = 0;
    }
  }

  //z80->pc = LITTLE_ENDIANIZE_INT32(addr);

  return 0;
}

JupiterPrefs *JupiterGetPrefs(void) {
  return &prefs;
}

/*
Boolean JupiterFormHandler(EventPtr event, Boolean *close)
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
