#include <PalmOS.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "machine.h"
#include "kbd.h"
#include "endian.h"
#include "apple.h"
#include "misc.h"
#include "gui.h"

static void apple_setgp(void) SECTION("machine");

static UInt8 id;
static ApplePrefs prefs;
static Hardware *hardware;
static UInt8 flash_count;
static Boolean flash_status;
static Boolean graphic, hires, mixed;
static Boolean border;
static UInt8 page;
static UInt16 start;
static UInt16 lastkey;
static Boolean dirty;
static Boolean ctrl;
static UInt16 key, joyx, joyy;

ColorType AppleColor[17] = {
  {0,  0x00, 0x00, 0x00}, // Black
  {1,  0xD0, 0x00, 0x30}, // Dark Red
  {2,  0x00, 0x00, 0x90}, // Dark Blue
  {3,  0xD0, 0x20, 0xD0}, // Purple
  {4,  0x00, 0x70, 0x20}, // Dark Green
  {5,  0x50, 0x50, 0x50}, // Dark Grey
  {6,  0x20, 0x20, 0xF0}, // Medium Blue
  {7,  0x60, 0xA0, 0xF0}, // Light Blue
  {8,  0x80, 0x50, 0x00}, // Brown
  {9,  0xF0, 0x60, 0x00}, // Orange
  {10, 0xA0, 0xA0, 0xA0}, // Light Grey
  {11, 0xF0, 0x90, 0x80}, // Pink
  {12, 0x10, 0xD0, 0x00}, // Light Green
  {13, 0xF0, 0xF0, 0x00}, // Yellow
  {14, 0x40, 0xF0, 0x90}, // Aquamarine
  {15, 0xF0, 0xF0, 0xF0}, // White
  {-1, 0, 0, 0}
};

static UInt16 AppleTextAddr[24] = {
  0x0000, 0x0080, 0x0100, 0x0180, 0x0200, 0x0280, 0x0300, 0x0380,
  0x0028, 0x00A8, 0x0128, 0x01A8, 0x0228, 0x02A8, 0x0328, 0x03A8,
  0x0050, 0x00D0, 0x0150, 0x01D0, 0x0250, 0x02D0, 0x0350, 0x03D0
};

static UInt16 AppleLoresAddr[192] = {
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
  0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 
  0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 0x0100, 
  0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 0x0180, 
  0x0200, 0x0200, 0x0200, 0x0200, 0x0200, 0x0200, 0x0200, 0x0200, 
  0x0280, 0x0280, 0x0280, 0x0280, 0x0280, 0x0280, 0x0280, 0x0280, 
  0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 0x0300, 
  0x0380, 0x0380, 0x0380, 0x0380, 0x0380, 0x0380, 0x0380, 0x0380, 
  0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 0x0028, 
  0x00A8, 0x00A8, 0x00A8, 0x00A8, 0x00A8, 0x00A8, 0x00A8, 0x00A8, 
  0x0128, 0x0128, 0x0128, 0x0128, 0x0128, 0x0128, 0x0128, 0x0128, 
  0x01A8, 0x01A8, 0x01A8, 0x01A8, 0x01A8, 0x01A8, 0x01A8, 0x01A8, 
  0x0228, 0x0228, 0x0228, 0x0228, 0x0228, 0x0228, 0x0228, 0x0228, 
  0x02A8, 0x02A8, 0x02A8, 0x02A8, 0x02A8, 0x02A8, 0x02A8, 0x02A8, 
  0x0328, 0x0328, 0x0328, 0x0328, 0x0328, 0x0328, 0x0328, 0x0328, 
  0x03A8, 0x03A8, 0x03A8, 0x03A8, 0x03A8, 0x03A8, 0x03A8, 0x03A8, 
  0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 0x0050, 
  0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 0x00D0, 
  0x0150, 0x0150, 0x0150, 0x0150, 0x0150, 0x0150, 0x0150, 0x0150, 
  0x01D0, 0x01D0, 0x01D0, 0x01D0, 0x01D0, 0x01D0, 0x01D0, 0x01D0, 
  0x0250, 0x0250, 0x0250, 0x0250, 0x0250, 0x0250, 0x0250, 0x0250, 
  0x02D0, 0x02D0, 0x02D0, 0x02D0, 0x02D0, 0x02D0, 0x02D0, 0x02D0, 
  0x0350, 0x0350, 0x0350, 0x0350, 0x0350, 0x0350, 0x0350, 0x0350, 
  0x03D0, 0x03D0, 0x03D0, 0x03D0, 0x03D0, 0x03D0, 0x03D0, 0x03D0
};

