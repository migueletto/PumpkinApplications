#include <PalmOS.h>

#include "endian.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "m6845.h"
#include "endian.h"
#include "gui.h"

void m6845_init(m6845_Regs *m6845, UInt8 *vram, UInt8 *cram, UInt8 *font) {
  m6845->index = 0;
  m6845->horizontal_total = 0;
  m6845->horizontal_displayed = 0;
  m6845->horizontal_sync_pos = 0;
  m6845->horizontal_length = 0;
  m6845->vertical_total = 0;
  m6845->vertical_adjust = 0;
  m6845->vertical_displayed = 0;
  m6845->vertical_sync_pos = 0;
  m6845->crt_mode = 0;
  m6845->scan_lines = 0;
  m6845->cursor_top = 0;
  m6845->cursor_bottom = 0;
  m6845->screen_address = 0;
  m6845->cursor_address = 0;
  m6845->cursor_visible = 0;
  m6845->off_x = 0;
  m6845->off_y = 0;
  m6845->mode = 0;
  m6845->frame = 0;

  m6845->dirty = 1;
  m6845->border = 1;
  m6845->fs0 = 1;
  m6845->fs1 = 1;
  m6845->bgcolor = 16;

  m6845->vram = vram;
  m6845->cram = cram;
  m6845->font = font;
}

static void m6845_calcxy(m6845_Regs *m6845);

void m6845_setmode(m6845_Regs *m6845, UInt32 mode) {
  if (m6845->mode != mode) {
    m6845->mode = mode;	// 1 = graphics, 0 = text
    m6845->border = 1;
    m6845->dirty = 1;
  }
}

void m6845_setfs(m6845_Regs *m6845, UInt32 fs, UInt32 value) {
  switch (fs) {
    case 0:	// 0x80 - 0xBF, 1=fixed, 0=user defined
      m6845->fs0 = value;
      m6845->dirty = 1;
      break;
    case 1:	// 0xC0 - 0xFF, 1=fixed, 0=user defined
      m6845->fs1 = value;
      m6845->dirty = 1;
  }
}

void m6845_setbg(m6845_Regs *m6845, UInt32 bg) {
  m6845->bgcolor = bg;
  m6845->dirty = 1;
}

void m6845_setindex(m6845_Regs *m6845, UInt32 index) {
  m6845->index = index & 0x1F;
}

UInt32 m6845_getindex(m6845_Regs *m6845) {
  return m6845->index;
}

void m6845_setvalue(m6845_Regs *m6845, UInt32 value) {
  switch (m6845->index) {
    case  0:
      if (m6845->horizontal_total == value)
        break;
      m6845->horizontal_total = value;
      m6845_calcxy(m6845);
      break;
    case  1:
      if (m6845->horizontal_displayed == value)
        break;
      m6845->horizontal_displayed = value;
      m6845->dirty = 1;
      m6845->border = 1;
      break;
    case  2:
      if (m6845->horizontal_sync_pos == value)
        break;
      m6845->horizontal_sync_pos = value;
      m6845_calcxy(m6845);
      break;
    case  3:
      value &= 0x0F;
      m6845->horizontal_length = value;
      break;
    case  4:
      value &= 0x7F;
      if (m6845->vertical_total == value)
        break;
      m6845->vertical_total = value;
      m6845_calcxy(m6845);
      break;
    case  5:
      value &= 0x1F;
      if (m6845->vertical_adjust == value)
        break;
      m6845->vertical_adjust = value;
      m6845_calcxy(m6845);
      break;
    case  6:
      value &= 0x7F;
      if (m6845->vertical_displayed == value)
        break;
      m6845->vertical_displayed = value;
      m6845->dirty = 1;
      m6845->border = 1;
      break;
    case  7:
      value &= 0x7F;
      if (m6845->vertical_sync_pos == value)
        break;
      m6845->vertical_sync_pos = value;
      m6845_calcxy(m6845);
      break;
    case  8:
      value &= 0x03;
      m6845->crt_mode = value;
      break;
    case  9:
      value &= 0x1F;
      if (m6845->scan_lines == value)
        break;
      m6845->scan_lines = value;
      m6845_calcxy(m6845);
      break;
    case 10:
      value &= 0x7F;
      if (m6845->cursor_top == value)
        break;
      m6845->cursor_top = value;
      m6845->dirty = 1;
      break;
    case 11:
      value &= 0x1F;
      if (m6845->cursor_bottom == value)
        break;
      m6845->cursor_bottom = value;
      m6845->dirty = 1;
      break;
    case 12:
      value = (value & 0x3F) << 8;
      if ((m6845->screen_address & 0xFF00) == value)
        break;
      m6845->screen_address = (m6845->screen_address & 0x00FF) | value;
      m6845->dirty = 1;
      m6845->border = 1;
      break;
    case 13:
      if ((m6845->screen_address & 0x00FF) == value)
        break;
      m6845->screen_address = (m6845->screen_address & 0xFF00) | value;
      m6845->dirty = 1;
      m6845->border = 1;
      break;
    case 14:
      value = (value & 0x3F) << 8;
      if ((m6845->cursor_address & 0xFF00) == value)
        break;
      m6845->cursor_address = (m6845->cursor_address & 0x00FF) | value;
      m6845->dirty = 1;
      break;
    case 15:
      if ((m6845->cursor_address & 0x00FF) == value)
        break;
      m6845->cursor_address = (m6845->cursor_address & 0xFF00) | value;
      m6845->dirty = 1;
  }
}

