// HNZVC
// ? = undefined
// * = affected
// - = unaffected
// 0 = cleared
// 1 = set
// # = ccr directly affected by instruction
// @ = special - carry set if bit 7 is set

static void illegal(m6803_Regs *m6803)
{
}

static void trap(m6803_Regs *m6803)
{
  TAKE_TRAP;
}

/* $00 ILLEGAL */

/* $01 NOP */
static void nop(m6803_Regs *m6803)
{
}

/* $02 ILLEGAL */

/* $03 ILLEGAL */

/* $04 LSRD inherent -0*-* */
static void lsrd(m6803_Regs *m6803)
{
  UInt16 t;
  CLR_NZC; t = D; CC|=(t&0x0001);
  t>>=1; SET_Z16(t); D=t;
}

/* $05 ASLD inherent ?**** */
static void asld(m6803_Regs *m6803)
{
  int r;
  UInt16 t;
  t = D; r=t<<1;
  CLR_NZVC; SET_FLAGS16(t,t,r);
  D=r;
}

/* $06 TAP inherent ##### */
static void tap(m6803_Regs *m6803)
{
  CC=A;
  ONE_MORE_INSN(m6803);
  CHECK_IRQ_LINES();
}

/* $07 TPA inherent ----- */
static void tpa(m6803_Regs *m6803)
{
  A=CC;
}

/* $08 INX inherent --*-- */
static void inx(m6803_Regs *m6803)
{
  ++X;
  CLR_Z; SET_Z16(X);
}

/* $09 DEX inherent --*-- */
static void dex(m6803_Regs *m6803)
{
  --X;
  CLR_Z; SET_Z16(X);
}

/* $0a CLV */
static void clv(m6803_Regs *m6803)
{
  CLV;
}

/* $0b SEV */
static void sev(m6803_Regs *m6803)
{
  SEV;
}

/* $0c CLC */
static void clc(m6803_Regs *m6803)
{
  CLC;
}

/* $0d SEC */
static void sec(m6803_Regs *m6803)
{
  SEC;
}

/* $0e CLI */
static void cli(m6803_Regs *m6803)
{
  CLI;
  ONE_MORE_INSN(m6803);
  CHECK_IRQ_LINES();
}

/* $0f SEI */
static void sei(m6803_Regs *m6803)
{
  SEI;
  ONE_MORE_INSN(m6803);
  CHECK_IRQ_LINES();
}

/* $10 SBA inherent -**** */
static void sba(m6803_Regs *m6803)
{
  UInt16 t;
  t=A-B;
  CLR_NZVC; SET_FLAGS8(A,B,t);
  A=t;
}

/* $11 CBA inherent -**** */
static void cba(m6803_Regs *m6803)
{
  UInt16 t;
  t=A-B;
  CLR_NZVC; SET_FLAGS8(A,B,t);
}

/* $12 ILLEGAL */
static void undoc1(m6803_Regs *m6803)
{
  X += RM( S + 1 );
}

/* $13 ILLEGAL */
static void undoc2(m6803_Regs *m6803)
{
  X += RM( S + 1 );
}


/* $14 ILLEGAL */

/* $15 ILLEGAL */

/* $16 TAB inherent -**0- */
static void tab(m6803_Regs *m6803)
{
  B=A;
  CLR_NZV; SET_NZ8(B);
}

/* $17 TBA inherent -**0- */
static void tba(m6803_Regs *m6803)
{
  A=B;
  CLR_NZV; SET_NZ8(A);
}

/* $18 XGDX inherent ----- */ /* HD63701YO only */
static void xgdx(m6803_Regs *m6803)
{
  UInt16 t = X;
  X = D;
  D=t;
}

/* $19 DAA inherent (A) -**0* */
static void daa(m6803_Regs *m6803)
{
  UInt8 msn, lsn;
  UInt16 t, cf = 0;
  msn=A & 0xf0; lsn=A & 0x0f;
  if( lsn>0x09 || CC&0x20 ) cf |= 0x06;
  if( msn>0x80 && lsn>0x09 ) cf |= 0x60;
  if( msn>0x90 || CC&0x01 ) cf |= 0x60;
  t = cf + A;
  CLR_NZV; /* keep carry from previous operation */
  SET_NZ8((UInt8)t); SET_C8(t);
  A = t;
}

/* $1a ILLEGAL */

/* $1b ABA inherent ***** */
static void aba(m6803_Regs *m6803)
{
  UInt16 t;
  t=A+B;
  CLR_HNZVC; SET_FLAGS8(A,B,t); SET_H(A,B,t);
  A=t;
}

/* $1c ILLEGAL */

/* $1d ILLEGAL */

/* $1e ILLEGAL */

/* $1f ILLEGAL */

/* $20 BRA relative ----- */
static void bra(m6803_Regs *m6803)
{
  UInt8 t;
  IMMBYTE(t);PC+=SIGNED(t);
  /* speed up busy loops */
  if (t==0xfe) EAT_CYCLES(m6803);
}

/* $21 BRN relative ----- */
static void brn(m6803_Regs *m6803)
{
  UInt8 t;
  IMMBYTE(t);
  (void)t;
}

/* $22 BHI relative ----- */
static void bhi(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(!(CC&0x05));
}

/* $23 BLS relative ----- */
static void bls(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(CC&0x05);
}

/* $24 BCC relative ----- */
static void bcc(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(!(CC&0x01));
}

/* $25 BCS relative ----- */
static void bcs(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(CC&0x01);
}

/* $26 BNE relative ----- */
static void bne(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(!(CC&0x04));
}

/* $27 BEQ relative ----- */
static void beq(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(CC&0x04);
}

/* $28 BVC relative ----- */
static void bvc(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(!(CC&0x02));
}

/* $29 BVS relative ----- */
static void bvs(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(CC&0x02);
}

/* $2a BPL relative ----- */
static void bpl(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(!(CC&0x08));
}

/* $2b BMI relative ----- */
static void bmi(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(CC&0x08);
}

/* $2c BGE relative ----- */
static void bge(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(!NXORV);
}

/* $2d BLT relative ----- */
static void blt(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(NXORV);
}

/* $2e BGT relative ----- */
static void bgt(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(!(NXORV||CC&0x04));
}

/* $2f BLE relative ----- */
static void ble(m6803_Regs *m6803)
{
  UInt8 t;
  BRANCH(NXORV||CC&0x04);
}

/* $30 TSX inherent ----- */
static void tsx(m6803_Regs *m6803)
{
  X = ( S + 1 );
}

