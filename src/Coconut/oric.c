#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "machine.h"
#include "kbd.h"
#include "ay8910.h"
#include "endian.h"
#include "m6502.h"
#include "oric.h"
#include "misc.h"
#include "gui.h"
#include "debug.h"

#define CHAR_WIDTH	6
#define CHAR_HEIGHT	8

#define NUM_COLUMNS	40
#define NUM_LINES	28

#define SCREEN_WIDTH	(NUM_COLUMNS * CHAR_WIDTH)
#define TEXT_HEIGHT	(NUM_LINES * CHAR_HEIGHT)
#define TOTAL_SIZE	(0xBFE0 - 0xA000)

#define GRAPH_HEIGHT  ((NUM_LINES-3) * CHAR_HEIGHT)
#define TEXT3_HEIGHT  (3 * CHAR_HEIGHT)

#define TEXT_SIZE (NUM_LINES * NUM_COLUMNS)
#define GRAPH_SIZE  (GRAPH_HEIGHT * NUM_COLUMNS)
#define TEXT3_SIZE  (3 * NUM_COLUMNS)
#define TOTAL_SIZE  (0xBFE0 - 0xA000)

#define INT_CA2   0x01
#define INT_CA1   0x02
#define INT_SR    0x04
#define INT_CB2   0x08
#define INT_CB1   0x10
#define INT_T2    0x20
#define INT_T1    0x40
#define INT_ANY   0x80

static UInt8 id;
static OricPrefs prefs;
static Hardware *hardware;
static UInt8 *video = NULL;
static UInt16 key;

ColorType OricColor[9] = {
  {0, 0x00, 0x00, 0x00},
  {1, 0xff, 0x00, 0x00},
  {2, 0x00, 0xff, 0x00},
  {3, 0xff, 0xff, 0x00},
  {4, 0x00, 0x00, 0xff},
  {5, 0xff, 0x00, 0xff},
  {6, 0x00, 0xff, 0xff},
  {7, 0xff, 0xff, 0xff},
  {-1, 0, 0, 0}
};

