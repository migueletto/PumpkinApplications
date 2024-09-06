#include <stdlib.h>
#include <stdbool.h>

#include "libnsfb.h"
#include "libnsfb_plot.h"
#include "libnsfb_event.h"
#include "nsfb.h"
#include "surface.h"
#include "plot.h"

#include <PalmOS.h>

#include "sys.h"
#include "bytes.h"
#include "debug.h"

#define Y0 30

extern void pumpkin_screen_dirty(WinHandle win, int x, int y, int w, int h);
extern Boolean EvtHandle(UInt32 timeout, EventType *ev);

typedef struct {
  int has_event;
  nsfb_event_t event;
} pit_events_t;

static int pit_defaults(nsfb_t *nsfb) {
  debug(DEBUG_INFO, "NetSurf", "pit_defaults");
  nsfb->width = 640;
  nsfb->height = 480;
  nsfb->bpp = 16;
  nsfb->format = NSFB_FMT_RGB565;

  select_plotters(nsfb);

  return 0;
}

static int pit_set_geometry(nsfb_t *nsfb, int width, int height, enum nsfb_format_e format) {
    int startsize, endsize;
    int prev_width, prev_height;
    enum nsfb_format_e prev_format;

    if (nsfb->bpp != 16 || nsfb->format != NSFB_FMT_RGB565) {
      debug(DEBUG_ERROR, "NetSurf", "pit_set_geometry invalid bpp %d or format %d", nsfb->bpp, format);
      return -1;
    }

    prev_width = nsfb->width;
    prev_height = nsfb->height;
    prev_format = nsfb->format;

    startsize = (nsfb->width * nsfb->height * nsfb->bpp) / 8;

    if (width > 0) {
      nsfb->width = width;
    }

    if (height > 0) {
      nsfb->height = height;
    }

    if (format != NSFB_FMT_ANY) {
      nsfb->format = format;
    }

    /* select soft plotters appropriate for format */
    select_plotters(nsfb);

    /* reallocate surface memory if necessary */
    endsize = (nsfb->width * nsfb->height * nsfb->bpp) / 8;
    if ((nsfb->ptr != NULL) && (startsize != endsize)) {
        uint8_t *fbptr;
        fbptr = sys_realloc(nsfb->ptr, endsize);
        if (fbptr == NULL) {
            /* allocation failed so put everything back as it was */
            nsfb->width = prev_width;
            nsfb->height = prev_height;
            nsfb->format = prev_format;
            select_plotters(nsfb);

            return -1;
        }
        nsfb->ptr = fbptr;
    }

    nsfb->linelen = (nsfb->width * nsfb->bpp) / 8;
    debug(DEBUG_INFO, "NetSurf", "pit_set_geometry width=%d height=%d bpp=%d", nsfb->width, nsfb->height, nsfb->bpp);

    return 0;
}

static int pit_initialise(nsfb_t *nsfb) {
    size_t size;
    uint8_t *fbptr;

    if (nsfb->bpp != 16 || nsfb->format != NSFB_FMT_RGB565) {
      debug(DEBUG_ERROR, "NetSurf", "pit_initialise invalid bpp %d or format %d", nsfb->bpp, nsfb->format);
      return -1;
    }

    size = (nsfb->width * nsfb->height * nsfb->bpp) / 8;
    fbptr = sys_realloc(nsfb->ptr, size);
    if (fbptr == NULL) {
        return -1;
    }

    nsfb->ptr = fbptr;
    nsfb->linelen = (nsfb->width * nsfb->bpp) / 8;
    debug(DEBUG_INFO, "NetSurf", "pit_initialize width=%d height=%d bpp=%d", nsfb->width, nsfb->height, nsfb->bpp);

    nsfb->surface_priv = sys_calloc(1, sizeof(pit_events_t));

    return 0;
}

static int pit_claim(nsfb_t *nsfb, nsfb_bbox_t *box) {
  (void)nsfb;
  (void)box;
  return 0;
}

static UInt16 *window_buffer(WinHandle wh) {
  BitmapType *bmp = WinGetBitmap(wh);
  return BmpGetBits(bmp);
}

