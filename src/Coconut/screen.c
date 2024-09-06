#include <PalmOS.h>

#include "section.h"
#include "screen.h"

static Int16 minX, minY, maxX, maxY;
static Int16 maxL;      // # maximo de linhas da fonte atual
static Int16 maxC;      // # maximo de colunas da fonte atual
static Int16 cursorOn;
static Int16 x, y, cx, cy, xt, yt, saved_x, saved_y, saved_xt, saved_yt;
static Int16 top, bottom;
static Int16 reverse, underscore, saved_reverse, saved_underscore;
static UInt8 fg_color, bg_color;
static WinHandle chbuf = NULL;

void InitScreen(Int16 r, Int16 c, Int16 y0, Int16 x0, UInt8 fg, UInt8 bg) {
  Err err;

  maxL = r;
  maxC = c;
  if (maxC == 64)
    cx = 5, cy = 9;
  else
    cx = 4, cy = 6;

  if (!chbuf) {
    WinSetCoordinateSystem(kCoordinatesDouble);
    chbuf = WinCreateOffscreenWindow(16, 16, nativeFormat, &err);
    WinSetCoordinateSystem(kCoordinatesStandard);
  }

  minX = x0;
  minY = y0;
  maxX = minX + maxC * cx - 1;
  maxY = minY + maxL * cy - 1;

  fg_color = fg;
  bg_color = bg;

  top = minY;
  bottom = maxY;
  saved_x = saved_y = saved_xt = saved_yt = 0;
  saved_reverse = 0;
  saved_underscore = 0;
  cursorOn = 0;

  Underscore(0);
  Reverse(0);
  SetX(0);
  SetY(0);
}

void CloseScreen(void)
{
  if (chbuf)
    WinDeleteWindow(chbuf, false);
  chbuf = NULL;
}

Int16 GetCols(void)
{
  return maxC;
}

Int16 GetRows(void)
{
  return maxL;
}

Int16 GetX(void)
{
  return xt;
}

Int16 GetY(void)
{
  return yt;
}

void IncX(Int16 d)
{
  x += d*cx;
  xt += d;
}

void DecX(void)
{
  x -= cx;
  xt--;
}

void IncY(void)
{         
  y += cy;
  yt++;   
}           

void DecY(void)
{
  if (yt) {
    y -= cy;
    yt--;
  }
}

void RestoreCursor(void)
{
  x = saved_x;
  xt = saved_xt;
  y = saved_y;
  yt = saved_yt;
  reverse = saved_reverse;
  underscore = saved_underscore;
}

void SaveCursor(void)
{
  saved_x = x;
  saved_xt = xt;
  saved_y = y;
  saved_yt = yt;
  saved_reverse = reverse;
  saved_underscore = underscore;
}

void SetX(Int16 x1)
{
  if (x1 >= maxC)
    x1 = maxC-1;
  xt = x1;
  x = xt*cx + minX;
}

void SetY(Int16 y1)
{
  if (y1 >= maxL)
    y1 = maxL-1;
  yt = y1;
  y = yt*cy + minY;
}

void Home(void)
{
  Cursor(0);
  SetX(0);
  SetY(0);
  Cursor(1);
}

void Reverse(Int16 u)
{
  reverse = u;
}

Int16 IsReverse()
{
  return reverse;
}

void Underscore(Int16 u)
{
  underscore = u;
}

Int16 IsUnderscore(void)
{
  return underscore;
}

void ClearScreen(void)
{
  RectangleType rect;
          
  WinSetCoordinateSystem(kCoordinatesDouble);
  RctSetRectangle(&rect, minX, minY, maxX-minX+1, maxY-minY+1);
  WinEraseRectangle(&rect, 0);
  WinSetCoordinateSystem(kCoordinatesStandard);
}

void ClearScreenToEnd(void)
{
  RectangleType rect;

  if (x == minX && y == minY)
    ClearScreen();
  else {
    ClearLineToEnd();
    if ((y+cy) < maxY) {
      WinSetCoordinateSystem(kCoordinatesDouble);
      RctSetRectangle(&rect, minX, y+cy, maxX-minX+1, maxY-(y+cy)+1);
      WinEraseRectangle(&rect, 0);
      WinSetCoordinateSystem(kCoordinatesStandard);
    }
  }
}

