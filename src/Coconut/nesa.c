#include <PceNativeCall.h>
#include <CoreTraps.h>
#include <Standalone.h>
STANDALONE_CODE_RESOURCE_ID(1);

#include "cpu.h"
#include "io.h"
#include "endian.h"
#include "armlet.h"
#include "kbd.h"
#include "nes.h"
#include "gui.h"
#include "arm.h"

/*
Real               Emulador
         NMI
0..19  : vblank    241..260
20     : dummy     261
21..260: screen    0..239
261    : nothing   240

Nintencer:

0          : nothing
1 (0..11)  : vblank
             NMI
1 (12..340): vblank
2..20      : vblank
21         : dummy
22..261    : screen

*/

#define LINE_SCREEN0	0
#define LINE_SCREEN1	239
#define LINE_NOTHING	240
#define LINE_VBLANK0	241
#define LINE_VBLANK1	260
#define LINE_DUMMY	261

#define SCREEN_WIDTH	256
#define SCREEN_HEIGHT	240

void arm_nes_event(Hardware *hardware, UInt32 e, UInt32 v);
void arm_nes_vsync(Hardware *hardware, UInt32 v);
void arm_nes_video(Hardware *hardware, UInt8 *s,
         Int32 width, Int32 height, Int32 x0, Int32 x1, Int32 y0, Int32 y1);
void arm_nes_writeb(Hardware *hardware, UInt16 a, UInt8 b);
UInt8 arm_nes_readb(Hardware *hardware, UInt16 a);

static void nes_drawline(UInt8 *s, UInt16 *color, Int32 width, Int32 row,
		 Int32 x0, Int32 y0, Int32 x1, Int32 y1, Int32 dx, Int32 sy,
		 UInt8 *pattern, UInt8 *name, UInt8 *attr, UInt8 *pallete,
		 NesVar *v);

static void nes_sprite(UInt8 *s, UInt16 *color, Int32 width, Int32 behind,
		       Int32 x0, Int32 y0, Int32 x1, Int32 y1,
		       UInt8 *pattern, UInt8 *pattern2, UInt8 *pallete,
		       UInt8 *spram, NesVar *v);

static void nes_cls(Hardware *hardware, UInt8 *s, Int32 width,
		    Int32 x0, Int32 y0, Int32 x1, Int32 y1, UInt8 c);

static UInt8 nes_read_joy(Hardware *hardware);
static void nes_calc_hit(Hardware *hardware);
static void nes_set_mirror(Hardware *hardware, UInt8 **ntram, UInt32 mirror);
static void nes_switch_prg(Hardware *hardware);
static void nes_switch_chr(Hardware *hardware, UInt8 **p, UInt32 chr0, UInt32 chr1);
static void nes_switch_chr2(Hardware *hardware, UInt8 **vrom, UInt32 *chr_idx);
static UInt32 nes_mmc1_bank(UInt32 mode, UInt32 f1, UInt32 f2, UInt32 f3);

UInt32 nes_callback(ArmletCallbackArg *arg)
{
  switch (arg->cmd) {
    case IO_EVENT:
      arm_nes_event(arg->hardware, arg->a1, arg->a2);
      break;
    case IO_VSYNC:
      arm_nes_vsync(arg->hardware, arg->a1);
      break;
    case IO_READB:
      return arm_nes_readb(arg->hardware, arg->a1);
    case IO_WRITEB:
      arm_nes_writeb(arg->hardware, arg->a1, arg->a2);
  }
  return 0;
}

