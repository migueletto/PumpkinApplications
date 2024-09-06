#include <PalmOS.h>

#include "cpu.h"
#include "armlet.h"
#include "m6803.h"
#include "m6803priv.h"
#include "io.h"
#include "debug.h"

#include "m6803mem.c"
#include "m6803ops.c"

void m6803_optable(UInt8 op, m6803_Regs *m6803) {
  switch (op) {
    case 0x00: illegal(m6803); break;
    case 0x01: nop(m6803); break;
    case 0x02: illegal(m6803); break;
    case 0x03: illegal(m6803); break;
    case 0x04: lsrd(m6803); break;
    case 0x05: asld(m6803); break;
    case 0x06: tap(m6803); break;
    case 0x07: tpa(m6803); break;
    case 0x08: inx(m6803); break;
    case 0x09: dex(m6803); break;
    case 0x0A: CLV; break;
    case 0x0B: SEV; break;
    case 0x0C: CLC; break;
    case 0x0D: SEC; break;
    case 0x0E: cli(m6803); break;
    case 0x0F: sei(m6803); break;
    case 0x10: sba(m6803); break;
    case 0x11: cba(m6803); break;
    case 0x12: illegal(m6803); break;
    case 0x13: illegal(m6803); break;
    case 0x14: illegal(m6803); break;
    case 0x15: illegal(m6803); break;
    case 0x16: tab(m6803); break;
    case 0x17: tba(m6803); break;
    case 0x18: illegal(m6803); break;
    case 0x19: daa(m6803); break;
    case 0x1a: illegal(m6803); break;
    case 0x1b: aba(m6803); break;
    case 0x1c: illegal(m6803); break;
    case 0x1d: illegal(m6803); break;
    case 0x1e: illegal(m6803); break;
    case 0x1f: illegal(m6803); break;
    case 0x20: bra(m6803); break;
    case 0x21: brn(m6803); break;
    case 0x22: bhi(m6803); break;
    case 0x23: bls(m6803); break;
    case 0x24: bcc(m6803); break;
    case 0x25: bcs(m6803); break;
    case 0x26: bne(m6803); break;
    case 0x27: beq(m6803); break;
    case 0x28: bvc(m6803); break;
    case 0x29: bvs(m6803); break;
    case 0x2a: bpl(m6803); break;
    case 0x2b: bmi(m6803); break;
    case 0x2c: bge(m6803); break;
    case 0x2d: blt(m6803); break;
    case 0x2e: bgt(m6803); break;
    case 0x2f: ble(m6803); break;
    case 0x30: tsx(m6803); break;
    case 0x31: ins(m6803); break;
    case 0x32: pula(m6803); break;
    case 0x33: pulb(m6803); break;
    case 0x34: des(m6803); break;
    case 0x35: txs(m6803); break;
    case 0x36: psha(m6803); break;
    case 0x37: pshb(m6803); break;
    case 0x38: pulx(m6803); break;
    case 0x39: rts(m6803); break;
    case 0x3a: abx(m6803); break;
    case 0x3b: rti(m6803); break;
    case 0x3c: pshx(m6803); break;
    case 0x3d: mul(m6803); break;
    case 0x3e: wai(m6803); break;
    case 0x3f: swi(m6803); break;
    case 0x40: nega(m6803); break;
    case 0x41: illegal(m6803); break;
    case 0x42: illegal(m6803); break;
    case 0x43: coma(m6803); break;
    case 0x44: lsra(m6803); break;
    case 0x45: illegal(m6803); break;
    case 0x46: rora(m6803); break;
    case 0x47: asra(m6803); break;
    case 0x48: asla(m6803); break;
    case 0x49: rola(m6803); break;
    case 0x4a: deca(m6803); break;
    case 0x4b: illegal(m6803); break;
    case 0x4c: inca(m6803); break;
    case 0x4d: tsta(m6803); break;
    case 0x4e: illegal(m6803); break;
    case 0x4f: clra(m6803); break;
    case 0x50: negb(m6803); break;
    case 0x51: illegal(m6803); break;
    case 0x52: illegal(m6803); break;
    case 0x53: comb(m6803); break;
    case 0x54: lsrb(m6803); break;
    case 0x55: illegal(m6803); break;
    case 0x56: rorb(m6803); break;
    case 0x57: asrb(m6803); break;
    case 0x58: aslb(m6803); break;
    case 0x59: rolb(m6803); break;
    case 0x5a: decb(m6803); break;
    case 0x5b: illegal(m6803); break;
    case 0x5c: incb(m6803); break;
    case 0x5d: tstb(m6803); break;
    case 0x5e: illegal(m6803); break;
    case 0x5f: clrb(m6803); break;
    case 0x60: neg_ix(m6803); break;
    case 0x61: illegal(m6803); break;
    case 0x62: illegal(m6803); break;
    case 0x63: com_ix(m6803); break;
    case 0x64: lsr_ix(m6803); break;
    case 0x65: illegal(m6803); break;
    case 0x66: ror_ix(m6803); break;
    case 0x67: asr_ix(m6803); break;
    case 0x68: asl_ix(m6803); break;
    case 0x69: rol_ix(m6803); break;
    case 0x6a: dec_ix(m6803); break;
    case 0x6b: illegal(m6803); break;
    case 0x6c: inc_ix(m6803); break;
    case 0x6d: tst_ix(m6803); break;
    case 0x6e: jmp_ix(m6803); break;
    case 0x6f: clr_ix(m6803); break;
    case 0x70: neg_ex(m6803); break;
    case 0x71: illegal(m6803); break;
    case 0x72: illegal(m6803); break;
    case 0x73: com_ex(m6803); break;
    case 0x74: lsr_ex(m6803); break;
    case 0x75: illegal(m6803); break;
    case 0x76: ror_ex(m6803); break;
    case 0x77: asr_ex(m6803); break;
    case 0x78: asl_ex(m6803); break;
    case 0x79: rol_ex(m6803); break;
    case 0x7a: dec_ex(m6803); break;
    case 0x7b: illegal(m6803); break;
    case 0x7c: inc_ex(m6803); break;
    case 0x7d: tst_ex(m6803); break;
    case 0x7e: jmp_ex(m6803); break;
    case 0x7f: clr_ex(m6803); break;
    case 0x80: suba_im(m6803); break;
    case 0x81: cmpa_im(m6803); break;
    case 0x82: sbca_im(m6803); break;
    case 0x83: subd_im(m6803); break;
    case 0x84: anda_im(m6803); break;
    case 0x85: bita_im(m6803); break;
    case 0x86: lda_im(m6803); break;
    case 0x87: sta_im(m6803); break;
    case 0x88: eora_im(m6803); break;
    case 0x89: adca_im(m6803); break;
    case 0x8a: ora_im(m6803); break;
    case 0x8b: adda_im(m6803); break;
    case 0x8c: cpx_im(m6803); break;
    case 0x8d: bsr(m6803); break;
    case 0x8e: lds_im(m6803); break;
    case 0x8f: sts_im(m6803); break;
    case 0x90: suba_di(m6803); break;
    case 0x91: cmpa_di(m6803); break;
    case 0x92: sbca_di(m6803); break;
    case 0x93: subd_di(m6803); break;
    case 0x94: anda_di(m6803); break;
    case 0x95: bita_di(m6803); break;
    case 0x96: lda_di(m6803); break;
    case 0x97: sta_di(m6803); break;
    case 0x98: eora_di(m6803); break;
    case 0x99: adca_di(m6803); break;
    case 0x9a: ora_di(m6803); break;
    case 0x9b: adda_di(m6803); break;
    case 0x9c: cpx_di(m6803); break;
    case 0x9d: jsr_di(m6803); break;
    case 0x9e: lds_di(m6803); break;
    case 0x9f: sts_di(m6803); break;
    case 0xa0: suba_ix(m6803); break;
    case 0xa1: cmpa_ix(m6803); break;
    case 0xa2: sbca_ix(m6803); break;
    case 0xa3: subd_ix(m6803); break;
    case 0xa4: anda_ix(m6803); break;
    case 0xa5: bita_ix(m6803); break;
    case 0xa6: lda_ix(m6803); break;
    case 0xa7: sta_ix(m6803); break;
    case 0xa8: eora_ix(m6803); break;
    case 0xa9: adca_ix(m6803); break;
    case 0xaa: ora_ix(m6803); break;
    case 0xab: adda_ix(m6803); break;
    case 0xac: cpx_ix(m6803); break;
    case 0xad: jsr_ix(m6803); break;
    case 0xae: lds_ix(m6803); break;
    case 0xaf: sts_ix(m6803); break;
    case 0xb0: suba_ex(m6803); break;
    case 0xb1: cmpa_ex(m6803); break;
    case 0xb2: sbca_ex(m6803); break;
    case 0xb3: subd_ex(m6803); break;
    case 0xb4: anda_ex(m6803); break;
    case 0xb5: bita_ex(m6803); break;
    case 0xb6: lda_ex(m6803); break;
    case 0xb7: sta_ex(m6803); break;
    case 0xb8: eora_ex(m6803); break;
    case 0xb9: adca_ex(m6803); break;
    case 0xba: ora_ex(m6803); break;
    case 0xbb: adda_ex(m6803); break;
    case 0xbc: cpx_ex(m6803); break;
    case 0xbd: jsr_ex(m6803); break;
    case 0xbe: lds_ex(m6803); break;
    case 0xbf: sts_ex(m6803); break;
    case 0xc0: subb_im(m6803); break;
    case 0xc1: cmpb_im(m6803); break;
    case 0xc2: sbcb_im(m6803); break;
    case 0xc3: addd_im(m6803); break;
    case 0xc4: andb_im(m6803); break;
    case 0xc5: bitb_im(m6803); break;
    case 0xc6: ldb_im(m6803); break;
    case 0xc7: stb_im(m6803); break;
    case 0xc8: eorb_im(m6803); break;
    case 0xc9: adcb_im(m6803); break;
    case 0xca: orb_im(m6803); break;
    case 0xcb: addb_im(m6803); break;
    case 0xcc: ldd_im(m6803); break;
    case 0xcd: std_im(m6803); break;
    case 0xce: ldx_im(m6803); break;
    case 0xcf: stx_im(m6803); break;
    case 0xd0: subb_di(m6803); break;
    case 0xd1: cmpb_di(m6803); break;
    case 0xd2: sbcb_di(m6803); break;
    case 0xd3: addd_di(m6803); break;
    case 0xd4: andb_di(m6803); break;
    case 0xd5: bitb_di(m6803); break;
    case 0xd6: ldb_di(m6803); break;
    case 0xd7: stb_di(m6803); break;
    case 0xd8: eorb_di(m6803); break;
    case 0xd9: adcb_di(m6803); break;
    case 0xda: orb_di(m6803); break;
    case 0xdb: addb_di(m6803); break;
    case 0xdc: ldd_di(m6803); break;
    case 0xdd: std_di(m6803); break;
    case 0xde: ldx_di(m6803); break;
    case 0xdf: stx_di(m6803); break;
    case 0xe0: subb_ix(m6803); break;
    case 0xe1: cmpb_ix(m6803); break;
    case 0xe2: sbcb_ix(m6803); break;
    case 0xe3: addd_ix(m6803); break;
    case 0xe4: andb_ix(m6803); break;
    case 0xe5: bitb_ix(m6803); break;
    case 0xe6: ldb_ix(m6803); break;
    case 0xe7: stb_ix(m6803); break;
    case 0xe8: eorb_ix(m6803); break;
    case 0xe9: adcb_ix(m6803); break;
    case 0xea: orb_ix(m6803); break;
    case 0xeb: addb_ix(m6803); break;
    case 0xec: ldd_ix(m6803); break;
    case 0xed: std_ix(m6803); break;
    case 0xee: ldx_ix(m6803); break;
    case 0xef: stx_ix(m6803); break;
    case 0xf0: subb_ex(m6803); break;
    case 0xf1: cmpb_ex(m6803); break;
    case 0xf2: sbcb_ex(m6803); break;
    case 0xf3: addd_ex(m6803); break;
    case 0xf4: andb_ex(m6803); break;
    case 0xf5: bitb_ex(m6803); break;
    case 0xf6: ldb_ex(m6803); break;
    case 0xf7: stb_ex(m6803); break;
    case 0xf8: eorb_ex(m6803); break;
    case 0xf9: adcb_ex(m6803); break;
    case 0xfa: orb_ex(m6803); break;
    case 0xfb: addb_ex(m6803); break;
    case 0xfc: ldd_ex(m6803); break;
    case 0xfd: std_ex(m6803); break;
    case 0xfe: ldx_ex(m6803); break;
    case 0xff: stx_ex(m6803); break;
  }
}