/* $31 INS inherent ----- */
static void ins(m6803_Regs *m6803)
{
  ++S;
}

/* $32 PULA inherent ----- */
static void pula(m6803_Regs *m6803)
{
  PULLBYTE(m6803->d.b.h);
}

/* $33 PULB inherent ----- */
static void pulb(m6803_Regs *m6803)
{
  PULLBYTE(m6803->d.b.l);
}

/* $34 DES inherent ----- */
static void des(m6803_Regs *m6803)
{
  --S;
}

/* $35 TXS inherent ----- */
static void txs(m6803_Regs *m6803)
{
  S = ( X - 1 );
}

/* $36 PSHA inherent ----- */
static void psha(m6803_Regs *m6803)
{
  PUSHBYTE(m6803->d.b.h);
}

/* $37 PSHB inherent ----- */
static void pshb(m6803_Regs *m6803)
{
  PUSHBYTE(m6803->d.b.l);
}

/* $38 PULX inherent ----- */
static void pulx(m6803_Regs *m6803)
{
  PULLWORD(pX);
}

/* $39 RTS inherent ----- */
static void rts(m6803_Regs *m6803)
{
  PULLWORD(pPC);
}

/* $3a ABX inherent ----- */
static void abx(m6803_Regs *m6803)
{
  X += B;
}

/* $3b RTI inherent ##### */
static void rti(m6803_Regs *m6803)
{
  PULLBYTE(CC);
  PULLBYTE(B);
  PULLBYTE(A);
  PULLWORD(pX);
  PULLWORD(pPC);
  CHECK_IRQ_LINES();
}

/* $3c PSHX inherent ----- */
static void pshx(m6803_Regs *m6803)
{
  PUSHWORD(pX);
}

/* $3d MUL inherent --*-@ */
static void mul(m6803_Regs *m6803)
{
  UInt16 t;
  t=A*B;
  CLR_C; if(t&0x80) SEC;
  D=t;
}

/* $3e WAI inherent ----- */
static void wai(m6803_Regs *m6803)
{
  /*
   * WAI stacks the entire machine state on the
   * hardware stack, then waits for an interrupt.
   */
  m6803->wai_state |= M6803_WAI;
  PUSHWORD(pPC);
  PUSHWORD(pX);
  PUSHBYTE(A);
  PUSHBYTE(B);
  PUSHBYTE(CC);
  CHECK_IRQ_LINES();
  if (m6803->wai_state & M6803_WAI) EAT_CYCLES(m6803);
}

/* $3f SWI absolute indirect ----- */
static void swi(m6803_Regs *m6803)
{
  PUSHWORD(pPC);
  PUSHWORD(pX);
  PUSHBYTE(A);
  PUSHBYTE(B);
    PUSHBYTE(CC);
    SEI;
  PCD = RM16(0xfffa);
}

/* $40 NEGA inherent ?**** */
static void nega(m6803_Regs *m6803)
{
  UInt16 r;
  r=-A;
  CLR_NZVC; SET_FLAGS8(0,A,r);
  A=r;
}

/* $41 ILLEGAL */

/* $42 ILLEGAL */

/* $43 COMA inherent -**01 */
static void coma(m6803_Regs *m6803)
{
  A = ~A;
  CLR_NZV; SET_NZ8(A); SEC;
}

/* $44 LSRA inherent -0*-* */
static void lsra(m6803_Regs *m6803)
{
  CLR_NZC; CC|=(A&0x01);
  A>>=1; SET_Z8(A);
}

/* $45 ILLEGAL */

/* $46 RORA inherent -**-* */
static void rora(m6803_Regs *m6803)
{
  UInt8 r;
  r=(CC&0x01)<<7;
  CLR_NZC; CC|=(A&0x01);
  r |= A>>1; SET_NZ8(r);
  A=r;
}

/* $47 ASRA inherent ?**-* */
static void asra(m6803_Regs *m6803)
{
  CLR_NZC; CC|=(A&0x01);
  A>>=1; A|=((A&0x40)<<1);
  SET_NZ8(A);
}

/* $48 ASLA inherent ?**** */
static void asla(m6803_Regs *m6803)
{
  UInt16 r;
  r=A<<1;
  CLR_NZVC; SET_FLAGS8(A,A,r);
  A=r;
}

/* $49 ROLA inherent -**** */
static void rola(m6803_Regs *m6803)
{
  UInt16 t,r;
  t = A; r = CC&0x01; r |= t<<1;
  CLR_NZVC; SET_FLAGS8(t,t,r);
  A=r;
}

/* $4a DECA inherent -***- */
static void deca(m6803_Regs *m6803)
{
  --A;
  CLR_NZV; SET_FLAGS8D(A);
}

/* $4b ILLEGAL */

/* $4c INCA inherent -***- */
static void inca(m6803_Regs *m6803)
{
  ++A;
  CLR_NZV; SET_FLAGS8I(A);
}

/* $4d TSTA inherent -**0- */
static void tsta(m6803_Regs *m6803)
{
  CLR_NZVC; SET_NZ8(A);
}

/* $4e ILLEGAL */

/* $4f CLRA inherent -0100 */
static void clra(m6803_Regs *m6803)
{
  A=0;
  CLR_NZVC; SEZ;
}

/* $50 NEGB inherent ?**** */
static void negb(m6803_Regs *m6803)
{
  UInt16 r;
  r=-B;
  CLR_NZVC; SET_FLAGS8(0,B,r);
  B=r;
}

/* $51 ILLEGAL */

/* $52 ILLEGAL */

/* $53 COMB inherent -**01 */
static void comb(m6803_Regs *m6803)
{
  B = ~B;
  CLR_NZV; SET_NZ8(B); SEC;
}

/* $54 LSRB inherent -0*-* */
static void lsrb(m6803_Regs *m6803)
{
  CLR_NZC; CC|=(B&0x01);
  B>>=1; SET_Z8(B);
}

/* $55 ILLEGAL */

/* $56 RORB inherent -**-* */
static void rorb(m6803_Regs *m6803)
{
  UInt8 r;
  r=(CC&0x01)<<7;
  CLR_NZC; CC|=(B&0x01);
  r |= B>>1; SET_NZ8(r);
  B=r;
}

/* $57 ASRB inherent ?**-* */
static void asrb(m6803_Regs *m6803)
{
  CLR_NZC; CC|=(B&0x01);
  B>>=1; B|=((B&0x40)<<1);
  SET_NZ8(B);
}

