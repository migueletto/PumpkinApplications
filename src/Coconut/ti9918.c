#include <PalmOS.h>
#include "cpu.h"
#include "endian.h"
#include "gui.h"
#include "video.h"
#include "ti9918.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "palm.h"
#include "endian.h"
#include "gui.h"

#define SCREEN_0	0x01
#define SCREEN_1	0x00
#define SCREEN_2	0x04
#define SCREEN_3	0x02

ColorType Ti9918Color[17] = {
  {0, 0x00, 0x00, 0x00},	// transparente
  {1, 0x00, 0x00, 0x00},
  {2, 0x21, 0xc8, 0x42},
  {3, 0x5e, 0xdc, 0x78},
  {4, 0x54, 0x55, 0xed},
  {5, 0x7d, 0x76, 0xfc},
  {6, 0xd4, 0x52, 0x4d},
  {7, 0x42, 0xeb, 0xf5},
  {8, 0xfc, 0x55, 0x54},
  {9, 0xff, 0x79, 0x78},
  {10, 0xd4, 0xc1, 0x54},
  {11, 0xe6, 0xce, 0x80},
  {12, 0x21, 0xb0, 0x3b},
  {13, 0xc9, 0x5b, 0xba},
  {14, 0xcc, 0xcc, 0xcc},
  {15, 0xff, 0xff, 0xff},
  {-1, 0, 0, 0}
};

void ti9918_init(TI9918 *ti9918, Hardware *hardware) {
  ti9918->hardware = hardware;
  ti9918_reset(ti9918);

  hardware->x0 = (hardware->display_width - 256) / 2;
  hardware->y0 = (hardware->display_height - 192 - lowBorder*2) / 2;
  hardware->dx = 256;
  hardware->dy = 192;
}

void ti9918_reset(TI9918 *ti9918) {
  Hardware *hardware = ti9918->hardware;
  MemSet(ti9918, sizeof(TI9918), 0);
  ti9918->hardware = hardware;
  ti9918->border = 1;
  ti9918->dirty = 1;
}

//static void ti9918_sprite(TI9918 *ti9918, UInt8 *s, UInt32 x0, UInt32 y0);
//static void ti9918_colision(TI9918 *ti9918, UInt32 x, UInt32 y, UInt8 *obj);

UInt8 ti9918_status(TI9918 *ti9918) {
  UInt8 b;

  // 0-4  5th   Number for the 5th sprite on a line
  // 5    C     1 if overlapping sprites
  // 6    5D    1 if more than 4 sprites on a horizontal line
  // 7    F     V-Blank IRQ Flag (1=interrupt) (See also IE0 flag)

  b = 0x00;

  if (ti9918->interrupt)
    b |= 0x80;
  if (ti9918->colision)
    b |= 0x20;

  ti9918->interrupt = 0;

  return b;
}