UInt32 m6845_getvalue(m6845_Regs *m6845) {
  switch (m6845->index) {
    case 14: return m6845->cursor_address >> 8;
    case 15: return m6845->cursor_address & 0xFF;
  }
  return 0;
}

static void m6845_calcxy(m6845_Regs *m6845) {
  if (m6845->horizontal_sync_pos)
    m6845->off_x = m6845->horizontal_total - m6845->horizontal_sync_pos - 14;
  else
    m6845->off_x = -15;

  m6845->off_y = (m6845->vertical_total - m6845->vertical_sync_pos) *
                 (m6845->scan_lines + 1) + m6845->vertical_adjust - 32;

  if (m6845->off_y < 0)
    m6845->off_y = 0;

  if (m6845->off_y > 128)
    m6845->off_y = 128;

  m6845->off_x = 0;	// XXX

  m6845->dirty = 1;
  m6845->border = 1;
}

void m6845_video(m6845_Regs *m6845, Hardware *hardware) {
  WinHandle prev;
  RectangleType rect;
  UInt8 *font, mask, *userfont;
  UInt32 height, address, screen, cursor, size, c1, c2, i, j, k, x, y;
  UInt32 min_x, max_x, min_y, max_y, code, caddr, cursor_visible;
  Err err;

  m6845->frame++;
  if (m6845->frame & 1)
    return;

  switch (m6845->cursor_top & 0x60) {
    case 0x00:	// no blinking
      cursor_visible = 1;
      break;
    case 0x20:	// no cursor
      cursor_visible = 0;
      break;
    case 0x40:	// blink at 16x
      cursor_visible = (m6845->frame >> 4) & 0x01;
      break;
    case 0x60:	// blink at 32x
      cursor_visible = (m6845->frame >> 3) & 0x01;
      break;
    default:
      cursor_visible = 0;
  }

  if (cursor_visible != m6845->cursor_visible) {
    m6845->cursor_visible = cursor_visible;
    m6845->dirty = 1;
  }

  if (hardware->dirty)
    m6845->dirty = 1;

  if (!m6845->dirty)
    return;

  if (!m6845->vertical_displayed || !m6845->horizontal_displayed)
    return;

  WinSetCoordinateSystem(kCoordinatesDouble);

  height = hardware->display_height - lowBorder * 2;

  hardware->dx = 0;
  hardware->dy = 0;	// forca ao swap no SaveScreen

  hardware->x0 = m6845->off_x;
  hardware->y0 = m6845->off_y;
  hardware->dx = m6845->horizontal_displayed * 8;
  hardware->dy = m6845->vertical_displayed * (m6845->scan_lines + 1);

  if (m6845->border) {
    c1 = hardware->color[16];
    WinSetBackColor(c1);
    RctSetRectangle(&rect, 0, 0, hardware->display_width, m6845->off_y);
    WinEraseRectangle(&rect, 0);
    RctSetRectangle(&rect, 0, m6845->off_y + m6845->vertical_displayed, hardware->display_width, height - (m6845->off_y + m6845->vertical_displayed) + 1);
    WinEraseRectangle(&rect, 0);
/*
    for (y = 0; y < m6845->off_y; y++, r += hardware->display_width)
      MemSet(r, hardware->display_width, c1);

    y += m6845->vertical_displayed;
    for (; y < height; y++, r += hardware->display_width)
      MemSet(r, hardware->display_width, c1);
*/

    m6845->border = 0;
    m6845->dirty = 1;
  }

  if (hardware->screen_wh == NULL) {
    hardware->screen_wh = WinCreateOffscreenWindow(hardware->display_width, hardware->display_height - 2*lowBorder, nativeFormat, &err);
  }

  prev = WinSetDrawWindow(hardware->screen_wh);

  screen = m6845->screen_address;
  cursor = m6845->cursor_address;
  size = m6845->horizontal_displayed * m6845->vertical_displayed;

  // user define font: 0x80 a 0xFF -> 0xF400 a 0xF7FF
  // usei 0xF000 porque 0x00 a 0x7F nunca serao usados
  userfont = &hardware->m0[0xF000 & 0x7FFF];

  for (address = 0; address < size; address++) {
    i = (screen + address) & 0x3FFF;
    x = address % m6845->horizontal_displayed + m6845->off_x;
    y = address / m6845->horizontal_displayed;

    min_x = x * 8;
    max_x = min_x + 7;
    min_y = y * (m6845->scan_lines + 1) + m6845->off_y;
    max_y = min_y + m6845->scan_lines;

    if (max_y >= height || max_x >= hardware->display_width)
      continue;

    code = m6845->vram[i];

    if (m6845->mode == 1) {	// graphics
      //r = s + min_y * hardware->display_width + min_x;

      for (k = 0; k <= m6845->scan_lines; k++) {
        mask = code;
        for (j = 0; j < 8; j += 2, mask <<= 2) {
          c1 = (mask & 0xC0) >> 6;
          switch (c1) {
            case 0:  c1 = hardware->color[m6845->bgcolor]; break;
            case 1:  c1 = hardware->color[8]; break; // blue
            case 2:  c1 = hardware->color[6]; break; // orange
            default: c1 = hardware->color[5];	 // green
          }
          //r[j] = c1;
          //r[j+1] = c1;
          WinSetForeColor(c1);
          WinDrawPixel(min_x + j, min_y + k);
          WinDrawPixel(min_x + j + 1, min_y + k);
        }
        //r += hardware->display_width;
      }

    } else {	// text
      if (code < 0x80)
        font = m6845->font;
      else if (code < 0xC0)
        font = m6845->fs0 ? m6845->font : userfont;
      else
        font = m6845->fs1 ? m6845->font : userfont;

      caddr = code * 8;
      c1 = hardware->color[m6845->cram[i & 0x3FF] & 0x0F];
      c2 = hardware->color[m6845->bgcolor];

      for (k = 0; k <= m6845->scan_lines; k++) {
        mask = font[caddr++];
        for (j = 0; j < 8; j++, mask <<= 1) {
          WinSetForeColor((mask & 0x80) ? c1 : c2);
          WinDrawPixel(min_x + j, min_y + k);
        }
      }
    }

    if (i == cursor && m6845->cursor_visible) {
      max_y = min_y + (m6845->cursor_bottom & 0x0F);
      min_y = min_y + (m6845->cursor_top & 0x0F);

      if (max_y > min_y && max_y < height &&
          max_x < hardware->display_width) {

        c1 = hardware->color[m6845->cram[i & 0x3FF] & 0x0F];
        WinSetForeColor(c1);

        for (k = min_y; k <= max_y; k++) {
          for (j = 0; j < 8; j++) {
            WinDrawPixel(min_x + j, k);
          }
        }
      }
    }
  }

  WinSetDrawWindow(prev);
  RctSetRectangle(&rect, 0, 0, hardware->dx, hardware->dy);
  WinCopyRectangle(hardware->screen_wh, NULL, &rect, 0, 0, winPaint);
  WinSetCoordinateSystem(kCoordinatesStandard);

  hardware->dirty = 0;
  m6845->dirty = 0;
}
