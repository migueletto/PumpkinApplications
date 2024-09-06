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
#include "m6502.h"
#include "vic20.h"
#include "misc.h"
#include "gui.h"

#define CHAR_WIDTH	8
#define CHAR_HEIGHT	8

#define NUM_LINES	23
#define NUM_COLS	22

#define VIC6560_CLOCK 1022727

#define CHAR_WIDTH  8
#define CHAR_HEIGHT 8

#define INT_CA2   0x01
#define INT_CA1   0x02
#define INT_SR    0x04
#define INT_CB2   0x08
#define INT_CB1   0x10
#define INT_T2    0x20
#define INT_T1    0x40
#define INT_ANY   0x80

#define min(a,b) ((a) < (b) ? (a) : (b))

static UInt8 id;
static VicPrefs prefs;
static Hardware *hardware;
static UInt8 *video = NULL;
static UInt16 key;

ColorType VicColor[17] = {
  {0,  0x00, 0x00, 0x00},	// black
  {1,  0xff, 0xff, 0xff},	// white
  {2,  0xf0, 0x00, 0x00},	// read
  {3,  0x00, 0xf0, 0xf0},	// cyan
  {4,  0x60, 0x00, 0x60},	// purple
  {5,  0x00, 0xa0, 0x00},	// green
  {6,  0x00, 0x00, 0xf0},	// blue
  {7,  0xd0, 0xd0, 0x00},	// yellow
  {8,  0xc0, 0xa0, 0x00},	// orange
  {9,  0xff, 0xa0, 0x00},	// light orange
  {10, 0xf0, 0x80, 0x80},	// pink
  {11, 0x00, 0xff, 0xff},	// light cyan
  {12, 0xff, 0x00, 0xff},	// light violet
  {13, 0x00, 0xff, 0x00},	// light green
  {14, 0x00, 0xa0, 0xff},	// light blue
  {15, 0xff, 0xff, 0x00},	// light yellow
  {-1, 0, 0, 0}
};