void arm_nes_event(Hardware *hardware, UInt32 e, UInt32 a)
{
  NesGlobals *g;
  Int32 i, width, height, x0, x1, y0, y1;
  UInt8 *s, *r;

  g = hardware->a.globals;
  g->line = e;

/*
  if (g->v.show_bg || g->v.show_sp) {

    // XXX DoLine() -> Fixit1()

    if (g->line > LINE_SCREEN0 && g->line <= LINE_NOTHING) {
      if ((g->v.tmp_addr & 0x7000) == 0x7000) {
        g->v.tmp_addr ^= 0x7000;

        if ((g->v.tmp_addr & 0x3E0) == 0x3A0) {
          g->v.tmp_addr ^= 0x3A0;
          g->v.tmp_addr ^= 0x800;

        } else {
          if ((g->v.tmp_addr & 0x3E0) == 0x3e0)
            g->v.tmp_addr ^= 0x3e0;
          else
            g->v.tmp_addr += 0x20;
        }
      } else
        g->v.tmp_addr += 0x1000;
    }

    // XXX DoLine() -> EndRL() -> RefreshLine() -> Fixit2()

    if (g->line == LINE_SCREEN0) {
      // frame start (if background and sprites are enabled):
      // vram_addr = tmp_addr
      g->vram_addr = g->v.tmp_addr;

    } else if (g->line <= LINE_NOTHING) {
      // scanline start (if background and sprites are enabled):
      //           FEDC BA98 7654 3210     FEDC BA98 7654 3210
      // vram_addr:0000 0100 0001 1111 = t:0000 0100 0001 1111
      g->vram_addr = (g->vram_addr & 0xFBE0) | (g->v.tmp_addr & 0x041F);
    }
  }
*/

  g->a[g->line].tmp_addr = g->v.tmp_addr;
  g->a[g->line].dx = g->v.dx;
  g->a[g->line].pattern_bg = g->v.pattern_bg;
  g->a[g->line].show_bg = g->v.show_bg;
  g->a[g->line].mirror = g->v.mirror;
  g->a[g->line].chr0 = g->v.chr0;
  g->a[g->line].chr1 = g->v.chr1;

  for (i = 0; i < 8; i++)
    g->a[g->line].chr_idx[i] = g->v.chr_idx[i];

  switch (g->line) {
    case LINE_SCREEN0:
      break;

    case LINE_SCREEN1:
      break;

    case LINE_NOTHING:
      g->frame++;

      if ((g->frame & 1) && g->dirty) {
        nes_cls(hardware, g->abuffer, SCREEN_WIDTH, 0, 8,
          SCREEN_WIDTH, SCREEN_HEIGHT-8, hardware->a.color[g->vram[0x3F00]]);

        arm_nes_video(hardware, g->abuffer, SCREEN_WIDTH, SCREEN_HEIGHT,
          0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);

        width = hardware->a.display_width;
        height = hardware->a.display_height - lowBorder * 2;
        x0 = (width  - SCREEN_WIDTH) / 2;
        y0 = (height - SCREEN_HEIGHT) / 2;
        x1 = x0 + SCREEN_WIDTH;
        y1 = y0 + SCREEN_HEIGHT;

        s = WinScreenLock(hardware);
        s += (y0 + 8) * width + x0;
        r = g->abuffer + 8 * SCREEN_WIDTH;
        for (i = 0; i < SCREEN_HEIGHT-16; i++, s += width, r += SCREEN_WIDTH)
          MemMove(s, r, SCREEN_WIDTH);
        WinScreenUnlock(hardware);

        g->dirty = 0;
      }

      arm_nes_vsync(hardware, 0);
      break;

    case LINE_VBLANK0:
      g->vblank = 1;
      g->hit = 0;

      if (g->intenable)
        hardware->nmi_request = 1;
      break;

    case LINE_VBLANK1:
      break;

    case LINE_DUMMY:
      g->vblank = 0;
      break;
  }
}

void arm_nes_vsync(Hardware *hardware, UInt32 v)
{
  Call68KFuncType *f = (Call68KFuncType *)hardware->a.call68KFuncP;
  ArmletArg *aarg = (ArmletArg *)hardware->a.userData68KP;
  ArmletCallback *c = aarg->callback;
  ArmletCallbackArg arg, *p;

  arg.cmd = IO_VSYNC2;
  p = (ArmletCallbackArg *)ByteSwap32(&arg);
  f(hardware->a.emulStateP, (unsigned long)c, &p, sizeof(void *));
}

void arm_nes_video(Hardware *hardware, UInt8 *s,
         Int32 width, Int32 height, Int32 x0, Int32 x1, Int32 y0, Int32 y1)
{
  NesGlobals *g;
  Int32 i, row, nametable1, nametable2, vscroll, y, dy;
  UInt8 *pattern, *pattern2, *name, *attr, *pallete;
  UInt8 *nt, *p[2], *ntram[4], *vrom[8], **pt;

  g = hardware->a.globals;

  if (g->v.show_sp) {
    if (g->v.double_sp) {
      pattern  = hardware->a.p[0];
      pattern2 = hardware->a.p[1];
    } else {
      pattern  = hardware->a.p[g->v.pattern_sp];
      pattern2 = NULL;
    }
    pallete  = &g->vram[0x3F10];

    nes_sprite(s, hardware->a.color, width, 1,
      x0, y0, x1, y1, pattern, pattern2, pallete, g->sp_ram, g->a);
  }

  pallete = &g->vram[0x3F00];

  // As PPU is drawing the background, it updates the address to point to the
  // nametable data currently being drawn.
  // bits  0-11 hold the nametable address (- 0x2000)
  // bits 12-14 are the tile Y offset

  // nametables:
  // 0  1
  // 2  3

  for (y = 0; y < SCREEN_HEIGHT; y += 8) {
    if (g->a[y].show_bg) {

      dy = (g->a[y].tmp_addr & 0x7000) >> 12;
      vscroll = ((g->a[y].tmp_addr & 0x3E0) >> 2) | dy;
      row = (y + vscroll) >> 3;

      if (row < 30) {
        nametable1 = 0x2000 +  (g->a[y].tmp_addr & 0x0C00);
        nametable2 = 0x2000 + ((g->a[y].tmp_addr & 0x0C00) ^ 0x0400);
      } else if (row < 60) {
        row -= 30;
        nametable1 = 0x2000 + ((g->a[y].tmp_addr & 0x0C00) ^ 0x0800);
        nametable2 = 0x2000 + ((g->a[y].tmp_addr & 0x0C00) ^ 0x0C00);
      } else
        continue;

      if (g->chr_count > 0) {
        nes_switch_chr2(hardware, vrom, g->a[y].chr_idx);
        //for (i = 0; i < 4; i++)
          //pt[i] = vrom[i + ((g->a[y].pattern_bg == 0) ? 0 : 4)];

        nes_switch_chr(hardware, p, g->a[y].chr0, g->a[y].chr1);
        pattern = p[g->a[y].pattern_bg];

      } else {
        //for (i = 0; i < 4; i++)
          //pt[i] = g->vrom[i + ((g->a[y].pattern_bg == 0) ? 0 : 4)];

        pattern = hardware->a.p[g->a[y].pattern_bg];
      }

      nes_set_mirror(hardware, ntram, g->a[y].mirror);

      nt = &ntram[(nametable1 >> 10) & 0x03][nametable1 & 0x3FF];
      name = nt + (row << 5);
      attr = nt + 960;

      nes_drawline(s, hardware->a.color, width, row,
                   x0, y0, x1, y1, 0, y + y0 - dy,
                   pattern, name, attr, pallete, g->a);

      nt = &ntram[(nametable2 >> 10) & 0x03][nametable2 & 0x3FF];
      name = nt + (row << 5);
      attr = nt + 960;

      nes_drawline(s, hardware->a.color, width, row,
                   x0, y0, x1, y1, SCREEN_WIDTH, y + y0 - dy,
                   pattern, name, attr, pallete, g->a);
    }
  }

  if (g->v.show_sp) {
    if (g->v.double_sp) {
      pattern  = hardware->a.p[0];
      pattern2 = hardware->a.p[1];
    } else {
      pattern  = hardware->a.p[g->v.pattern_sp];
      pattern2 = NULL;
    }
    pallete  = &g->vram[0x3F10];

    nes_sprite(s, hardware->a.color, width, 0,
      x0, y0, x1, y1, pattern, pattern2, pallete, g->sp_ram, g->a);
  }
}