UInt16 OricControl[9] = {stopCmd, restartCmd, configCmd,
                         loadSnapCmd, 0, 0,
                         0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef OricButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {KEY_UP,    internalFontID, "\x80",  C1, C2, C3},
  {KEY_DOWN,  internalFontID, "\x81", C1, C2, C3},
  {KEY_ESC,   internalFontID, "ESC",  C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_LEFT,  internalFontID, "\x82", C1, C2, C3},
  {KEY_RIGHT, internalFontID, "\x83", C1, C2, C3},
  {KEY_CTRL,  internalFontID, "CTL", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {KEY_RESET, internalFontID, "RST", C1, C2, C3}
};

/*
Key required	Accumulator	X register
1 2 3	0 2 0	DF BF 7F 
4 5 6	2 0 2	F7 FB FD 
7 8 9	0 7 3	FE FE FD
0 - =	7 3 7	FB F7 7F
\ ESC Q	3 1 1	BF DF BF 
W E R	6 6 1	7F F7 FB 
T Y U 	1 6 5	FD FE FE
I 0 P	5 5 5	FD FB F7 
[ ] DEL	5 5 5	7F BF DF 
CTRL A S	2 6 6	EF DF BF 
D F G 	1 1 6	7F F7 FB
H J K 	6 1 3	FD FE FE 
L ; "	7 3 3	FD FB 7F
ENTER	7	DF
LSHIFT		4	EF
Z X C	2 0 2	DF BF 7F
V B N	0 2 0	F7 FB FD
M comma period	2 4 4 	FE FD FB
/ RSHIFT 	7 7	F7 EF
LEFT ARROW 	4	DF 
DOWN ARROW	4	BF
SPACE	4	FE
UP ARROW	4	F7
RIGHT ARROW	4	7F 
*/

static KeyMap OricMap[] = {
  {'1', 0x00, 0x20, 0, 0},
  {'!', 0x00, 0x20, 1, 0},
  {'2', 0x02, 0x40, 0, 0},
  {'@', 0x02, 0x40, 1, 0},
  {'3', 0x00, 0x80, 0, 0},
  {'#', 0x00, 0x80, 1, 0},
  {'4', 0x02, 0x08, 0, 0},
  {'$', 0x02, 0x08, 1, 0},
  {'5', 0x00, 0x04, 0, 0},
  {'%', 0x00, 0x04, 1, 0},
  {'6', 0x02, 0x02, 0, 0},
  {'^', 0x02, 0x02, 1, 0},
  {'7', 0x00, 0x01, 0, 0},
  {'&', 0x00, 0x01, 1, 0},
  {'8', 0x07, 0x01, 0, 0},
  {'*', 0x07, 0x01, 1, 0},
  {'9', 0x03, 0x02, 0, 0},
  {'(', 0x03, 0x02, 1, 0},
  {'0', 0x07, 0x04, 0, 0},
  {')', 0x07, 0x04, 1, 0},
  {'-', 0x03, 0x08, 0, 0},
  {163, 0x03, 0x08, 1, 0},	// pound
  {'=', 0x07, 0x80, 0, 0},
  {'+', 0x07, 0x80, 1, 0},
  {'\\', 0x03, 0x40, 0, 0},
  {'|', 0x03, 0x40, 1, 0},
  {KEY_ESC, 0x01, 0x20, 0, 0},
  {'Q', 0x01, 0x40, 0, 0},
  {'W', 0x06, 0x80, 0, 0},
  {'E', 0x06, 0x08, 0, 0},
  {'R', 0x01, 0x04, 0, 0},
  {'T', 0x01, 0x02, 0, 0},
  {'Y', 0x06, 0x01, 0, 0},
  {'U', 0x05, 0x01, 0, 0},
  {'I', 0x05, 0x02, 0, 0},
  {'O', 0x05, 0x04, 0, 0},
  {'P', 0x05, 0x08, 0, 0},
  {'[', 0x05, 0x80, 0, 0},
  {'{', 0x05, 0x80, 1, 0},
  {']', 0x05, 0x40, 0, 0},
  {'}', 0x05, 0x40, 1, 0},
  {KEY_BACK, 0x05, 0x20, 0, 0},
  {KEY_CTRL, 0x02, 0x10, 0, 0},
  {'A', 0x06, 0x20, 0, 0},
  {'S', 0x06, 0x40, 0, 0},
  {'D', 0x01, 0x80, 0, 0},
  {'F', 0x01, 0x08, 0, 0},
  {'G', 0x06, 0x04, 0, 0},
  {'H', 0x06, 0x02, 0, 0},
  {'J', 0x01, 0x01, 0, 0},
  {'K', 0x03, 0x01, 0, 0},
  {'L', 0x07, 0x02, 0, 0},
  {';', 0x03, 0x04, 0, 0},
  {':', 0x03, 0x04, 1, 0},
  {'\'', 0x03, 0x80, 0, 0},
  {'"', 0x03, 0x80, 1, 0},
  {KEY_ENTER, 0x07, 0x20, 0, 0},
  {KEY_SHIFT, 0x04, 0x10, 0, 0},
  {'Z', 0x02, 0x20, 0, 0},
  {'X', 0x00, 0x40, 0, 0},
  {'C', 0x02, 0x80, 0, 0},
  {'V', 0x00, 0x08, 0, 0},
  {'B', 0x02, 0x04, 0, 0},
  {'N', 0x00, 0x02, 0, 0},
  {'M', 0x02, 0x01, 0, 0},
  {',', 0x04, 0x02, 0, 0},
  {'<', 0x04, 0x02, 1, 0},
  {'.', 0x04, 0x04, 0, 0},
  {'>', 0x04, 0x04, 1, 0},
  {'/', 0x07, 0x08, 0, 0},
  {'?', 0x07, 0x08, 1, 0},
  {KEY_LEFT, 0x04, 0x20, 0, 0},
  {KEY_DOWN, 0x04, 0x40, 0, 0},
  {' ', 0x04, 0x01, 0, 0},
  {KEY_UP, 0x04, 0x08, 0, 0},
  {KEY_RIGHT, 0x04, 0x80, 0, 0},
  {0, 0, 0, 0, 0}
};

static void oric_video(void);
static void via_setint(Int16 i, UInt8 b);
static void via_clearint(Int16 i, UInt8 b);

UInt32 oric_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_VSYNC:
      snd_calcnoise();
      oric_video();
      oric_vsync(arg->a1);
      via_setint(0, INT_T1);
      break;
    case IO_READB:
      return oric_readb(arg->a1);
    case IO_WRITEB:
      oric_writeb(arg->a1, arg->a2);
  }
  return 0;
}

