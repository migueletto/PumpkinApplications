#include "palm.h"
#include "debug.h"
#include "librogue.h"

extern RoguePreferenceType my_prefs;

void qWinEraseRectangle(RectanglePtr r, Word cornerDiam)
{
  if (my_prefs.black_bg)
    WinDrawRectangle(r, cornerDiam);
  else
    WinEraseRectangle(r, cornerDiam);
}
void qWinDrawLine(short x1, short y1, short x2, short y2)
{
  if (my_prefs.black_bg)
    WinEraseLine(x1, y1, x2, y2);
  else
    WinDrawLine(x1, y1, x2, y2);
}
void qWinDrawChars(CharPtr chars, Word len, SWord x, SWord y)
{
  if (my_prefs.black_bg) {
    WinDrawInvertedChars(chars, len, x, y);
  } else {
    WinDrawChars(chars, len, x, y);
  }
}