static void nes_cls(Hardware *hardware, UInt8 *s, Int32 width,
                    Int32 x0, Int32 y0, Int32 x1, Int32 y1, UInt8 c)
{
  UInt8 *r;
  Int32 y, dx;

  r = s + y0 * width + x0;
  dx = x1 - x0;

  for (y = y0; y < y1; y++, r += width)
    MemSet(r, dx, c);
}

  /*

        0     1     2     3     4     5     6     7
   0: 00-03 04-07 08-11 12-15 16-19 20-23 24-27 28-31

        0     1     2     3     4     5     6     7
   1: 00-03 04-07 08-11 12-15 16-19 20-23 24-27 28-31

        0     1     2     3     4     5     6     7
   2: 00-03 04-07 08-11 12-15 16-19 20-23 24-27 28-31

        0     1     2     3     4     5     6     7
   3: 00-03 04-07 08-11 12-15 16-19 20-23 24-27 28-31

        8     9     10    11    12    13    14    15
   4: 00-03 04-07 08-11 12-15 16-19 20-23 24-27 28-31

        8     9     10    11    12    13    14    15
   5: 00-03 04-07 08-11 12-15 16-19 20-23 24-27 28-31

  offset = ((y >> 2) << 3) + (x >> 2)
  Ex.: y=5, x=29
       ((5 >> 2) << 3) + (29 >> 2) = (1 << 3) + (7) = 8 + 7 = 15

  +----------+-----------+
  | Square A |  Square B |
  |  0  1    |   4  5    |
  |  2  3    |   6  7    |
  +----------+-----------+
  | Square C |  Square D |
  |  8  9    |   C  D    |
  |  A  B    |   E  F    |
  +-----------+----------+

  33221100
  ||||||+--- upper 2 color bits for square A (tiles 0,1,2,3)
  ||||+----- upper 2 color bits for square B (tiles 4,5,6,7)
  ||+------- upper 2 color bits for square C (tiles 8,9,A,B)
  +--------- upper 2 color bits for square D (tiles C,D,E,F)

      0                   1                   2                   3
   0: 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      0 0 2 2 0 0 2 2 0 0 2 2 0 0 2 2 0 0 2 2 0 0 2 2 0 0 2 2 0 0 2 2

      0                   1                   2                   3
   1: 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      0 0 2 2 0 0 2 2 0 0 2 2 0 0 2 2 0 0 2 2 0 0 2 2 0 0 2 2 0 0 2 2

      0                   1                   2                   3
   2: 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      4 4 6 6 4 4 6 6 4 4 6 6 4 4 6 6 4 4 6 6 4 4 6 6 4 4 6 6 4 4 6 6

      0                   1                   2                   3
   3: 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      4 4 6 6 4 4 6 6 4 4 6 6 4 4 6 6 4 4 6 6 4 4 6 6 4 4 6 6 4 4 6 6

   bitpos = (x & 0x02) | ((y & 0x02) << 1)
  */

static void nes_drawline(UInt8 *s, UInt16 *color, Int32 width, Int32 row,
                 Int32 x0, Int32 y0, Int32 x1, Int32 y1, Int32 dx, Int32 sy,
                 UInt8 *pattern, UInt8 *name, UInt8 *attr, UInt8 *pallete,
                 NesVar *v)
{
  UInt32 code, caddr, mask1, mask2, c, c1, c2, offset, bitpos, hscroll;
  Int32 j, k, col, col1, col2, sx, cx, sy0;
  UInt8 *r;

  if (sy < y0)
    hscroll = ((v[0].tmp_addr & 0x1F) << 3) | v[0].dx;
  else if (sy >= y1)
    hscroll = ((v[SCREEN_HEIGHT-1].tmp_addr & 0x1F) << 3) | v[SCREEN_HEIGHT-1].dx;
  else
    hscroll = ((v[sy - y0].tmp_addr & 0x1F) << 3) | v[sy - y0].dx;

  sy0 = sy;

  if (dx) {
    col1 = 0;
    col2 = (hscroll + 7) / 8;
  } else {
    col1 = hscroll / 8;
    col2 = 32;
  }

  for (col = col1; col < col2; col++) {
    code = name[col];
    caddr = code << 4;
    offset = ((row >> 2) << 3) + (col >> 2);
    bitpos = (col & 0x02) | ((row & 0x02) << 1);
    c2 = ((attr[offset] >> bitpos) & 0x03) << 2;	// bits 2-3

    for (j = 0, sy = sy0; j < 8; j++, sy++) {
      if (sy >= y0+8 && sy < y1-8) {
        mask1 = pattern[caddr];				// bit 0
        mask2 = pattern[caddr + 8] << 1;		// bit 1

        hscroll = ((v[sy - y0].tmp_addr & 0x1F) << 3) | v[sy - y0].dx;
        sx = x0 + dx + (col << 3) - hscroll;

        r = s + sy * width + sx;
        cx = v[sy - y0].clip_bg ? x0 + 8 : x0;

        for (k = 7; k >= 0; k--) {
          if ((sx + k) >= cx && (sx + k) < x1) {
            c1 = (mask1 & 0x01) | (mask2 & 0x02);	// bits 0-1
            if (c1) {
              c = pallete[c1 | c2] & 0x3F;
              r[k] = color[c];
            }
          }

          mask1 >>= 1;
          mask2 >>= 1;
        }
      }

      r += width;
      caddr++;
    }
  }
}