static void oric_video(void) {
  RectangleType rect;
  WinHandle prev;
  UInt32 i, j, k, m, charmap_ram, ink, paper;
  UInt32 addr, caddr, c1, c2, lines;
  UInt8 *video_ptr, *charmap_ptr, code, mask, mask0;
  UInt32 r0, blink, double_height, control, inverted, dirty, last3;
  OricGlobals *g;
  Err err;

  g = hardware->globals;
  g->frame++;

  if (g->frame == 20) {
    g->frame = 0;
    g->blink_status = !g->blink_status;
  }

  if (g->frame & 1) {
    return;
  }

  video_ptr = hardware->m1;
  dirty = 0;

  for (i = 0; !dirty && i < TOTAL_SIZE; i++) {
    if (video_ptr[0x2000 + i] != g->video[i]) {
      dirty = 1;
    }
  }

  if (!g->border && !dirty) {
    return;
  }

  WinSetCoordinateSystem(kCoordinatesDouble);

  if (g->border) {
    c1 = hardware->color[0];
    WinSetBackColor(c1);
    RctSetRectangle(&rect, 0, 0, hardware->display_width, g->area_height);
    WinEraseRectangle(&rect, 0);
    g->border = 0;
    dirty = 1;
  }

  if (!dirty) {
    WinSetCoordinateSystem(kCoordinatesStandard);
    return;
  }

  if (hardware->screen_wh == NULL) {
    hardware->screen_wh = WinCreateOffscreenWindow(SCREEN_WIDTH, TEXT_HEIGHT, nativeFormat, &err);
  }

  prev = WinSetDrawWindow(hardware->screen_wh);

  if (g->text) {
    addr = 0x3B80;
    lines = NUM_LINES;
  } else {
    addr = 0x2000;
    lines = TEXT_HEIGHT;
  }

  last3 = 0;
  charmap_ptr = hardware->m1;

  for (k = 0; k < lines; k++) {
    ink = 7;
    paper = 0;
    blink = 0;
    double_height = 0;
    charmap_ram = 0x3400;

    if (!g->text) {
      if (k == 0) {
        addr = 0x2000;
        lines = TEXT_HEIGHT;
        last3 = 0;
      } else if (k == GRAPH_HEIGHT) {
        k = NUM_LINES - 3;
        addr = 0x3B80 + k * NUM_COLUMNS;
        lines = NUM_LINES;
        last3 = 1;
      }
      charmap_ram = 0x1800;
    }

    if (g->text || last3) {
      r0 = k * CHAR_HEIGHT;
    } else {
      r0 = k;
    }

    for (i = 0; i < NUM_COLUMNS; i++, addr++) {
      code = video_ptr[addr];

      control = (code & 0x60) == 0;
      inverted = (code & 0x80);
      code &= 0x7F;

      if (!control) {
        if (inverted) {
          c1 = hardware->color[paper];
          c2 = hardware->color[ink];
        } else {
          c1 = hardware->color[ink];
          c2 = hardware->color[paper];
        }

        if (blink && g->blink_status) {
          c1 = c2;
        }

        if (g->text || last3) {
          caddr = charmap_ram + (code << 3);

          if (double_height) {
            if (k % 2)
              caddr += 4;

            for (m = 0; m < CHAR_HEIGHT; m += 2) {
              mask = mask0 = charmap_ptr[caddr++] << 2;
              for (j = 0; j < CHAR_WIDTH; j++, mask <<= 1) {
                WinSetForeColor((mask & 0x80) ? c1 : c2);
                WinDrawPixel(i * CHAR_WIDTH + j, r0 + m);
              }

              mask = mask0;
              for (j = 0; j < CHAR_WIDTH; j++, mask <<= 1) {
                WinSetForeColor((mask & 0x80) ? c1 : c2);
                WinDrawPixel(i * CHAR_WIDTH + j, r0 + m + 1);
              }
            }
          } else {
            for (m = 0; m < CHAR_HEIGHT; m++) {
              mask = charmap_ptr[caddr++] << 2;
              for (j = 0; j < CHAR_WIDTH; j++, mask <<= 1) {
                WinSetForeColor((mask & 0x80) ? c1 : c2);
                WinDrawPixel(i * CHAR_WIDTH + j, r0 + m);
              }
            }
          }
        } else {
          mask = (code & 0x3F) << 2;
          for (j = 0; j < 6; j++, mask <<= 1) {
            WinSetForeColor((mask & 0x80) ? c1 : c2);
            WinDrawPixel(i * CHAR_WIDTH + j, r0);
          }
        }

      } else {			// control code
        code &= 0x1F;

        switch (code) {
          case 0:
          case 1:
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7:
            ink = code;
            break;
          case 8:
            blink = 0;
            double_height = 0;
            charmap_ram = g->text ? 0x3400 : 0x1800;
            break;
          case 9:
            blink = 0;
            double_height = 0;
            charmap_ram = g->text ? 0x3800 : 0x1C00;
            break;
          case 10:
            blink = 0;
            double_height = 1;
            charmap_ram = g->text ? 0x3400 : 0x1800;
            break;
          case 11:
            blink = 0;
            double_height = 1;
            charmap_ram = g->text ? 0x3800 : 0x1C00;
            break;
          case 12:
            blink = 1;
            double_height = 0;
            charmap_ram = g->text ? 0x3400 : 0x1800;
            break;
          case 13:
            blink = 1;
            double_height = 0;
            charmap_ram = g->text ? 0x3800 : 0x1C00;
            break;
          case 14:
            blink = 1;
            double_height = 1;
            charmap_ram = g->text ? 0x3400 : 0x1800;
            break;
          case 15:
            blink = 1;
            double_height = 1;
            charmap_ram = g->text ? 0x3800 : 0x1C00;
            break;
          case 16:
          case 17:
          case 18:
          case 19:
          case 20:
          case 21:
          case 22:
          case 23:
            paper = code - 16;
            break;
          case 24:
          case 25:
          case 26:
          case 27:
            if (!g->text) {
              lines = NUM_LINES;
              if (!last3)
                k = k / CHAR_HEIGHT;
              addr = 0x3B80 + k * NUM_COLUMNS + i;
              charmap_ram = (charmap_ram == 0x1800) ? 0x3400 : 0x3800;
              g->text = 1;
            }
            break;
          case 28:
          case 29:
          case 30:
          case 31:
            if (g->text) {
              if (k >= (NUM_LINES - 3)) {
                last3 = 1;
              } else {
                lines = TEXT_HEIGHT;
                k = k * CHAR_HEIGHT + CHAR_HEIGHT - 1;
                addr = 0x2000 + k * NUM_COLUMNS + i;
              }
              charmap_ram = (charmap_ram == 0x3400) ? 0x1800 : 0x1C00;
              g->text = 0;
            }
        }

        c1 = hardware->color[paper];
        WinSetForeColor(c1);

        if (g->text || last3) {
          for (m = 0; m < CHAR_HEIGHT; m++) {
            WinDrawLine(i*CHAR_WIDTH, r0+m, (i+1)*CHAR_WIDTH-1, r0+m);
          }
        } else {
          WinDrawLine(i*CHAR_WIDTH, r0, (i+1)*CHAR_WIDTH-1, r0);
        }
      }
    }
  }

  MemMove(g->video, &video_ptr[0x2000], TOTAL_SIZE);

  WinSetDrawWindow(prev);
  RctSetRectangle(&rect, 0, 0, SCREEN_WIDTH, TEXT_HEIGHT);
  WinCopyRectangle(hardware->screen_wh, NULL, &rect, hardware->x0, hardware->y0, winPaint);
  WinSetCoordinateSystem(kCoordinatesStandard);
}