UInt16 VicControl[9] = {stopCmd, restartCmd, configCmd,
                        loadSnapCmd, 0, 0,
                        0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef VicButtonDef[12] = {
  {KEY_F1,    internalFontID, "F1",  C1, C2, C3},
  {KEY_F3,    internalFontID, "F3",  C1, C2, C3},
  {KEY_F5,    internalFontID, "F5",  C1, C2, C3},
  {KEY_F7,    internalFontID, "F7",  C1, C2, C3},
  {KEY_UP,    internalFontID, "\x80", C1, C2, C3},
  {KEY_DOWN,  internalFontID, "\x81", C1, C2, C3},
  {KEY_LEFT,  internalFontID, "\x82", C1, C2, C3},
  {KEY_RIGHT, internalFontID, "\x83", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {KEY_CTRL,  internalFontID, "CTL", C1, C2, C3},
  {KEY_RESET, internalFontID, "RST", C1, C2, C3}
};

static KeyMap VicMap[] = {
  {'1', 0x01, 0xFE, 0, 0},
  {'!', 0x01, 0xFE, 1, 0},
  {'3', 0x01, 0xFD, 0, 0},
  {'#', 0x01, 0xFD, 1, 0},
  {'5', 0x01, 0xFB, 0, 0},
  {'%', 0x01, 0xFB, 1, 0},
  {'7', 0x01, 0xF7, 0, 0},
  {'\'', 0x01, 0xF7, 1, 0},
  {'9', 0x01, 0xEF, 0, 0},
  {')', 0x01, 0xEF, 1, 0},
  {'+', 0x01, 0xDF, 0, 0},
  {163, 0x01, 0xBF, 0, 0},	// pound
  {KEY_BACK, 0x01, 0x7F, 0, 0},

  {KEY_LEFT, 0x02, 0xFE, 0, 0},
  {'W', 0x02, 0xFD, 0, 0},
  {'R', 0x02, 0xFB, 0, 0},
  {'Y', 0x02, 0xF7, 0, 0},
  {'I', 0x02, 0xEF, 0, 0},
  {'P', 0x02, 0xDF, 0, 0},
  {'*', 0x02, 0xBF, 0, 0},
  {KEY_ENTER, 0x02, 0x7F, 0, 0},

  {KEY_CTRL, 0x04, 0xFE, 0, 0},
  {'A', 0x04, 0xFD, 0, 0},
  {'D', 0x04, 0xFB, 0, 0},
  {'G', 0x04, 0xF7, 0, 0},
  {'J', 0x04, 0xEF, 0, 0},
  {'L', 0x04, 0xDF, 0, 0},
  {';', 0x04, 0xBF, 0, 0},
  {']', 0x04, 0xBF, 1, 0},
  {KEY_RIGHT, 0x04, 0x7F, 0, 0},

  {KEY_SHIFT, 0x08, 0xFD, 0, 0},
  {'X', 0x08, 0xFB, 0, 0},
  {'V', 0x08, 0xF7, 0, 0},
  {'N', 0x08, 0xEF, 0, 0},
  {',', 0x08, 0xDF, 0, 0},
  {'<', 0x08, 0xDF, 1, 0},
  {'/', 0x08, 0xBF, 0, 0},
  {'?', 0x08, 0xBF, 1, 0},
  {KEY_DOWN, 0x08, 0x7F, 0, 0},

  {' ', 0x10, 0xFE, 0, 0},
  {'Z', 0x10, 0xFD, 0, 0},
  {'C', 0x10, 0xFB, 0, 0},
  {'B', 0x10, 0xF7, 0, 0},
  {'M', 0x10, 0xEF, 0, 0},
  {'.', 0x10, 0xDF, 0, 0},
  {'>', 0x10, 0xDF, 1, 0},
  {KEY_F1, 0x10, 0x7F, 0, 0},

  {'S', 0x20, 0xFD, 0, 0},
  {'F', 0x20, 0xFB, 0, 0},
  {'H', 0x20, 0xF7, 0, 0},
  {'K', 0x20, 0xEF, 0, 0},
  {':', 0x20, 0xDF, 0, 0},
  {'[', 0x20, 0xDF, 1, 0},
  {'=', 0x20, 0xBF, 0, 0},
  {KEY_F3, 0x20, 0x7F, 0, 0},

  {'Q', 0x40, 0xFE, 0, 0},
  {'E', 0x40, 0xFD, 0, 0},
  {'T', 0x40, 0xFB, 0, 0},
  {'U', 0x40, 0xF7, 0, 0},
  {'O', 0x40, 0xEF, 0, 0},
  {'@', 0x40, 0xDF, 0, 0},
  {KEY_UP, 0x40, 0xBF, 0, 0},
  {KEY_F5, 0x40, 0x7F, 0, 0},

  {'2', 0x80, 0xFE, 0, 0},
  {'"', 0x80, 0xFE, 1, 0},
  {'4', 0x80, 0xFD, 0, 0},
  {'$', 0x80, 0xFD, 1, 0},
  {'6', 0x80, 0xFB, 0, 0},
  {'&', 0x80, 0xFB, 1, 0},
  {'8', 0x80, 0xF7, 0, 0},
  {'(', 0x80, 0xF7, 1, 0},
  {'0', 0x80, 0xEF, 0, 0},
  {'-', 0x80, 0xDF, 0, 0},
  {KEY_F7, 0x80, 0x7F, 0, 0},

  {0, 0, 0, 0, 0}
};

static void vic_video(Hardware *hardware);
static void get_basic(UInt16 *start, UInt16 *end) SECTION("machine");
static void set_basic(UInt16 start, UInt16 end) SECTION("machine");

UInt32 vic_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_VSYNC:
      vic_video(arg->hardware);
      vic_vsync(arg->a1);
      return 0;
    case IO_READB:
      return vic_readb(arg->a1);
    case IO_WRITEB:
      vic_writeb(arg->a1, arg->a2);
      return 0;
  }
  return 0;
}