void ENTER_INTERRUPT(m6803_Regs *m6803, UInt16 irq_vector) {
  if (m6803->wai_state & (M6803_WAI|M6803_SLP)) {
    if (m6803->wai_state & M6803_WAI)
      m6803->extracycles += 4;
    m6803->wai_state &= ~(M6803_WAI|M6803_SLP);
  } else {
    PUSHWORD(pPC);
    PUSHWORD(pX);
    PUSHBYTE(A);
    PUSHBYTE(B);
    PUSHBYTE(CC);
    m6803->extracycles += 12;
  }
  SEI;
  PC = RM16(irq_vector);
}

// check OCI or TOI
void check_timer_event(m6803_Regs *m6803) {
  // OCI
  if (CTD >= OCD) {
    OCH++;	// next IRQ point
    m6803->tcsr |= TCSR_OCF;
    m6803->pending_tcsr |= TCSR_OCF;
    MODIFIED_tcsr;
    if (!(CC & 0x10) && (m6803->tcsr & TCSR_EOCI))
      TAKE_OCI;
  }
  // TOI
  if (CTD >= TOD) {
    TOH++;	// next IRQ point
    m6803->tcsr |= TCSR_TOF;
    m6803->pending_tcsr |= TCSR_TOF;
    MODIFIED_tcsr;
    if (!(CC & 0x10) && (m6803->tcsr & TCSR_ETOI))
      TAKE_TOI;
  }
  // set next event
  SET_TIMER_EVENT;
}