static void nes_sprite(UInt8 *s, UInt16 *color, Int32 width, Int32 behind,
                       Int32 x0, Int32 y0, Int32 x1, Int32 y1,
                       UInt8 *pattern, UInt8 *pattern2, UInt8 *pallete,
                       UInt8 *spram, NesVar *v)
{
  // Byte 0: y coordinate
  // Byte 1: tile index in pattern table
  // Byte 2: attributes
  //   Bits 0-1: upper 2 color bits
  //   Bits 2-4: unused
  //   Bit 5: priority (0=in front, 1=behind)
  //   Bit 6: horizontal flip (1=flip)
  //   Bit 7: vertical flip (1=flip)
  // Byte 3: x coordinate

  // If a sprite has the the Sprite Priority bit set and has a higher priority
  // than a sprite at the same location, then the background will be in front
  // of both sprites. Konami's "Castlevania" cart uses this feature.

  // Sprites which are 8x16 in size function a little bit differently. A
  // 8x16 sprite which is even-numbered uses the Pattern Table at $0000 in
  // VRAM. If the Sprite Tile # is odd, the sprite uses $1000 in VRAM.
  // NOTE: Register $2000 has no effect on 8x16 sprites.

  UInt32 code, caddr, mask1, mask2, c, c1, c2;
  Int32 i, j, k, x, y, sx, sy, cx, vflip, hflip, sp_behind, first;
  UInt8 *r, *p, attr;

  spram += 252;

  for (i = 63; i >= 0; i--, spram -= 4) {
    code = spram[1];
    attr = spram[2];
    x = spram[3];
    y = spram[0];

    caddr = code << 4;
    vflip = attr & 0x80;
    hflip = (attr & 0x40) ? 7 : 0;
    sp_behind = (attr & 0x20) ? 1 : 0;
    c2 = (attr & 0x03) << 2;				// bits 2-3

    if (sp_behind != behind || y >= SCREEN_HEIGHT)
      continue;

    sx = x0 + x;
    sy = y0 + y;
    r = s + sy * width + sx;

    if (pattern2)
      p = (code & 1) ? pattern2 : pattern;
    else
      p = pattern;

    first = 1;
    if (pattern2 && vflip)
      caddr += 16;

    for (j = 0; j < 8; j++, sy++) {
      if (sy >= y0+8 && sy < y1-8) {
        if (vflip) {
          mask1 = p[caddr + 7 - j];
          mask2 = p[caddr + 7 - j + 8] << 1;
        } else {
          mask1 = p[caddr + j];
          mask2 = p[caddr + j + 8] << 1;
        }

        k = hflip ? 0 : 7;
        cx = v[y + j].clip_sp ? x0 + 8 : x0;

        for (;;) {
          if ((sx + k) >= cx && (sx + k) < x1) {
            c1 = (mask1 & 0x01) | (mask2 & 0x02);	// bits 0-1
            if (c1) {
              c = pallete[c1 | c2] & 0x3F;
              r[k] = color[c];
            }
          }

          if (hflip) {
            k++;
            if (k == 8)
              break;
          } else {
            k--;
            if (k < 0)
              break;
          }

          mask1 >>= 1;
          mask2 >>= 1;
        }
      }

      r += width;

      if (pattern2 && first && j == 7) {
        j = -1;
        if (vflip)
          caddr -= 16;
        else
          caddr += 16;
        first = 0;
      }
    }
  }
}