static void vic_video(Hardware *hardware) {
  WinHandle prev;
  RectangleType rect;
  UInt8 *video_ptr, *char_ptr;
  UInt32 c[4], ccode, index, code, mask;
  UInt32 i, j, x, y, dx0, dy0, x1, x2, x3, y1;
  UInt32 addr, caddr, char_line, ram_line, offset, vr;
  VicGlobals *g;
  Err err;

  g = hardware->globals;

  g->frame++;
  if (g->frame & 1)
    return;

  g->video_size = g->num_lines * g->num_cols;

  if (g->video_ram < 0x8000) {
    video_ptr = hardware->m0;
    vr = g->video_ram;
  } else {
    video_ptr = hardware->m1;
    vr = g->video_ram & 0x7FFF;
  }

  // video ram

  for (i = 0; !g->dirty && i < g->video_size; i++) {
    if (video_ptr[vr + i] != g->video[i]) {
      g->dirty = 1;
    }
  }

  // color ram

  for (i = 0; !g->dirty && i < g->video_size; i++) {
    if (hardware->m1[g->color_ram + i] != g->video[0x400 + i]) {
      g->dirty = 1;
    }
  }

  if (!g->border && !g->dirty)
    return;

  WinSetCoordinateSystem(kCoordinatesDouble);

  if (hardware->screen_wh == NULL) {
    hardware->screen_wh = WinCreateOffscreenWindow(NUM_COLS * CHAR_WIDTH, NUM_LINES * CHAR_HEIGHT, nativeFormat, &err);
  }

  prev = WinSetDrawWindow(hardware->screen_wh);

  g->width = g->num_cols * CHAR_WIDTH;
  g->height = g->num_lines * g->char_height;

  dx0 = g->x0 << 2;
  dy0 = g->y0 << 1;
  if (dy0 < 16) {
    dy0 = 0;
  } else {
    dy0 -= 16;
    if ((dy0 + g->height) > g->area_height) {
      UInt32 d = dy0 + g->height - g->area_height;
      if (d < 64) {
        if (dy0 < d) {
          dy0 = 0;
        } else {
          dy0 -= d;
        }
      }
    }
  }

  c[0] = hardware->color[g->backg_color];
  c[1] = hardware->color[g->border_color];
  c[3] = hardware->color[g->aux_color];

  y1 = min(dy0, g->area_height);

  for (y = 0; y < y1; y++) {
    if (g->border) {
      //MemSet(s, hardware->display_width, c[1]);
    }
    //s += hardware->display_width;
  }

  x1 = min(dx0, hardware->display_width);
  x2 = x1 < hardware->display_width ? min(g->width, hardware->display_width - x1) : 0;
  x3 = ((x1 + x2) < hardware->display_width) ? hardware->display_width - (x1 + x2) : 0; 

  hardware->x0 = x1;
  hardware->y0 = y1;
  hardware->dx = g->width;
  hardware->dy = g->height;

  y1 = min(dy0 + g->height, g->area_height);

  char_line = 0;
  ram_line = 0;

  for (; y < y1; y++) {
    if (g->border && x1 > 0) {
      //MemSet(s, x1, c[1]);
    }

    if (g->dirty) {
      addr = vr + ram_line;
      caddr = g->color_ram + ram_line;
      x = x1;

      for (i = 0; i < g->num_cols; i++) {
        code = video_ptr[addr + i];
        offset = g->double_char ? (code << 4) : (code << 3);
        offset += char_line;

        // VIC tem apenas 16K de memoria: 0 a 0x3FFF

        if ((g->vic_char_map + offset) >= 0x4000) {
          char_ptr = hardware->m1;
          offset = g->vic_char_map + offset - 0x4000;
        } else {
          char_ptr = g->char_map < 0x8000 ?  hardware->m0 + g->char_map : hardware->m1 + (g->char_map & 0x7FFF);
        }

        mask = char_ptr[offset];

        ccode = hardware->m1[caddr + i];
        c[2] = hardware->color[ccode & 0x07];

        if (ccode & 0x08) {	// multicolor mode
          for (j = 0; j < CHAR_WIDTH; j += 2, mask <<= 2) {
            if (x < hardware->display_width) {
              index = (mask & 0xC0) >> 6;
              //s[x] = s[x+1] = c[index];
              WinSetForeColor(c[index]);
              WinDrawPixel(i * CHAR_WIDTH + j, y - dy0);
              WinDrawPixel(i * CHAR_WIDTH + j + 1, y - dy0);
            }
            x += 2;
          }
        } else {
          for (j = 0; j < CHAR_WIDTH; j++, mask <<= 1) {
            if (x < hardware->display_width) {
              //s[x] = (mask & 0x80) ? c[2] : c[0];
              WinSetForeColor((mask & 0x80) ? c[2] : c[0]);
              WinDrawPixel(i * CHAR_WIDTH + j, y - dy0);
            }
            x++;
          }
        }
      }

      if (++char_line == g->char_height) {
        char_line = 0;
        ram_line += g->num_cols;
      }

      // video ram
      MemMove(g->video, &video_ptr[vr], g->video_size);

      // color ram
      MemMove(&g->video[0x400], &hardware->m1[g->color_ram], g->video_size);
    }

    if (g->border && x3 > 0) {
      //MemSet(s + x1 + x2, x3, c[1]);
    }
    //s += hardware->display_width;
  }

  y1 = g->area_height;

  for (; y < y1; y++) {
    if (g->border) {
      //MemSet(s, hardware->display_width, c[1]);
    }
    //s += hardware->display_width;
  }

  WinSetDrawWindow(prev);
  RctSetRectangle(&rect, 0, 0, NUM_COLS * CHAR_WIDTH, NUM_LINES * CHAR_HEIGHT);
  //WinCopyRectangle(hardware->screen_wh, NULL, &rect, hardware->x0, hardware->y0, winPaint);
  WinCopyRectangle(hardware->screen_wh, NULL, &rect, 0, 0, winPaint);
  WinSetCoordinateSystem(kCoordinatesStandard);

  g->border = 0;
  g->dirty = 0;
}