void m6803_init(m6803_Regs *m6803) {
}

void m6803_reset(m6803_Regs *m6803) {
  SEI;

  m6803->wai_state = 0;
  m6803->nmi_state = 0;
  m6803->irq_state[M6803_IRQ_LINE] = 0;
  m6803->irq_state[M6803_TIN_LINE] = 0;
  m6803->ic_eddge = 0;

  m6803->port1_ddr = 0x00;
  m6803->port2_ddr = 0x00;
  m6803->tcsr = 0x00;
  m6803->pending_tcsr = 0x00;
  m6803->irq2 = 0;
  CTD = 0x0000;
  OCD = 0xffff;
  TOD = 0xffff;
  m6803->ram_ctrl |= 0x40;

  m6803->count = 10000;
  m6803->extracycles = 0;

  m6803->hardware->eventcount = 0;
  m6803->hardware->totalcycles = 0;
  m6803->hardware->firq_request = 0;

  PC = RM16(0xfffe);
}

void m6803_exit(m6803_Regs *m6803) {
}

void m6803_set_irq_line(m6803_Regs *m6803, Int32 irqline, Int32 state) {
	if (irqline == M6803_NMI_LINE) {
		if (m6803->nmi_state == state) return;
		m6803->nmi_state = state;
		if (state == CLEAR_LINE) return;

		/* NMI */
		ENTER_INTERRUPT(m6803, 0xfffc);
	} else {
		//int eddge;

		if (m6803->irq_state[irqline] == state) return;
		m6803->irq_state[irqline] = state;

		switch(irqline)
		{
		case M6803_IRQ_LINE:
			if (state == CLEAR_LINE) return;
			break;
		case M6803_TIN_LINE:
			//eddge = (state == CLEAR_LINE ) ? 2 : 0;
			if( ((m6803->tcsr&TCSR_IEDG) ^ (state==CLEAR_LINE ? TCSR_IEDG : 0))==0 )
				return;
			/* active edge in */
			m6803->tcsr |= TCSR_ICF;
			m6803->pending_tcsr |= TCSR_ICF;
			m6803->input_capture = CT;
			MODIFIED_tcsr;
			if( !(CC & 0x10) )
				CHECK_IRQ2
			break;
		default:
			return;
		}
		CHECK_IRQ_LINES(); /* HJB 990417 */
	}
}