UInt8 arm_nes_readb(Hardware *hardware, UInt16 a)
{
  NesGlobals *g;
  UInt16 addr;
  UInt8 b;

  g = hardware->a.globals;

  switch (a) {
    case 0x2002:	// PPU Status Register (R)
      b = 0x1F;

      if (g->vblank) {
        b |= 0x80;
        g->vblank = 0;
      }

      if (g->hit)
        b |= 0x40;

      g->phase = 0;
      return b;

    case 0x2004:	// Sprite RAM I/O Register (RW)
      b = 0xFF;
      //b = g->sp_ram[g->sp_addr];
      //g->sp_addr = (g->sp_addr + 1) & 0xFF;
      return b;

    case 0x2007:	// VRAM I/O Register (RW)
      if (g->vram_addr >= 0x3000 && g->vram_addr < 0x3F00)
        addr = g->vram_addr & 0x2FFF;
      else if (g->vram_addr >= 0x3F20 && g->vram_addr < 0x4000)
        addr = g->vram_addr & 0x3F1F;
      else
        addr = g->vram_addr;

      switch (addr & 0xF000) {
        case 0x0000:
        case 0x1000:
          //b = g->vrom[(addr >> 10) & 0x07][addr & 0x3FF];
          b = hardware->a.p[addr >> 12][addr & 0xFFF];
          break;
        case 0x2000:
          b = g->ntram[(addr >> 10) & 0x03][addr & 0x3FF];
          break;
        default:
          b = g->vram[g->vram_addr];
      }

      g->vram_addr = (g->vram_addr + g->v.addr_incr) & 0x3FFF;
      return b;

    case 0x4016:	// Joypad 1 (RW)
      if (g->joystick == 0)
        return nes_read_joy(hardware);
      return 0x00;

    case 0x4017:	// Joypad 2 (RW)
      if (g->joystick == 1)
        return nes_read_joy(hardware);
      return 0x00;
  }

  return 0x00;
}

static UInt8 nes_read_joy(Hardware *hardware)
{
  NesGlobals *g;
  UInt8 b;

  g = hardware->a.globals;
  b = 0x00;

  // 0 = A          8 = A         16 = +--+
  // 1 = B          9 = B         17 =    +-- Signature
  // 2 = SELECT    10 = SELECT    18 =    |
  // 3 = START     11 = START     19 = +--+
  // 4 = UP        12 = UP        20 = 0
  // 5 = DOWN      13 = DOWN      21 = 0
  // 6 = LEFT      14 = LEFT      22 = 0
  // 7 = RIGHT     15 = RIGHT     23 = 0

  // Signature:
  // 0000 = Disconnected
  // 0001 = Joypad ($4016 only)
  // 0010 = Joypad ($4017 only)

  switch (g->joy_state) {
    case 0:
      if (hardware->m.button & g->button1)
        b |= 0x01;
      break;
    case 1:
      if (hardware->m.button & g->button2)
        b |= 0x01;
      break;
    case 2:
      if (hardware->a.key == KEY_SEL)
        b |= 0x01;
      break;
    case 3:
      if (hardware->a.key == KEY_START)
        b |= 0x01;
      break;
    case 4:
      if (hardware->a.joyy < 16)
        b |= 0x01;
      break;
    case 5:
      if (hardware->a.joyy > 48)
        b |= 0x01;
      break;
    case 6:
      if (hardware->a.joyx < 16)
        b |= 0x01;
      break;
    case 7:
      if (hardware->a.joyx > 48)
        b |= 0x01;
      break;
    case 16:
      break;
    case 17:
      break;
    case 18:
      if (g->joystick == 1)
        b |= 0x01;
      break;
    case 19:
      if (g->joystick == 0)
        b |= 0x01;
  }

  if (g->joy_state == 23)
    g->joy_state = 0;
  else
    g->joy_state++;

  return b;
}