static int pit_update(nsfb_t *nsfb, nsfb_bbox_t *box) {
  FormType *frm;
  WinHandle wh, wh2;
  UInt32 offset, i;
  UInt16 *bits, *ptr, *dst, *dst2, *src;
  int x, y;

  debug(DEBUG_TRACE, "NetSurf", "pit_update (%d,%d) (%d,%d)", box->x0, box->y0, box->x1, box->y1);

  if ((frm = FrmGetActiveForm()) != NULL) {
    wh = FrmGetWindowHandle(frm);
    bits = window_buffer(wh);
    offset = Y0 * nsfb->width + box->y0 * nsfb->width + box->x0;
    dst = &bits[offset];
    if (wh == WinGetActiveWindow()) {
      wh2 = WinGetDisplayWindow();
      bits = window_buffer(wh2);
      dst2 = &bits[offset];
    } else {
      dst2 = NULL;
    }
    ptr = (UInt16 *)nsfb->ptr;
    offset = box->y0 * nsfb->width + box->x0;
    src = &ptr[offset];
    for (y = box->y0; y < box->y1; y++) {
      for (x = box->x0, i = 0; x < box->x1; x++, i++) {
        put2b(src[i], (uint8_t *)&dst[i], 0);
        if (dst2) put2b(src[i], (uint8_t *)&dst2[i], 0);
      }
      dst += nsfb->width;
      if (dst2) dst2 += nsfb->width;
      src += nsfb->width;
    }
    pumpkin_screen_dirty(wh, box->x0, Y0 + box->y0, box->x1 - box->x0, box->y1 - box->y0);
  }

  return 0;
}

static int pit_finalise(nsfb_t *nsfb) {
  debug(DEBUG_INFO, "NetSurf", "pit_finalise");
  if (nsfb->surface_priv) sys_free(nsfb->surface_priv);
  sys_free(nsfb->ptr);

  return 0;
}

static bool pit_input(nsfb_t *nsfb, nsfb_event_t *event, int timeout) {
  EventType ev;
  pit_events_t *e;
  UInt16 key;
  bool r = false;

  e = (pit_events_t *)nsfb->surface_priv;

  if (e->has_event) {
    sys_memcpy(event, &e->event, sizeof(nsfb_event_t));
    e->has_event = false;
    return true;
  }

  if (EvtHandle(timeout*10, &ev)) {
    return false;
  }

  ev.screenX <<= 1;
  ev.screenY <<= 1;
  event->type = NSFB_EVENT_NONE;

  switch (ev.eType) {
    case penDownEvent:
      if (ev.screenY >= Y0) {
        event->type = NSFB_EVENT_MOVE_ABSOLUTE;
        event->value.vector.x = ev.screenX;
        event->value.vector.y = ev.screenY - Y0;
        event->value.vector.z = 0;
        e->event.type = NSFB_EVENT_KEY_DOWN;
        e->event.value.keycode = NSFB_KEY_MOUSE_1;
        e->has_event = true;
        r = true;
      }
      break;
    case penUpEvent:
      if (ev.screenY >= Y0) {
        event->type = NSFB_EVENT_KEY_UP;
        event->value.keycode = NSFB_KEY_MOUSE_1;
        r = true;
      }
      break;
    case penMoveEvent:
      if (ev.screenY >= Y0) {
        event->type = NSFB_EVENT_MOVE_ABSOLUTE;
        event->value.vector.x = ev.screenX;
        event->value.vector.y = ev.screenY - Y0;
        event->value.vector.z = 0;
        r = true;
      }
      break;
    case keyDownEvent:
      key = ev.data.keyDown.chr;
      if (key == 10) key = 13;
      event->type = NSFB_EVENT_KEY_DOWN;
      event->value.keycode = key;
      e->event.type = NSFB_EVENT_KEY_UP;
      e->event.value.keycode = key;
      e->has_event = true;
      r = true;
      break;
    default:
      break;
  }

  return r;
}

const nsfb_surface_rtns_t pit_rtns = {
  .defaults = pit_defaults,
  .initialise = pit_initialise,
  .finalise = pit_finalise,
  .input = pit_input,
  .claim = pit_claim,
  .update = pit_update,
  .geometry = pit_set_geometry,
};

NSFB_SURFACE_DEF(pit, NSFB_SURFACE_PIT, &pit_rtns)
