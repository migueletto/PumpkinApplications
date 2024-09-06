#ifndef _M6502PRIV_H
#define _M6502PRIV_H

#define A	m6502->a_reg.b.l
#define X	m6502->x_reg.b.l
#define Y	m6502->y_reg.b.l
#define P	m6502->flag_reg.b.l
#define S	m6502->s_reg.b.l
#define PC	m6502->pc_reg.w.l

#define VALUE	m6502->value.b.l
#define SAVEPC	m6502->savepc.w.l
#define HELP	m6502->help.w.l

#define M6502_RDMEM(Addr)	GetByte(m6502->hardware,Addr)
#define M6502_WRMEM(Addr,Value)	SetByte(m6502->hardware,Addr,Value)

#define RM(Addr)	M6502_RDMEM(Addr)
#define RM16(Addr)	((RM(((Addr)+1)&0xffff) << 8) | RM(Addr))
#define WM(Addr,Value)	M6502_WRMEM(Addr,Value)

#endif
