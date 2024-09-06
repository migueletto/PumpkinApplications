#ifndef _M6502_H
#define _M6502_H

#define M6502_IRQ_LINE	0
#define M6502_NMI_LINE	1

typedef struct {
  PAIR pc_reg;
  PAIR a_reg;
  PAIR x_reg;
  PAIR y_reg;
  PAIR flag_reg;
  PAIR s_reg;

  UInt32 opcode;
  PAIR savepc;
  PAIR help;
  PAIR value;
  Int32 sum;
  Int32 saveflags;
  Int32 clockticks6502;

  Hardware *hardware;
} m6502_Regs;

void m6502_init(m6502_Regs *m6502);
void m6502_reset(m6502_Regs *m6502);
Int32 m6502_execute(m6502_Regs *m6502, Int32 cycles);
void m6502_set_irq_line(m6502_Regs *m6502, Int32 irqline, Int32 state);
void m6502_exit(m6502_Regs *m6502);

uint32_t m6502_ArmletStart(ArmletArg *arg);

#endif
