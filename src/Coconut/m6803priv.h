#ifndef _M6803PRIV_H
#define _M6803PRIV_H

#define M6803_WAI	8	// set when WAI is waiting for an interrupt
#define M6803_SLP	0x10	// HD63701 only

#define M6803_DDR1	0x00
#define M6803_DDR2	0x01

#define M6803_PORT1	0x100
#define M6803_PORT2	0x101

#define M6803_RDMEM(Addr)       GetByte(m6803->hardware,Addr)
#define M6803_WRMEM(Addr,Value) SetByte(m6803->hardware,Addr,Value)
#define M6803_RDOP(Addr)        M6803_RDMEM(Addr)
#define M6803_RDOP_ARG(Addr)    M6803_RDOP(Addr)

#define RM(Addr)		M6803_RDMEM(Addr)
#define WM(Addr,Value)		M6803_WRMEM(Addr,Value)
#define ROP(Addr)		M6803_RDOP(Addr)
#define ROP_ARG(Addr)		M6803_RDOP_ARG(Addr)

#define RM16(Addr)		((RM(Addr) << 8) | RM(((Addr)+1)&0xffff))
#define WM16(Addr,p)		(WM((Addr),p.b.h),WM(((Addr)+1)&0xffff,p.b.l))

#define pPPC    m6803->ppc
#define pPC     m6803->pc
#define pS      m6803->s
#define pX      m6803->x
#define pD      m6803->d

#define PC      m6803->pc.w.l
#define PCD     m6803->pc.d
#define S       m6803->s.w.l
#define SD      m6803->s.d
#define X       m6803->x.w.l
#define D       m6803->d.w.l
#define A       m6803->d.b.h
#define B       m6803->d.b.l
#define CC      m6803->cc

#define CT      m6803->counter.w.l
#define CTH     m6803->counter.w.h
#define CTD     m6803->counter.d
#define OC      m6803->output_compare.w.l
#define OCH     m6803->output_compare.w.h
#define OCD     m6803->output_compare.d
#define TOH     m6803->timer_over.w.l
#define TOD     m6803->timer_over.d

#define EAD     m6803->ea.d
#define EA      m6803->ea.w.l

#define m6803_ICount m6803->count

#define IMMBYTE(b)      b = ROP_ARG(PC); PC++
#define IMMWORD(w)      w.d = (ROP_ARG(PC)<<8) | ROP_ARG((PC+1)&0xffff); PC+=2

#define PUSHBYTE(b) WM(SD,b); --S
#define PUSHWORD(w) WM(SD,w.b.l); --S; WM(SD,w.b.h); --S
#define PULLBYTE(b) S++; b = RM(SD)
#define PULLWORD(w) S++; w.d = RM(SD)<<8; S++; w.d |= RM(SD)

#define MODIFIED_tcsr { \
  m6803->irq2 = (m6803->tcsr&(m6803->tcsr<<3))&(TCSR_ICF|TCSR_OCF|TCSR_TOF); \
}

#define SET_FLAGS8I(t)          {CC|=m6803->hardware->flags8i[(t)&0xff];}
#define SET_FLAGS8D(t)          {CC|=m6803->hardware->flags8d[(t)&0xff];}

#define SET_NZ8(a)              {SET_N8(a);SET_Z(a);}
#define SET_NZ16(a)             {SET_N16(a);SET_Z(a);}
#define SET_FLAGS8(a,b,r)       {SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_FLAGS16(a,b,r)      {SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

#define SIGNED(b) ((Int16)(b&0x80?b|0xff00:b))

#define DIRECT IMMBYTE(EAD)
#define IMM8 EA=PC++
#define IMM16 {EA=PC;PC+=2;}
#define EXTENDED IMMWORD(m6803->ea)
#define INDEXED {EA=X+(UInt8)ROP_ARG(PCD);PC++;}

#define SEC CC|=0x01
#define CLC CC&=0xfe
#define SEZ CC|=0x04
#define CLZ CC&=0xfb
#define SEN CC|=0x08
#define CLN CC&=0xf7
#define SEV CC|=0x02
#define CLV CC&=0xfd
#define SEH CC|=0x20
#define CLH CC&=0xdf
#define SEI CC|=0x10
#define CLI CC&=~0x10

#define TCSR_OLVL 0x01
#define TCSR_IEDG 0x02
#define TCSR_ETOI 0x04
#define TCSR_EOCI 0x08
#define TCSR_EICI 0x10
#define TCSR_TOF  0x20
#define TCSR_OCF  0x40
#define TCSR_ICF  0x80

