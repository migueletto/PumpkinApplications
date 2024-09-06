// HNZVC
// ? = undefined
// * = affected
// - = unaffected
// 0 = cleared
// 1 = set
// # = CCr directly affected by instruction
// @ = special - carry set if bit 7 is set

void illegal(m6809_Regs *m6809, UInt32 cod, UInt8 op)
{
  ILLEGAL(m6809->hardware, cod, op);
}

// case 0x00: //NEG direct ?****
void neg_di(m6809_Regs *m6809)
{
  UInt16 r,t;
  DIRBYTE(t);
  r = -t;
  CLR_NZVC;
  SET_FLAGS8(0,t,r);
  WM(EAD,r);
}

// case 0x01: //ILLEGAL

// case 0x02: //ILLEGAL

// case 0x03: //COM direct -**01
void com_di(m6809_Regs *m6809)
{
  UInt8 t;
  DIRBYTE(t);
  t = ~t;
  CLR_NZV;
  SET_NZ8(t);
  SEC;
  WM(EAD,t);
}

// case 0x04: //LSR direct -0*-*
void lsr_di(m6809_Regs *m6809)
{
  UInt8 t;
  DIRBYTE(t);
  CLR_NZC;
  CC |= (t & CC_C);
  t >>= 1;
  SET_Z8(t);
  WM(EAD,t);
}

// case 0x05: //ILLEGAL

// case 0x06: //ROR direct -**-*
void ror_di(m6809_Regs *m6809)
{
  UInt8 t,r;
  DIRBYTE(t);
  r= (CC & CC_C) << 7;
  CLR_NZC;
  CC |= (t & CC_C);
  r |= t>>1;
  SET_NZ8(r);
  WM(EAD,r);
}

// case 0x07: //ASR direct ?**-*
void asr_di(m6809_Regs *m6809)
{
  UInt8 t;
  DIRBYTE(t);
  CLR_NZC;
  CC |= (t & CC_C);
  t = (t & 0x80) | (t >> 1);
  SET_NZ8(t);
  WM(EAD,t);
}

// case 0x08: //ASL direct ?****
void asl_di(m6809_Regs *m6809)
{
  UInt16 t,r;
  DIRBYTE(t);
  r = t << 1;
  CLR_NZVC;
  SET_FLAGS8(t,t,r);
  WM(EAD,r);
}

// case 0x09: //ROL direct -****
void rol_di(m6809_Regs *m6809)
{
  UInt16 t,r;
  DIRBYTE(t);
  r = (CC & CC_C) | (t << 1);
  CLR_NZVC;
  SET_FLAGS8(t,t,r);
  WM(EAD,r);
}

// case 0x0A: //DEC direct -***-
void dec_di(m6809_Regs *m6809)
{
  UInt8 t;
  DIRBYTE(t);
  --t;
  CLR_NZV;
  SET_FLAGS8D(t);
  WM(EAD,t);
}

// case 0x0B: //ILLEGAL

// case 0xOC: //INC direct -***-
void inc_di(m6809_Regs *m6809)
{
  UInt8 t;
  DIRBYTE(t);
  ++t;
  CLR_NZV;
  SET_FLAGS8I(t);
  WM(EAD,t);
}

// case 0xOD: //TST direct -**0-
void tst_di(m6809_Regs *m6809)
{
  UInt8 t;
  DIRBYTE(t);
  CLR_NZV;
  SET_NZ8(t);
}

// case 0x0E: //JMP direct -----
void jmp_di(m6809_Regs *m6809)
{
  DIRECT;
  PCD = EAD;
}

// case 0x0F: //CLR direct -0100
void clr_di(m6809_Regs *m6809)
{
  DIRECT;
  RM(EAD);
  WM(EAD,0);
  CLR_NZVC;
  SEZ;
}

// case 0x10: //FLAG

// case 0x11: //FLAG

// case 0x12: //NOP inherent -----
void nop(m6809_Regs *m6809)
{
}

// case 0x13: //SYNC inherent -----
void sync(m6809_Regs *m6809)
{
  // SYNC stops processing instructions until an interrupt request happens.
  // This doesn't require the corresponding interrupt to be enabled: if it
  // is disabled, execution continues with the next instruction.
  m6809->int_state |= M6809_SYNC;
  CHECK_IRQ_LINES;
  // if M6809_SYNC has not been cleared by CHECK_IRQ_LINES,
  // stop execution until the interrupt lines change.
  if( m6809->int_state & M6809_SYNC )
    if (m6809_ICount > 0) m6809_ICount = 0;
}

// case 0x14: //ILLEGAL

// case 0x15: //ILLEGAL

// case 0x16: //LBRA relative -----
void lbra(m6809_Regs *m6809)
{
  IMMWORD(mEA);
  PC += EA;

  if ( EA == 0xfffd )  // EHC 980508 speed up busy loop
    if ( m6809_ICount > 0)
      m6809_ICount = 0;
}

// case 0x17: //LBSR relative -----
void lbsr(m6809_Regs *m6809)
{
  IMMWORD(mEA);
  PUSHWORD(pPC);
  PC += EA;
}

// case 0x18: //ILLEGAL

// case 0x19: //DAA inherent (A) -**0*
void daa(m6809_Regs *m6809)
{
  UInt8 msn, lsn;
  UInt16 t, cf = 0;
  msn = A & 0xf0; lsn = A & 0x0f;
  if( lsn>0x09 || CC & CC_H) cf |= 0x06;
  if( msn>0x80 && lsn>0x09 ) cf |= 0x60;
  if( msn>0x90 || CC & CC_C) cf |= 0x60;
  t = cf + A;
  CLR_NZV; // keep carry from previous operation
  SET_NZ8((UInt8)t); SET_C8(t);
  A = t;
}

// case 0x1A: //ORCC immediate #####
void orcc(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  CC |= t;
  CHECK_IRQ_LINES;
}

// case 0x1B: //ILLEGAL

// case 0x1C: //ANDCC immediate #####
void andcc(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  CC &= t;
  CHECK_IRQ_LINES;
}

// case 0x1D: //SEX inherent -**0-
void sex(m6809_Regs *m6809)
{
  UInt16 t;
  t = SIGNED(B);
  D = t;
  //  CLR_NZV;  Tim Lindner 20020905: verified that V flag is not affected
  CLR_NZ;
  SET_NZ16(t);
}

// case 0x1E: //EXG inherent -----
void exg(m6809_Regs *m6809)
{
  UInt16 t1,t2;
  UInt8 tb;

  IMMBYTE(tb);
  if ((tb^(tb>>4)) & 0x08) {
    // transfer $ff to both registers
    t1 = t2 = 0xff;
  } else {
    switch (tb>>4) {
      case  0: t1 = D;  break;
      case  1: t1 = X;  break;
      case  2: t1 = Y;  break;
      case  3: t1 = U;  break;
      case  4: t1 = S;  break;
      case  5: t1 = PC; break;
      case  8: t1 = A;  break;
      case  9: t1 = B;  break;
      case 10: t1 = CC; break;
      case 11: t1 = DP; break;
      default: t1 = 0xff;
    }
    switch (tb&15) {
      case  0: t2 = D;  break;
      case  1: t2 = X;  break;
      case  2: t2 = Y;  break;
      case  3: t2 = U;  break;
      case  4: t2 = S;  break;
      case  5: t2 = PC; break;
      case  8: t2 = A;  break;
      case  9: t2 = B;  break;
      case 10: t2 = CC; break;
      case 11: t2 = DP; break;
      default: t2 = 0xff;
    }
  }
  switch (tb>>4) {
    case  0: D = t2;  break;
    case  1: X = t2;  break;
    case  2: Y = t2;  break;
    case  3: U = t2;  break;
    case  4: S = t2;  break;
    case  5: PC = t2; break;
    case  8: A = t2;  break;
    case  9: B = t2;  break;
    case 10: CC = t2; break;
    case 11: DP = t2; break;
  }
  switch (tb&15) {
    case  0: D = t1;  break;
    case  1: X = t1;  break;
    case  2: Y = t1;  break;
    case  3: U = t1;  break;
    case  4: S = t1;  break;
    case  5: PC = t1; break;
    case  8: A = t1;  break;
    case  9: B = t1;  break;
    case 10: CC = t1; break;
    case 11: DP = t1; break;
  }
}