void arm_nes_writeb(Hardware *hardware, UInt16 a, UInt8 b)
{
  NesGlobals *g;
  UInt32 i;
  UInt16 addr;

  g = hardware->a.globals;

  if (a >= 0x8000) {	// memory mappers
    switch (hardware->a.banksw[1]) {
      case 1:	// MMC1
        if (b & 0x80) {
          g->mmc1_value = 0;
          g->mmc1_count = 0;

          g->prg_addr = 0x8000;
          g->prg_size = 0x4000;
          nes_switch_prg(hardware);
          break;
        }

        g->mmc1_value = (g->mmc1_value >> 1) | ((b & 0x01) ? 0x10: 0x00);
        g->mmc1_count++;

        if (g->mmc1_count < 5)
          break;

        switch ((a >> 13) & 0x03) {

          case 0:	// MMC1 register 0

            if (g->mmc1_value & 0x02) {
              // regular mirroring
              if (g->mmc1_value & 0x01)
                g->v.mirror = MIRROR_2H;
              else
                g->v.mirror = MIRROR_2V;
            } else {
              // one screen mirroring
              if (g->mmc1_value & 0x01)
                g->v.mirror = MIRROR_1U;
              else
                g->v.mirror = MIRROR_1L;
            }

            g->chr_size = (g->mmc1_value & 0x10) ? 0x1000 : 0x2000;
            g->prg_addr = (g->mmc1_value & 0x04) ? 0x8000 : 0xC000;
            g->prg_size = (g->mmc1_value & 0x08) ? 0x4000 : 0x8000;
            g->mmc1_f1 = (g->mmc1_value & 0x10) ? 1: 0;
            g->prg_bank = nes_mmc1_bank(g->mmc1_mode,
                              g->mmc1_f1, g->mmc1_f2, g->mmc1_f3);

            nes_set_mirror(hardware, g->ntram, g->v.mirror);

            nes_switch_chr2(hardware, g->vrom, g->v.chr_idx);
            nes_switch_chr(hardware, hardware->a.p, g->v.chr0, g->v.chr1);

            nes_switch_prg(hardware);
            break;

          case 1:	// MMC1 register 1
            g->v.chr_idx[0] = g->mmc1_value & 0x0F;
            g->v.chr_idx[1] = g->mmc1_value & 0x0F;
            g->v.chr_idx[2] = g->mmc1_value & 0x0F;
            g->v.chr_idx[3] = g->mmc1_value & 0x0F;
            g->v.chr0 = g->mmc1_value & 0x0F;

            g->mmc1_f2 = (g->mmc1_value & 0x10) ? 1: 0;
            g->prg_bank = nes_mmc1_bank(g->mmc1_mode,
                              g->mmc1_f1, g->mmc1_f2, g->mmc1_f3);

            nes_switch_chr2(hardware, g->vrom, g->v.chr_idx);
            nes_switch_chr(hardware, hardware->a.p, g->v.chr0, g->v.chr1);

            nes_switch_prg(hardware);
            break;

          case 2:	// MMC1 register 2
            g->v.chr_idx[4] = g->mmc1_value & 0x0F;
            g->v.chr_idx[5] = g->mmc1_value & 0x0F;
            g->v.chr_idx[6] = g->mmc1_value & 0x0F;
            g->v.chr_idx[7] = g->mmc1_value & 0x0F;
            g->v.chr1 = g->mmc1_value & 0x0F;

            g->mmc1_f3 = (g->mmc1_value & 0x10) ? 1: 0;
            g->prg_bank = nes_mmc1_bank(g->mmc1_mode,
                              g->mmc1_f1, g->mmc1_f2, g->mmc1_f3);

            nes_switch_chr2(hardware, g->vrom, g->v.chr_idx);
            nes_switch_chr(hardware, hardware->a.p, g->v.chr0, g->v.chr1);

            nes_switch_prg(hardware);
            break;

          case 3:	// MMC1 register 3
            // Bit 4: WRAM enable:
            // 0: WRAM is enabled and can be read/written to
            // 1: WRAM is disabled and cannot be accessed at all

            g->prg_idx = g->mmc1_value & 0x0F;
            nes_switch_prg(hardware);
            break;
        }

        g->mmc1_value = 0;
        g->mmc1_count = 0;

        break;

      case 2:	// UNROM
        // 8000-FFFF: bits 0-3 have the 16K bank # to load at 8000
        g->prg_idx = b & 0x0F;
        nes_switch_prg(hardware);
        break;

      case 3:	// CNROM
        // 8000-FFFF: bits 0-1 have the 8K bank # to load at PPU 0000
        for (i = 0; i < 8; i++)
          g->v.chr_idx[i] = b & 0x03;
        g->v.chr0 = g->v.chr1 = b & 0x03;

        nes_switch_chr2(hardware, g->vrom, g->v.chr_idx);
        nes_switch_chr(hardware, hardware->a.p, g->v.chr0, g->v.chr1);
        break;

      case 7:	// AOROM
        // 8000-FFFF: bits 0-3 have the 32K bank # to load at 8000
        g->prg_idx = (b & 0x0F) << 1;
        g->v.mirror = (b & 0x10) ? MIRROR_1U : MIRROR_1L;
        nes_set_mirror(hardware, g->ntram, g->v.mirror);
        nes_switch_prg(hardware);
        break;
    }
    return;
  }

  switch (a) {
    case 0x2000:	// PPU Control Register 1 (W)
      // XXX
      if (!g->intenable && (b & 0x80) && g->vblank)
        hardware->nmi_request = 1;

      g->intenable = b & 0x80;
      g->v.double_sp = b & 0x20;
      g->v.pattern_bg = (b & 0x10) ? 1 : 0;
      g->v.pattern_sp = (b & 0x08) ? 1 : 0;
      g->v.addr_incr = (b & 0x04) ? 32 : 1;
      g->dirty = 1;

      //          FEDC BA98 7654 3210     7654 3210
      // tmp_addr:0000 1100 0000 0000 = b:0000 0011

      g->v.tmp_addr = (g->v.tmp_addr & 0xF3FF) | ((b & 0x03) << 10);

      break;

    case 0x2001:	// PPU Control Register 2 (W)
      g->v.show_sp = b & 0x10;
      g->v.show_bg = b & 0x08;
      g->v.clip_sp = !(b & 0x04);
      g->v.clip_bg = !(b & 0x02);
      g->v.mono = b & 0x01;
      g->v.lumi = b >> 5;
      g->dirty = 1;
      break;

    case 0x2003:	// Sprite RAM Address Register (W)
      g->sp_addr = b;
      break;

    case 0x2004:	// Sprite RAM I/O Register (RW)
      if (g->sp_ram[g->sp_addr] != b) {
        g->sp_ram[g->sp_addr] = b;
        g->dirty = 1;

        if (g->sp_addr < 4)	// sprite 0
          nes_calc_hit(hardware);
      }

      g->sp_addr = (g->sp_addr + 1) & 0xFF;
      break;

    case 0x2005:	// Background Scroll Register (W2)
      if (g->phase == 0) {
        //          FEDC BA98 7654 3210     7654 3210
        // tmp_addr:0000 0000 0001 1111 = b:1111 1000

        g->v.tmp_addr = (g->v.tmp_addr & 0xFFE0) | ((b & 0xF8) >> 3);

        //    210     7654 3210
        // dx:111 = b:0000 0111

        g->v.dx = b & 0x07;

        g->dirty = 1;

      } else {
        //          FEDC BA98 7654 3210     7654 3210
        // tmp_addr:0000 0011 1110 0000 = b:1111 1000
        // tmp_addr:0111 0000 0000 0000 = b:0000 0111

        g->v.tmp_addr = (g->v.tmp_addr & 0x8C1F) | ((b & 0xF8) << 2) |
                                                   ((b & 0x07) << 12);
        g->dirty = 1;
      }

      g->phase = !g->phase;
      break;

    case 0x2006:	// VRAM Address Register (W2)
      if (g->line < SCREEN_HEIGHT)
        hardware->a.banksw[2]++;

      if (g->phase == 0) {
        //          FEDC BA98 7654 3210     7654 3210
        // tmp_addr:0011 1111 0000 0000 = d:0011 1111
        // tmp_addr:1100 0000 0000 0000 = 0

        g->v.tmp_addr = (g->v.tmp_addr & 0x00FF) | ((b & 0x3F) << 8);

      } else {
        //          FEDC BA98 7654 3210     7654 3210
        // tmp_addr:0000 0000 1111 1111 = d:1111 1111
        // vram_addr = tmp_addr

        g->v.tmp_addr = (g->v.tmp_addr & 0xFF00) | b;
        g->vram_addr = g->v.tmp_addr;
      }

      g->dirty = 1;

      g->phase = !g->phase;
      break;

    case 0x2007:	// VRAM I/O Register (RW)
      if (g->vram_addr >= 0x3000 && g->vram_addr < 0x3F00)
        addr = g->vram_addr & 0x2FFF;
      else if (g->vram_addr >= 0x3F20 && g->vram_addr < 0x4000)
        addr = g->vram_addr & 0x3F1F;
      else
        addr = g->vram_addr;

      switch (addr & 0xF000) {
        case 0x0000:
        case 0x1000:
          if (g->chr_count == 0) {
            //g->vrom[(addr >> 10) & 0x07][addr & 0x3FF] = b;
            hardware->a.p[addr >> 12][addr & 0xFFF] = b;
            g->dirty = 1;
          }
          break;
        case 0x2000:
          g->ntram[(addr >> 10) & 0x03][addr & 0x3FF] = b;
          g->dirty = 1;
          break;
        default:
          // Image pallete:
          // 3F00-3F0F, where 3F04, 3F08 and 3F0C mirror 3F00
  
          // Sprite pallete:  
          // 3F10-3F1F, where 3F10, 3F14, 3F18 and 3F1C mirror 3F00

          switch (addr) {
            case 0x3F00:
            case 0x3F04:
            case 0x3F08:
            case 0x3F0C:
            case 0x3F10:
            case 0x3F14:
            case 0x3F18:
            case 0x3F1C:
              if (g->vram[0x3F00] != b) {
                g->vram[0x3F00] = b;
                g->vram[0x3F04] = b;
                g->vram[0x3F08] = b;
                g->vram[0x3F0C] = b;
                g->vram[0x3F10] = b;
                g->vram[0x3F14] = b;
                g->vram[0x3F18] = b;
                g->vram[0x3F1C] = b;
                g->dirty = 1;
              }
              break;
            default:
              if (g->vram[addr] != b) {
                g->vram[addr] = b;
                g->dirty = 1;
              }
          }
      }

      g->vram_addr = (g->vram_addr + g->v.addr_incr) & 0x3FFF;
      break;

    case 0x4014:	// Sprite RAM DMA Register (W)
      addr = b << 8;

      for (i = 0; i < 256; i++, addr++) {
        if (addr >= 0x8000)
          g->sp_ram[(g->sp_addr+i) & 0xFF] = hardware->a.p[addr >> 12][addr & 0x0FFF];
        else if (addr >= 0x6000)
          g->sp_ram[(g->sp_addr+i) & 0xFF] = hardware->a.m0[addr];
        else if (addr < 0x2000)
          g->sp_ram[(g->sp_addr+i) & 0xFF] = hardware->a.m0[addr & 0x07FF];
        else
          g->sp_ram[(g->sp_addr+i) & 0xFF] = 0;
      }

      nes_calc_hit(hardware);
      g->dirty = 1;
      break;

    case 0x4016:	// Joypad 1 (RW)
      if (!(b & 0x01) && (g->joy_strobe & 0x01))
        g->joy_state = 0;

      g->joy_strobe = b;
  }
}