void via_setint(Hardware *hardware, Int16 i, UInt8 b) {
  VicGlobals *g = hardware->globals;

  g->ifr[i] |= b;

  if (g->ier[i] & g->ifr[i]) {
    g->ifr[i] |= INT_ANY;
    switch (i) {
      case 0:
        hardware->irq_request = 1;
        break;
      case 1:
        hardware->nmi_request = 1;
    }
  }
}

void via_clearint(Hardware *hardware, Int16 i, UInt8 b) {
  VicGlobals *g = hardware->globals;

  g->ifr[i] = (g->ifr[i] & ~b) & 0x7f;

  if (g->ifr[i] & g->ier[i])
    g->ifr[i] |= INT_ANY;
}

UInt8 vic_readb(UInt16 a) {
  VicGlobals *g;
  UInt32 key;
  UInt8 b;

  g = hardware->globals;

  switch (a) {
    case 0x9000:	// distance from left border
      return g->x0;
    case 0x9001:	// distance from the top border
      return g->y0;
    case 0x9002:	// video ram bit 9, num of columns
      return g->video_color_bit | g->num_cols;
    case 0x9003:	// num of lines, normal X double, raster counter (bit 0)
      return (g->num_lines << 1) | g->double_char;
    case 0x9004:	// raster counter (bits 1-8)
      return 0;
    case 0x9005:	// video ram addr (4-7), character map addr (0-3)
      return hardware->m1[a & 0x7FFF];
    case 0x9006:	// light pen X
      return 0;
    case 0x9007:	// light pen Y
      return 0;
    case 0x9008:	// joystick X
      return hardware->joyx * 4;
    case 0x9009:	// joystick Y
      return hardware->joyy * 4;
    case 0x900A:	// speaker 1
    case 0x900B:	// speaker 2
    case 0x900C:	// speaker 3
    case 0x900D:	// speaker 4
      return g->sound[a - 0x900A];
    case 0x900E:	// speakers volume (0-3), aux color (4-7)
      return hardware->m1[a & 0x7FFF];
    case 0x900F:	// border color (0-2), reverse (3), bg color (4-7)
      return hardware->m1[a & 0x7FFF];

    case 0x9110:	// via 0 port b
      return 0xFF;

    case 0x9111:	// via 0 port a
    case 0x911F:	// via 0 port a (without handshake)
      b = 0xFF;

      if (hardware->button & g->button)
        b &= 0xDF;	// joystick button

      if (hardware->joyx < 16) b &= 0xEF;	// joystick left
      if (hardware->joyy > 48) b &= 0xF7;	// joystick down
      if (hardware->joyy < 16) b &= 0xFB;	// joystick up

      b = (hardware->m1[0x9111 & 0x7FFF] & g->ddra[0]) + (b & ~g->ddra[0]);
      via_clearint(hardware, 0, INT_CA1);
      return b;

    case 0x9112:	// via 0 data direction b
      return g->ddrb[0];

    case 0x9113:	// via 0 data direction a
      return g->ddra[0];

    case 0x911D:	// interrupt flag register
      return g->ifr[0];

    case 0x911E:	// interrupt enable register
      return g->ier[0] | 0x80;

    case 0x9120:	// via 1 port b
      b = 0xFF;
      key = hardware->key;
      if (key) {
        if (!(g->kbselect2 & ~g->keyMap[key].column))
          b &= ~g->keyMap[key].line;
        if (g->keyMap[key].shift && !(g->kbselect2 & ~g->keyMap[KEY_SHIFT].column))
          b &= ~g->keyMap[KEY_SHIFT].line;
        if (g->ctrl && !(g->kbselect2 & ~g->keyMap[KEY_CTRL].column))
          b &= ~g->keyMap[KEY_CTRL].line;
      }

      if (hardware->joyx > 48) b &= 0x7F;	// joystick right

      b = (g->kbselect1 & g->ddrb[1]) + (b & ~g->ddrb[1]);
      via_clearint(hardware, 1, INT_CB1);
      return b;

    case 0x9121:	// via 1 port a
    case 0x912F:	// via 0 port a (without handshake)
      b = 0xFF;
      key = hardware->key;
      if (key) {
        if (!(g->kbselect1 & g->keyMap[key].line))
          b &= g->keyMap[key].column;
        if (g->keyMap[key].shift && !(g->kbselect1 & g->keyMap[KEY_SHIFT].line))
          b &= g->keyMap[KEY_SHIFT].column;
        if (g->ctrl && !(g->kbselect1 & g->keyMap[KEY_CTRL].line))
          b &= g->keyMap[KEY_CTRL].column;
      }
      b = (g->kbselect2 & g->ddra[1]) + (b & ~g->ddra[1]);
      via_clearint(hardware, 1, INT_CA1);
      return b;

    case 0x9122:	// via 1 data direction b
      return g->ddrb[1];

    case 0x9123:	// via 1 data direction a
      return g->ddra[1];

    case 0x912D:	// interrupt flag register
      return g->ifr[1];

    case 0x912E:	// interrupt enable register
      return g->ier[1] | 0x80;
  }

  return 0x00;
}