void ti9918_setb(TI9918 *ti9918, UInt8 b) {
  UInt32 mode, fgcolor, bgcolor;

  if (ti9918->phase == 0) {	// first byte
    ti9918->tmp = b;
    ti9918->phase = 1;

  } else {			// second byte
    if (b & 0x80) {		// register setup

      // Byte 1, Bit 0-7  Data  (New value for the register)
      // Byte 2, Bit 0-6  Index (VDP register number) (MSX1: 0-7)
      // Byte 2, Bit 7    Must be "1" for VDP Register setup

      switch (b & 0x07) {
        case 0:	// Mode register 0
          // 0    D     External video input       (0=input disable, 1=enable)
          // 1    M3    Mode M3 (Screen 2,5,7,8)
          // 2    M4    Mode M4 (Screen 4,5,8,0Hi) (MSX2 only)
          // 3    M5    Mode M5 (Screen 6,7,8)     (MSX2 only)
          // 4    IE1   H-Blank Interrupt Enable   (MSX2 only)
          // 5    IE2   Light pen/mouse on ???     (MSX2 ONLY not MSX2+)
          // 6    DG    DiGitize mode              (MSX2 only)
          // 7    0     Not Used

          // M1 M2 M3 M4 M5  Screen format
          // 1  0  0  0  0   Text       40x24             (BASIC SCREEN 0)
          // 0  0  0  0  0   Half text  32x24             (BASIC SCREEN 1)
          // 0  0  1  0  0   Hi resolution 256x192        (BASIC SCREEN 2)
          // 0  1  0  0  0   Multicolour  4x4pix blocks   (BASIC SCREEN 3)

          mode = (ti9918->mode & 0x03) | ((ti9918->tmp & 0x0E) << 1);

          if (ti9918->mode != mode) {
            ti9918->mode = mode;
            ti9918->border = 1;
            ti9918->dirty = 1;
          }

          ti9918->reg[0] = ti9918->tmp & 0x03;
          break;

        case 1:	// Mode register 1
          // 0    MAG   Sprite zoom                (0=x1, 1=x2)
          // 1    SZ    Sprite size                (0=8x8, 1=16x16)
          // 2    0     Not Used
          // 3    M2    Mode M2 (Screen 3: Block)
          // 4    M1    Mode M1 (Screen 0: Text)
          // 5    IE0   V-Blank Interrupt Enable   (0=Disable, 1=Enable)
          // 6    BLK   Screen output control      (0=Disable, 1=Enable)
          // 7    416   VRAM size control          (0=4K, 1=16K) (not used)

          mode = (ti9918->mode & 0x1C) | ((ti9918->tmp & 0x10) >> 4) |
                 ((ti9918->tmp & 0x08) >> 3);

          if (ti9918->mode != mode) {
            ti9918->mode = mode;
            ti9918->border = 1;
            ti9918->dirty = 1;
          }

          ti9918->intenable = (ti9918->tmp & 0x20) ? 1 : 0;

          ti9918->reg[1] = ti9918->tmp & 0xFB;
          break;

        case 2:
          ti9918->reg[2] = ti9918->tmp & 0x0F;
          ti9918->dirty = 1;
          break;
        case 3:
          ti9918->reg[3] = ti9918->tmp & 0xFF;
          ti9918->dirty = 1;
          break;
        case 4:
          ti9918->reg[4] = ti9918->tmp & 0x07;
          ti9918->dirty = 1;
          break;
        case 5:
          ti9918->reg[5] = ti9918->tmp & 0x7F;
          ti9918->dirty = 1;
          break;
        case 6:
          ti9918->reg[6] = ti9918->tmp & 0x07;
          ti9918->dirty = 1;
          break;

        case 7:	// Color register
          // 0-3  TC0-3 bg colour in SCREEN 0 (also border colour in SCREEN 1-3)
          // 4-7  BD0-3 fg colour in SCREEN 0

          bgcolor = ti9918->tmp & 0x0F;

          if (bgcolor != ti9918->bgcolor) {
            ti9918->bgcolor = bgcolor;
            ti9918->border = 1;
            ti9918->dirty = 1;
          }

          fgcolor = ti9918->tmp >> 4;

          if (fgcolor != ti9918->fgcolor) {
            ti9918->fgcolor = fgcolor;
            ti9918->border = 1;
            ti9918->dirty = 1;
          }

          ti9918->reg[7] = ti9918->tmp & 0xFF;
      }

    } else {			// pointer setup

      // Byte 1/Bit 0-7  Lower bits of VRAM Pointer
      // Byte 2/Bit 0-5  Upper bits of VRAM Pointer
      // Byte 2/Bit 6    Desired VRAM Direction (0=Reading, 1=Writing)
      // Byte 2/Bit 7    Must be "0" for VRAM Pointer setup

      if (b & 0x40)		// write pointer
        ti9918->writep = ti9918->tmp | ((UInt32)(b & 0x3F) << 8);
      else			// read pointer
        ti9918->readp = ti9918->tmp | ((UInt32)(b & 0x3F) << 8);
    }
    ti9918->phase = 0;
  }
}