UInt32 nes_mmc1_bank(UInt32 mode, UInt32 f1, UInt32 f2, UInt32 f3)
{
  /*
  mode 0: 256K cart
  mode 1: 512K cart
  mode 2: 1024K cart

  f1:
    1024K carts:
      0 = Ignore 256K selection register (2 bits)
      1 = Use 256K selection register (2 bits)

  f2:
    512K carts:
      0 = Swap banks from first 256K of PRG
      1 = Swap banks from second 256K of PRG
    1024K carts with f1==0:
      0 = Swap banks from first 256K of PRG
      1 = Swap banks from third 256K of PRG
    1024K carts with f1==1:
      Low bit of 256K PRG bank selection

  f3:
    1024K carts with f1==0:
      Store but ignore this bit
    1024K carts with f1==1:
      High bit of 256K PRG bank selection
  */

  UInt32 bank;

  switch (mode) {
    case 0:	// 256K
      bank = 0;
      break;
    case 1:	// 512K
      bank = f2 ? 1 : 0;
      break;
    case 2:	// 1024K
      if (f1)
        bank = f2 | (f3 << 1);
      else
        bank = f2 ? 2 : 0;	// 3 ou 2 ???
      break;
    default:
      bank = 0;
  }

  return bank;
}

void nes_set_mirror(Hardware *hardware, UInt8 **ntram , UInt32 mirror)
{
  NesGlobals *g;

  g = hardware->a.globals;

  switch (mirror) {
    case MIRROR_4:
      ntram[0] = &hardware->a.m1[0x2000];
      ntram[1] = &hardware->a.m1[0x2400];
      ntram[2] = &hardware->a.m1[0x2800];
      ntram[3] = &hardware->a.m1[0x2C00];
      break;
    case MIRROR_2H:
      ntram[0] = &hardware->a.m1[0x2000];
      ntram[1] = &hardware->a.m1[0x2000];
      ntram[2] = &hardware->a.m1[0x2400];
      ntram[3] = &hardware->a.m1[0x2400];
      break;
    case MIRROR_2V:
      ntram[0] = &hardware->a.m1[0x2000];
      ntram[1] = &hardware->a.m1[0x2400];
      ntram[2] = &hardware->a.m1[0x2000];
      ntram[3] = &hardware->a.m1[0x2400];
      break;
    case MIRROR_1L:
      ntram[0] = &hardware->a.m1[0x2000];
      ntram[1] = &hardware->a.m1[0x2000];
      ntram[2] = &hardware->a.m1[0x2000];
      ntram[3] = &hardware->a.m1[0x2000];
      break;
    case MIRROR_1U:
      ntram[0] = &hardware->a.m1[0x2400];
      ntram[1] = &hardware->a.m1[0x2400];
      ntram[2] = &hardware->a.m1[0x2400];
      ntram[3] = &hardware->a.m1[0x2400];
  }
}