/* $58 ASLB inherent ?**** */
static void aslb(m6803_Regs *m6803)
{
  UInt16 r;
  r=B<<1;
  CLR_NZVC; SET_FLAGS8(B,B,r);
  B=r;
}

/* $59 ROLB inherent -**** */
static void rolb(m6803_Regs *m6803)
{
  UInt16 t,r;
  t = B; r = CC&0x01; r |= t<<1;
  CLR_NZVC; SET_FLAGS8(t,t,r);
  B=r;
}

/* $5a DECB inherent -***- */
static void decb(m6803_Regs *m6803)
{
  --B;
  CLR_NZV; SET_FLAGS8D(B);
}

/* $5b ILLEGAL */

/* $5c INCB inherent -***- */
static void incb(m6803_Regs *m6803)
{
  ++B;
  CLR_NZV; SET_FLAGS8I(B);
}

/* $5d TSTB inherent -**0- */
static void tstb(m6803_Regs *m6803)
{
  CLR_NZVC; SET_NZ8(B);
}

/* $5e ILLEGAL */

/* $5f CLRB inherent -0100 */
static void clrb(m6803_Regs *m6803)
{
  B=0;
  CLR_NZVC; SEZ;
}

/* $60 NEG indexed ?**** */
static void neg_ix(m6803_Regs *m6803)
{
  UInt16 r,t;
  IDXBYTE(t); r=-t;
  CLR_NZVC; SET_FLAGS8(0,t,r);
  WM(EAD,r);
}

/* $61 AIM --**0- */ /* HD63701YO only */
static void aim_ix(m6803_Regs *m6803)
{
  UInt8 t, r;
  IMMBYTE(t);
  IDXBYTE(r);
  r &= t;
  CLR_NZV; SET_NZ8(r);
  WM(EAD,r);
}

/* $62 OIM --**0- */ /* HD63701YO only */
static void oim_ix(m6803_Regs *m6803)
{
  UInt8 t, r;
  IMMBYTE(t);
  IDXBYTE(r);
  r |= t;
  CLR_NZV; SET_NZ8(r);
  WM(EAD,r);
}

/* $63 COM indexed -**01 */
static void com_ix(m6803_Regs *m6803)
{
  UInt8 t;
  IDXBYTE(t); t = ~t;
  CLR_NZV; SET_NZ8(t); SEC;
  WM(EAD,t);
}

/* $64 LSR indexed -0*-* */
static void lsr_ix(m6803_Regs *m6803)
{
  UInt8 t;
  IDXBYTE(t); CLR_NZC; CC|=(t&0x01);
  t>>=1; SET_Z8(t);
  WM(EAD,t);
}

/* $65 EIM --**0- */ /* HD63701YO only */
static void eim_ix(m6803_Regs *m6803)
{
  UInt8 t, r;
  IMMBYTE(t);
  IDXBYTE(r);
  r ^= t;
  CLR_NZV; SET_NZ8(r);
  WM(EAD,r);
}

/* $66 ROR indexed -**-* */
static void ror_ix(m6803_Regs *m6803)
{
  UInt8 t,r;
  IDXBYTE(t); r=(CC&0x01)<<7;
  CLR_NZC; CC|=(t&0x01);
  r |= t>>1; SET_NZ8(r);
  WM(EAD,r);
}

/* $67 ASR indexed ?**-* */
static void asr_ix(m6803_Regs *m6803)
{
  UInt8 t;
  IDXBYTE(t); CLR_NZC; CC|=(t&0x01);
  t>>=1; t|=((t&0x40)<<1);
  SET_NZ8(t);
  WM(EAD,t);
}

/* $68 ASL indexed ?**** */
static void asl_ix(m6803_Regs *m6803)
{
  UInt16 t,r;
  IDXBYTE(t); r=t<<1;
  CLR_NZVC; SET_FLAGS8(t,t,r);
  WM(EAD,r);
}

/* $69 ROL indexed -**** */
static void rol_ix(m6803_Regs *m6803)
{
  UInt16 t,r;
  IDXBYTE(t); r = CC&0x01; r |= t<<1;
  CLR_NZVC; SET_FLAGS8(t,t,r);
  WM(EAD,r);
}

/* $6a DEC indexed -***- */
static void dec_ix(m6803_Regs *m6803)
{
  UInt8 t;
  IDXBYTE(t); --t;
  CLR_NZV; SET_FLAGS8D(t);
  WM(EAD,t);
}

/* $6b TIM --**0- */ /* HD63701YO only */
static void tim_ix(m6803_Regs *m6803)
{
  UInt8 t, r;
  IMMBYTE(t);
  IDXBYTE(r);
  r &= t;
  CLR_NZV; SET_NZ8(r);
}

/* $6c INC indexed -***- */
static void inc_ix(m6803_Regs *m6803)
{
  UInt8 t;
  IDXBYTE(t); ++t;
  CLR_NZV; SET_FLAGS8I(t);
  WM(EAD,t);
}

/* $6d TST indexed -**0- */
static void tst_ix(m6803_Regs *m6803)
{
  UInt8 t;
  IDXBYTE(t); CLR_NZVC; SET_NZ8(t);
}

/* $6e JMP indexed ----- */
static void jmp_ix(m6803_Regs *m6803)
{
  INDEXED; PC=EA;
}

/* $6f CLR indexed -0100 */
static void clr_ix(m6803_Regs *m6803)
{
  INDEXED; WM(EAD,0);
  CLR_NZVC; SEZ;
}

/* $70 NEG extended ?**** */
static void neg_ex(m6803_Regs *m6803)
{
  UInt16 r,t;
  EXTBYTE(t); r=-t;
  CLR_NZVC; SET_FLAGS8(0,t,r);
  WM(EAD,r);
}

/* $71 AIM --**0- */ /* HD63701YO only */
static void aim_di(m6803_Regs *m6803)
{
  UInt8 t, r;
  IMMBYTE(t);
  DIRBYTE(r);
  r &= t;
  CLR_NZV; SET_NZ8(r);
  WM(EAD,r);
}

/* $72 OIM --**0- */ /* HD63701YO only */
static void oim_di(m6803_Regs *m6803)
{
  UInt8 t, r;
  IMMBYTE(t);
  DIRBYTE(r);
  r |= t;
  CLR_NZV; SET_NZ8(r);
  WM(EAD,r);
}

/* $73 COM extended -**01 */
static void com_ex(m6803_Regs *m6803)
{
  UInt8 t;
  EXTBYTE(t); t = ~t;
  CLR_NZV; SET_NZ8(t); SEC;
  WM(EAD,t);
}

