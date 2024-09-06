#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define KEY_BREAK       0x80
#define KEY_CLEAR       0x81
#define KEY_INVERSE     0x82
#define KEY_UP          0x83
#define KEY_DOWN        0x84
#define KEY_LEFT        0x85
#define KEY_RIGHT       0x86
#define KEY_CTRL        0x87
#define KEY_SHIFT       0x88
#define KEY_CAPS        0x89
#define KEY_SYMBOL      0x8A
#define KEY_EXT         0x8B
#define KEY_GRAPH       0x8C
#define KEY_INSERT      0x8D
#define KEY_RESET       0x8E
#define KEY_ESC		0x8F
#define KEY_F1		0x90
#define KEY_F2		0x91
#define KEY_F3		0x92
#define KEY_F4		0x93
#define KEY_F5		0x94
#define KEY_F6		0x95
#define KEY_F7		0x96
#define KEY_F8		0x97
#define KEY_COPY	0x98
#define KEY_REPT	0x99
#define KEY_END		0x9A
#define KEY_STOP	0x9B
#define KEY_SEL		0x9C
#define KEY_HOME	0x9D
#define KEY_DEL		0x9E
#define KEY_CODE	0x9F
#define KEY_CAP		0xA0
#define KEY_START	0xA1

#define KEY_BACK        0x08    // back space
#define KEY_ENTER       0x0A    // line feed

#define JOY_XAXIS       0
#define JOY_YAXIS       1
#define JOY_RIGHT       0
#define JOY_LEFT        1

typedef struct {
  UInt8 key; 
  UInt8 line, column; 
  UInt8 shift, ctrl;
} KeyMap;

typedef struct {
  UInt32 key; 
  UInt32 line, column; 
  UInt32 shift, ctrl;
} ArmKeyMap;

typedef struct {
  UInt16 key;
  UInt16 font;
  const char *label;
  RGBColorType fore, back, border;
} ButtonDef;

void kbd_initmap(KeyMap *in, KeyMap *out) SECTION("aux");

#endif