void vic_writeb(UInt16 a, UInt8 b) {
  VicGlobals *g;
  UInt32 aux1, aux2, aux3;
  UInt8 b1;

  g = hardware->globals;

  switch (a) {
    case 0x9000:	// distance from left border
      if (g->x0 != (b & 0x7F)) {
        g->x0 = (b & 0x7F);	// granularity = 4 pixels
        g->border = g->dirty = 1;
      }
      break;
    case 0x9001:	// distance from the top border
      if (g->y0 != b) {
        g->y0 = b;	// granularity = 2 pixels
        g->border = g->dirty = 1;
      }
      break;
    case 0x9002:	// video ram bit 9, num of columns
      if (g->video_color_bit != (b & 0x80)) {
        g->video_color_bit = b & 0x80;
        g->color_ram = g->video_color_bit ? 0x1600 : 0x1400;
        g->border = g->dirty = 1;
      }
      if (g->num_cols != (b & 0x7F)) {
        g->num_cols = b & 0x7F;
        g->dirty = 1;
      }
      break;
    case 0x9003:	// num of lines, normal X double, raster counter (bit 0)
      if (g->double_char != (b & 0x01)) {
        g->double_char = b & 0x01;
        g->char_height = g->double_char ? CHAR_HEIGHT * 2 : CHAR_HEIGHT;
        g->border = g->dirty = 1;
      }
      if (g->num_lines != ((b & 0x7E) >> 1)) {
        g->num_lines = (b & 0x7E) >> 1;
        g->border = g->dirty = 1;
      }
      break;
    case 0x9005:	// video ram addr (4-7), character map addr (0-3)

      //   |--|
      // FEDCBA98 VIC     CPU
      // 00000000 0000    8000    Unreversed Character ROM
      // 00000100 0400    8400    Reversed Character ROM  
      // 00001000 0800    8800    Unreversed upper/lower case ROM 
      // 00001100 0C00    8C00    Reversed upper/lower case ROM   
      // 00010000 1000    9000    VIC and VIA chips
      // 00010100 1400    9400    Colour memory
      // 00011000 1800    9800    Reserved for expansion
      // 00011100 1C00    9C00    Reserved for expansion
      // 00100000 2000    0000    System memory
      // 00100100 2400    0400    Reserved for expansion
      // 00101000 2800    0800    ????
      // 00101100 2C00    0C00    ????
      // 00110000 3000    1000    Program
      // 00110100 3400    1400    ????
      // 00111000 3800    1800    ????
      // 00111100 3C00    1C00    Screen

      // CHARGENADDR (vic6560[5] & 0x0F) << 10
      // CHARGENSIZE 256 * HEIGHTPIXEL

      g->vic_char_map = ((b & 0x0F) << 10);

      aux1 = ((b & 0x07) << 10);
      if (!(b & 0x08))
        aux1 |= 0x8000;

      if (g->char_map != aux1) {
        g->char_map = aux1;
        g->dirty = 1;
      }

      //   |--|X
      // FEDCBA98
      // 001111X0 3C00/3E00  1C00/1E00 (X = bit 7 do 0x9002)
      // 001100X0 3000/3200  1000/1200 (X = bit 7 do 0x9002)

      // VIDEOADDR ((vic6560[5] & 0xF0) << (10-4)) |
      //           ((vic6560[2] & 0x80) << ( 9-7))
      // VIDEORAMSIZE YSIZE * XSIZE

      aux1 = ((b & 0x70) << 6);
      if (!(b & 0x80))
        aux1 |= 0x8000;
      if (g->video_color_bit)
        aux1 |= 0x0200;

      if (g->video_ram != aux1) {
        g->video_ram = aux1;
        g->dirty = 1;
      }

      hardware->m1[a & 0x7FFF] = b;
      break;
    case 0x900A:	// speaker 1 (low)
    case 0x900B:	// speaker 2 (medium)
    case 0x900C:	// speaker 3 (high)
    case 0x900D:	// speaker 4 (noise)

      // VIC6560_CLOCK = 14318181/14 = 1022727.21 (ntsc)
      // VIC6561_CLOCK = 4433618/4   = 1108404.50 (pal)

      // TONE_FREQUENCY_MIN  = VIC6560_CLOCK/256/128 = 31.21
      // NOISE_FREQUENCY_MAX = VIC6560_CLOCK/32/1    = 31960.22

      // #define TONE1_VALUE (8*(128-((vic6560[0xa]+1)&0x7f)))
      // #define TONE1_FREQUENCY (VIC6560_CLOCK/32/TONE1_VALUE)

      // #define TONE2_VALUE (4*(128-((vic6560[0xb]+1)&0x7f)))
      // #define TONE2_FREQUENCY (VIC6560_CLOCK/32/TONE2_VALUE)

      // #define TONE3_VALUE (2*(128-((vic6560[0xc]+1)&0x7f)))
      // #define TONE3_FREQUENCY (VIC6560_CLOCK/32/TONE3_VALUE)

      // #define NOISE_VALUE (32*(128-((vic6560[0xd]+1)&0x7f)))
      // #define NOISE_FREQUENCY (VIC6560_CLOCK/NOISE_VALUE)

      g->sound[a - 0x900A] = b;
      aux1 = aux2 = 0;

      if (g->sound[0] & 0x80) {
        aux3 = (0x80 - ((g->sound[0] + 1) & 0x7F)) << 3;
        aux3 = VIC6560_CLOCK / (aux3 << 5);
        aux2 += aux3;
        aux1++;
      }
      if (g->sound[1] & 0x80) {
        aux3 = (0x80 - ((g->sound[1] + 1) & 0x7F)) << 2;
        aux3 = VIC6560_CLOCK / (aux3 << 5);
        aux2 += aux3;
        aux1++;
      }
      if (g->sound[2] & 0x80) {
        aux3 = (0x80 - ((g->sound[2] + 1) & 0x7F)) << 1;
        aux3 = VIC6560_CLOCK / (aux3 << 5);
        aux2 += aux3;
        aux1++;
      }
/*
      if (g->sound[3] & 0x80) {
        aux3 = (0x80 - ((g->sound[3] + 1) & 0x7F)) << 5;
        aux3 = VIC6560_CLOCK / aux3;
        aux2 += aux3;
        aux1++;
      }
*/

      hardware->snd_tonefreq = aux1 ? aux2 / aux1 : 0;
      break;
    case 0x900E:	// speakers volume (0-3), aux color (4-7)
      hardware->m1[a & 0x7FFF] = b;
      g->aux_color = b >> 4;

      hardware->snd_toneamp = (b & 0x0F) << 2;	// 0 - 60
      break;
    case 0x900F:	// border color (0-2), reverse (3), bg color (4-7)
      b1 = b >> 4;
      if (g->backg_color != b1) {
        g->backg_color = b1;
        g->dirty = 1;
      }
      b1 = b & 0x07;
      if (g->border_color != b1) {
        g->border_color = b & 0x07;
        g->border = 1;
      }
      g->normal = (b & 0x08) ? 1 : 0;
      hardware->m1[a & 0x7FFF] = b;
      break;

    case 0x9110:	// via 0 port b
      hardware->m1[a & 0x7FFF] = b;
      via_clearint(hardware, 0, INT_CB1);
      break;
    case 0x9111:	// via 0 port a
    case 0x911F:	// via 0 port a (without handshake)
      hardware->m1[0x9111 & 0x7FFF] = b;
      via_clearint(hardware, 0, INT_CA1);
      break;
    case 0x9112:	// via 0 data direction b
      g->ddrb[0] = b;
      break;
    case 0x9113:	// via 0 data direction a
      g->ddra[0] = b;
      break;
    case 0x911D:	// interrupt flag register
      if (b & INT_ANY)
        b = 0x7F;
      via_clearint(hardware, 0, b);
      break;
    case 0x911E:	// interrupt enable register
      if (b & 0x80)
        g->ier[0] |= b & 0x7F;
      else
        g->ier[0] &= ~(b & 0x7F);

      if (g->ifr[0] & INT_ANY) {
        if (((g->ifr[0] & g->ier[0]) & 0x7F) == 0) {
          g->ifr[0] &= ~INT_ANY;
        }
      } else {
        if ((g->ier[0] & g->ifr[0]) & 0x7F) {
          g->ifr[0] |= INT_ANY;
          hardware->nmi_request = 1;
        }
      }
      break;

    case 0x9120:	// via 1 port b
      g->kbselect1 = b;
      via_clearint(hardware, 1, INT_CB1);
      break;
    case 0x9121:	// via 1 port a
    case 0x912F:	// via 1 port a (without handshake)
      g->kbselect2 = b;
      via_clearint(hardware, 1, INT_CA1);
      break;
    case 0x9122:	// via 1 data direction b
      g->ddrb[1] = b;
      break;
    case 0x9123:	// via 1 data direction a
      g->ddra[1] = b;
      break;
    case 0x912D:	// interrupt flag register
      if (b & INT_ANY)
        b = 0x7F;
      via_clearint(hardware, 1, b);
      break;
    case 0x912E:	// interrupt enable register
      if (b & 0x80)
        g->ier[1] |= b & 0x7F;
      else
        g->ier[1] &= ~(b & 0x7F);

      if (g->ifr[1] & INT_ANY) {
        if (((g->ifr[1] & g->ier[1]) & 0x7F) == 0) {
          g->ifr[1] &= ~INT_ANY;
        }
      } else {
        if ((g->ier[1] & g->ifr[1]) & 0x7F) {
          g->ifr[1] |= INT_ANY;
          hardware->irq_request = 1;
        }
      }
  }
}

