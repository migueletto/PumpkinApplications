#ifndef _Z80_H
#define _Z80_H

typedef struct z80_Regs {
  UInt32 af[2];
  UInt32 bc[2];
  UInt32 de[2];
  UInt32 hl[2];
  UInt32 ir;
  UInt32 ix;
  UInt32 iy;
  UInt32 sp;
  UInt32 pc;
  UInt32 iff1, iff2;
  UInt32 sela;
  UInt32 selr;
  UInt32 im;

  Int32  irq_state;
  Int32  nmi_state;
  Int32  count;

  Hardware *hardware;

} z80_Regs;

void z80_init(z80_Regs *z80);
void z80_reset(z80_Regs *z80);
Int32 z80_execute(z80_Regs *z80, Int32 cycles);
void z80_set_irq_line(z80_Regs *z80, Int32 irqline, Int32 state);
void z80_exit(z80_Regs *z80);

uint32_t z80_ArmletStart(ArmletArg *arg);

#endif