void nes_switch_prg(Hardware *hardware)
{
  NesGlobals *g;
  UInt32 index;
  UInt8 *prg;

  g = hardware->a.globals;

  if (g->prg_size == 0x8000) {
    // 32K switch at 0x8000
    index = (g->prg_bank << 4) + (g->prg_idx & 0x0E);

    if ((index+1) < g->prg_count && g->prg[index] && g->prg[index+1]) {
      prg = (UInt8 *)ByteSwap32(g->prg[index]);
      hardware->a.p[0x08] = &prg[0x0000];
      hardware->a.p[0x09] = &prg[0x1000];
      hardware->a.p[0x0A] = &prg[0x2000];
      hardware->a.p[0x0B] = &prg[0x3000];

      prg = (UInt8 *)ByteSwap32(g->prg[index+1]);
      hardware->a.p[0x0C] = &prg[0x0000];
      hardware->a.p[0x0D] = &prg[0x1000];
      hardware->a.p[0x0E] = &prg[0x2000];
      hardware->a.p[0x0F] = &prg[0x3000];
    }
  } else {
    index = (g->prg_bank << 4) + (g->prg_idx & 0x0F);

    if (index < g->prg_count && g->prg[index]) {
      prg = (UInt8 *)ByteSwap32(g->prg[index]);

      if (g->prg_addr == 0x8000) {
        // 16K switch at 0x8000
        hardware->a.p[0x08] = &prg[0x0000];
        hardware->a.p[0x09] = &prg[0x1000];
        hardware->a.p[0x0A] = &prg[0x2000];
        hardware->a.p[0x0B] = &prg[0x3000];
      } else {
        // 16K switch at 0xC000
        hardware->a.p[0x0C] = &prg[0x0000];
        hardware->a.p[0x0D] = &prg[0x1000];
        hardware->a.p[0x0E] = &prg[0x2000];
        hardware->a.p[0x0F] = &prg[0x3000];
      }
    }
  }
}

void nes_switch_chr2(Hardware *hardware, UInt8 **vrom, UInt32 *chr_idx)
{
  NesGlobals *g;
  UInt32 i, index;
  UInt8 *chr;

  g = hardware->a.globals;

  for (i = 0; i < 8; i++) {
    index = chr_idx[i];

    if (index < g->chr_count && g->chr[index]) {
      chr = (UInt8 *)ByteSwap32(g->chr[index]);
      vrom[i] = &chr[i * 0x400];
    }
  }
}

void nes_switch_chr(Hardware *hardware, UInt8 **p, UInt32 chr0, UInt32 chr1)
{
  NesGlobals *g;
  UInt32 index;
  UInt8 *chr;

  g = hardware->a.globals;

  index = chr0 & 0x0F;

  if (index < g->chr_count && g->chr[index]) {
    chr = (UInt8 *)ByteSwap32(g->chr[index]);

    if (g->chr_size == 0x1000)
      p[0] = &chr[0x0000];
    else {
      p[0] = &chr[0x0000];
      p[1] = &chr[0x1000];
    }
  }

  index = chr1 & 0x0F;

  if (index < g->chr_count && g->chr[index]) {
    chr = (UInt8 *)ByteSwap32(g->chr[index]);

    if (g->chr_size == 0x1000)
      p[1] = &chr[0x1000];
  }
}


static void nes_calc_hit(Hardware *hardware)
{
  // scanline: 42 + 256 + 43 = 341 pixels
  // 341 pixels -> 113.85 CPU cycles

  NesGlobals *g;
  UInt32 x, y, visible;
  UInt8 code, attr;

  g = hardware->a.globals;
  y = g->sp_ram[0];	// 0-239
  x = g->sp_ram[3];	// 0-255

  visible = 0;

  if (y < SCREEN_HEIGHT) {
    code = g->sp_ram[1];
    attr = g->sp_ram[2];

    visible = 1;
  }

  // hardware->a.hcycle[y] + ((x + 42) * hardware->a.hcycle[1]) / 341;
}