// case 0x1F: //TFR inherent -----
void tfr(m6809_Regs *m6809)
{
  UInt8 tb;
  UInt16 t;

  IMMBYTE(tb);
  if ((tb^(tb>>4)) & 0x08) {
    // transfer $ff to register
    t = 0xff;
  } else {
    switch (tb>>4) {
      case  0: t = D;  break;
      case  1: t = X;  break;
      case  2: t = Y;  break;
      case  3: t = U;  break;
      case  4: t = S;  break;
      case  5: t = PC; break;
      case  8: t = A;  break;
      case  9: t = B;  break;
      case 10: t = CC; break;
      case 11: t = DP; break;
      default: t = 0xff;
    }
  }
  switch (tb&15) {
    case  0: D = t;  break;
    case  1: X = t;  break;
    case  2: Y = t;  break;
    case  3: U = t;  break;
    case  4: S = t;  break;
    case  5: PC = t; break;
    case  8: A = t;  break;
    case  9: B = t;  break;
    case 10: CC = t; break;
    case 11: DP = t; break;
  }
}

// case 0x20: //BRA relative -----
void bra(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  PC += SIGNED(t);
  // JB 970823 - speed up busy loops
  if( t == 0xfe )
    if( m6809_ICount > 0 ) m6809_ICount = 0;
}

// case 0x21: //BRN relative -----
void brn(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  (void)t;
}

// case 0x22: //BHI relative -----
void bhi(m6809_Regs *m6809)
{
  BRANCH( !(CC & (CC_Z|CC_C)) );
}

// case 0x23: //BLS relative -----
void bls(m6809_Regs *m6809)
{
  BRANCH( (CC & (CC_Z|CC_C)) );
}

// case 0x24: //BCC relative -----
void bcc(m6809_Regs *m6809)
{
  BRANCH( !(CC&CC_C) );
}

// case 0x25: //BCS relative -----
void bcs(m6809_Regs *m6809)
{
  BRANCH( (CC&CC_C) );
}

// case 0x26: //BNE relative -----
void bne(m6809_Regs *m6809)
{
  BRANCH( !(CC&CC_Z) );
}

// case 0x27: //BEQ relative -----
void beq(m6809_Regs *m6809)
{
  BRANCH( (CC&CC_Z) );
}

// case 0x28: //BVC relative -----
void bvc(m6809_Regs *m6809)
{
  BRANCH( !(CC&CC_V) );
}

// case 0x29: //BVS relative -----
void bvs(m6809_Regs *m6809)
{
  BRANCH( (CC&CC_V) );
}

// case 0x2A: //BPL relative -----
void bpl(m6809_Regs *m6809)
{
  BRANCH( !(CC&CC_N) );
}

// case 0x2B: //BMI relative -----
void bmi(m6809_Regs *m6809)
{
  BRANCH( (CC&CC_N) );
}

// case 0x2C: //BGE relative -----
void bge(m6809_Regs *m6809)
{
  BRANCH( !NXORV );
}

// case 0x2D: //BLT relative -----
void blt(m6809_Regs *m6809)
{
  BRANCH( NXORV );
}

// case 0x2E: //BGT relative -----
void bgt(m6809_Regs *m6809)
{
  BRANCH( !(NXORV || (CC&CC_Z)) );
}

// case 0x2F: //BLE relative -----
void ble(m6809_Regs *m6809)
{
  BRANCH( (NXORV || (CC&CC_Z)) );
}

// case 0x30: //LEAX indexed --*--
void leax(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    X = EA;
  CLR_Z;
  SET_Z(X);
}

// case 0x31: //LEAY indexed --*--
void leay(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    Y = EA;
  CLR_Z;
  SET_Z(Y);
}

// case 0x32: //LEAS indexed -----
void leas(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    S = EA;
  m6809->int_state |= M6809_LDS;
}

// case 0x33: //LEAU indexed -----
void leau(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    U = EA;
}

// case 0x34: //PSHS inherent -----
void pshs(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  if( t&0x80 ) { PUSHWORD(pPC); m6809_ICount -= 2; }
  if( t&0x40 ) { PUSHWORD(pU);  m6809_ICount -= 2; }
  if( t&0x20 ) { PUSHWORD(pY);  m6809_ICount -= 2; }
  if( t&0x10 ) { PUSHWORD(pX);  m6809_ICount -= 2; }
  if( t&0x08 ) { PUSHBYTE(DP);  m6809_ICount -= 1; }
  if( t&0x04 ) { PUSHBYTE(B);   m6809_ICount -= 1; }
  if( t&0x02 ) { PUSHBYTE(A);   m6809_ICount -= 1; }
  if( t&0x01 ) { PUSHBYTE(CC);  m6809_ICount -= 1; }
}

// 35 PULS inherent -----
void puls(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  if( t&0x01 ) { PULLBYTE(CC); m6809_ICount -= 1; }
  if( t&0x02 ) { PULLBYTE(A);  m6809_ICount -= 1; }
  if( t&0x04 ) { PULLBYTE(B);  m6809_ICount -= 1; }
  if( t&0x08 ) { PULLBYTE(DP); m6809_ICount -= 1; }
  if( t&0x10 ) { PULLWORD(XD); m6809_ICount -= 2; }
  if( t&0x20 ) { PULLWORD(YD); m6809_ICount -= 2; }
  if( t&0x40 ) { PULLWORD(UD); m6809_ICount -= 2; }
  if( t&0x80 ) { PULLWORD(PCD); m6809_ICount -= 2; }

  if( t&0x01 ) { CHECK_IRQ_LINES; }
}

// case 0x36: //PSHU inherent -----
void pshu(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  if( t&0x80 ) { PSHUWORD(pPC); m6809_ICount -= 2; }
  if( t&0x40 ) { PSHUWORD(pS);  m6809_ICount -= 2; }
  if( t&0x20 ) { PSHUWORD(pY);  m6809_ICount -= 2; }
  if( t&0x10 ) { PSHUWORD(pX);  m6809_ICount -= 2; }
  if( t&0x08 ) { PSHUBYTE(DP);  m6809_ICount -= 1; }
  if( t&0x04 ) { PSHUBYTE(B);   m6809_ICount -= 1; }
  if( t&0x02 ) { PSHUBYTE(A);   m6809_ICount -= 1; }
  if( t&0x01 ) { PSHUBYTE(CC);  m6809_ICount -= 1; }
}

// 37 PULU inherent -----
void pulu(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  if( t&0x01 ) { PULUBYTE(CC); m6809_ICount -= 1; }
  if( t&0x02 ) { PULUBYTE(A);  m6809_ICount -= 1; }
  if( t&0x04 ) { PULUBYTE(B);  m6809_ICount -= 1; }
  if( t&0x08 ) { PULUBYTE(DP); m6809_ICount -= 1; }
  if( t&0x10 ) { PULUWORD(XD); m6809_ICount -= 2; }
  if( t&0x20 ) { PULUWORD(YD); m6809_ICount -= 2; }
  if( t&0x40 ) { PULUWORD(SD); m6809_ICount -= 2; }
  if( t&0x80 ) { PULUWORD(PCD); m6809_ICount -= 2; }

  if( t&0x01 ) { CHECK_IRQ_LINES; }
}

// case 0x38: //ILLEGAL

// case 0x39: //RTS inherent -----
void rts(m6809_Regs *m6809)
{
  PULLWORD(PCD);
}

// case 0x3A: //ABX inherent -----
void abx(m6809_Regs *m6809)
{
  X += B;
}

// case 0x3B: //RTI inherent #####
void rti(m6809_Regs *m6809)
{
  UInt8 t;
  PULLBYTE(CC);
  t = CC & CC_E;
  if(t)
  {
        m6809_ICount -= 9;
    PULLBYTE(A);
    PULLBYTE(B);
    PULLBYTE(DP);
    PULLWORD(XD);
    PULLWORD(YD);
    PULLWORD(UD);
  }
  PULLWORD(PCD);
  CHECK_IRQ_LINES;
}

// case 0x3C: //CWAI inherent ----1
void cwai(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  CC &= t;

  // CWAI stacks the entire machine state on the hardware stack,
  // then waits for an interrupt; when the interrupt is taken
  // later, the state is *not* saved again after CWAI.
    
  CC |= CC_E;
  PUSHWORD(pPC);
  PUSHWORD(pU);
  PUSHWORD(pY);
  PUSHWORD(pX);
  PUSHBYTE(DP);
  PUSHBYTE(B);
  PUSHBYTE(A);
  PUSHBYTE(CC);
  m6809->int_state |= M6809_CWAI;
    CHECK_IRQ_LINES;
  if( m6809->int_state & M6809_CWAI )
    if( m6809_ICount > 0 )
      m6809_ICount = 0;
}

