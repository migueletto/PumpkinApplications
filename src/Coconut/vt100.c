#include <PalmOS.h>

#include "section.h"
#include "palm.h"
#include "vt100.h"
#include "screen.h"

#define maxVT100Arg 8

static Int16 vt100_s, vt100_num, vt100_arg[maxVT100Arg+1];
static Int16 rows, cols;
static Int16 keyMode;
static Boolean vt100_firstdigit;
static Boolean lastcol;

void InitTerminal(Int16 r, Int16 c)
{
  vt100_s = 0;
  lastcol = false;
  rows = r;
  cols = c;
  keyMode = 0;	// cursor mode
}

void EmitTerminal(char *buf, Int16 n, Int16 done)
{
  Int16 k, i, m;
  char c;

  Cursor(0);

  for (k = 0; k < n; k++) {
    c = buf[k];
    switch (vt100_s) {
      case 0:
        if (c == ESC) {
          vt100_num = 0;
          vt100_firstdigit = true;
          vt100_arg[0] = 0;
          vt100_s = 1;
        }
        else
          EmitChar(c);
        break;

      case 1:
        vt100_s = 0;
        switch (c) {
          case '[':
            vt100_s = 2;
            break;
          case 'D':		/* Scroll text up */
            ScrollUp();
            break;
          case 'E':		/* Newline (behaves like cr followed by do) */
            EmitChar('\r');
            EmitChar('\n');
            break;
          case 'M':		/* Scroll text down */
            ScrollDown();
            break;
          case '7':		/* Save cursor position & attributes */
            SaveCursor();
            break;
          case '8':		/* Restore cursor position & attributes */
            RestoreCursor();
            break;
          case '=':
          case '>':
            break;
          case '(':
          case ')':		/* Start/End alternate character set */
          case '#':
            vt100_s = 3;
            break;
          default:
            break;
        }
        break;

      case 2:
        vt100_s = 0;
        switch (c) {
          case 'A':		/* Upline (cursor up) */
            m = vt100_num ? vt100_arg[0] : 1;   
            for (i = 0; i < m; i++)
              DecY();
            break;
          case 'B':		/* Down one line */
            m = vt100_num ? vt100_arg[0] : 1;   
            for (i = 0; i < m; i++)
              EmitChar('\n');
            break;
          case 'C':		/* Non-destructive space (cursor right) */
            m = vt100_num ? vt100_arg[0] : 1;   
            for (i = 0; i < m; i++)
              if (GetX() < cols-1)
                IncX(1);
            break;
          case 'D':		/* Move cursor left n positions */
            m = vt100_num ? vt100_arg[0] : 1;   
            for (i = 0; i < m; i++)
              if (GetX())
                DecX();
            lastcol = false;
            break;
          case 'H':
          case 'f':
            if (vt100_num == 2) {	/* Screen-relative cursor motion */
              SetY(vt100_arg[0]-1);
              SetX(vt100_arg[1]-1);
            }
            else if (vt100_num == 0)	/* Home cursor */
              Home();
            lastcol = false;
            break;
          case 'J':		/* Clear display */
            if (!vt100_num)
              vt100_arg[0] = 0;
            switch (vt100_arg[0]) {
              case 0: ClearScreenToEnd(); break;
              case 1: ClearScreenFromBegin(); break;
              case 2: ClearScreen();
            }
            lastcol = false;
            break;
          case 'K':		/* Clear line */
            if (!vt100_num)
              vt100_arg[0] = 0;
            switch (vt100_arg[0]) {
              case 0: ClearLineToEnd(); break;
              case 1: ClearLineFromBegin(); break;
              case 2: ClearLine();
            }
            lastcol = false;
            break;
          case 'h':
            if (vt100_num == 1) switch (vt100_arg[0]) {
              case 1:
                keyMode = 1;	// application
            }
            break;
          case 'l':
            if (vt100_num == 1) switch (vt100_arg[0]) {
              case 1:
                keyMode = 0;	// cursor
            }
            break;
          case 'm':
            if (vt100_num == 0) {
              /* Turn off all attributes */
              Underscore(0);
              Reverse(0);
            }
            else for (i = 0; i < vt100_num; i++) switch (vt100_arg[i]) {
              case 0:		/* Turn off all attributes */
                Underscore(0);
                Reverse(0);
                break;
              case 1:		/* Turn on bold (extra bright) attribute */
                break;
              case 4:		/* Start underscore mode */
                Underscore(1);
                break;
              case 5:		/* Turn on blinking attribute */
                break;
              case 7:		/* Turn on reverse-video attribute */
                Reverse(1);
                break;
            }
            break;
          case 'r':		/* Change scrolling region (VT100) */
            if (vt100_num != 2) {
              vt100_arg[0] = 1;
              vt100_arg[1] = GetRows();
            }
            else if (vt100_arg[1] > GetRows())
              vt100_arg[1] = GetRows();
            ChangeScrollingRegion(vt100_arg[0]-1, vt100_arg[1]-1);
            rows = vt100_arg[1];
            break;
          case 'L':		/* Insert line(s) */
            if (!vt100_num)
              vt100_arg[0] = 1;
            ChangeScrollingRegion(GetY(), GetRows()-1);
            for (i = 0; i < vt100_arg[0]; i++)
              ScrollDown();
            ChangeScrollingRegion(0, GetRows()-1);
            break;
          case 'M':		/* Delete line(s) */
            if (!vt100_num)
              vt100_arg[0] = 1;
            ChangeScrollingRegion(GetY(), GetRows()-1);
            for (i = 0; i < vt100_arg[0]; i++)
              ScrollUp();
            ChangeScrollingRegion(0, GetRows()-1);
            break;
          case 'P':		/* Delete char(s) */
            if (!vt100_num)
              vt100_arg[0] = 1;
            for (i = 0; i < vt100_arg[0]; i++)
              DeleteChar();
            break;
          case '?':
            vt100_s = 2;
            break;
          case '0':
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          case '9':
            if (vt100_firstdigit) {
              vt100_num++;
              vt100_arg[vt100_num] = 0;
              vt100_firstdigit = false;
            }
            vt100_arg[vt100_num-1] *= 10;
            vt100_arg[vt100_num-1] += c - '0';
            vt100_s = 2;
            break;
          case ';':
            vt100_firstdigit = true;
            vt100_s = 2;
            break;
        }
        break;

      case 3:
        vt100_s = 0;
    }
  }

  Cursor(1);
}