// The cassette circuitry connects to the CB1 line on the 6522. When this goes from low to high the CB1 flag is set in the interrupt flag register of the 6522. T2 is used to count time before looking at the CB1 flag, and each bit is built up into a whole byte using a series of rotate instructions.

// The cassette relay connection is activated before any of the cassette routines by setting bit 6 of port B on the 6522. This bit is cleared after all cassette operations, deactivating the relay.

// During cassette loading the timers operate in a different fashion:

// T2, which is idle at other times, is used when receiving bits from the cassette input port in order to wait an exact amount of time.

// The function of T1 is altered (by setting bits 6 and 7 of the auxiliary control register) so that instead of causing an interrupt bit 7 of port B is toggled and the timer is automatically set running again.

// When a cassette operation is complete, the registers in the 6522 are set back to their initial values in order for the keyboard and printer to work normally.

static void via_setint(Int16 i, UInt8 b) {
  OricGlobals *g = hardware->globals;

  g->ifr[i] |= b;

  if (g->ier[i] & g->ifr[i]) {
    g->ifr[i] |= INT_ANY;
/*
    switch (i) {
      case 0:
        hardware->irq_request = 1;
        break;
      case 1:
        hardware->nmi_request = 1;
    }
*/
  }
}

static void via_clearint(Int16 i, UInt8 b) {
  OricGlobals *g = hardware->globals;

  g->ifr[i] = (g->ifr[i] & ~b) & 0x7f;

  if (g->ifr[i] & g->ier[i]) {
    g->ifr[i] |= INT_ANY;
  }
}