// case 0x3D: //MUL inherent --*-@
void mul(m6809_Regs *m6809)
{
  UInt16 t;
  t = A * B;
  CLR_ZC; SET_Z16(t); if(t&0x80) SEC;
  D = t;
}

// case 0x3E: //ILLEGAL

// case 0x3F: //SWI (SWI2 SWI3) absolute indirect -----
void swi(m6809_Regs *m6809)
{
  CC |= CC_E;
  PUSHWORD(pPC);
  PUSHWORD(pU);
  PUSHWORD(pY);
  PUSHWORD(pX);
  PUSHBYTE(DP);
  PUSHBYTE(B);
  PUSHBYTE(A);
  PUSHBYTE(CC);
  CC |= CC_IF | CC_II;  // inhibit FIRQ and IRQ
  PCD=RM16(0xfffa);
}

// $103F SWI2 absolute indirect -----
void swi2(m6809_Regs *m6809)
{
  CC |= CC_E;
  PUSHWORD(pPC);
  PUSHWORD(pU);
  PUSHWORD(pY);
  PUSHWORD(pX);
  PUSHBYTE(DP);
  PUSHBYTE(B);
  PUSHBYTE(A);
    PUSHBYTE(CC);
  PCD = RM16(0xfff4);
}

// $113F SWI3 absolute indirect -----
void swi3(m6809_Regs *m6809)
{
  CC |= CC_E;
  PUSHWORD(pPC);
  PUSHWORD(pU);
  PUSHWORD(pY);
  PUSHWORD(pX);
  PUSHBYTE(DP);
  PUSHBYTE(B);
  PUSHBYTE(A);
    PUSHBYTE(CC);
  PCD = RM16(0xfff2);
}

// case 0x40: //NEGA inherent ?****
void nega(m6809_Regs *m6809)
{
  UInt16 r;
  r = -A;
  CLR_NZVC;
  SET_FLAGS8(0,A,r);
  A = r;
}

// case 0x41: //ILLEGAL

// case 0x42: //ILLEGAL

// case 0x43: //COMA inherent -**01
void coma(m6809_Regs *m6809)
{
  A = ~A;
  CLR_NZV;
  SET_NZ8(A);
  SEC;
}

// case 0x44: //LSRA inherent -0*-*
void lsra(m6809_Regs *m6809)
{
  CLR_NZC;
  CC |= (A & CC_C);
  A >>= 1;
  SET_Z8(A);
}

// case 0x45: //ILLEGAL

// case 0x46: //RORA inherent -**-*
void rora(m6809_Regs *m6809)
{
  UInt8 r;
  r = (CC & CC_C) << 7;
  CLR_NZC;
  CC |= (A & CC_C);
  r |= A >> 1;
  SET_NZ8(r);
  A = r;
}

// case 0x47: //ASRA inherent ?**-*
void asra(m6809_Regs *m6809)
{
  CLR_NZC;
  CC |= (A & CC_C);
  A = (A & 0x80) | (A >> 1);
  SET_NZ8(A);
}

// case 0x48: //ASLA inherent ?****
void asla(m6809_Regs *m6809)
{
  UInt16 r;
  r = A << 1;
  CLR_NZVC;
  SET_FLAGS8(A,A,r);
  A = r;
}

// case 0x49: //ROLA inherent -****
void rola(m6809_Regs *m6809)
{
  UInt16 t,r;
  t = A;
  r = (CC & CC_C) | (t<<1);
  CLR_NZVC; SET_FLAGS8(t,t,r);
  A = r;
}

// case 0x4A: //DECA inherent -***-
void deca(m6809_Regs *m6809)
{
  --A;
  CLR_NZV;
  SET_FLAGS8D(A);
}

// case 0x4B: //ILLEGAL

// case 0x4C: //INCA inherent -***-
void inca(m6809_Regs *m6809)
{
  ++A;
  CLR_NZV;
  SET_FLAGS8I(A);
}

// case 0x4D: //TSTA inherent -**0-
void tsta(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ8(A);
}

// case 0x4E: //ILLEGAL

// case 0x4F: //CLRA inherent -0100
void clra(m6809_Regs *m6809)
{
  A = 0;
  CLR_NZVC; SEZ;
}

// case 0x50: //NEGB inherent ?****
void negb(m6809_Regs *m6809)
{
  UInt16 r;
  r = -B;
  CLR_NZVC;
  SET_FLAGS8(0,B,r);
  B = r;
}

// case 0x51: //ILLEGAL

// case 0x52: //ILLEGAL

// case 0x53: //COMB inherent -**01
void comb(m6809_Regs *m6809)
{
  B = ~B;
  CLR_NZV;
  SET_NZ8(B);
  SEC;
}

// case 0x54: //LSRB inherent -0*-*
void lsrb(m6809_Regs *m6809)
{
  CLR_NZC;
  CC |= (B & CC_C);
  B >>= 1;
  SET_Z8(B);
}

// case 0x55: //ILLEGAL

// case 0x56: //RORB inherent -**-*
void rorb(m6809_Regs *m6809)
{
  UInt8 r;
  r = (CC & CC_C) << 7;
  CLR_NZC;
  CC |= (B & CC_C);
  r |= B >> 1;
  SET_NZ8(r);
  B = r;
}

// case 0x57: //ASRB inherent ?**-*
void asrb(m6809_Regs *m6809)
{
  CLR_NZC;
  CC |= (B & CC_C);
  B= (B & 0x80) | (B >> 1);
  SET_NZ8(B);
}

// case 0x58: //ASLB inherent ?****
void aslb(m6809_Regs *m6809)
{
  UInt16 r;
  r = B << 1;
  CLR_NZVC;
  SET_FLAGS8(B,B,r);
  B = r;
}

// case 0x59: //ROLB inherent -****
void rolb(m6809_Regs *m6809)
{
  UInt16 t,r;
  t = B;
  r = CC & CC_C;
  r |= t << 1;
  CLR_NZVC;
  SET_FLAGS8(t,t,r);
  B = r;
}

// case 0x5A: //DECB inherent -***-
void decb(m6809_Regs *m6809)
{
  --B;
  CLR_NZV;
  SET_FLAGS8D(B);
}

// case 0x5B: //ILLEGAL

// case 0x5C: //INCB inherent -***-
void incb(m6809_Regs *m6809)
{
  ++B;
  CLR_NZV;
  SET_FLAGS8I(B);
}

// case 0x5D: //TSTB inherent -**0-
void tstb(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ8(B);
}

// case 0x5E: //ILLEGAL

// case 0x5F: //CLRB inherent -0100
void clrb(m6809_Regs *m6809)
{
  B = 0;
  CLR_NZVC; SEZ;
}

// case 0x60: //NEG indexed ?****
void neg_ix(m6809_Regs *m6809)
{
  UInt16 r,t;
  fetch_effective_address(m6809);
  t = RM(EAD);
  r=-t;
  CLR_NZVC;
  SET_FLAGS8(0,t,r);
  WM(EAD,r);
}

// case 0x61: //ILLEGAL

// case 0x62: //ILLEGAL

// case 0x63: //COM indexed -**01
void com_ix(m6809_Regs *m6809)
{
  UInt8 t;
  fetch_effective_address(m6809);
  t = ~RM(EAD);
  CLR_NZV;
  SET_NZ8(t);
  SEC;
  WM(EAD,t);
}

// case 0x64: //LSR indexed -0*-*
void lsr_ix(m6809_Regs *m6809)
{
  UInt8 t;
  fetch_effective_address(m6809);
  t=RM(EAD);
  CLR_NZC;
  CC |= (t & CC_C);
  t>>=1; SET_Z8(t);
  WM(EAD,t);
}

// case 0x65: //ILLEGAL

// case 0x66: //ROR indexed -**-*
void ror_ix(m6809_Regs *m6809)
{
  UInt8 t,r;
  fetch_effective_address(m6809);
  t=RM(EAD);
  r = (CC & CC_C) << 7;
  CLR_NZC;
  CC |= (t & CC_C);
  r |= t>>1; SET_NZ8(r);
  WM(EAD,r);
}

