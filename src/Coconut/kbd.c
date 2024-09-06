#include <PalmOS.h>

#include "section.h"
#include "kbd.h"

void kbd_initmap(KeyMap *in, KeyMap *out)
{
  Int16 i;

  for (i = 0; i < 256; i++) {
    out[i].key = 0;
    out[i].line = 0xFF;
    out[i].column = 0xFF;
    out[i].shift = 0;
    out[i].ctrl = 0;
  }

  if (in) {
    for (i = 0; in[i].key; i++) {
      out[in[i].key].key = in[i].key;
      out[in[i].key].line = in[i].line;
      out[in[i].key].column = in[i].column;
      out[in[i].key].shift = in[i].shift;
      out[in[i].key].ctrl = in[i].ctrl;
    }
  }
}