/* $74 LSR extended -0*-* */
static void lsr_ex(m6803_Regs *m6803)
{
  UInt8 t;
  EXTBYTE(t);
  CLR_NZC;
  CC|=(t&0x01);
  t>>=1;
  SET_Z8(t);
  WM(EAD,t);
}

/* $75 EIM --**0- */ /* HD63701YO only */
static void eim_di(m6803_Regs *m6803)
{
  UInt8 t, r;
  IMMBYTE(t);
  DIRBYTE(r);
  r ^= t;
  CLR_NZV; SET_NZ8(r);
  WM(EAD,r);
}

/* $76 ROR extended -**-* */
static void ror_ex(m6803_Regs *m6803)
{
  UInt8 t,r;
  EXTBYTE(t); r=(CC&0x01)<<7;
  CLR_NZC; CC|=(t&0x01);
  r |= t>>1; SET_NZ8(r);
  WM(EAD,r);
}

/* $77 ASR extended ?**-* */
static void asr_ex(m6803_Regs *m6803)
{
  UInt8 t;
  EXTBYTE(t); CLR_NZC; CC|=(t&0x01);
  t>>=1; t|=((t&0x40)<<1);
  SET_NZ8(t);
  WM(EAD,t);
}

/* $78 ASL extended ?**** */
static void asl_ex(m6803_Regs *m6803)
{
  UInt16 t,r;
  EXTBYTE(t); r=t<<1;
  CLR_NZVC; SET_FLAGS8(t,t,r);
  WM(EAD,r);
}

/* $79 ROL extended -**** */
static void rol_ex(m6803_Regs *m6803)
{
  UInt16 t,r;
  EXTBYTE(t); r = CC&0x01; r |= t<<1;
  CLR_NZVC; SET_FLAGS8(t,t,r);
  WM(EAD,r);
}

/* $7a DEC extended -***- */
static void dec_ex(m6803_Regs *m6803)
{
  UInt8 t;
  EXTBYTE(t); --t;
  CLR_NZV; SET_FLAGS8D(t);
  WM(EAD,t);
}

/* $7b TIM --**0- */ /* HD63701YO only */
static void tim_di(m6803_Regs *m6803)
{
  UInt8 t, r;
  IMMBYTE(t);
  DIRBYTE(r);
  r &= t;
  CLR_NZV; SET_NZ8(r);
}

/* $7c INC extended -***- */
static void inc_ex(m6803_Regs *m6803)
{
  UInt8 t;
  EXTBYTE(t); ++t;
  CLR_NZV; SET_FLAGS8I(t);
  WM(EAD,t);
}

/* $7d TST extended -**0- */
static void tst_ex(m6803_Regs *m6803)
{
  UInt8 t;
  EXTBYTE(t); CLR_NZVC; SET_NZ8(t);
}

/* $7e JMP extended ----- */
static void jmp_ex(m6803_Regs *m6803)
{
  EXTENDED; PC=EA;
}

/* $7f CLR extended -0100 */
static void clr_ex(m6803_Regs *m6803)
{
  EXTENDED; WM(EAD,0);
  CLR_NZVC; SEZ;
}

/* $80 SUBA immediate ?**** */
static void suba_im(m6803_Regs *m6803)
{
  UInt16	  t,r;
  IMMBYTE(t); r = A-t;
  CLR_NZVC; SET_FLAGS8(A,t,r);
  A = r;
}

/* $81 CMPA immediate ?**** */
static void cmpa_im(m6803_Regs *m6803)
{
  UInt16	  t,r;
  IMMBYTE(t); r = A-t;
  CLR_NZVC; SET_FLAGS8(A,t,r);
}

/* $82 SBCA immediate ?**** */
static void sbca_im(m6803_Regs *m6803)
{
  UInt16	  t,r;
  IMMBYTE(t); r = A-t-(CC&0x01);
  CLR_NZVC; SET_FLAGS8(A,t,r);
  A = r;
}

/* $83 SUBD immediate -**** */
static void subd_im(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  IMMWORD(b);
  d = D;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  D = r;
}

/* $84 ANDA immediate -**0- */
static void anda_im(m6803_Regs *m6803)
{
  UInt8 t;
  IMMBYTE(t); A &= t;
  CLR_NZV; SET_NZ8(A);
}

/* $85 BITA immediate -**0- */
static void bita_im(m6803_Regs *m6803)
{
  UInt8 t,r;
  IMMBYTE(t); r = A&t;
  CLR_NZV; SET_NZ8(r);
}

/* $86 LDA immediate -**0- */
static void lda_im(m6803_Regs *m6803)
{
  IMMBYTE(A);
  CLR_NZV; SET_NZ8(A);
}

/* is this a legal instruction? */
/* $87 STA immediate -**0- */
static void sta_im(m6803_Regs *m6803)
{
  CLR_NZV; SET_NZ8(A);
  IMM8; WM(EAD,A);
}

/* $88 EORA immediate -**0- */
static void eora_im(m6803_Regs *m6803)
{
  UInt8 t;
  IMMBYTE(t); A ^= t;
  CLR_NZV; SET_NZ8(A);
}

/* $89 ADCA immediate ***** */
static void adca_im(m6803_Regs *m6803)
{
  UInt16 t,r;
  IMMBYTE(t); r = A+t+(CC&0x01);
  CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
  A = r;
}

/* $8a ORA immediate -**0- */
static void ora_im(m6803_Regs *m6803)
{
  UInt8 t;
  IMMBYTE(t); A |= t;
  CLR_NZV; SET_NZ8(A);
}

/* $8b ADDA immediate ***** */
static void adda_im(m6803_Regs *m6803)
{
  UInt16 t,r;
  IMMBYTE(t); r = A+t;
  CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
  A = r;
}

/* $8c CMPX immediate -***- */
static void cmpx_im(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  IMMWORD(b);
  d = X;
  r = d - b.d;
  CLR_NZV;
  SET_NZ16(r); SET_V16(d,b.d,r);
}

/* $8c CPX immediate -**** (6803) */
static void cpx_im(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  IMMWORD(b);
  d = X;
  r = d - b.d;
  CLR_NZVC; SET_FLAGS16(d,b.d,r);
}

/* $8d BSR ----- */
static void bsr(m6803_Regs *m6803)
{
  UInt8 t;
  IMMBYTE(t);
  PUSHWORD(pPC);
  PC += SIGNED(t);
}

/* $8e LDS immediate -**0- */
static void lds_im(m6803_Regs *m6803)
{
  IMMWORD(m6803->s);
  CLR_NZV;
  SET_NZ16(S);
}