static UInt8 AppleLowColor[192] = {
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 
  1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0
};

static UInt16 AppleHiresAddr[192] = {
  0x0000, 0x0400, 0x0800, 0x0C00, 0x1000, 0x1400, 0x1800, 0x1C00, 
  0x0080, 0x0480, 0x0880, 0x0C80, 0x1080, 0x1480, 0x1880, 0x1C80, 
  0x0100, 0x0500, 0x0900, 0x0D00, 0x1100, 0x1500, 0x1900, 0x1D00, 
  0x0180, 0x0580, 0x0980, 0x0D80, 0x1180, 0x1580, 0x1980, 0x1D80, 
  0x0200, 0x0600, 0x0A00, 0x0E00, 0x1200, 0x1600, 0x1A00, 0x1E00, 
  0x0280, 0x0680, 0x0A80, 0x0E80, 0x1280, 0x1680, 0x1A80, 0x1E80, 
  0x0300, 0x0700, 0x0B00, 0x0F00, 0x1300, 0x1700, 0x1B00, 0x1F00, 
  0x0380, 0x0780, 0x0B80, 0x0F80, 0x1380, 0x1780, 0x1B80, 0x1F80, 
  0x0028, 0x0428, 0x0828, 0x0C28, 0x1028, 0x1428, 0x1828, 0x1C28, 
  0x00A8, 0x04A8, 0x08A8, 0x0CA8, 0x10A8, 0x14A8, 0x18A8, 0x1CA8, 
  0x0128, 0x0528, 0x0928, 0x0D28, 0x1128, 0x1528, 0x1928, 0x1D28, 
  0x01A8, 0x05A8, 0x09A8, 0x0DA8, 0x11A8, 0x15A8, 0x19A8, 0x1DA8, 
  0x0228, 0x0628, 0x0A28, 0x0E28, 0x1228, 0x1628, 0x1A28, 0x1E28, 
  0x02A8, 0x06A8, 0x0AA8, 0x0EA8, 0x12A8, 0x16A8, 0x1AA8, 0x1EA8, 
  0x0328, 0x0728, 0x0B28, 0x0F28, 0x1328, 0x1728, 0x1B28, 0x1F28, 
  0x03A8, 0x07A8, 0x0BA8, 0x0FA8, 0x13A8, 0x17A8, 0x1BA8, 0x1FA8, 
  0x0050, 0x0450, 0x0850, 0x0C50, 0x1050, 0x1450, 0x1850, 0x1C50, 
  0x00D0, 0x04D0, 0x08D0, 0x0CD0, 0x10D0, 0x14D0, 0x18D0, 0x1CD0, 
  0x0150, 0x0550, 0x0950, 0x0D50, 0x1150, 0x1550, 0x1950, 0x1D50, 
  0x01D0, 0x05D0, 0x09D0, 0x0DD0, 0x11D0, 0x15D0, 0x19D0, 0x1DD0, 
  0x0250, 0x0650, 0x0A50, 0x0E50, 0x1250, 0x1650, 0x1A50, 0x1E50, 
  0x02D0, 0x06D0, 0x0AD0, 0x0ED0, 0x12D0, 0x16D0, 0x1AD0, 0x1ED0, 
  0x0350, 0x0750, 0x0B50, 0x0F50, 0x1350, 0x1750, 0x1B50, 0x1F50, 
  0x03D0, 0x07D0, 0x0BD0, 0x0FD0, 0x13D0, 0x17D0, 0x1BD0, 0x1FD0
};

static UInt8 *AppleEvenHiresColor = NULL;
static UInt8 *AppleOddHiresColor = NULL;