void ti9918_video(TI9918 *ti9918) {
  RectangleType rect;
  WinHandle prev;
  UInt8 *font, *color, mask;
  UInt32 address, j, k, x, y, c, c1, c2, visible;
  UInt32 code, caddr, offset;
  Err err;

  ti9918->frame++;
  if (ti9918->frame & 1)
    return;

  if (!ti9918->dirty && !ti9918->border)
    return;

  WinSetCoordinateSystem(kCoordinatesDouble);

  if (ti9918->hardware->screen_wh == NULL) {
    ti9918->hardware->screen_wh = WinCreateOffscreenWindow(256, 192, nativeFormat, &err);
  }

  prev = WinSetDrawWindow(ti9918->hardware->screen_wh);

  if (ti9918->border) {
    c1 = ti9918->hardware->color[ti9918->bgcolor];
    RctSetRectangle(&rect, 0, 0, 256, 192);
    WinSetBackColor(c1);
    WinEraseRectangle(&rect, 0);
    ti9918->border = 0;
    ti9918->dirty = 1;
  }

  if (ti9918->dirty) switch (ti9918->mode) {
    case SCREEN_0:	// 40x24 text mode

      ti9918->fontmap   = ti9918->reg[4] << 11;
      ti9918->screenmap = ti9918->reg[2] << 10;

      if (ti9918->fontmap == ti9918->screenmap)
        break;

      font = &ti9918->vram[ti9918->fontmap];

      for (address = 0; address < 960; address++) {
        code = ti9918->vram[ti9918->screenmap + address];

        x = (address % 40);
        y = address / 40;

        caddr = code * 8;
        c1 = ti9918->hardware->color[ti9918->fgcolor];
        c2 = ti9918->hardware->color[ti9918->bgcolor];
        visible = ti9918->fgcolor;

        for (k = 0; k < 8; k++) {
          mask = font[caddr++];

          for (j = 0; j < 6; j++, mask <<= 1) {
            if (mask & 0x80) {
              if (visible) {
                WinSetForeColor(c1);
                WinDrawPixel(8+x*6+j, y*8+k);
              }
            } else {
              WinSetForeColor(c2);
              WinDrawPixel(8+x*6+j, y*8+k);
            }
          }
        }
      }
      break;

    case SCREEN_1:	// 32x24 coloured text mode

      ti9918->fontmap   = ti9918->reg[4] << 11;
      ti9918->screenmap = ti9918->reg[2] << 10;
      ti9918->colormap  = ti9918->reg[3] << 6;
      ti9918->attrmap   = ti9918->reg[5] << 7;
      ti9918->objmap    = ti9918->reg[6] << 11;

      if (ti9918->fontmap == ti9918->screenmap)
        break;

      font  = &ti9918->vram[ti9918->fontmap];
      color = &ti9918->vram[ti9918->colormap];

      for (address = 0; address < 768; address++) {
        code = ti9918->vram[ti9918->screenmap + address];

        x = (address % 32);
        y = address / 32;

        caddr = code * 8;

        c = color[code >> 3];
        c2 = (c & 0x0F) ? ti9918->hardware->color[c & 0x0F] : ti9918->hardware->color[ti9918->bgcolor];
        c1 = (c & 0xF0) ? ti9918->hardware->color[c >> 4] : c2;

        for (k = 0; k < 8; k++) {
          mask = font[caddr++];

          for (j = 0; j < 8; j++, mask <<= 1) {
            WinSetForeColor((mask & 0x80) ? c1 : c2);
            WinDrawPixel(x*8+j, y*8+k);
          }
        }
      }

      //ti9918_sprite(ti9918, s, x0, y0);
      break;

    case SCREEN_2:	// 256x192 hi-res mode

      ti9918->fontmap   = (ti9918->reg[4] & 0xFC) << 11;
      ti9918->screenmap = ti9918->reg[2] << 10;
      ti9918->colormap  = (ti9918->reg[3] & 0x80) << 6;
      ti9918->attrmap   = ti9918->reg[5] << 7;
      ti9918->objmap    = ti9918->reg[6] << 11;

      if (ti9918->fontmap == ti9918->screenmap)
        break;

      for (address = 0; address < 768; address++) {
        code = ti9918->vram[ti9918->screenmap + address];

        x = (address % 32);
        y = address / 32;

        offset = (y >> 3) * 0x800;
        caddr = code * 8;

        font  = &ti9918->vram[ti9918->fontmap  + offset];
        color = &ti9918->vram[ti9918->colormap + offset];

        for (k = 0; k < 8; k++) {
          mask = font[caddr];

          c = color[caddr];
          c2 = (c & 0x0F) ? ti9918->hardware->color[c & 0x0F] : ti9918->hardware->color[ti9918->bgcolor];
          c1 = (c & 0xF0) ? ti9918->hardware->color[c >> 4] : c2;

          for (j = 0; j < 8; j++, mask <<= 1) {
            //r[j] = (mask & 0x80) ? c1 : c2;
          }
          caddr++;
        }
      }

      //ti9918_sprite(ti9918, s, x0, y0);
      break;

    case SCREEN_3:	// 64x48 block graphics multicolour mode
      // 0000-07FF   BG Tiles (block colors)
      // 0800-0AFF   BG Map
      // 1B00-1B7F   Sprite attribute table
      // 3800-3FFF   Sprite character patterns

      ti9918->fontmap   = ti9918->reg[4] << 11;
      ti9918->screenmap = ti9918->reg[2] << 10;
      ti9918->attrmap   = ti9918->reg[5] << 7;
      ti9918->objmap    = ti9918->reg[6] << 11;

      font  = &ti9918->vram[ti9918->fontmap];

      for (address = 0; address < 768; address++) {
        code = ti9918->vram[ti9918->screenmap + address];

        x = (address % 32);
        y = address / 32;

        offset = (y & 0x03) << 1;
        caddr = code * 8 + offset;

        c = font[caddr++];
        c1 = (c & 0xF0) ? ti9918->hardware->color[c >> 4] : ti9918->hardware->color[ti9918->bgcolor];
        c2 = (c & 0x0F) ? ti9918->hardware->color[c & 0x0F] : ti9918->hardware->color[ti9918->bgcolor];

        for (k = 0; k < 4; k++) {
          for (j = 0, mask = 0xF0; j < 8; j++, mask <<= 1) {
            //r[j] = (mask & 0x80) ? c1 : c2;
          }
        }

        c = font[caddr++];
        c1 = (c & 0xF0) ? ti9918->hardware->color[c >> 4] : ti9918->hardware->color[ti9918->bgcolor];
        c2 = (c & 0x0F) ? ti9918->hardware->color[c & 0x0F] : ti9918->hardware->color[ti9918->bgcolor];

        for (; k < 8; k++) {
          for (j = 0, mask = 0xF0; j < 8; j++, mask <<= 1) {
            //r[j] = (mask & 0x80) ? c1 : c2;
          }
        }
      }
  }

  WinSetDrawWindow(prev);
  RctSetRectangle(&rect, 0, 0, 256, 192);
  WinCopyRectangle(ti9918->hardware->screen_wh, NULL, &rect, ti9918->hardware->x0, ti9918->hardware->y0, winPaint);
  WinSetCoordinateSystem(kCoordinatesStandard);

  ti9918->dirty = 0;
}