void vic_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  VicGlobals *globals;
  KeyMap *keyMap;
  UInt16 i, len, x0, y0, width, height, area_height;

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

  width = NUM_COLS * CHAR_WIDTH;
  height = NUM_LINES * CHAR_HEIGHT;
  area_height = hardware->display_height - lowBorder * 2;
  x0 = ((hardware->display_width  - width) / 2) / 4;
  y0 = ((area_height - height) / 2) / 2;

  if (!video) {
    video = MemPtrNew(0x800);
    MemSet(video, 0x800, 0);
  }

  if (!hardware->globals) {
    hardware->globals = MemPtrNew(sizeof(VicGlobals));
    MemSet(hardware->globals, sizeof(VicGlobals), 0);
  }
  globals = hardware->globals;

  globals->frame = 0;
  globals->dirty = 1;
  globals->area_height = area_height;
  globals->x0 = (x0);
  globals->y0 = (y0);
  globals->border = 1;
  globals->border_color = 0;
  globals->backg_color = 0;
  globals->double_char = 0;
  globals->char_height = (CHAR_HEIGHT);
  globals->num_cols = (NUM_COLS);
  globals->num_lines = (NUM_LINES);
  globals->width = (width);
  globals->height = (height);
  globals->normal = 1;
  globals->char_map = (0x8000);
  globals->vic_char_map = (0x0000);
  globals->color_ram = (0x1400);
  globals->video_color_bit = 0;
  globals->video_ram = (0x1000);
  globals->ier[0] = globals->ier[1] = 0;
  globals->ifr[0] = globals->ifr[1] = 0;
  globals->ddra[0] = globals->ddra[1] = 0;
  globals->ddrb[0] = globals->ddrb[1] = 0;
  globals->kbselect1 = (0xFF);
  globals->kbselect2 = (0xFF);
  globals->button = prefs.button;
  globals->ctrl = 0;
  globals->video = (UInt8 *)(video);

  keyMap = MemPtrNew(256 * sizeof(KeyMap));
  kbd_initmap(VicMap, keyMap);

  for (i = 0; i < 256; i++) {
    globals->keyMap[i].key = (keyMap[i].key);
    globals->keyMap[i].line = (keyMap[i].line);
    globals->keyMap[i].column = (keyMap[i].column);
    globals->keyMap[i].shift = (keyMap[i].shift);
    globals->keyMap[i].ctrl = (keyMap[i].ctrl);
  }
  MemPtrFree(keyMap);
}