UInt16 AppleControl[9] = {stopCmd, restartCmd, 0,
                          0, 0, 0,
                          0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef AppleButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {KEY_UP,    internalFontID, "\x80",  C1, C2, C3},
  {KEY_DOWN,  internalFontID, "\x81", C1, C2, C3},
  {KEY_BREAK, internalFontID, "BRK", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_LEFT,  internalFontID, "\x82", C1, C2, C3},
  {KEY_RIGHT, internalFontID, "\x83", C1, C2, C3},
  {KEY_CTRL,  internalFontID, "CTL", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {KEY_RESET, internalFontID, "RST", C1, C2, C3}
};

static void apple_border(UInt8 *s) SECTION("machine");
static void apple_textline(UInt8 *s, UInt16 base, UInt16 i0, UInt16 i1) SECTION("machine");
static void apple_text(Boolean dirty) SECTION("machine");
static void apple_lores(Boolean dirty) SECTION("machine");
static void apple_hires(Boolean dirty) SECTION("machine");

UInt32 apple_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_VSYNC:
      apple_vsync(arg->a1);
      return 0;
    case IO_READB:
      return apple_readb(arg->a1);
    case IO_WRITEB:
      apple_writeb(arg->a1, arg->a2);
      return 0;
  }
  return 0;
}

void apple_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  UInt16 len;

  flash_count = 0;
  flash_status = false;
  graphic = false;
  hires = false;
  mixed = false;
  border = true;
  page = 0;
  start = 0x400;
  dirty = true;

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

  hardware->x0 = (hardware->display_width  - 280) / 2;
  hardware->y0 = (hardware->display_height - 192 - lowBorder * 2) / 2;

  ctrl = false;
  joyx = joyy = 32;
  key = lastkey = 0;

  if (!AppleEvenHiresColor) {
    MemHandle hcol;
    UInt8 *p, nibble;
    UInt16 i, j, k, m;
    Boolean low;

    hcol = DmGet1Resource(APPLE_HCOL, APPLE_HCOL_ID);
    p = MemHandleLock(hcol);

    AppleEvenHiresColor = MemPtrNew(1024 * 8);
    MemSet(AppleEvenHiresColor, 1024 * 8, hardware->color[0]);

    AppleOddHiresColor = MemPtrNew(1024 * 8);
    MemSet(AppleOddHiresColor, 1024 * 8, hardware->color[0]);

    k = 0;
    m = 0;
    low = true;

    for (i = 0; i < 1024; i++) {
      for (j = 0; j < 8; j++) {
        if (low)
          nibble = p[m] >> 4;
        else {
          nibble = p[m] & 0x0F;
          m++;
        }

        AppleEvenHiresColor[k] = hardware->color[nibble];
        AppleOddHiresColor[k] = hardware->color[nibble];

        k++;
        low = !low;
      }
    }

    MemHandleUnlock(hcol);
    DmReleaseResource(hcol);
  }
}

void apple_init(void) {
  apple_reset();
}

void apple_reset(void) {
  apple_writeb(0x51, 0);    // TXTSET
  apple_writeb(0x52, 0);    // MIXCLR
  apple_writeb(0x54, 0);    // PAGE1
  apple_writeb(0x56, 0);    // LORES
}

void apple_finish(void) {
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);

  if (AppleEvenHiresColor) {
    MemPtrFree(AppleEvenHiresColor);
    AppleEvenHiresColor = NULL;

    MemPtrFree(AppleOddHiresColor);
    AppleOddHiresColor = NULL;
  }
}

void apple_key(UInt16 c) {
  switch (c) {
    case KEY_CTRL:
      ctrl = ~ctrl;
      return;
    case KEY_BREAK:
      ctrl = true;
      c = 0x03;
      lastkey = c | 0x80;
      break;
    case KEY_ENTER:
      c = 13;
      lastkey = c | 0x80;
      break;
    case 0:
      ctrl = false;
      break;
    default:
      if (c >= 'a' && c <= 'z')
        c &= 0xDF;

      if (ctrl) {
        if (c >= 'A' && c <= 'Z')
          c = c - 'A' + 1;
        ctrl = false;
      }

      lastkey = c | 0x80;
  }

  key = c;
}

void apple_joystick(UInt16 x, UInt16 y) {
  joyx = x;
  joyy = y;
}

void apple_video(Boolean dirty) {
  if (!graphic)
    apple_text(dirty);
  else if (!hires)
    apple_lores(dirty);
  else
    apple_hires(dirty);

  flash_count++;

  if (flash_count == 16) {
    flash_count = 0;
    flash_status = !flash_status;
  }
}

static void apple_setgp(void) {
  UInt32 begin, end;

  if (graphic && hires) {
    if (mixed) {
      if (page == 1) {
        begin = 0x0800;
        end = 0x6000;
      } else {
        begin = 0x0400;
        end = 0x4000;
      }
    } else {
      if (page == 1)
        begin = 0x4000;
      else
        begin = 0x2000;
      end = begin + 0x2000;
    }
  } else {
    if (page == 1)
      begin = 0x0800;
    else
      begin = 0x0400;
    end = begin + 0x0400;
  }

  io_setgp(begin, end);
}

