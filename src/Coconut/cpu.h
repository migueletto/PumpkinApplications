#ifndef _CPU_H
#define _CPU_H

enum { CPU_M6809=1, CPU_M6803, CPU_Z80, CPU_M6502 };
enum { CLEAR_LINE=0, ASSERT_LINE};

typedef union {
#ifdef LSB_FIRST
  struct { UInt8 l,h,h2,h3; } b;
  struct { UInt16 l,h; } w;
#else
  struct { UInt8 h3,h2,h,l; } b;
  struct { UInt16 h,l; } w;
#endif
  UInt32 d;
} PAIR;

typedef void (*CpuOpcode)(void *cpu);

typedef struct Hardware {
  UInt32 dirty;
  UInt32 nmi_request;
  UInt32 irq_request;
  UInt32 firq_request;
  UInt32 vsync_irq;
  UInt32 m1w;

  UInt8 *bank0;
  UInt8 *bank1;
  UInt8 *bank2;
  UInt8 *bank3;
  UInt8 *bank4;

  void *globals;
  void *arg;
  void *cpu;
  UInt8 *cycles;
  UInt8 *flags8i;
  UInt8 *flags8d;
  UInt8 *partab;
  CpuOpcode *optable;
  UInt8 *m0, *m1, *m2, *m3, *m4;
  UInt8 *p[16];	// 4K pages
  UInt32 banksw[4];
  UInt32 display_width, display_height;
  UInt32 vdg, gp_begin, gp_end;
  UInt32 button;
  UInt32 x0, y0, dx, dy;
  UInt32 totalcycles, eventcount;
  UInt32 vsync, useevents, nevents, event, ecycle[280], earg[280];
  UInt32 ramsize, memmode;
  UInt32 key, joyx, joyy;
  UInt32 snd_count, snd_samples;
  UInt32 snd_tonefreq, snd_toneamp;
  UInt32 snd_noisefreq, snd_noiseamp;
  UInt32 cycles_per_sample;
  UInt32 tape;
  UInt8 *rnd;
  void *snd_buffer;
  UInt16 color[64];
  WinHandle screen_wh;

} Hardware;

extern UInt8 m6809_cycles[256];
extern UInt8 m6809_flags8i[256];
extern UInt8 m6809_flags8d[256];

extern UInt8 m6803_cycles[256];
extern UInt8 m6803_flags8i[256];
extern UInt8 m6803_flags8d[256];

extern UInt8 z80_cycles[256];
extern UInt8 z80_partab[256];

extern UInt8 m6502_cycles[256];

#endif