Int32 m6803_execute(m6803_Regs *m6803, Int32 cycles) {
  UInt8 ireg;
  m6803_ICount = cycles;

  CLEANUP_conters;
  INCREMENT_COUNTER(m6803, m6803->extracycles);
  m6803->extracycles = 0;

  do {
    if (m6803->wai_state & M6803_WAI) {
      EAT_CYCLES(m6803);
    } else {
      pPPC = pPC;
      ireg = ROP(PCD);
      PC++;

      m6803_optable(ireg, m6803);
      INCREMENT_COUNTER(m6803, m6803->hardware->cycles[ireg]);
      m6803->hardware->totalcycles += m6803->hardware->cycles[ireg];
      m6803->hardware->eventcount += m6803->hardware->cycles[ireg];

      if (m6803->hardware->totalcycles >= m6803->hardware->vsync) {
        m6803->hardware->totalcycles = 0;
        IO_vsync(m6803->hardware, 0);
        if (m6803->hardware->vsync_irq)
          m6803_set_irq_line(m6803, M6803_IRQ_LINE, ASSERT_LINE);
      }
    }
  } while (m6803_ICount > 0);

  m6803->hardware->totalcycles += m6803->extracycles;
  INCREMENT_COUNTER(m6803, m6803->extracycles);
  m6803->extracycles = 0;

  return cycles - m6803_ICount;
}

