#ifndef _M6809_H
#define _M6809_H

typedef struct m6809_Regs {
  PAIR  pc;             // Program counter
  PAIR  ppc;            // Previous program counter
  PAIR  d;              // Accumulator a and b
  PAIR  dp;             // Direct Page register (page in MSB)
  PAIR  u, s;           // Stack pointers
  PAIR  x, y;           // Index registers
  PAIR  ea;
  UInt32 cc;
  UInt32 irq_state[2];
  UInt32 int_state;      // SYNC and CWAI flags
  UInt32 nmi_state;      
  Int32  extracycles;   // cycles used up by interrupts
  Int32  count;

  Hardware *hardware;

} m6809_Regs;

void m6809_init(m6809_Regs *m6809);
void m6809_reset(m6809_Regs *m6809);
Int32 m6809_execute(m6809_Regs *m6809, Int32 cycles);
void m6809_set_irq_line(m6809_Regs *m6809, Int32 irqline, Int32 state);
void m6809_exit(m6809_Regs *m6809);

uint32_t m6809_ArmletStart(ArmletArg *arg);

#endif