/* $8f STS immediate -**0- */
static void sts_im(m6803_Regs *m6803)
{
  CLR_NZV;
  SET_NZ16(S);
  IMM16;
  WM16(EAD,m6803->s);
}

/* $90 SUBA direct ?**** */
static void suba_di(m6803_Regs *m6803)
{
  UInt16	  t,r;
  DIRBYTE(t); r = A-t;
  CLR_NZVC; SET_FLAGS8(A,t,r);
  A = r;
}

/* $91 CMPA direct ?**** */
static void cmpa_di(m6803_Regs *m6803)
{
  UInt16	  t,r;
  DIRBYTE(t); r = A-t;
  CLR_NZVC; SET_FLAGS8(A,t,r);
}

/* $92 SBCA direct ?**** */
static void sbca_di(m6803_Regs *m6803)
{
  UInt16	  t,r;
  DIRBYTE(t); r = A-t-(CC&0x01);
  CLR_NZVC; SET_FLAGS8(A,t,r);
  A = r;
}

/* $93 SUBD direct -**** */
static void subd_di(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  DIRWORD(b);
  d = D;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  D=r;
}

/* $94 ANDA direct -**0- */
static void anda_di(m6803_Regs *m6803)
{
  UInt8 t;
  DIRBYTE(t); A &= t;
  CLR_NZV; SET_NZ8(A);
}

/* $95 BITA direct -**0- */
static void bita_di(m6803_Regs *m6803)
{
  UInt8 t,r;
  DIRBYTE(t); r = A&t;
  CLR_NZV; SET_NZ8(r);
}

/* $96 LDA direct -**0- */
static void lda_di(m6803_Regs *m6803)
{
  DIRBYTE(A);
  CLR_NZV; SET_NZ8(A);
}

/* $97 STA direct -**0- */
static void sta_di(m6803_Regs *m6803)
{
  CLR_NZV; SET_NZ8(A);
  DIRECT; WM(EAD,A);
}

/* $98 EORA direct -**0- */
static void eora_di(m6803_Regs *m6803)
{
  UInt8 t;
  DIRBYTE(t); A ^= t;
  CLR_NZV; SET_NZ8(A);
}

/* $99 ADCA direct ***** */
static void adca_di(m6803_Regs *m6803)
{
  UInt16 t,r;
  DIRBYTE(t); r = A+t+(CC&0x01);
  CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
  A = r;
}

/* $9a ORA direct -**0- */
static void ora_di(m6803_Regs *m6803)
{
  UInt8 t;
  DIRBYTE(t); A |= t;
  CLR_NZV; SET_NZ8(A);
}

/* $9b ADDA direct ***** */
static void adda_di(m6803_Regs *m6803)
{
  UInt16 t,r;
  DIRBYTE(t); r = A+t;
  CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
  A = r;
}

/* $9c CMPX direct -***- */
static void cmpx_di(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  DIRWORD(b);
  d = X;
  r = d - b.d;
  CLR_NZV;
  SET_NZ16(r); SET_V16(d,b.d,r);
}

/* $9c CPX direct -**** (6803) */
static void cpx_di(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  DIRWORD(b);
  d = X;
  r = d - b.d;
  CLR_NZVC; SET_FLAGS16(d,b.d,r);
}

/* $9d JSR direct ----- */
static void jsr_di(m6803_Regs *m6803)
{
  DIRECT;
  PUSHWORD(pPC);
    PC = EA;
}

/* $9e LDS direct -**0- */
static void lds_di(m6803_Regs *m6803)
{
  DIRWORD(m6803->s);
  CLR_NZV;
  SET_NZ16(S);
}

/* $9f STS direct -**0- */
static void sts_di(m6803_Regs *m6803)
{
  CLR_NZV;
  SET_NZ16(S);
  DIRECT;
  WM16(EAD,m6803->s);
}

/* $a0 SUBA indexed ?**** */
static void suba_ix(m6803_Regs *m6803)
{
  UInt16	  t,r;
  IDXBYTE(t); r = A-t;
  CLR_NZVC; SET_FLAGS8(A,t,r);
  A = r;
}

/* $a1 CMPA indexed ?**** */
static void cmpa_ix(m6803_Regs *m6803)
{
  UInt16	  t,r;
  IDXBYTE(t); r = A-t;
  CLR_NZVC; SET_FLAGS8(A,t,r);
}

/* $a2 SBCA indexed ?**** */
static void sbca_ix(m6803_Regs *m6803)
{
  UInt16	  t,r;
  IDXBYTE(t); r = A-t-(CC&0x01);
  CLR_NZVC; SET_FLAGS8(A,t,r);
  A = r;
}

/* $a3 SUBD indexed -**** */
static void subd_ix(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  IDXWORD(b);
  d = D;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  D = r;
}

/* $a4 ANDA indexed -**0- */
static void anda_ix(m6803_Regs *m6803)
{
  UInt8 t;
  IDXBYTE(t); A &= t;
  CLR_NZV; SET_NZ8(A);
}

/* $a5 BITA indexed -**0- */
static void bita_ix(m6803_Regs *m6803)
{
  UInt8 t,r;
  IDXBYTE(t); r = A&t;
  CLR_NZV; SET_NZ8(r);
}

/* $a6 LDA indexed -**0- */
static void lda_ix(m6803_Regs *m6803)
{
  IDXBYTE(A);
  CLR_NZV; SET_NZ8(A);
}

/* $a7 STA indexed -**0- */
static void sta_ix(m6803_Regs *m6803)
{
  CLR_NZV; SET_NZ8(A);
  INDEXED; WM(EAD,A);
}

/* $a8 EORA indexed -**0- */
static void eora_ix(m6803_Regs *m6803)
{
  UInt8 t;
  IDXBYTE(t); A ^= t;
  CLR_NZV; SET_NZ8(A);
}

/* $a9 ADCA indexed ***** */
static void adca_ix(m6803_Regs *m6803)
{
  UInt16 t,r;
  IDXBYTE(t); r = A+t+(CC&0x01);
  CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
  A = r;
}

/* $aa ORA indexed -**0- */
static void ora_ix(m6803_Regs *m6803)
{
  UInt8 t;
  IDXBYTE(t); A |= t;
  CLR_NZV; SET_NZ8(A);
}

/* $ab ADDA indexed ***** */
static void adda_ix(m6803_Regs *m6803)
{
  UInt16 t,r;
  IDXBYTE(t); r = A+t;
  CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
  A = r;
}