#define INCREMENT_COUNTER(m6803, amount)                \
{                                                       \
  m6803_ICount -= amount;                               \
  CTD += amount;                                        \
  if (CTD >= m6803->timer_next)                         \
    check_timer_event(m6803);                           \
}

#define EAT_CYCLES(m6803)                                               \
{                                                                       \
  Int32 cycles_to_eat;                                                  \
  cycles_to_eat = m6803->timer_next - CTD;                              \
  if (cycles_to_eat > m6803_ICount) cycles_to_eat = m6803_ICount;       \
  if (cycles_to_eat > 0)                                                \
    INCREMENT_COUNTER(m6803, cycles_to_eat);                            \
}

#define DIRBYTE(b) {DIRECT;b=RM(EAD);}
#define DIRWORD(w) {DIRECT;w.d=RM16(EAD);}
#define EXTBYTE(b) {EXTENDED;b=RM(EAD);}
#define EXTWORD(w) {EXTENDED;w.d=RM16(EAD);}

#define IDXBYTE(b) {INDEXED;b=RM(EAD);}
#define IDXWORD(w) {INDEXED;w.d=RM16(EAD);}

#define BRANCH(f) {IMMBYTE(t);if(f){PC+=SIGNED(t);}}
#define NXORV  ((CC&0x08)^((CC&0x02)<<2))

#define TAKE_ICI ENTER_INTERRUPT(m6803, 0xfff6)
#define TAKE_OCI ENTER_INTERRUPT(m6803, 0xfff4)
#define TAKE_TOI ENTER_INTERRUPT(m6803, 0xfff2)
#define TAKE_SCI ENTER_INTERRUPT(m6803, 0xfff0)
#define TAKE_TRAP ENTER_INTERRUPT(m6803, 0xffee)

#define SET_TIMER_EVENT m6803->timer_next = (OCD - CTD < TOD - CTD) ? OCD : TOD

#define CLEANUP_conters {  \
  OCH -= CTH;        \
  TOH -= CTH;        \
  CTH = 0;           \
  SET_TIMER_EVENT;   \
}

#define MODIFIED_counters { \
  OCH = (OC >= CT) ? CTH : CTH+1; \
  SET_TIMER_EVENT;                \
}

#define CLR_HNZVC       CC&=0xd0
#define CLR_NZV         CC&=0xf1
#define CLR_HNZC        CC&=0xd2
#define CLR_NZVC        CC&=0xf0
#define CLR_Z           CC&=0xfb
#define CLR_NZC         CC&=0xf2
#define CLR_ZC          CC&=0xfa
#define CLR_C           CC&=0xfe

#define SET_Z(a)        if(!a)SEZ
#define SET_Z8(a)       SET_Z((UInt8)a)
#define SET_Z16(a)      SET_Z((UInt16)a)
#define SET_N8(a)       CC|=((a&0x80)>>4)
#define SET_N16(a)      CC|=((a&0x8000)>>12)
#define SET_H(a,b,r)    CC|=(((a^b^r)&0x10)<<1)
#define SET_C8(a)       CC|=((a&0x100)>>8)
#define SET_C16(a)      CC|=((a&0x10000)>>16)
#define SET_V8(a,b,r)   CC|=(((a^b^r^(r>>1))&0x80)>>6)
#define SET_V16(a,b,r)  CC|=(((a^b^r^(r>>1))&0x8000)>>14)

#define CHECK_IRQ2 {						\
  if (m6803->irq2 & (TCSR_ICF|TCSR_OCF|TCSR_TOF)) {		\
    if (m6803->irq2 & TCSR_ICF) {				\
      TAKE_ICI;							\
      m6803->irq_state[M6803_TIN_LINE] = CLEAR_LINE;		\
    } else if (m6803->irq2 & TCSR_OCF)				\
      TAKE_OCI;							\
    else if (m6803->irq2 & TCSR_TOF)				\
      TAKE_TOI;							\
  }								\
}

#define ONE_MORE_INSN(m6803) {					\
  UInt8 ireg; 							\
  pPPC = pPC; 							\
  ireg=ROP(PCD);						\
  PC++;								\
  m6803_optable(ireg, m6803); 					\
  INCREMENT_COUNTER(m6803, m6803->hardware->cycles[ireg]);	\
}

#define CHECK_IRQ_LINES() {					\
  if (!(CC & 0x10)) {						\
    if (m6803->irq_state[M6803_IRQ_LINE] != CLEAR_LINE) {	\
      ENTER_INTERRUPT(m6803,0xfff8);				\
      m6803->irq_state[M6803_IRQ_LINE] = CLEAR_LINE;		\
    } else							\
      CHECK_IRQ2;						\
  }								\
}

#endif
