#ifndef _Z80PRIV_H
#define _Z80PRIV_H

typedef UInt32 FASTREG;
typedef UInt32 FASTWORK;

#define FLAG_C  1
#define FLAG_N  2
#define FLAG_P  4
#define FLAG_H  16
#define FLAG_Z  64
#define FLAG_S  128

#define ldig(x)         ((x) & 0xf)
#define hdig(x)         (((x)>>4)&0xf)
#define lreg(x)         ((x)&0xff)
#define hreg(x)         (((x)>>8)&0xff)

#define Setlreg(x,v)    x = (((x)&0xff00) | ((v)&0xff))
#define Sethreg(x,v)    x = (((x)&0xff) | (((v)&0xff) << 8))

#define GetBYTE(a)      GetByte(z80->hardware,(a))
#define PutBYTE(a,v)    SetByte(z80->hardware,(a),(v))
#define GetWORD(a)      (GetBYTE(a) | (GetBYTE((a)+1) << 8))
#define PutWORD(a,v)    (PutBYTE((a),(UInt8)(v)),PutBYTE(((a)+1),((v) >> 8)))

#define CheckInput(p)   IO_check(z80->hardware,(p))
#define Input(p)        IO_readb(z80->hardware,(p))
#define Output(p,v)     IO_writeb(z80->hardware,(p),(v))

#define parity(x)       z80->hardware->partab[(x)&0xff]

#define AF		z80->af[z80->sela]
#define BC		z80->bc[z80->selr]
#define DE		z80->de[z80->selr]
#define HL		z80->hl[z80->selr]
#define IR		z80->ir
#define IX		z80->ix
#define IY		z80->iy
#define SP		z80->sp
#define PC		z80->pc
#define IFF1		z80->iff1
#define IFF2		z80->iff2
#define SELA		z80->sela
#define SELR		z80->selr
#define IM		z80->im

#define z80_ICount	z80->count

#define SETFLAG(f,c)    AF = (c) ? AF | FLAG_ ## f : AF & ~FLAG_ ## f
#define TSTFLAG(f)      ((AF & FLAG_ ## f) != 0) 

#define POP(x) {					\
        FASTREG y = GetBYTE(SP); SP++;			\
        x = y + (GetBYTE(SP) << 8); SP++;		\
}

#define PUSH(x) {					\
        --SP; PutBYTE(SP, (x) >> 8);			\
        --SP; PutBYTE(SP, x);				\
}

#define JPC(cond) PC = cond ? GetWORD(PC) : PC+2

#define CALLC(cond) {					\
    if (cond) {						\
        FASTREG adrr = GetWORD(PC);			\
        PUSH(PC+2);					\
        PC = adrr;					\
    } else						\
        PC += 2;					\
}

#endif