void vic_finish(void) {
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);

  if (video) {
    MemPtrFree(video);
    video = NULL;
  }

  if (hardware->globals) {
    MemPtrFree(hardware->globals);
    hardware->globals = NULL;
  }
}

void vic_key(UInt16 c) {
  VicGlobals *g;

  if (c == KEY_CTRL) {
    g = hardware->globals;
    g->ctrl = 1;
    return;
  }

  if (c >= 'a' && c <= 'z')
    c &= 0xDF;
  else if (c == 0) {
    if (key != KEY_CTRL) {
      g = hardware->globals;
      g->ctrl = 0;
    }
  }

  key = c;
  hardware->key = key;
}

void vic_joystick(UInt16 x, UInt16 y) {
  hardware->joyx = x;
  hardware->joyy = y;
}

static void get_basic(UInt16 *start, UInt16 *end) {
  if (start)
    *start = hardware->m0[0x2B] | (hardware->m0[0x2C] << 8);
  if (end)
    *end = hardware->m0[0x2D] | (hardware->m0[0x2E] << 8);
}

static void set_basic(UInt16 start, UInt16 end) {
  hardware->m0[0x2B] = hardware->m0[0xAC] = start & 0xFF;
  hardware->m0[0x2C] = hardware->m0[0xAD] = start >> 8;
  hardware->m0[0x2D] = hardware->m0[0x2F] = hardware->m0[0x31] = hardware->m0[0xAE] = end & 0xFF;
  hardware->m0[0x2E] = hardware->m0[0x30] = hardware->m0[0x32] = hardware->m0[0xAF] = end >> 8;
}