/* $ac CMPX indexed -***- */
static void cmpx_ix(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  IDXWORD(b);
  d = X;
  r = d - b.d;
  CLR_NZV;
  SET_NZ16(r); SET_V16(d,b.d,r);
}

/* $ac CPX indexed -**** (6803)*/
static void cpx_ix(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  IDXWORD(b);
  d = X;
  r = d - b.d;
  CLR_NZVC; SET_FLAGS16(d,b.d,r);
}

/* $ad JSR indexed ----- */
static void jsr_ix(m6803_Regs *m6803)
{
  INDEXED;
  PUSHWORD(pPC);
    PC = EA;
}

/* $ae LDS indexed -**0- */
static void lds_ix(m6803_Regs *m6803)
{
  IDXWORD(m6803->s);
  CLR_NZV;
  SET_NZ16(S);
}

/* $af STS indexed -**0- */
static void sts_ix(m6803_Regs *m6803)
{
  CLR_NZV;
  SET_NZ16(S);
  INDEXED;
  WM16(EAD,m6803->s);
}

/* $b0 SUBA extended ?**** */
static void suba_ex(m6803_Regs *m6803)
{
  UInt16	  t,r;
  EXTBYTE(t); r = A-t;
  CLR_NZVC; SET_FLAGS8(A,t,r);
  A = r;
}

/* $b1 CMPA extended ?**** */
static void cmpa_ex(m6803_Regs *m6803)
{
  UInt16	  t,r;
  EXTBYTE(t); r = A-t;
  CLR_NZVC; SET_FLAGS8(A,t,r);
}

/* $b2 SBCA extended ?**** */
static void sbca_ex(m6803_Regs *m6803)
{
  UInt16	  t,r;
  EXTBYTE(t); r = A-t-(CC&0x01);
  CLR_NZVC; SET_FLAGS8(A,t,r);
  A = r;
}

/* $b3 SUBD extended -**** */
static void subd_ex(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  EXTWORD(b);
  d = D;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  D=r;
}

/* $b4 ANDA extended -**0- */
static void anda_ex(m6803_Regs *m6803)
{
  UInt8 t;
  EXTBYTE(t); A &= t;
  CLR_NZV; SET_NZ8(A);
}

/* $b5 BITA extended -**0- */
static void bita_ex(m6803_Regs *m6803)
{
  UInt8 t,r;
  EXTBYTE(t); r = A&t;
  CLR_NZV; SET_NZ8(r);
}

/* $b6 LDA extended -**0- */
static void lda_ex(m6803_Regs *m6803)
{
  EXTBYTE(A);
  CLR_NZV; SET_NZ8(A);
}

/* $b7 STA extended -**0- */
static void sta_ex(m6803_Regs *m6803)
{
  CLR_NZV; SET_NZ8(A);
  EXTENDED; WM(EAD,A);
}

/* $b8 EORA extended -**0- */
static void eora_ex(m6803_Regs *m6803)
{
  UInt8 t;
  EXTBYTE(t); A ^= t;
  CLR_NZV; SET_NZ8(A);
}

/* $b9 ADCA extended ***** */
static void adca_ex(m6803_Regs *m6803)
{
  UInt16 t,r;
  EXTBYTE(t); r = A+t+(CC&0x01);
  CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
  A = r;
}

/* $ba ORA extended -**0- */
static void ora_ex(m6803_Regs *m6803)
{
  UInt8 t;
  EXTBYTE(t); A |= t;
  CLR_NZV; SET_NZ8(A);
}

/* $bb ADDA extended ***** */
static void adda_ex(m6803_Regs *m6803)
{
  UInt16 t,r;
  EXTBYTE(t); r = A+t;
  CLR_HNZVC; SET_FLAGS8(A,t,r); SET_H(A,t,r);
  A = r;
}

/* $bc CMPX extended -***- */
static void cmpx_ex(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  EXTWORD(b);
  d = X;
  r = d - b.d;
  CLR_NZV;
  SET_NZ16(r); SET_V16(d,b.d,r);
}

/* $bc CPX extended -**** (6803) */
static void cpx_ex(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  EXTWORD(b);
  d = X;
  r = d - b.d;
  CLR_NZVC; SET_FLAGS16(d,b.d,r);
}

/* $bd JSR extended ----- */
static void jsr_ex(m6803_Regs *m6803)
{
  EXTENDED;
  PUSHWORD(pPC);
    PC = EA;
}

/* $be LDS extended -**0- */
static void lds_ex(m6803_Regs *m6803)
{
  EXTWORD(m6803->s);
  CLR_NZV;
  SET_NZ16(S);
}

/* $bf STS extended -**0- */
static void sts_ex(m6803_Regs *m6803)
{
  CLR_NZV;
  SET_NZ16(S);
  EXTENDED;
  WM16(EAD,m6803->s);
}

/* $c0 SUBB immediate ?**** */
static void subb_im(m6803_Regs *m6803)
{
  UInt16	  t,r;
  IMMBYTE(t); r = B-t;
  CLR_NZVC; SET_FLAGS8(B,t,r);
  B = r;
}

/* $c1 CMPB immediate ?**** */
static void cmpb_im(m6803_Regs *m6803)
{
  UInt16	  t,r;
  IMMBYTE(t); r = B-t;
  CLR_NZVC; SET_FLAGS8(B,t,r);
}

/* $c2 SBCB immediate ?**** */
static void sbcb_im(m6803_Regs *m6803)
{
  UInt16	  t,r;
  IMMBYTE(t); r = B-t-(CC&0x01);
  CLR_NZVC; SET_FLAGS8(B,t,r);
  B = r;
}

/* $c3 ADDD immediate -**** */
static void addd_im(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  IMMWORD(b);
  d = D;
  r = d + b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  D = r;
}

/* $c4 ANDB immediate -**0- */
static void andb_im(m6803_Regs *m6803)
{
  UInt8 t;
  IMMBYTE(t); B &= t;
  CLR_NZV; SET_NZ8(B);
}

/* $c5 BITB immediate -**0- */
static void bitb_im(m6803_Regs *m6803)
{
  UInt8 t,r;
  IMMBYTE(t); r = B&t;
  CLR_NZV; SET_NZ8(r);
}

/* $c6 LDB immediate -**0- */
static void ldb_im(m6803_Regs *m6803)
{
  IMMBYTE(B);
  CLR_NZV; SET_NZ8(B);
}