// timer 1: used by the rom to provide an interrupt every 1/100th a second
// latch1 = 0x2710 = 1/100 seconds = 10000 cycles
//
// cycles     counter1
//      0  -> 0x2710
//  vsync  -> 0x0000

UInt8 oric_readb(UInt16 a) {
  OricGlobals *g;
  UInt32 key;
  UInt8 b;

  g = hardware->globals;

  a &= 0xFF0F;

  switch (a) {
    case 0x300:	// via 0 port b
      b = 0xB0 | g->motor | g->kbselect1;
      key = hardware->key;
      if (key) {
        if (g->kbselect1 == g->keyMap[key].line && (g->kbselect2 & g->keyMap[key].column))
          b |= 0x08;
        if (g->keyMap[key].shift &&
            g->kbselect1 == g->keyMap[KEY_SHIFT].line &&
            (g->kbselect2 & g->keyMap[KEY_SHIFT].column))
          b |= 0x08;
        if (g->ctrl && 
            g->kbselect1 == g->keyMap[KEY_CTRL].line &&
            (g->kbselect2 & g->keyMap[KEY_CTRL].column))
          b |= 0x08;
      }
      b = (hardware->m0[a] & g->ddrb[0]) + (b & ~g->ddrb[0]);
      return b;

    case 0x301:	// via 0 port a
    case 0x30F:	// via 0 port a (without handshake)
      b = 0xFF;
      b = (hardware->m0[0x301] & g->ddra[0]) + (b & ~g->ddra[0]);
      return b;

    case 0x302:	// via 0 data direction b
      return g->ddrb[0];

    case 0x303:	// via 0 data direction a
      return g->ddra[0];

    case 0x304:	// timer 1 counter (low)
      via_clearint(0, INT_T1);
      return g->counter1 & 0xFF;

    case 0x305:	// timer 1 counter (high)
      return g->counter1 >> 8;

    case 0x306:	// timer 1 latch   (low)
      return g->latch1 & 0xFF;

    case 0x307:	// timer 1 latch (high)
      return g->latch1 >> 8;

    case 0x30C:	// pcr
      return g->pcr[0];

    case 0x30D:	// interrupt flag register
      return g->ifr[0];

    case 0x30E:	// interrupt enable register
      return g->ier[0] | 0x80;
  }

  return 0x00;
}