void ClearScreenFromBegin(void)
{
  RectangleType rect;

  if (x != minX || y != minY) {
    if (y >= (minY+cy)) {
      WinSetCoordinateSystem(kCoordinatesDouble);
      RctSetRectangle(&rect, minX, minY, maxX-minX+1, y-minY-1);
      WinEraseRectangle(&rect, 0);
      WinSetCoordinateSystem(kCoordinatesStandard);
    }
    ClearLineFromBegin();
  }
}

void ClearLine(void)
{
  Int16 x1;

  x1 = GetX();
  SetX(0);
  ClearLineToEnd();
  SetX(x1);
}

void ClearLineToEnd(void)
{
  RectangleType rect;
  Int16 x0;

  x0 = x;
  WinSetCoordinateSystem(kCoordinatesDouble);
  RctSetRectangle(&rect, x0, y, maxX-minX+1, cy);
  WinEraseRectangle(&rect, 0);
  WinSetCoordinateSystem(kCoordinatesStandard);
}

void ClearLineFromBegin(void)
{   
  RectangleType rect;
  Int16 x0;
   
  x0 = x;
  WinSetCoordinateSystem(kCoordinatesDouble);
  RctSetRectangle(&rect, minX, y, x0-minX, cy);
  WinEraseRectangle(&rect, 0);
  WinSetCoordinateSystem(kCoordinatesStandard);
}

void ChangeScrollingRegion(Int16 l0, Int16 l1)
{
  if (l0 < 0 || l0 >= maxL || l1 < 0 || l1 >= maxL || l1 < l0)
    return;

  top = l0*cy;
  bottom = (l1+1)*cy - 1;
}

void ScrollUp(void)
{
  RectangleType rect;
  WinHandle wh;

  wh = WinGetDrawWindow();

  WinSetCoordinateSystem(kCoordinatesDouble);
  RctSetRectangle(&rect, minX, top+cy, maxX-minX+1, bottom-top-cy+1);
  WinCopyRectangle(wh, wh, &rect, minX, top, winPaint);
  RctSetRectangle(&rect, minX, bottom-cy+1, maxX-minX+1, cy);
  WinEraseRectangle(&rect, 0);
  WinSetCoordinateSystem(kCoordinatesStandard);
}

void ScrollDown(void)
{ 
  RectangleType rect;
  WinHandle wh;
            
  wh = WinGetDrawWindow();

  WinSetCoordinateSystem(kCoordinatesDouble);
  RctSetRectangle(&rect, minX, top, maxX-minX+1, bottom-top-cy+1);
  WinCopyRectangle(wh, wh, &rect, minX, top+cy, winPaint);
  RctSetRectangle(&rect, minX, top, maxX-minX+1, cy);
  WinEraseRectangle(&rect, 0);
  WinSetCoordinateSystem(kCoordinatesStandard);
}

void Cursor(Int16 on)
{
  cursorOn = on;
}

void DeleteChar(void)
{
  RectangleType rect;
  WinHandle wh;
  Int16 x0, xi, xf, i;

  wh = WinGetDrawWindow();

  WinSetCoordinateSystem(kCoordinatesDouble);
  for (i = xt, x0 = x; i < maxC; i++, x0 += cx) {
    xi = x0+cx;
    xf = x0;
    RctSetRectangle(&rect, xi, y, cx, cy);
    WinCopyRectangle(wh, wh, &rect, xf, y, winPaint);
  }
  xi = (maxC-1)*cx;
  RctSetRectangle(&rect, xi, y, cx, cy);
  WinEraseRectangle(&rect, 0);
  WinSetCoordinateSystem(kCoordinatesStandard);
}

void DrawChar(Int16 c, UInt16 attr, Int16 col, Int16 row)
{
  WinHandle wh;
  RectangleType rect;

  if (c < 32 || c >= 128)
    return;
  
  WinSetCoordinateSystem(kCoordinatesDouble);

  if (attr & m_reverse) {
    WinSetTextColor(bg_color);
    WinSetBackColor(fg_color);
  } else {
    WinSetTextColor(fg_color);
    WinSetBackColor(bg_color);
  }

  if (maxC == 80)
    WinPaintChar(c, minX + col * cx, minY + row * cy);
  else {
    wh = WinSetDrawWindow(chbuf);
    WinPaintChar(c, 0, 0);
    WinSetDrawWindow(wh);
    RctSetRectangle(&rect, 1, 1, cx, cy);
    WinCopyRectangle(chbuf, wh, &rect, minX + col*cx, minY + row*cy, winPaint);
  }

  WinSetCoordinateSystem(kCoordinatesStandard);
}