/* is this a legal instruction? */
/* $c7 STB immediate -**0- */
static void stb_im(m6803_Regs *m6803)
{
  CLR_NZV; SET_NZ8(B);
  IMM8; WM(EAD,B);
}

/* $c8 EORB immediate -**0- */
static void eorb_im(m6803_Regs *m6803)
{
  UInt8 t;
  IMMBYTE(t); B ^= t;
  CLR_NZV; SET_NZ8(B);
}

/* $c9 ADCB immediate ***** */
static void adcb_im(m6803_Regs *m6803)
{
  UInt16 t,r;
  IMMBYTE(t); r = B+t+(CC&0x01);
  CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
  B = r;
}

/* $ca ORB immediate -**0- */
static void orb_im(m6803_Regs *m6803)
{
  UInt8 t;
  IMMBYTE(t); B |= t;
  CLR_NZV; SET_NZ8(B);
}

/* $cb ADDB immediate ***** */
static void addb_im(m6803_Regs *m6803)
{
  UInt16 t,r;
  IMMBYTE(t); r = B+t;
  CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
  B = r;
}

/* $CC LDD immediate -**0- */
static void ldd_im(m6803_Regs *m6803)
{
  IMMWORD(m6803->d);
  CLR_NZV;
  SET_NZ16(D);
}

/* is this a legal instruction? */
/* $cd STD immediate -**0- */
static void std_im(m6803_Regs *m6803)
{
  IMM16;
  CLR_NZV;
  SET_NZ16(D);
  WM16(EAD,m6803->d);
}

/* $ce LDX immediate -**0- */
static void ldx_im(m6803_Regs *m6803)
{
  IMMWORD(m6803->x);
  CLR_NZV;
  SET_NZ16(X);
}

/* $cf STX immediate -**0- */
static void stx_im(m6803_Regs *m6803)
{
  CLR_NZV;
  SET_NZ16(X);
  IMM16;
  WM16(EAD,m6803->x);
}

/* $d0 SUBB direct ?**** */
static void subb_di(m6803_Regs *m6803)
{
  UInt16	  t,r;
  DIRBYTE(t); r = B-t;
  CLR_NZVC; SET_FLAGS8(B,t,r);
  B = r;
}

/* $d1 CMPB direct ?**** */
static void cmpb_di(m6803_Regs *m6803)
{
  UInt16	  t,r;
  DIRBYTE(t); r = B-t;
  CLR_NZVC; SET_FLAGS8(B,t,r);
}

/* $d2 SBCB direct ?**** */
static void sbcb_di(m6803_Regs *m6803)
{
  UInt16	  t,r;
  DIRBYTE(t); r = B-t-(CC&0x01);
  CLR_NZVC; SET_FLAGS8(B,t,r);
  B = r;
}

/* $d3 ADDD direct -**** */
static void addd_di(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  DIRWORD(b);
  d = D;
  r = d + b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  D = r;
}

/* $d4 ANDB direct -**0- */
static void andb_di(m6803_Regs *m6803)
{
  UInt8 t;
  DIRBYTE(t); B &= t;
  CLR_NZV; SET_NZ8(B);
}

/* $d5 BITB direct -**0- */
static void bitb_di(m6803_Regs *m6803)
{
  UInt8 t,r;
  DIRBYTE(t); r = B&t;
  CLR_NZV; SET_NZ8(r);
}

/* $d6 LDB direct -**0- */
static void ldb_di(m6803_Regs *m6803)
{
  DIRBYTE(B);
  CLR_NZV; SET_NZ8(B);
}

/* $d7 STB direct -**0- */
static void stb_di(m6803_Regs *m6803)
{
  CLR_NZV; SET_NZ8(B);
  DIRECT; WM(EAD,B);
}

/* $d8 EORB direct -**0- */
static void eorb_di(m6803_Regs *m6803)
{
  UInt8 t;
  DIRBYTE(t); B ^= t;
  CLR_NZV; SET_NZ8(B);
}

/* $d9 ADCB direct ***** */
static void adcb_di(m6803_Regs *m6803)
{
  UInt16 t,r;
  DIRBYTE(t); r = B+t+(CC&0x01);
  CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
  B = r;
}

/* $da ORB direct -**0- */
static void orb_di(m6803_Regs *m6803)
{
  UInt8 t;
  DIRBYTE(t); B |= t;
  CLR_NZV; SET_NZ8(B);
}

/* $db ADDB direct ***** */
static void addb_di(m6803_Regs *m6803)
{
  UInt16 t,r;
  DIRBYTE(t); r = B+t;
  CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
  B = r;
}

/* $dc LDD direct -**0- */
static void ldd_di(m6803_Regs *m6803)
{
  DIRWORD(m6803->d);
  CLR_NZV;
  SET_NZ16(D);
}

/* $dd STD direct -**0- */
static void std_di(m6803_Regs *m6803)
{
  DIRECT;
  CLR_NZV;
  SET_NZ16(D);
  WM16(EAD,m6803->d);
}

/* $de LDX direct -**0- */
static void ldx_di(m6803_Regs *m6803)
{
  DIRWORD(m6803->x);
  CLR_NZV;
  SET_NZ16(X);
}

/* $dF STX direct -**0- */
static void stx_di(m6803_Regs *m6803)
{
  CLR_NZV;
  SET_NZ16(X);
  DIRECT;
  WM16(EAD,m6803->x);
}

/* $e0 SUBB indexed ?**** */
static void subb_ix(m6803_Regs *m6803)
{
  UInt16	  t,r;
  IDXBYTE(t); r = B-t;
  CLR_NZVC; SET_FLAGS8(B,t,r);
  B = r;
}

/* $e1 CMPB indexed ?**** */
static void cmpb_ix(m6803_Regs *m6803)
{
  UInt16	  t,r;
  IDXBYTE(t); r = B-t;
  CLR_NZVC; SET_FLAGS8(B,t,r);
}

/* $e2 SBCB indexed ?**** */
static void sbcb_ix(m6803_Regs *m6803)
{
  UInt16	  t,r;
  IDXBYTE(t); r = B-t-(CC&0x01);
  CLR_NZVC; SET_FLAGS8(B,t,r);
  B = r;
}

/* $e3 ADDD indexed -**** */
static void addd_ix(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  IDXWORD(b);
  d = D;
  r = d + b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  D = r;
}

/* $e4 ANDB indexed -**0- */
static void andb_ix(m6803_Regs *m6803)
{
  UInt8 t;
  IDXBYTE(t); B &= t;
  CLR_NZV; SET_NZ8(B);
}

