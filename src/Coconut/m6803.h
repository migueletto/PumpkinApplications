#ifndef _M6803_H
#define _M6803_H

#define M6803_IRQ_LINE  0       // IRQ line number
#define M6803_TIN_LINE  1       // P20/Tin Input Capture line (eddge sense)
#define M6803_NMI_LINE  2

typedef struct m6803_Regs {
  PAIR pc;          // Program counter
  PAIR ppc;         // Previous program counter
  PAIR d;           // Accumulator a and b
  PAIR s;           // Stack pointers
  PAIR x;           // Index registers
  PAIR ea;
  UInt32 cc;        // Condition codes
  UInt32 wai_state; // WAI opcode state ,(or sleep opcode state)
  UInt32 nmi_state; // NMI line state */
  UInt32 irq_state[2]; // IRQ line state [IRQ1,TIN]
  UInt32 ic_eddge;  // InputCapture eddge , b.0=fall,b.1=raise

  UInt32 port1_ddr;
  UInt32 port2_ddr;
  UInt32 port1_data;
  UInt32 port2_data;
  UInt32 tcsr;          // Timer Control and Status Register
  UInt32 pending_tcsr;  // pending IRQ flag for clear IRQflag process
  UInt32 ram_ctrl;
  Int32  latch09;
  PAIR   counter;        // free running counter
  PAIR   output_compare; // output compare
  UInt32 input_capture;  // input capture
  UInt32 irq2;          // IRQ2 flags
  UInt32 timer_next;

  PAIR   timer_over;
  Int32  extracycles;   // cycles used for interrupts
  Int32  count;

  Hardware *hardware;

} m6803_Regs;

void m6803_init(m6803_Regs *m6803);
void m6803_reset(m6803_Regs *m6803);
Int32 m6803_execute(m6803_Regs *m6803, Int32 cycles);
void m6803_set_irq_line(m6803_Regs *m6803, Int32 irqline, Int32 state);
void m6803_exit(m6803_Regs *m6803);

UInt8 m6803_getreg(m6803_Regs *m6803, UInt8 a);
void m6803_setreg(m6803_Regs *m6803, UInt8 a, UInt8 b);

void m6803_optable(UInt8 op, m6803_Regs *m6803);
void ENTER_INTERRUPT(m6803_Regs *m6803, UInt16 irq_vector);
void check_timer_event(m6803_Regs *m6803);

uint32_t m6803_ArmletStart(ArmletArg *arg);

#endif