UInt8 m6803_getreg(m6803_Regs *m6803, UInt8 a) {
  switch (a) {
    case 0x00:
      return m6803->port1_ddr;
    case 0x01:
      return m6803->port2_ddr;
    case 0x02:
      return (IO_readb(m6803->hardware, 0x81) & (m6803->port1_ddr ^ 0xff)) |
             (m6803->port1_data & m6803->port1_ddr);
    case 0x03:
      return (IO_readb(m6803->hardware, 0x82) & (m6803->port2_ddr ^ 0xff)) |
             (m6803->port2_data & m6803->port2_ddr);
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
      return 0;
    case 0x08:
      m6803->pending_tcsr = 0;
      return m6803->tcsr;
    case 0x09:
      if (!(m6803->pending_tcsr & TCSR_TOF)) {
        m6803->tcsr &= ~TCSR_TOF;
        m6803->irq2 = (m6803->tcsr&(m6803->tcsr<<3))&(TCSR_ICF|TCSR_OCF|TCSR_TOF);
      }
      return m6803->counter.b.h;
    case 0x0A:
      return m6803->counter.b.l;
    case 0x0B:
      if (!(m6803->pending_tcsr & TCSR_OCF)) {
        m6803->tcsr &= ~TCSR_OCF;
        m6803->irq2 = (m6803->tcsr&(m6803->tcsr<<3))&(TCSR_ICF|TCSR_OCF|TCSR_TOF);
      }
      return m6803->output_compare.b.h;
    case 0x0C:
      if (!(m6803->pending_tcsr & TCSR_OCF)) {
        m6803->tcsr &= ~TCSR_OCF;
        m6803->irq2 = (m6803->tcsr&(m6803->tcsr<<3))&(TCSR_ICF|TCSR_OCF|TCSR_TOF);
      }
      return m6803->output_compare.b.l;
    case 0x0D:
      if (!(m6803->pending_tcsr & TCSR_ICF)) {
        m6803->tcsr &= ~TCSR_ICF;
        m6803->irq2 = (m6803->tcsr&(m6803->tcsr<<3))&(TCSR_ICF|TCSR_OCF|TCSR_TOF);
      }
      return (m6803->input_capture >> 0) & 0xff;
    case 0x0E:
      return (m6803->input_capture >> 8) & 0xff;
    case 0x0F:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
      return 0;
    case 0x14:
      return m6803->ram_ctrl;
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
    case 0x1D:
    case 0x1E:
    case 0x1F:
    default:
      return 0;
  }

  return 0;
}