/* $e5 BITB indexed -**0- */
static void bitb_ix(m6803_Regs *m6803)
{
  UInt8 t,r;
  IDXBYTE(t); r = B&t;
  CLR_NZV; SET_NZ8(r);
}

/* $e6 LDB indexed -**0- */
static void ldb_ix(m6803_Regs *m6803)
{
  IDXBYTE(B);
  CLR_NZV; SET_NZ8(B);
}

/* $e7 STB indexed -**0- */
static void stb_ix(m6803_Regs *m6803)
{
  CLR_NZV; SET_NZ8(B);
  INDEXED; WM(EAD,B);
}

/* $e8 EORB indexed -**0- */
static void eorb_ix(m6803_Regs *m6803)
{
  UInt8 t;
  IDXBYTE(t); B ^= t;
  CLR_NZV; SET_NZ8(B);
}

/* $e9 ADCB indexed ***** */
static void adcb_ix(m6803_Regs *m6803)
{
  UInt16 t,r;
  IDXBYTE(t); r = B+t+(CC&0x01);
  CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
  B = r;
}

/* $ea ORB indexed -**0- */
static void orb_ix(m6803_Regs *m6803)
{
  UInt8 t;
  IDXBYTE(t); B |= t;
  CLR_NZV; SET_NZ8(B);
}

/* $eb ADDB indexed ***** */
static void addb_ix(m6803_Regs *m6803)
{
  UInt16 t,r;
  IDXBYTE(t); r = B+t;
  CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
  B = r;
}

/* $ec LDD indexed -**0- */
static void ldd_ix(m6803_Regs *m6803)
{
  IDXWORD(m6803->d);
  CLR_NZV;
  SET_NZ16(D);
}

/* $ec ADCX immediate -****    NSC8105 only.  Flags are a guess - copied from addb_im() */
static void adcx_im(m6803_Regs *m6803)
{
  UInt16 t,r;
  IMMBYTE(t); r = X+t+(CC&0x01);
  CLR_HNZVC; SET_FLAGS8(X,t,r); SET_H(X,t,r);
  X = r;
}

/* $ed STD indexed -**0- */
static void std_ix(m6803_Regs *m6803)
{
  INDEXED;
  CLR_NZV;
  SET_NZ16(D);
  WM16(EAD,m6803->d);
}

/* $ee LDX indexed -**0- */
static void ldx_ix(m6803_Regs *m6803)
{
  IDXWORD(m6803->x);
  CLR_NZV;
  SET_NZ16(X);
}

/* $ef STX indexed -**0- */
static void stx_ix(m6803_Regs *m6803)
{
  CLR_NZV;
  SET_NZ16(X);
  INDEXED;
  WM16(EAD,m6803->x);
}

/* $f0 SUBB extended ?**** */
static void subb_ex(m6803_Regs *m6803)
{
  UInt16	  t,r;
  EXTBYTE(t); r = B-t;
  CLR_NZVC; SET_FLAGS8(B,t,r);
  B = r;
}

/* $f1 CMPB extended ?**** */
static void cmpb_ex(m6803_Regs *m6803)
{
  UInt16	  t,r;
  EXTBYTE(t); r = B-t;
  CLR_NZVC; SET_FLAGS8(B,t,r);
}

/* $f2 SBCB extended ?**** */
static void sbcb_ex(m6803_Regs *m6803)
{
  UInt16	  t,r;
  EXTBYTE(t); r = B-t-(CC&0x01);
  CLR_NZVC; SET_FLAGS8(B,t,r);
  B = r;
}

/* $f3 ADDD extended -**** */
static void addd_ex(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  EXTWORD(b);
  d = D;
  r = d + b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  D = r;
}

/* $f4 ANDB extended -**0- */
static void andb_ex(m6803_Regs *m6803)
{
  UInt8 t;
  EXTBYTE(t);
  B &= t;
  CLR_NZV;
  SET_NZ8(B);
}

/* $f5 BITB extended -**0- */
static void bitb_ex(m6803_Regs *m6803)
{
  UInt8 t,r;
  EXTBYTE(t);
  r = B & t;
  CLR_NZV;
  SET_NZ8(r);
}

/* $f6 LDB extended -**0- */
static void ldb_ex(m6803_Regs *m6803)
{
  EXTBYTE(B);
  CLR_NZV;
  SET_NZ8(B);
}

/* $f7 STB extended -**0- */
static void stb_ex(m6803_Regs *m6803)
{
  CLR_NZV; SET_NZ8(B);
  EXTENDED; WM(EAD,B);
}

/* $f8 EORB extended -**0- */
static void eorb_ex(m6803_Regs *m6803)
{
  UInt8 t;
  EXTBYTE(t); B ^= t;
  CLR_NZV; SET_NZ8(B);
}

/* $f9 ADCB extended ***** */
static void adcb_ex(m6803_Regs *m6803)
{
  UInt16 t,r;
  EXTBYTE(t); r = B+t+(CC&0x01);
  CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
  B = r;
}

/* $fa ORB extended -**0- */
static void orb_ex(m6803_Regs *m6803)
{
  UInt8 t;
  EXTBYTE(t); B |= t;
  CLR_NZV; SET_NZ8(B);
}

/* $fb ADDB extended ***** */
static void addb_ex(m6803_Regs *m6803)
{
  UInt16 t,r;
  EXTBYTE(t); r = B+t;
  CLR_HNZVC; SET_FLAGS8(B,t,r); SET_H(B,t,r);
  B = r;
}

/* $fc LDD extended -**0- */
static void ldd_ex(m6803_Regs *m6803)
{
  EXTWORD(m6803->d);
  CLR_NZV;
  SET_NZ16(D);
}

/* $fc ADDX extended -****    NSC8105 only.  Flags are a guess */
static void addx_ex(m6803_Regs *m6803)
{
  UInt32 r,d;
  PAIR b;
  EXTWORD(b);
  d = X;
  r = d + b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  X = r;
}

/* $fd STD extended -**0- */
static void std_ex(m6803_Regs *m6803)
{
  EXTENDED;
  CLR_NZV;
  SET_NZ16(D);
  WM16(EAD,m6803->d);
}

/* $fe LDX extended -**0- */
static void ldx_ex(m6803_Regs *m6803)
{
  EXTWORD(m6803->x);
  CLR_NZV;
  SET_NZ16(X);
}

/* $ff STX extended -**0- */
static void stx_ex(m6803_Regs *m6803)
{
  CLR_NZV;
  SET_NZ16(X);
  EXTENDED;
  WM16(EAD,m6803->x);
}