UInt8 apple_readb(UInt16 a) {
  switch (a & 0xFF) {
    case 0x00:	// KBD
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
      return lastkey;
    case 0x10:	// KBDSTRB
      lastkey &= 0x7F;
      return lastkey;
    case 0x1A:	// RDTEXT
      return graphic ? 0x00 : 0x80;
    case 0x1B:	// RDMIXED
      return mixed ? 0x80 : 0x00;
    case 0x1C:	// RDPAGE2
      return page == 1 ? 0x80 : 0x00;
    case 0x1D:	// RDHIRES
      return hires ? 0x80 : 0x00;
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x53:
    case 0x54:
    case 0x55:
    case 0x56:
    case 0x57:
      apple_writeb(a, 0);
  }

  return 0x00;
}

void apple_writeb(UInt16 a, UInt8 b) {
  switch (a & 0xFF) {
    case 0x10:	// KBDSTRB
      lastkey &= 0x7F;
      break;
    case 0x50:	// TXTCLR
      graphic = true;
      apple_setgp();
      break;
    case 0x51:	// TXTSET
      graphic = false;
      apple_setgp();
      break;
    case 0x52:	// MIXCLR
      mixed = false;
      apple_setgp();
      break;
    case 0x53:	// MIXSET
      mixed = true;
      apple_setgp();
      break;
    case 0x54:	// PAGE1
      page = 0;
      apple_setgp();
      break;
    case 0x55:	// PAGE2
      page = 1;
      apple_setgp();
      break;
    case 0x56:	// LORES
      hires = false;
      apple_setgp();
      break;
    case 0x57:	// HIRES
      hires = true;
      apple_setgp();
  }
}

static void apple_border(UInt8 *s) {
  Coord i;

  for (i = 0; i < hardware->y0; i++, s += hardware->display_width) {
    MemSet(s, hardware->display_width, hardware->color[0]);
  }

  for (; i < hardware->y0 + 192; i++) {
    MemSet(s, hardware->x0, hardware->color[0]);
    s += hardware->x0 + 280;
    MemSet(s, hardware->x0, hardware->color[0]);
    s += hardware->x0;
  }
  
  for (; i < hardware->y0 + 192 + hardware->y0; i++, s += hardware->display_width) {
    MemSet(s, hardware->display_width, hardware->color[0]);
  }

  border = false;
}

static void apple_textline(UInt8 *s, UInt16 base, UInt16 i0, UInt16 i1) {
  UInt16 i, j, addr, c, x, y, y0;

  FntSetFont(fontID);
  y0 = (hardware->display_height - 192 - lowBorder * 2) / 2 + i0 * 8;

  for (i = i0, y = y0; i < i1; i++, y += 8) {
    addr = AppleTextAddr[i];
    x = (hardware->display_width - 280) / 2;
    for (j = 0; j < 40; j++, x += 7) {
      c = hardware->m0[base + addr + j];
      if (c < 0x40) {
        WinSetTextColor(hardware->color[0]);
        WinSetBackColor(hardware->color[15]);
      } else if (c < 0x80) {
        if (flash_status) {
          WinSetTextColor(hardware->color[15]);
          WinSetBackColor(hardware->color[0]);
        } else {
          WinSetTextColor(hardware->color[0]);
          WinSetBackColor(hardware->color[15]);
        }
        c -= 0x40;
      } else if (c < 0xA0) {
        WinSetTextColor(hardware->color[15]);
        WinSetBackColor(hardware->color[0]);
        c -= 0x80;
      } else if (c < 0xC0) {
        WinSetTextColor(hardware->color[15]);
        WinSetBackColor(hardware->color[0]);
        c -= 0x80;
      } else if (c < 0xE0) {
        WinSetTextColor(hardware->color[15]);
        WinSetBackColor(hardware->color[0]);
        c -= 0xC0;
      } else {
        WinSetTextColor(hardware->color[15]);
        WinSetBackColor(hardware->color[0]);
        c -= 0xA0;
      }
      WinPaintChar(c, x, y);
    }
  }
}