void m6803_setreg(m6803_Regs *m6803, UInt8 a, UInt8 b) {
  switch (a) {
    case 0x00:
      if (m6803->port1_ddr != b) {
        m6803->port1_ddr = b;
        if (m6803->port1_ddr == 0xff)
          IO_writeb(m6803->hardware, 0x81, m6803->port1_data);
        else
          IO_writeb(m6803->hardware, 0x81,(m6803->port1_data & m6803->port1_ddr) |
            (IO_readb(m6803->hardware, 0x81) & (m6803->port1_ddr ^ 0xff)));
      }
      break;
    case 0x01:
      if (m6803->port2_ddr != b) {
        m6803->port2_ddr = b;
        if (m6803->port2_ddr == 0xff)
          IO_writeb(m6803->hardware, 0x82, m6803->port2_data);
        else
          IO_writeb(m6803->hardware, 0x82, (m6803->port2_data & m6803->port2_ddr) |
            (IO_readb(m6803->hardware, 0x82) & (m6803->port2_ddr ^ 0xff)));
      }
      break;
    case 0x02:
      m6803->port1_data = b;
      if (m6803->port1_ddr == 0xff)
        IO_writeb(m6803->hardware, 0x81, m6803->port1_data);
      else
        IO_writeb(m6803->hardware, 0x81, (m6803->port1_data & m6803->port1_ddr) |
          (IO_readb(m6803->hardware, 0x81) & (m6803->port1_ddr ^ 0xff)));
      break;
    case 0x03:
      m6803->port2_data = b;
      m6803->port2_ddr = b;
      if (m6803->port2_ddr == 0xff)
        IO_writeb(m6803->hardware, 0x82, m6803->port2_data);
      else
        IO_writeb(m6803->hardware, 0x82, (m6803->port2_data & m6803->port2_ddr) |
          (IO_readb(m6803->hardware, 0x82) & (m6803->port2_ddr ^ 0xff)));
      break;
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
      break;
    case 0x08:
      m6803->tcsr = b;
      m6803->pending_tcsr &= m6803->tcsr;
      m6803->irq2 = (m6803->tcsr&(m6803->tcsr<<3))&(TCSR_ICF|TCSR_OCF|TCSR_TOF);
      if (!(CC & 0x10))
        CHECK_IRQ2;
      break;
    case 0x09:
      m6803->latch09 = b & 0xff;  /* 6301 only */
      CT  = 0xfff8;
      TOH = CTH;
      OCH = (OC >= CT) ? CTH : CTH+1;
      m6803->timer_next = (OCD - CTD < TOD - CTD) ? OCD : TOD;
      break;
    case 0x0a:  /* 6301 only */
      CT = (m6803->latch09 << 8) | (b & 0xff);
      TOH = CTH;
      OCH = (OC >= CT) ? CTH : CTH+1;
      m6803->timer_next = (OCD - CTD < TOD - CTD) ? OCD : TOD;
      break;
    case 0x0b:
      if (m6803->output_compare.b.h != b) {
        m6803->output_compare.b.h = b;
        OCH = (OC >= CT) ? CTH : CTH+1;
        m6803->timer_next = (OCD - CTD < TOD - CTD) ? OCD : TOD;
      }
      break;
    case 0x0c:
      if (m6803->output_compare.b.l != b) {
        m6803->output_compare.b.l = b;
        OCH = (OC >= CT) ? CTH : CTH+1;
        m6803->timer_next = (OCD - CTD < TOD - CTD) ? OCD : TOD;
      }
      break;
    case 0x0d:
    case 0x0e:
      break;
    case 0x0f:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
      break;
    case 0x14:
      m6803->ram_ctrl = b;
      break;
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1a:
    case 0x1b:
    case 0x1c:
    case 0x1d:
    case 0x1e:
    case 0x1f:
    default:
      break;
  }
}