#if 0
static void ti9918_sprite(TI9918 *ti9918, UInt8 *s, UInt32 x0, UInt32 y0) {
  UInt32 i, j, k, m, x, y, p, c, caddr, quad, zoom;
  UInt8 *r, *attr, *obj;
  UInt16 mask;

  // maximo de 32 sprites simultaneos
  // attrmap: 4 bytes por sprite, 32 * 4 = 128 bytes
  // objmap: 256 sprites de 8x8: 256 * 8 = 2048 bytes

  ti9918->colision = 0;

  attr = &ti9918->vram[ti9918->attrmap];

  for (i = 0; i < 32; i++, attr += 4)
    if (attr[0] == 0xD0)
      break;

  if (i == 0)
    return;

  zoom = (ti9918->reg[1] & 0x01) ? 1 : 0;
  quad = (ti9918->reg[1] & 0x02) ? 1 : 0;
  attr = &ti9918->vram[ti9918->attrmap + (i-1) * 4];
  obj  = &ti9918->vram[ti9918->objmap];

  MemSet(ti9918->coltable, 0x1800, 0);

  for (; i > 0; i--, attr -= 4) {
    y = attr[0];
    x = attr[1];
    p = quad ? (attr[2] & 0xFC) : attr[2];
    c = attr[3] & 0x0F;
    caddr = p * 8;

    if (y == 0xFF)
      y = 0;
    else
      y++;

    if (c == 0 || y >= 192)
      continue;

    if (attr[3] & 0x80) {
      if (x < 32)
        continue;
      x -= 32;
    }

    if (!ti9918->colision)
      ti9918_colision(ti9918, x, y, &obj[caddr]);

    if (!ti9918->colision && quad) {
      ti9918_colision(ti9918, x, y+8, &obj[caddr+8]);
      if (!ti9918->colision)
        ti9918_colision(ti9918, x+8, y, &obj[caddr+16]);
      if (!ti9918->colision)
        ti9918_colision(ti9918, x+8, y+8, &obj[caddr+24]);
    }

    c = ti9918->hardware->color[c];
    r = s + (y0 + y) * ti9918->hardware->display_width + x0 + x;

    if (quad) {
      for (k = 0; k < 16 && y < 192; k++, caddr++) {
        mask = obj[caddr];
        mask = (mask << 8) | obj[caddr + 16];

        for (j = 0, m = 0; j < 16 && (x+m) < 256; j++, mask <<= 1) {
          if (mask & 0x8000) {
            r[m] = c;
            if (zoom) r[m+1] = c;
          }
          m++;
          if (zoom) m++;
        }
        r += ti9918->hardware->display_width;
        y++;

        if (zoom) {
          mask = obj[caddr];
          mask = (mask << 8) | obj[caddr + 16];

          for (j = 0, m = 0; j < 16 && (x+m) < 256; j++, mask <<= 1) {
            if (mask & 0x8000) {
              r[m] = c;
              if (zoom) r[m+1] = c;
            }
            m++;
            if (zoom) m++;
          }
          r += ti9918->hardware->display_width;
          y++;
        }
      }
    } else {
      for (k = 0; k < 8 && y < 192; k++, caddr++) {
        mask = obj[caddr];

        for (j = 0, m = 0; j < 8 && (x+m) < 256; j++, mask <<= 1) {
          if (mask & 0x80) {
            r[m] = c;
            if (zoom) r[m+1] = c;
          }
          m++;
          if (zoom) m++;
        }
        r += ti9918->hardware->display_width;
        y++;

        if (zoom) {
          mask = obj[caddr];

          for (j = 0, m = 0; j < 8 && (x+m) < 256; j++, mask <<= 1) {
            if (mask & 0x80) {
              r[m] = c;
              if (zoom) r[m+1] = c;
            }
            m++;
            if (zoom) m++;
          }
          r += ti9918->hardware->display_width;
          y++;
        }
      }
    }
  }
}

static void ti9918_colision(TI9918 *ti9918, UInt32 x, UInt32 y, UInt8 *obj) {
  UInt32 r, i, k;
  UInt8 m1, m2, b;

  r = x & 7;
  i = (y << 5) + (x >> 3);	// y * 32 + x / 8

  if (r) {
    b = 0xFF << (7 - r);

    for (k = 0; k < 8 && y < 192; k++, y++, i += 32) {
      m1 = obj[k] >> r;
      m2 = obj[k] & b;

      if (ti9918->coltable[i] & m1) {
        ti9918->colision = 1;
        return;
      }
      ti9918->coltable[i] |= m1;

      if (x < 248) {
        if (ti9918->coltable[i+1] & m2) {
          ti9918->colision = 1;
          return;
        }
        ti9918->coltable[i+1] |= m2;
      }
    }
  } else {
    for (k = 0; k < 8 && y < 192; k++, y++, i += 32) {
      m1 = obj[k];

      if (ti9918->coltable[i] & m1) {
        ti9918->colision = 1;
        return;
      }
      ti9918->coltable[i] |= m1;
    }
  }
}
#endif