static void apple_text(Boolean dirty) {
  UInt16 addr;
  UInt8 *s;

  start = page ? 0x0800 : 0x0400;

  if (flash_count == 15) {
    for (addr = 0; !dirty && addr < 0x400; addr++) {
      if (hardware->m0[start + addr] >= 0x40 &&
          hardware->m0[start + addr] < 0x80)
        dirty = true;
    }
  }

  if (border || dirty) {
    WinSetCoordinateSystem(kCoordinatesDouble);
    s = WinScreenLock(winLockCopy);
  
    if (border)
      apple_border(s);

    if (dirty)
      apple_textline(s, start, 0, 24);

    WinScreenUnlock();
    WinSetCoordinateSystem(kCoordinatesStandard);
  }
}

static void apple_lores(Boolean dirty) {
  UInt16 i, j, addr, delta, c, i1;
  UInt8 *s, b;

  start = page ? 0x0800 : 0x0400;

  if (flash_count == 15) {
    for (addr = 0; !dirty && addr < 0x400; addr++) {
      if (hardware->m0[start + addr] >= 0x40 && hardware->m0[start + addr] < 0x80) {
        dirty = true;
      }
    }
  }

  if (border || dirty) {
    WinSetCoordinateSystem(kCoordinatesDouble);
    s = WinScreenLock(winLockCopy);
  
    if (border)
      apple_border(s);

    if (dirty) {
      i1 = mixed ? 20*8 : 24*8;
      delta = hardware->display_width - 280;

      s += hardware->y0 * hardware->display_width + hardware->x0;

      for (i = 0; i < i1; i++) {
        addr = AppleLoresAddr[i];
        for (j = 0; j < 40; j++) {
          c = hardware->m0[start + addr + j];
          b = AppleLowColor[i] ? hardware->color[c & 0x0F] : hardware->color[c >> 4];
          MemSet(s, 7, b);
          s += 7;
        }
        s += delta;
      }

      if (mixed) {
        apple_textline(s, start, 20, 24);
      }
    }

    WinScreenUnlock();
    WinSetCoordinateSystem(kCoordinatesStandard);
  }
}

static void apple_hires(Boolean dirty) {
  UInt16 i, j, tstart, addr, laddr, delta, c, c1, c2, c3, i1, x0, y0;
  UInt8 *s;

  start  = page ? 0x4000 : 0x2000;
  tstart = page ? 0x0800 : 0x0400;

  if (flash_count == 15)
    for (addr = 0; !dirty && addr < 0x400; addr++)
      if (hardware->m0[tstart + addr] >= 0x40 && hardware->m0[tstart + addr] < 0x80)
        dirty = true;

  if (border || dirty) {
    WinSetCoordinateSystem(kCoordinatesDouble);
    s = WinScreenLock(winLockCopy);
  
    if (border)
      apple_border(s);
    
    if (dirty) {
      i1 = mixed ? 20*8 : 24*8;
      x0 = (hardware->display_width  - 280) / 2;
      y0 = (hardware->display_height - 192 - lowBorder * 2) / 2;
      delta = hardware->display_width - 280; 

      s += y0 * hardware->display_width + x0;

      for (i = 0; i < i1; i++) {
        addr = AppleHiresAddr[i];
        laddr = start + addr;
        c1 = 0;
        c2 = hardware->m0[laddr];
        c3 = hardware->m0[laddr + 1];

        for (j = 0; j < 40; j += 2) {
          c = ((c1 & 0x40) >> 6) | (c2 << 1) | ((c3 & 0x01) << 9);
          MemMove(s, &AppleEvenHiresColor[c * 8], 7);
          s += 7;

          laddr++;
          c1 = c2;
          c2 = c3;
          c3 = hardware->m0[laddr + 1];

          c = ((c1 & 0x40) >> 6) | (c2 << 1) | ((c3 & 0x01) << 9);
          MemMove(s, &AppleOddHiresColor[c * 8], 7);
          s += 7;

          laddr++;
          c1 = c2;
          c2 = c3;
          c3 = hardware->m0[laddr + 1];
        }
        s += delta;
      }

      if (mixed)
        apple_textline(s, tstart, 20, 24);
    }

    WinScreenUnlock();
    WinSetCoordinateSystem(kCoordinatesStandard);

  }
}

ApplePrefs *AppleGetPrefs(void) {
  return &prefs;
}

/*
Boolean AppleFormHandler(EventPtr event, Boolean *close)
{
  FormPtr frm;
  ApplePrefs *prefs;
  Boolean handled;

  frm = FrmGetActiveForm();
  prefs = AppleGetPrefs();
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