Err vic_readprg(FileRef f) {
  UInt8 b[2];
  UInt16 addr;
  UInt32 size, hsize, r;
  Err err;

  hsize = 2;

  if ((err = VFSFileSize(f, &size)) != 0)
    return -1;

  if (size < hsize)
    return -2;

  if ((err = VFSFileRead(f, hsize, b, &r)) != 0)
    return -3;

  if (r != hsize)
    return -4;

  if (b[0] == 1)
    get_basic(&addr, NULL);
  else
    addr = b[0] | (b[1] << 8);

  size -= hsize;

  if ((addr + size) >= 0x8000)
    return -5;

  if ((err = VFSFileRead(f, size, &hardware->m0[addr], &r)) != 0)
    return -6;

  if (r != size)
    return -7;

  if (b[0] == 1)
    set_basic(addr, addr + size);

  return 0;
}

VicPrefs *VicGetPrefs(void) {
  return &prefs;
}

Boolean VicFormHandler(EventPtr event, Boolean *close) {
  FormPtr frm;
  ListPtr lst;
  UInt32 button;
  VicGlobals *g;
  Boolean handled;

  frm = FrmGetActiveForm();
  handled = false;
  *close = false;

  switch (event->eType) {
    case frmOpenEvent:
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
          g = hardware->globals;
          g->button = prefs.button;
      }

    default:
      break;
  }

  return handled;
}
