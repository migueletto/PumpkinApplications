typedef struct {
  Hardware *hardware;
  UInt32 mode;
  UInt32 fontmap, screenmap, colormap, attrmap, objmap;
  UInt32 fgcolor, bgcolor;
  UInt32 intenable, interrupt;
  UInt32 phase, tmp;
  UInt32 readp, writep;
  UInt32 frame, border, dirty;
  UInt32 colision;
  UInt8 reg[8];
  UInt8 vram[0x4000];
  UInt8 coltable[0x1800];	// 256*192 / 8
} TI9918;

void ti9918_init(TI9918 *ti9918, Hardware *hardware);
void ti9918_reset(TI9918 *ti9918);

extern ColorType Ti9918Color[17];

UInt8 ti9918_status(TI9918 *ti9918);
void ti9918_setb(TI9918 *ti9918, UInt8 b);
void ti9918_video(TI9918 *ti9918);