// case 0x67: //ASR indexed ?**-*
void asr_ix(m6809_Regs *m6809)
{
  UInt8 t;
  fetch_effective_address(m6809);
  t=RM(EAD);
  CLR_NZC;
  CC |= (t & CC_C);
  t=(t&0x80)|(t>>1);
  SET_NZ8(t);
  WM(EAD,t);
}

// case 0x68: //ASL indexed ?****
void asl_ix(m6809_Regs *m6809)
{
  UInt16 t,r;
  fetch_effective_address(m6809);
  t=RM(EAD);
  r = t << 1;
  CLR_NZVC;
  SET_FLAGS8(t,t,r);
  WM(EAD,r);
}

// case 0x69: //ROL indexed -****
void rol_ix(m6809_Regs *m6809)
{
  UInt16 t,r;
  fetch_effective_address(m6809);
  t=RM(EAD);
  r = CC & CC_C;
  r |= t << 1;
  CLR_NZVC;
  SET_FLAGS8(t,t,r);
  WM(EAD,r);
}

// case 0x6A: //DEC indexed -***-
void dec_ix(m6809_Regs *m6809)
{
  UInt8 t;
  fetch_effective_address(m6809);
  t = RM(EAD) - 1;
  CLR_NZV; SET_FLAGS8D(t);
  WM(EAD,t);
}

// case 0x6B: //ILLEGAL

// case 0x6C: //INC indexed -***-
void inc_ix(m6809_Regs *m6809)
{
  UInt8 t;
  fetch_effective_address(m6809);
  t = RM(EAD) + 1;
  CLR_NZV; SET_FLAGS8I(t);
  WM(EAD,t);
}

// case 0x6D: //TST indexed -**0-
void tst_ix(m6809_Regs *m6809)
{
  UInt8 t;
  fetch_effective_address(m6809);
  t = RM(EAD);
  CLR_NZV;
  SET_NZ8(t);
}

// case 0x6E: //JMP indexed -----
void jmp_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
  PCD = EAD;
}

// case 0x6F: //CLR indexed -0100
void clr_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
  RM(EAD);
  WM(EAD,0);
  CLR_NZVC; SEZ;
}

// case 0x70: //NEG extended ?****
void neg_ex(m6809_Regs *m6809)
{
  UInt16 r,t;
  EXTBYTE(t); r=-t;
  CLR_NZVC; SET_FLAGS8(0,t,r);
  WM(EAD,r);
}

// case 0x71: //ILLEGAL

// case 0x72: //ILLEGAL

// case 0x73: //COM extended -**01
void com_ex(m6809_Regs *m6809)
{
  UInt8 t;
  EXTBYTE(t); t = ~t;
  CLR_NZV; SET_NZ8(t); SEC;
  WM(EAD,t);
}

// case 0x74: //LSR extended -0*-*
void lsr_ex(m6809_Regs *m6809)
{
  UInt8 t;
  EXTBYTE(t); CLR_NZC; CC |= (t & CC_C);
  t>>=1; SET_Z8(t);
  WM(EAD,t);
}

// case 0x75: //ILLEGAL

// case 0x76: //ROR extended -**-*
void ror_ex(m6809_Regs *m6809)
{
  UInt8 t,r;
  EXTBYTE(t); r=(CC & CC_C) << 7;
  CLR_NZC; CC |= (t & CC_C);
  r |= t>>1; SET_NZ8(r);
  WM(EAD,r);
}

// case 0x77: //ASR extended ?**-*
void asr_ex(m6809_Regs *m6809)
{
  UInt8 t;
  EXTBYTE(t); CLR_NZC; CC |= (t & CC_C);
  t=(t&0x80)|(t>>1);
  SET_NZ8(t);
  WM(EAD,t);
}

// case 0x78: //ASL extended ?****
void asl_ex(m6809_Regs *m6809)
{
  UInt16 t,r;
  EXTBYTE(t); r=t<<1;
  CLR_NZVC; SET_FLAGS8(t,t,r);
  WM(EAD,r);
}

// case 0x79: //ROL extended -****
void rol_ex(m6809_Regs *m6809)
{
  UInt16 t,r;
  EXTBYTE(t); r = (CC & CC_C) | (t << 1);
  CLR_NZVC; SET_FLAGS8(t,t,r);
  WM(EAD,r);
}

// case 0x7A: //DEC extended -***-
void dec_ex(m6809_Regs *m6809)
{
  UInt8 t;
  EXTBYTE(t); --t;
  CLR_NZV; SET_FLAGS8D(t);
  WM(EAD,t);
}

// case 0x7B: //ILLEGAL

// case 0x7C: //INC extended -***-
void inc_ex(m6809_Regs *m6809)
{
  UInt8 t;
  EXTBYTE(t); ++t;
  CLR_NZV; SET_FLAGS8I(t);
  WM(EAD,t);
}

// case 0x7D: //TST extended -**0-
void tst_ex(m6809_Regs *m6809)
{
  UInt8 t;
  EXTBYTE(t); CLR_NZV; SET_NZ8(t);
}

// case 0x7E: //JMP extended -----
void jmp_ex(m6809_Regs *m6809)
{
  EXTENDED;
  PCD = EAD;
}

// case 0x7F: //CLR extended -0100
void clr_ex(m6809_Regs *m6809)
{
  EXTENDED;
  RM(EAD);
  WM(EAD,0);
  CLR_NZVC; SEZ;
}

// case 0x80: //SUBA immediate ?****
void suba_im(m6809_Regs *m6809)
{
  UInt16 t,r;
  IMMBYTE(t);
  r = A - t;
  CLR_NZVC;
  SET_FLAGS8(A,t,r);
  A = r;
}

// case 0x81: //CMPA immediate ?****
void cmpa_im(m6809_Regs *m6809)
{
  UInt16    t,r;
  IMMBYTE(t);
  r = A - t;
  CLR_NZVC;
  SET_FLAGS8(A,t,r);
}

// case 0x82: //SBCA immediate ?****
void sbca_im(m6809_Regs *m6809)
{
  UInt16    t,r;
  IMMBYTE(t);
  r = A - t - (CC & CC_C);
  CLR_NZVC;
  SET_FLAGS8(A,t,r);
  A = r;
}

// case 0x83: //SUBD (CMPD CMPU) immediate -****
void subd_im(m6809_Regs *m6809)
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