void oric_writeb(UInt16 a, UInt8 b) {
  OricGlobals *g;

  g = hardware->globals;

  a &= 0xFF0F;

  switch (a) {
    case 0x300:	// via 0 port b
      g->kbselect1 = b & 0x07;
      g->motor = b & 0x40;
      hardware->m0[a] = b;
      break;
    case 0x301:	// via 0 port a
    case 0x30F:	// via 0 port a (without handshake)
      hardware->m0[0x301] = b;
      break;
    case 0x302:	// via 0 data direction b
      g->ddrb[0] = b;
      break;
    case 0x303:	// via 0 data direction a
      g->ddra[0] = b;
      break;
    case 0x304:	// timer 1 counter (low)
      g->latch1 = (g->latch1 & 0xFF00) | b;
      break;
    case 0x305:	// timer 1 counter (high)
      g->latch1 = (g->latch1 & 0x00FF) | (b << 8);
      g->counter1 = g->latch1;
      via_clearint(0, INT_T1);
      break;
    case 0x306:	// timer 1 latch   (low)
      g->latch1 = (g->latch1 & 0xFF00) | b;
      break;
    case 0x307:	// timer 1 latch (high)
      g->latch1 = (g->latch1 & 0x00FF) | (b << 8);
      via_clearint(0, INT_T1);
      break;
    case 0x30C:	// pcr
      //  CB2  CA2
      // 7654 3210
      // 1101 1101

      // Bit 7 = 1: output
      // Bits 65 = 10: manual mode, CB2 low
      // Bit 3 = 1: output
      // Bits 21 = 10: manual mode, CA2 low

      // CA2 = bit 1 do PCR = 0x02
      // CB2 = bit 5 do PCR = 0x20

      g->pcr[0] = b;

      if (g->pcr[0] & 0x04) {	// CA2 in manual mode
        if (g->pcr[0] & 0x02) {	// CA2=1, 8912 register
          ay8910_setindex(&g->ay8910, hardware->m0[0x301]);
          g->psg = hardware->m0[0x301] & 0x0F;
        } else {
          ay8910_setvalue(&g->ay8910, hardware->m0[0x301]);
          switch (g->psg) {	// CA2=0, 8912 data
            case 14:	// port A
              g->kbselect2 = (~hardware->m0[0x301]) & 0xFF;
          }
        }
      }
      break;
    case 0x30D:	// interrupt flag register
      if (b & INT_ANY)
        b = 0x7F;
      via_clearint(0, b);
      break;
    case 0x30E:	// interrupt enable register
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
          hardware->irq_request = 1;
        }
      }
  }
}

void oric_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  OricGlobals *globals;
  KeyMap *keyMap;
  UInt16 i, len;
  UInt16 area_height;

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

  area_height = hardware->display_height - lowBorder * 2;
  hardware->x0 = (hardware->display_width  - SCREEN_WIDTH) / 2;
  hardware->y0 = (hardware->display_height - TEXT_HEIGHT - lowBorder * 2) / 2;

  if (!video) {
    video = MemPtrNew(TOTAL_SIZE);
    MemSet(video, TOTAL_SIZE, 0);
  }

  if (!hardware->globals) {
    hardware->globals = MemPtrNew(sizeof(OricGlobals));
    MemSet(hardware->globals, sizeof(OricGlobals), 0);
  }
  globals = hardware->globals;

  globals->frame = 0;
  globals->area_height = area_height;
  globals->border = 1;
  globals->text = 1;
  globals->blink_status = 0;
  globals->ier[0] = 0;
  globals->ifr[0] = 0;
  globals->ddra[0] = 0;
  globals->ddrb[0] = 0;
  globals->pcr[0] = 0;
  globals->kbselect1 = 0;
  globals->kbselect2 = 0;
  globals->latch1 = 0;
  globals->counter1 = 0;
  globals->psg = 0;
  globals->motor = 0;
  globals->button = prefs.button;
  globals->ctrl = 0;
  globals->video = video;

  keyMap = MemPtrNew(256 * sizeof(KeyMap));
  kbd_initmap(OricMap, keyMap);

  for (i = 0; i < 256; i++) {
    globals->keyMap[i].key = (keyMap[i].key);
    globals->keyMap[i].line = (keyMap[i].line);
    globals->keyMap[i].column = (keyMap[i].column);
    globals->keyMap[i].shift = (keyMap[i].shift);
    globals->keyMap[i].ctrl = (keyMap[i].ctrl);
  }
  MemPtrFree(keyMap);

  ay8910_init(&globals->ay8910, hardware, AY_3_8910, 2000000L);

  hardware->dx = SCREEN_WIDTH;
  hardware->dy = TEXT_HEIGHT;
}