void EmitChar(UInt8 c)
{
  Int16 lxt = 0, lyt = 0, n;
  Boolean draw;
  UInt16 attr;

  draw = false;

  switch (c) {
    case '\0':
      break;
    case '\n':
      IncY();
      break;
    case '\r':
      SetX(0);
      lastcol = false;
      break;
    case 0x7:
      SndPlaySystemSound(sndInfo);
      break;
    case '\b':
      if (GetX()) {
        if (lastcol)
          lastcol = false;
        else
          DecX();
      }
      break;
    case '\t':
      n = 8 - (GetX() % 8);
      if (GetX()+n == cols)
        n--;
      IncX(n);
      break;
    default:
      if (c < 32)
        break;
      draw = true;
      lxt = GetX();
      lyt = GetY();
      if (lxt < (cols-1))
        IncX(1);
      else {
        if (lastcol) {
          lxt = 0;
          SetX(1);
          IncY();
          if (GetY() >= rows) {
            ScrollUp();
            DecY();
          }
          lyt = GetY();
          lastcol = false;
        }
        else
          lastcol = true;
      }
      if (GetX() >= cols) {
        SetX(0);
        IncY();
      }
  }

  if (draw && lyt < rows) {
    attr = 0;
    if (IsUnderscore()) attr |= m_underscore;
    if (IsReverse()) attr |= m_reverse;
    DrawChar(c, attr, lxt, lyt);
  }

  if (GetY() >= rows) {
    ScrollUp();
    DecY();
  }
}

void EmitString(char *s)
{
  Int16 i;

  for (i = 0; s[i]; i++)
    EmitChar(s[i]);
}

Char *GetKeySeq(CursorKey key)
{
  if (keyMode == 0)	// cursor
    switch (key) {
      case upArrow   : return "\033[A";
      case downArrow : return "\033[B";
      case leftArrow : return "\033[C";
      case rightArrow: return "\033[D";
    }
  else
    switch (key) {	// application
      case upArrow   : return "\033OA";
      case downArrow : return "\033OB";
      case leftArrow : return "\033OC";
      case rightArrow: return "\033OD";
    }

  return "";
}