// $1083 CMPD immediate -****
void cmpd_im(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  IMMWORD(b);
  d = D;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// $1183 CMPU immediate -****
void cmpu_im(m6809_Regs *m6809)
{
  UInt32 r, d;
  PAIR b;
  IMMWORD(b);
  d = U;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// case 0x84: //ANDA immediate -**0-
void anda_im(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  A &= t;
  CLR_NZV;
  SET_NZ8(A);
}

// case 0x85: //BITA immediate -**0-
void bita_im(m6809_Regs *m6809)
{
  UInt8 t,r;
  IMMBYTE(t);
  r = A & t;
  CLR_NZV;
  SET_NZ8(r);
}

// case 0x86: //LDA immediate -**0-
void lda_im(m6809_Regs *m6809)
{
  IMMBYTE(A);
  CLR_NZV;
  SET_NZ8(A);
}

// is this a legal instruction?
// case 0x87: //STA immediate -**0-
void sta_im(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ8(A);
  IMM8;
  WM(EAD,A);
}

// case 0x88: //EORA immediate -**0-
void eora_im(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  A ^= t;
  CLR_NZV;
  SET_NZ8(A);
}

// case 0x89: //ADCA immediate *****
void adca_im(m6809_Regs *m6809)
{
  UInt16 t,r;
  IMMBYTE(t);
  r = A + t + (CC & CC_C);
  CLR_HNZVC;
  SET_FLAGS8(A,t,r);
  SET_H(A,t,r);
  A = r;
}

// case 0x8A: //ORA immediate -**0-
void ora_im(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  A |= t;
  CLR_NZV;
  SET_NZ8(A);
}

// case 0x8B: //ADDA immediate *****
void adda_im(m6809_Regs *m6809)
{
  UInt16 t,r;
  IMMBYTE(t);
  r = A + t;
  CLR_HNZVC;
  SET_FLAGS8(A,t,r);
  SET_H(A,t,r);
  A = r;
}

// case 0x8C: //CMPX (CMPY CMPS) immediate -****
void cmpx_im(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  IMMWORD(b);
  d = X;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// $108C CMPY immediate -****
void cmpy_im(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  IMMWORD(b);
  d = Y;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// $118C CMPS immediate -****
void cmps_im(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  IMMWORD(b);
  d = S;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// case 0x8D: //BSR -----
void bsr(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  PUSHWORD(pPC);
  PC += SIGNED(t);
}

// case 0x8E: //LDX (LDY) immediate -**0-
void ldx_im(m6809_Regs *m6809)
{
  IMMWORD(pX);
  CLR_NZV;
  SET_NZ16(X);
}

// $108E LDY immediate -**0-
void ldy_im(m6809_Regs *m6809)
{
  IMMWORD(pY);
  CLR_NZV;
  SET_NZ16(Y);
}

// is this a legal instruction?
// case 0x8F: //STX (STY) immediate -**0-
void stx_im(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(X);
  IMM16;
  WM16(EAD,pX);
}

// is this a legal instruction?
// $108F STY immediate -**0-
void sty_im(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(Y);
  IMM16;
  WM16(EAD,pY);
}

// case 0x90: //SUBA direct ?****
void suba_di(m6809_Regs *m6809)
{
  UInt16    t,r;
  DIRBYTE(t);
  r = A - t;
  CLR_NZVC;
  SET_FLAGS8(A,t,r);
  A = r;
}

// case 0x91: //CMPA direct ?****
void cmpa_di(m6809_Regs *m6809)
{
  UInt16    t,r;
  DIRBYTE(t);
  r = A - t;
  CLR_NZVC;
  SET_FLAGS8(A,t,r);
}

// case 0x92: //SBCA direct ?****
void sbca_di(m6809_Regs *m6809)
{
  UInt16    t,r;
  DIRBYTE(t);
  r = A - t - (CC & CC_C);
  CLR_NZVC;
  SET_FLAGS8(A,t,r);
  A = r;
}

// case 0x93: //SUBD (CMPD CMPU) direct -****
void subd_di(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  DIRWORD(b);
  d = D;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  D = r;
}

// $1093 CMPD direct -****
void cmpd_di(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  DIRWORD(b);
  d = D;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// $1193 CMPU direct -****
void cmpu_di(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  DIRWORD(b);
  d = U;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(U,b.d,r);
}

// case 0x94: //ANDA direct -**0-
void anda_di(m6809_Regs *m6809)
{
  UInt8 t;
  DIRBYTE(t);
  A &= t;
  CLR_NZV;
  SET_NZ8(A);
}

// case 0x95: //BITA direct -**0-
void bita_di(m6809_Regs *m6809)
{
  UInt8 t,r;
  DIRBYTE(t);
  r = A & t;
  CLR_NZV;
  SET_NZ8(r);
}

// case 0x96: //LDA direct -**0-
void lda_di(m6809_Regs *m6809)
{
  DIRBYTE(A);
  CLR_NZV;
  SET_NZ8(A);
}

// case 0x97: //STA direct -**0-
void sta_di(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ8(A);
  DIRECT;
  WM(EAD,A);
}

// case 0x98: //EORA direct -**0-
void eora_di(m6809_Regs *m6809)
{
  UInt8 t;
  DIRBYTE(t);
  A ^= t;
  CLR_NZV;
  SET_NZ8(A);
}

// case 0x99: //ADCA direct *****
void adca_di(m6809_Regs *m6809)
{
  UInt16 t,r;
  DIRBYTE(t);
  r = A + t + (CC & CC_C);
  CLR_HNZVC;
  SET_FLAGS8(A,t,r);
  SET_H(A,t,r);
  A = r;
}

// case 0x9A: //ORA direct -**0-
void ora_di(m6809_Regs *m6809)
{
  UInt8 t;
  DIRBYTE(t);
  A |= t;
  CLR_NZV;
  SET_NZ8(A);
}

// case 0x9B: //ADDA direct *****
void adda_di(m6809_Regs *m6809)
{
  UInt16 t,r;
  DIRBYTE(t);
  r = A + t;
  CLR_HNZVC;
  SET_FLAGS8(A,t,r);
  SET_H(A,t,r);
  A = r;
}

// case 0x9C: //CMPX (CMPY CMPS) direct -****
void cmpx_di(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  DIRWORD(b);
  d = X;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// $109C CMPY direct -****
void cmpy_di(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  DIRWORD(b);
  d = Y;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// $119C CMPS direct -****
void cmps_di(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  DIRWORD(b);
  d = S;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// case 0x9D: //JSR direct -----
void jsr_di(m6809_Regs *m6809)
{
  DIRECT;
  PUSHWORD(pPC);
  PCD = EAD;
}

// case 0x9E: //LDX (LDY) direct -**0-
void ldx_di(m6809_Regs *m6809)
{
  DIRWORD(pX);
  CLR_NZV;
  SET_NZ16(X);
}

// $109E LDY direct -**0-
void ldy_di(m6809_Regs *m6809)
{
  DIRWORD(pY);
  CLR_NZV;
  SET_NZ16(Y);
}

// case 0x9F: //STX (STY) direct -**0-
void stx_di(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(X);
  DIRECT;
  WM16(EAD,pX);
}

// $109F STY direct -**0-
void sty_di(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(Y);
  DIRECT;
  WM16(EAD,pY);
}

// case 0xa0: //SUBA indexed ?****
void suba_ix(m6809_Regs *m6809)
{
  UInt16 t,r;
  fetch_effective_address(m6809);
  t = RM(EAD);
  r = A - t;
  CLR_NZVC;
  SET_FLAGS8(A,t,r);
  A = r;
}

// case 0xa1: //CMPA indexed ?****
void cmpa_ix(m6809_Regs *m6809)
{
  UInt16 t,r;
  fetch_effective_address(m6809);
  t = RM(EAD);
  r = A - t;
  CLR_NZVC;
  SET_FLAGS8(A,t,r);
}

// case 0xa2: //SBCA indexed ?****
void sbca_ix(m6809_Regs *m6809)
{
  UInt16    t,r;
  fetch_effective_address(m6809);
  t = RM(EAD);
  r = A - t - (CC & CC_C);
  CLR_NZVC;
  SET_FLAGS8(A,t,r);
  A = r;
}

// case 0xa3: //SUBD (CMPD CMPU) indexed -****
void subd_ix(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  fetch_effective_address(m6809);
    b.d=RM16(EAD);
  d = D;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  D = r;
}

// $10a3 CMPD indexed -****
void cmpd_ix(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  fetch_effective_address(m6809);
    b.d=RM16(EAD);
  d = D;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// $11a3 CMPU indexed -****
void cmpu_ix(m6809_Regs *m6809)
{
  UInt32 r;
  PAIR b;
  fetch_effective_address(m6809);
    b.d=RM16(EAD);
  r = U - b.d;
  CLR_NZVC;
  SET_FLAGS16(U,b.d,r);
}

// case 0xa4: //ANDA indexed -**0-
void anda_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
  A &= RM(EAD);
  CLR_NZV;
  SET_NZ8(A);
}

// case 0xa5: //BITA indexed -**0-
void bita_ix(m6809_Regs *m6809)
{
  UInt8 r;
  fetch_effective_address(m6809);
  r = A & RM(EAD);
  CLR_NZV;
  SET_NZ8(r);
}

// case 0xa6: //LDA indexed -**0-
void lda_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
  A = RM(EAD);
  CLR_NZV;
  SET_NZ8(A);
}

// case 0xa7: //STA indexed -**0-
void sta_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    CLR_NZV;
  SET_NZ8(A);
  WM(EAD,A);
}

// case 0xa8: //EORA indexed -**0-
void eora_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
  A ^= RM(EAD);
  CLR_NZV;
  SET_NZ8(A);
}

// case 0xa9: //ADCA indexed *****
void adca_ix(m6809_Regs *m6809)
{
  UInt16 t,r;
  fetch_effective_address(m6809);
  t = RM(EAD);
  r = A + t + (CC & CC_C);
  CLR_HNZVC;
  SET_FLAGS8(A,t,r);
  SET_H(A,t,r);
  A = r;
}

// case 0xaA: //ORA indexed -**0-
void ora_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
  A |= RM(EAD);
  CLR_NZV;
  SET_NZ8(A);
}

// case 0xaB: //ADDA indexed *****
void adda_ix(m6809_Regs *m6809)
{
  UInt16 t,r;
  fetch_effective_address(m6809);
  t = RM(EAD);
  r = A + t;
  CLR_HNZVC;
  SET_FLAGS8(A,t,r);
  SET_H(A,t,r);
  A = r;
}

// case 0xaC: //CMPX (CMPY CMPS) indexed -****
void cmpx_ix(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  fetch_effective_address(m6809);
    b.d=RM16(EAD);
  d = X;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// $10aC CMPY indexed -****
void cmpy_ix(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  fetch_effective_address(m6809);
    b.d=RM16(EAD);
  d = Y;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// $11aC CMPS indexed -****
void cmps_ix(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  fetch_effective_address(m6809);
    b.d=RM16(EAD);
  d = S;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// case 0xaD: //JSR indexed -----
void jsr_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    PUSHWORD(pPC);
  PCD = EAD;
}

// case 0xaE: //LDX (LDY) indexed -**0-
void ldx_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    X=RM16(EAD);
  CLR_NZV;
  SET_NZ16(X);
}

// $10aE LDY indexed -**0-
void ldy_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    Y=RM16(EAD);
  CLR_NZV;
  SET_NZ16(Y);
}

// case 0xaF: //STX (STY) indexed -**0-
void stx_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    CLR_NZV;
  SET_NZ16(X);
  WM16(EAD,pX);
}

// $10aF STY indexed -**0-
void sty_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    CLR_NZV;
  SET_NZ16(Y);
  WM16(EAD,pY);
}

// case 0xb0: //SUBA extended ?****
void suba_ex(m6809_Regs *m6809)
{
  UInt16    t,r;
  EXTBYTE(t);
  r = A - t;
  CLR_NZVC;
  SET_FLAGS8(A,t,r);
  A = r;
}

// case 0xb1: //CMPA extended ?****
void cmpa_ex(m6809_Regs *m6809)
{
  UInt16    t,r;
  EXTBYTE(t);
  r = A - t;
  CLR_NZVC;
  SET_FLAGS8(A,t,r);
}

// case 0xb2: //SBCA extended ?****
void sbca_ex(m6809_Regs *m6809)
{
  UInt16    t,r;
  EXTBYTE(t);
  r = A - t - (CC & CC_C);
  CLR_NZVC;
  SET_FLAGS8(A,t,r);
  A = r;
}

// case 0xb3: //SUBD (CMPD CMPU) extended -****
void subd_ex(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  EXTWORD(b);
  d = D;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  D = r;
}

// $10b3 CMPD extended -****
void cmpd_ex(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  EXTWORD(b);
  d = D;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// $11b3 CMPU extended -****
void cmpu_ex(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  EXTWORD(b);
  d = U;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// case 0xb4: //ANDA extended -**0-
void anda_ex(m6809_Regs *m6809)
{
  UInt8 t;
  EXTBYTE(t);
  A &= t;
  CLR_NZV;
  SET_NZ8(A);
}

// case 0xb5: //BITA extended -**0-
void bita_ex(m6809_Regs *m6809)
{
  UInt8 t,r;
  EXTBYTE(t);
  r = A & t;
  CLR_NZV; SET_NZ8(r);
}

// case 0xb6: //LDA extended -**0-
void lda_ex(m6809_Regs *m6809)
{
  EXTBYTE(A);
  CLR_NZV;
  SET_NZ8(A);
}

// case 0xb7: //STA extended -**0-
void sta_ex(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ8(A);
  EXTENDED;
  WM(EAD,A);
}

// case 0xb8: //EORA extended -**0-
void eora_ex(m6809_Regs *m6809)
{
  UInt8 t;
  EXTBYTE(t);
  A ^= t;
  CLR_NZV;
  SET_NZ8(A);
}

// case 0xb9: //ADCA extended *****
void adca_ex(m6809_Regs *m6809)
{
  UInt16 t,r;
  EXTBYTE(t);
  r = A + t + (CC & CC_C);
  CLR_HNZVC;
  SET_FLAGS8(A,t,r);
  SET_H(A,t,r);
  A = r;
}

// case 0xbA: //ORA extended -**0-
void ora_ex(m6809_Regs *m6809)
{
  UInt8 t;
  EXTBYTE(t);
  A |= t;
  CLR_NZV;
  SET_NZ8(A);
}

// case 0xbB: //ADDA extended *****
void adda_ex(m6809_Regs *m6809)
{
  UInt16 t,r;
  EXTBYTE(t);
  r = A + t;
  CLR_HNZVC;
  SET_FLAGS8(A,t,r);
  SET_H(A,t,r);
  A = r;
}

// case 0xbC: //CMPX (CMPY CMPS) extended -****
void cmpx_ex(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  EXTWORD(b);
  d = X;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// $10bC CMPY extended -****
void cmpy_ex(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  EXTWORD(b);
  d = Y;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// $11bC CMPS extended -****
void cmps_ex(m6809_Regs *m6809)
{
  UInt32 r,d;
  PAIR b;
  EXTWORD(b);
  d = S;
  r = d - b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
}

// case 0xbD: //JSR extended -----
void jsr_ex(m6809_Regs *m6809)
{
  EXTENDED;
  PUSHWORD(pPC);
  PCD = EAD;
}

// case 0xbE: //LDX (LDY) extended -**0-
void ldx_ex(m6809_Regs *m6809)
{
  EXTWORD(pX);
  CLR_NZV;
  SET_NZ16(X);
}

// $10bE LDY extended -**0-
void ldy_ex(m6809_Regs *m6809)
{
  EXTWORD(pY);
  CLR_NZV;
  SET_NZ16(Y);
}

// case 0xbF: //STX (STY) extended -**0-
void stx_ex(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(X);
  EXTENDED;
  WM16(EAD,pX);
}

// $10bF STY extended -**0-
void sty_ex(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(Y);
  EXTENDED;
  WM16(EAD,pY);
}

// case 0xc0: //SUBB immediate ?****
void subb_im(m6809_Regs *m6809)
{
  UInt16    t,r;
  IMMBYTE(t);
  r = B - t;
  CLR_NZVC;
  SET_FLAGS8(B,t,r);
  B = r;
}

// case 0xc1: //CMPB immediate ?****
void cmpb_im(m6809_Regs *m6809)
{
  UInt16    t,r;
  IMMBYTE(t);
  r = B - t;
  CLR_NZVC; SET_FLAGS8(B,t,r);
}

// case 0xc2: //SBCB immediate ?****
void sbcb_im(m6809_Regs *m6809)
{
  UInt16    t,r;
  IMMBYTE(t);
  r = B - t - (CC & CC_C);
  CLR_NZVC;
  SET_FLAGS8(B,t,r);
  B = r;
}

// case 0xc3: //ADDD immediate -****
void addd_im(m6809_Regs *m6809)
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

// case 0xc4: //ANDB immediate -**0-
void andb_im(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  B &= t;
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xc5: //BITB immediate -**0-
void bitb_im(m6809_Regs *m6809)
{
  UInt8 t,r;
  IMMBYTE(t);
  r = B & t;
  CLR_NZV;
  SET_NZ8(r);
}

// case 0xc6: //LDB immediate -**0-
void ldb_im(m6809_Regs *m6809)
{
  IMMBYTE(B);
  CLR_NZV;
  SET_NZ8(B);
}

// is this a legal instruction?
// case 0xc7: //STB immediate -**0-
void stb_im(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ8(B);
  IMM8;
  WM(EAD,B);
}

// case 0xc8: //EORB immediate -**0-
void eorb_im(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  B ^= t;
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xc9: //ADCB immediate *****
void adcb_im(m6809_Regs *m6809)
{
  UInt16 t,r;
  IMMBYTE(t);
  r = B + t + (CC & CC_C);
  CLR_HNZVC;
  SET_FLAGS8(B,t,r);
  SET_H(B,t,r);
  B = r;
}

// case 0xcA: //ORB immediate -**0-
void orb_im(m6809_Regs *m6809)
{
  UInt8 t;
  IMMBYTE(t);
  B |= t;
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xcB: //ADDB immediate *****
void addb_im(m6809_Regs *m6809)
{
  UInt16 t,r;
  IMMBYTE(t);
  r = B + t;
  CLR_HNZVC;
  SET_FLAGS8(B,t,r);
  SET_H(B,t,r);
  B = r;
}

// case 0xcC: //LDD immediate -**0-
void ldd_im(m6809_Regs *m6809)
{
  IMMWORD(pD);
  CLR_NZV;
  SET_NZ16(D);
}

// is this a legal instruction?
// case 0xcD: //STD immediate -**0-
void std_im(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(D);
    IMM16;
  WM16(EAD,pD);
}

// case 0xcE: //LDU (LDS) immediate -**0-
void ldu_im(m6809_Regs *m6809)
{
  IMMWORD(pU);
  CLR_NZV;
  SET_NZ16(U);
}

// $10cE LDS immediate -**0-
void lds_im(m6809_Regs *m6809)
{
  IMMWORD(pS);
  CLR_NZV;
  SET_NZ16(S);
  m6809->int_state |= M6809_LDS;
}

// is this a legal instruction?
// case 0xcF: //STU (STS) immediate -**0-
void stu_im(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(U);
    IMM16;
  WM16(EAD,pU);
}

// is this a legal instruction?
// $10cF STS immediate -**0-
void sts_im(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(S);
    IMM16;
  WM16(EAD,pS);
}

// case 0xd0: //SUBB direct ?****
void subb_di(m6809_Regs *m6809)
{
  UInt16    t,r;
  DIRBYTE(t);
  r = B - t;
  CLR_NZVC;
  SET_FLAGS8(B,t,r);
  B = r;
}

// case 0xd1: //CMPB direct ?****
void cmpb_di(m6809_Regs *m6809)
{
  UInt16    t,r;
  DIRBYTE(t);
  r = B - t;
  CLR_NZVC;
  SET_FLAGS8(B,t,r);
}

// case 0xd2: //SBCB direct ?****
void sbcb_di(m6809_Regs *m6809)
{
  UInt16    t,r;
  DIRBYTE(t);
  r = B - t - (CC & CC_C);
  CLR_NZVC;
  SET_FLAGS8(B,t,r);
  B = r;
}

// case 0xd3: //ADDD direct -****
void addd_di(m6809_Regs *m6809)
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

// case 0xd4: //ANDB direct -**0-
void andb_di(m6809_Regs *m6809)
{
  UInt8 t;
  DIRBYTE(t);
  B &= t;
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xd5: //BITB direct -**0-
void bitb_di(m6809_Regs *m6809)
{
  UInt8 t,r;
  DIRBYTE(t);
  r = B & t;
  CLR_NZV;
  SET_NZ8(r);
}

// case 0xd6: //LDB direct -**0-
void ldb_di(m6809_Regs *m6809)
{
  DIRBYTE(B);
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xd7: //STB direct -**0-
void stb_di(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ8(B);
  DIRECT;
  WM(EAD,B);
}

// case 0xd8: //EORB direct -**0-
void eorb_di(m6809_Regs *m6809)
{
  UInt8 t;
  DIRBYTE(t);
  B ^= t;
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xd9: //ADCB direct *****
void adcb_di(m6809_Regs *m6809)
{
  UInt16 t,r;
  DIRBYTE(t);
  r = B + t + (CC & CC_C);
  CLR_HNZVC;
  SET_FLAGS8(B,t,r);
  SET_H(B,t,r);
  B = r;
}

// case 0xdA: //ORB direct -**0-
void orb_di(m6809_Regs *m6809)
{
  UInt8 t;
  DIRBYTE(t);
  B |= t;
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xdB: //ADDB direct *****
void addb_di(m6809_Regs *m6809)
{
  UInt16 t,r;
  DIRBYTE(t);
  r = B + t;
  CLR_HNZVC;
  SET_FLAGS8(B,t,r);
  SET_H(B,t,r);
  B = r;
}

// case 0xdC: //LDD direct -**0-
void ldd_di(m6809_Regs *m6809)
{
  DIRWORD(pD);
  CLR_NZV;
  SET_NZ16(D);
}

// case 0xdD: //STD direct -**0-
void std_di(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(D);
    DIRECT;
  WM16(EAD,pD);
}

// case 0xdE: //LDU (LDS) direct -**0-
void ldu_di(m6809_Regs *m6809)
{
  DIRWORD(pU);
  CLR_NZV;
  SET_NZ16(U);
}

// $10dE LDS direct -**0-
void lds_di(m6809_Regs *m6809)
{
  DIRWORD(pS);
  CLR_NZV;
  SET_NZ16(S);
  m6809->int_state |= M6809_LDS;
}

// case 0xdF: //STU (STS) direct -**0-
void stu_di(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(U);
  DIRECT;
  WM16(EAD,pU);
}

// $10dF STS direct -**0-
void sts_di(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(S);
  DIRECT;
  WM16(EAD,pS);
}

// case 0xe0: //SUBB indexed ?****
void subb_ix(m6809_Regs *m6809)
{
  UInt16    t,r;
  fetch_effective_address(m6809);
  t = RM(EAD);
  r = B - t;
  CLR_NZVC;
  SET_FLAGS8(B,t,r);
  B = r;
}

// case 0xe1: //CMPB indexed ?****
void cmpb_ix(m6809_Regs *m6809)
{
  UInt16    t,r;
  fetch_effective_address(m6809);
  t = RM(EAD);
  r = B - t;
  CLR_NZVC;
  SET_FLAGS8(B,t,r);
}

// case 0xe2: //SBCB indexed ?****
void sbcb_ix(m6809_Regs *m6809)
{
  UInt16    t,r;
  fetch_effective_address(m6809);
  t = RM(EAD);
  r = B - t - (CC & CC_C);
  CLR_NZVC;
  SET_FLAGS8(B,t,r);
  B = r;
}

// case 0xe3: //ADDD indexed -****
void addd_ix(m6809_Regs *m6809)
{
  UInt32 r,d;
    PAIR b;
    fetch_effective_address(m6809);
  b.d=RM16(EAD);
  d = D;
  r = d + b.d;
  CLR_NZVC;
  SET_FLAGS16(d,b.d,r);
  D = r;
}

// case 0xe4: //ANDB indexed -**0-
void andb_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
  B &= RM(EAD);
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xe5: //BITB indexed -**0-
void bitb_ix(m6809_Regs *m6809)
{
  UInt8 r;
  fetch_effective_address(m6809);
  r = B & RM(EAD);
  CLR_NZV;
  SET_NZ8(r);
}

// case 0xe6: //LDB indexed -**0-
void ldb_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
  B = RM(EAD);
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xe7: //STB indexed -**0-
void stb_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    CLR_NZV;
  SET_NZ8(B);
  WM(EAD,B);
}

// case 0xe8: //EORB indexed -**0-
void eorb_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
  B ^= RM(EAD);
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xe9: //ADCB indexed *****
void adcb_ix(m6809_Regs *m6809)
{
  UInt16 t,r;
  fetch_effective_address(m6809);
  t = RM(EAD);
  r = B + t + (CC & CC_C);
  CLR_HNZVC;
  SET_FLAGS8(B,t,r);
  SET_H(B,t,r);
  B = r;
}

// case 0xeA: //ORB indexed -**0-
void orb_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
  B |= RM(EAD);
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xeB: //ADDB indexed *****
void addb_ix(m6809_Regs *m6809)
{
  UInt16 t,r;
  fetch_effective_address(m6809);
  t = RM(EAD);
  r = B + t;
  CLR_HNZVC;
  SET_FLAGS8(B,t,r);
  SET_H(B,t,r);
  B = r;
}

// case 0xeC: //LDD indexed -**0-
void ldd_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    D=RM16(EAD);
  CLR_NZV; SET_NZ16(D);
}

// case 0xeD: //STD indexed -**0-
void std_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    CLR_NZV;
  SET_NZ16(D);
  WM16(EAD,pD);
}

// case 0xeE: //LDU (LDS) indexed -**0-
void ldu_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    U=RM16(EAD);
  CLR_NZV;
  SET_NZ16(U);
}

// $10eE LDS indexed -**0-
void lds_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    S=RM16(EAD);
  CLR_NZV;
  SET_NZ16(S);
  m6809->int_state |= M6809_LDS;
}

// case 0xeF: //STU (STS) indexed -**0-
void stu_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    CLR_NZV;
  SET_NZ16(U);
  WM16(EAD,pU);
}

// $10eF STS indexed -**0-
void sts_ix(m6809_Regs *m6809)
{
  fetch_effective_address(m6809);
    CLR_NZV;
  SET_NZ16(S);
  WM16(EAD,pS);
}

// case 0xf0: //SUBB extended ?****
void subb_ex(m6809_Regs *m6809)
{
  UInt16    t,r;
  EXTBYTE(t);
  r = B - t;
  CLR_NZVC;
  SET_FLAGS8(B,t,r);
  B = r;
}

// case 0xf1: //CMPB extended ?****
void cmpb_ex(m6809_Regs *m6809)
{
  UInt16    t,r;
  EXTBYTE(t);
  r = B - t;
  CLR_NZVC;
  SET_FLAGS8(B,t,r);
}

// case 0xf2: //SBCB extended ?****
void sbcb_ex(m6809_Regs *m6809)
{
  UInt16    t,r;
  EXTBYTE(t);
  r = B - t - (CC & CC_C);
  CLR_NZVC;
  SET_FLAGS8(B,t,r);
  B = r;
}

// case 0xf3: //ADDD extended -****
void addd_ex(m6809_Regs *m6809)
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

// case 0xf4: //ANDB extended -**0-
void andb_ex(m6809_Regs *m6809)
{
  UInt8 t;
  EXTBYTE(t);
  B &= t;
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xf5: //BITB extended -**0-
void bitb_ex(m6809_Regs *m6809)
{
  UInt8 t,r;
  EXTBYTE(t);
  r = B & t;
  CLR_NZV;
  SET_NZ8(r);
}

// case 0xf6: //LDB extended -**0-
void ldb_ex(m6809_Regs *m6809)
{
  EXTBYTE(B);
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xf7: //STB extended -**0-
void stb_ex(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ8(B);
  EXTENDED;
  WM(EAD,B);
}

// case 0xf8: //EORB extended -**0-
void eorb_ex(m6809_Regs *m6809)
{
  UInt8 t;
  EXTBYTE(t);
  B ^= t;
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xf9: //ADCB extended *****
void adcb_ex(m6809_Regs *m6809)
{
  UInt16 t,r;
  EXTBYTE(t);
  r = B + t + (CC & CC_C);
  CLR_HNZVC;
  SET_FLAGS8(B,t,r);
  SET_H(B,t,r);
  B = r;
}

// case 0xfA: //ORB extended -**0-
void orb_ex(m6809_Regs *m6809)
{
  UInt8 t;
  EXTBYTE(t);
  B |= t;
  CLR_NZV;
  SET_NZ8(B);
}

// case 0xfB: //ADDB extended *****
void addb_ex(m6809_Regs *m6809)
{
  UInt16 t,r;
  EXTBYTE(t);
  r = B + t;
  CLR_HNZVC;
  SET_FLAGS8(B,t,r);
  SET_H(B,t,r);
  B = r;
}

// case 0xfC: //LDD extended -**0-
void ldd_ex(m6809_Regs *m6809)
{
  EXTWORD(pD);
  CLR_NZV;
  SET_NZ16(D);
}

// case 0xfD: //STD extended -**0-
void std_ex(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(D);
    EXTENDED;
  WM16(EAD,pD);
}

// case 0xfE: //LDU (LDS) extended -**0-
void ldu_ex(m6809_Regs *m6809)
{
  EXTWORD(pU);
  CLR_NZV;
  SET_NZ16(U);
}

// $10fE LDS extended -**0-
void lds_ex(m6809_Regs *m6809)
{
  EXTWORD(pS);
  CLR_NZV;
  SET_NZ16(S);
  m6809->int_state |= M6809_LDS;
}

// case 0xfF: //STU (STS) extended -**0-
void stu_ex(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(U);
  EXTENDED;
  WM16(EAD,pU);
}

// $10fF STS extended -**0-
void sts_ex(m6809_Regs *m6809)
{
  CLR_NZV;
  SET_NZ16(S);
  EXTENDED;
  WM16(EAD,pS);
}

// $10xx opcodes
void pref10(m6809_Regs *m6809)
{
  UInt8 ireg2 = ROP(PCD);
  PC++;
  switch( ireg2 ) {
    case 0x21: IMMWORD(mEA);    m6809_ICount-=5;  break;
    case 0x22: LBRANCH( !(CC & (CC_Z|CC_C)) );    m6809_ICount-=5;  break;
    case 0x23: LBRANCH( (CC&(CC_Z|CC_C)) );;    m6809_ICount-=5;  break;
    case 0x24: LBRANCH( !(CC&CC_C) );    m6809_ICount-=5;  break;
    case 0x25: LBRANCH( (CC&CC_C) );    m6809_ICount-=5;  break;
    case 0x26: LBRANCH( !(CC&CC_Z) );    m6809_ICount-=5;  break;
    case 0x27: LBRANCH( (CC&CC_Z) );    m6809_ICount-=5;  break;
    case 0x28: LBRANCH( !(CC&CC_V) );    m6809_ICount-=5;  break;
    case 0x29: LBRANCH( (CC&CC_V) );    m6809_ICount-=5;  break;
    case 0x2a: LBRANCH( !(CC&CC_N) );    m6809_ICount-=5;  break;
    case 0x2b: LBRANCH( (CC&CC_N) );    m6809_ICount-=5;  break;
    case 0x2c: LBRANCH( !NXORV );    m6809_ICount-=5;  break;
    case 0x2d: LBRANCH( NXORV );    m6809_ICount-=5;  break;
    case 0x2e: LBRANCH( !(NXORV || (CC&CC_Z)) );    m6809_ICount-=5;  break;
    case 0x2f: LBRANCH( (NXORV || (CC&CC_Z)) );    m6809_ICount-=5;  break;

    case 0x3f: swi2(m6809);    m6809_ICount-=20;  break;

    case 0x83: cmpd_im(m6809);  m6809_ICount-=5;  break;
    case 0x8c: cmpy_im(m6809);  m6809_ICount-=5;  break;
    case 0x8e: ldy_im(m6809);  m6809_ICount-=4;  break;
    case 0x8f: sty_im(m6809);  m6809_ICount-=4;  break;

    case 0x93: cmpd_di(m6809);  m6809_ICount-=7;  break;
    case 0x9c: cmpy_di(m6809);  m6809_ICount-=7;  break;
    case 0x9e: ldy_di(m6809);  m6809_ICount-=6;  break;
    case 0x9f: sty_di(m6809);  m6809_ICount-=6;  break;

    case 0xa3: cmpd_ix(m6809);  m6809_ICount-=7;  break;
    case 0xac: cmpy_ix(m6809);  m6809_ICount-=7;  break;
    case 0xae: ldy_ix(m6809);  m6809_ICount-=6;  break;
    case 0xaf: sty_ix(m6809);  m6809_ICount-=6;  break;

    case 0xb3: cmpd_ex(m6809);  m6809_ICount-=8;  break;
    case 0xbc: cmpy_ex(m6809);  m6809_ICount-=8;  break;
    case 0xbe: ldy_ex(m6809);  m6809_ICount-=7;  break;
    case 0xbf: sty_ex(m6809);  m6809_ICount-=7;  break;

    case 0xce: lds_im(m6809);  m6809_ICount-=4;  break;
    case 0xcf: sts_im(m6809);  m6809_ICount-=4;  break;

    case 0xde: lds_di(m6809);  m6809_ICount-=6;  break;
    case 0xdf: sts_di(m6809);  m6809_ICount-=6;  break;

    case 0xee: lds_ix(m6809);  m6809_ICount-=6;  break;
    case 0xef: sts_ix(m6809);  m6809_ICount-=6;  break;

    case 0xfe: lds_ex(m6809);  m6809_ICount-=7;  break;
    case 0xff: sts_ex(m6809);  m6809_ICount-=7;  break;

    default:   illegal(m6809, ILLEGAL1, ireg2);            break;
  }
}

// $11xx opcodes
void pref11(m6809_Regs *m6809)
{
  UInt8 ireg2 = ROP(PCD);
  PC++;
  switch( ireg2 ) {
    case 0x3f: swi3(m6809);    m6809_ICount-=20;  break;

    case 0x83: cmpu_im(m6809);  m6809_ICount-=5;  break;
    case 0x8c: cmps_im(m6809);  m6809_ICount-=5;  break;

    case 0x93: cmpu_di(m6809);  m6809_ICount-=7;  break;
    case 0x9c: cmps_di(m6809);  m6809_ICount-=7;  break;

    case 0xa3: cmpu_ix(m6809);  m6809_ICount-=7;  break;
    case 0xac: cmps_ix(m6809);  m6809_ICount-=7;  break;

    case 0xb3: cmpu_ex(m6809);  m6809_ICount-=8;  break;
    case 0xbc: cmps_ex(m6809);  m6809_ICount-=8;  break;

    default:   illegal(m6809, ILLEGAL2, ireg2);            break;
  }
}