void oric_finish(void) {
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

void oric_key(UInt16 c) {
  OricGlobals *g;

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

void oric_joystick(UInt16 x, UInt16 y) {
  hardware->joyx = x;
  hardware->joyy = y;
}

// 9800 - 9FFF: character map for last 3 lines in graphic mode
// A000 - BF3F: graphic video ram (200 * 40 = 8000 bytes)
// B400 - B7FF: standard character map
// B800 - BC00: alternate character map
// BB80 - BFDF: text video ram (28 * 40 = 1120 bytes)


// The cassette circuitry connects to the CB1 line on the 6522. When this goes from low to high the CB1 flag is set in the interrupt flag register of the 6522. T2 is used to count time before looking at the CB1 flag, and each bit is built up into a whole byte using a series of rotate instructions.

// The cassette relay connection is activated before any of the cassette routines by setting bit 6 of port B on the 6522. This bit is cleared after all cassette operations, deactivating the relay.

// During cassette loading the timers operate in a different fashion:

// T2, which is idle at other times, is used when receiving bits from the cassette input port in order to wait an exact amount of time.

// The function of T1 is altered (by setting bits 6 and 7 of the auxiliary control register) so that instead of causing an interrupt bit 7 of port B is toggled and the timer is automatically set running again.

// When a cassette operation is complete, the registers in the 6522 are set back to their initial values in order for the keyboard and printer to work normally.


Err oric_readtap(FileRef f, m6502_Regs *m6502, Hardware *hardware) {
  UInt32 size, r, aux, start, end;
  UInt16 i, type;
  UInt8 buf[32];
  Err err;

  // 00000000  1616 1624 0000 00c7 27c2 0501 0048 4f56   ...$....'....HOV
  // 00000010  4552 2e54 4150 0008 055e 009d 2a00 2d05   ER.TAP...^..*.-.

  if ((err = VFSFileSize(f, &size)) != 0 || size < 16)
    return -1;

  aux = 4;
  if ((err = VFSFileRead(f, aux, buf, &r)) != 0)
    return -2;
  if (r != aux)
    return -3;

  if (buf[0] != 0x16 || buf[1] != 0x16 || buf[2] != 0x16 || buf[3] != 0x24)
    return -4;

  aux = 9;
  if ((err = VFSFileRead(f, aux, buf, &r)) != 0)
    return -5;
  if (r != aux)
    return -6;

  type = buf[2];
  start = (buf[6] * 256 + buf[7]) & 0xFFFF;
  end = (buf[4] * 256 + buf[5]) & 0xFFFF;

  if (start < 0x500 || end >= (0xC000 - 16) || start >= end)
    return -7;

  for (i = 0; i < 32; i++) {
    if ((err = VFSFileRead(f, 1, &buf[i], &r)) != 0)
      return -8;
    if (buf[i] == 0)
      break;
  }

  if (i == 32)
    return -9;

  if (end < 0x8000) {
    aux = end - start;
    if ((err = VFSFileRead(f, aux, &hardware->m0[start], &r)) != 0)
      return -10;
    if (r != aux)
      return -11;
  } else if (start >= 0x8000) {
    aux = end - start;
    if ((err = VFSFileRead(f, aux, &hardware->m1[start & 0x7FFF], &r)) != 0)
      return -12;
    if (r != aux)
      return -13;
  } else {
    aux = 0x8000 - start;
    if ((err = VFSFileRead(f, aux, &hardware->m0[start], &r)) != 0)
      return -14;
    if (r != aux)
      return -15;

    aux = end - 0x8000;
    if ((err = VFSFileRead(f, aux, hardware->m1, &r)) != 0)
      return -16;
    if (r != aux)
      return -17;
  }

  // #86  #87  Address of last temporary string.
  // #88  #90  A table of temporary strings.
  // #9A  #9B  Start of BASIC pointer.
  // #9C  #9D  Start of variables pointer.
  // #9E  #9F  Start of arrays pointer.
  // #A0  #A1  End of arrays pointer.
  // #A2  #A3  Pointer to next free string space.
  // #A6  #A7  Highest available memory location available to BASIC.

  if (type == 0x00) {	// BASIC
    if (end< 0x8000) {
      hardware->m0[end] = 0x00;
      hardware->m0[end+1] = 0x00;
    } else {
      hardware->m1[end & 0x7FFF] = 0x00;
      hardware->m1[(end+1) & 0x7FFF] = 0x00;
    }

    end += 2;
    hardware->m0[0x9C] = end & 0xFF;
    hardware->m0[0x9D] = (end >> 8) & 0xFF;
    hardware->m0[0x9E] = end & 0xFF;
    hardware->m0[0x9F] = (end >> 8) & 0xFF;

    end += 7;
    hardware->m0[0xA0] = end & 0xFF;
    hardware->m0[0xA1] = (end >> 8) & 0xFF;
  } else {		// machine code
    m6502->pc_reg.w.h = start;
  }

  return 0;
}

OricPrefs *OricGetPrefs(void) {
  return &prefs;
}

Boolean OricFormHandler(EventPtr event, Boolean *close) {
  FormPtr frm;
  ListPtr lst;
  UInt32 button;
  OricGlobals *g;
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
