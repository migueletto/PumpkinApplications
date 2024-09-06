#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "endian.h"
#include "palm.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "machine.h"
#include "kbd.h"
#include "m6502.h"
#include "atom.h"
#include "misc.h"
#include "gui.h"

#define INT_CA2         0x01
#define INT_CA1         0x02
#define INT_SR          0x04
#define INT_CB2         0x08
#define INT_CB1         0x10
#define INT_T2          0x20
#define INT_T1          0x40
#define INT_ANY         0x80

typedef struct {
  char name[16];
  UInt16 start;
  UInt16 exec;
  UInt16 size;
} ATM;

static UInt8 id;
static AtomPrefs prefs;
static Hardware *hardware;
static KeyMap *keyMap = NULL;
static Boolean ctrl;
static UInt16 key, kbselect;
static UInt8 vdg, portc;
static UInt8 ier, ifr, ddra, ddrb, pcr, latch1, counter1;

static void via_clearint(Int16 i, UInt8 b);

UInt16 AtomControl[9] = {stopCmd, restartCmd, 0,
                         loadSnapCmd, 0, 0,
                         0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef AtomButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {KEY_ESC,   internalFontID, "ESC",  C1, C2, C3},
  {'^',       internalFontID, "\x80", C1, C2, C3},
  {KEY_BREAK, internalFontID, "BRK", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_RIGHT, internalFontID, "\x85", C1, C2, C3},
  {KEY_COPY,  internalFontID, "CPY", C1, C2, C3},
  {KEY_BACK,  internalFontID, "DEL", C1, C2, C3},
  {0, 0, 0, C1, C1, C1},
  {KEY_UP,    internalFontID, "\x84", C1, C2, C3},
  {KEY_REPT,  internalFontID, "RPT", C1, C2, C3},
  {KEY_CTRL,  internalFontID, "CTL", C1, C2, C3}
};

static KeyMap AtomMap[] = {
  {'3', 0x00, 0xFD, 0, 0},
  {'#', 0x00, 0xFD, 1, 0},
  {'-', 0x00, 0xFB, 0, 0},
  {'=', 0x00, 0xFB, 1, 0},
  {'G', 0x00, 0xF7, 0, 0},
  {'Q', 0x00, 0xEF, 0, 0},
  {KEY_ESC, 0x00, 0xDF, 0, 0},

  {'2', 0x01, 0xFD, 0, 0},
  {'"', 0x01, 0xFD, 1, 0},
  {',', 0x01, 0xFB, 0, 0},
  {'<', 0x01, 0xFB, 1, 0},
  {'F', 0x01, 0xF7, 0, 0},
  {'P', 0x01, 0xEF, 0, 0},
  {'Z', 0x01, 0xDF, 0, 0},

  {KEY_UP, 0x02, 0xFE, 0, 0},
  {'1', 0x02, 0xFD, 0, 0},
  {'!', 0x02, 0xFD, 1, 0},
  {';', 0x02, 0xFB, 0, 0},
  {'E', 0x02, 0xF7, 0, 0},
  {'O', 0x02, 0xEF, 0, 0},
  {'Y', 0x02, 0xDF, 0, 0},

  {KEY_RIGHT, 0x03, 0xFE, 0, 0},
  {'0', 0x03, 0xFD, 0, 0},
  {':', 0x03, 0xFB, 0, 0},
  {'*', 0x03, 0xFB, 1, 0},
  {'D', 0x03, 0xF7, 0, 0},
  {'N', 0x03, 0xEF, 0, 0},
  {'X', 0x03, 0xDF, 0, 0},

  {KEY_BACK, 0x04, 0xFD, 0, 0},
  {'9', 0x04, 0xFB, 0, 0},
  {')', 0x04, 0xFB, 1, 0},
  {'C', 0x04, 0xF7, 0, 0},
  {'M', 0x04, 0xEF, 0, 0},
  {'W', 0x04, 0xDF, 0, 0},

  {'^', 0x05, 0xFE, 0, 0},
  {KEY_COPY, 0x05, 0xFD, 0, 0},
  {'8', 0x05, 0xFB, 0, 0},
  {'(', 0x05, 0xFB, 1, 0},
  {'B', 0x05, 0xF7, 0, 0},
  {'L', 0x05, 0xEF, 0, 0},
  {'V', 0x05, 0xDF, 0, 0},

  {']', 0x06, 0xFE, 0, 0},
  {KEY_ENTER, 0x06, 0xFD, 0, 0},
  {'7', 0x06, 0xFB, 0, 0},
  {'|', 0x06, 0xFB, 1, 0},
  {'A', 0x06, 0xF7, 0, 0},
  {'K', 0x06, 0xEF, 0, 0},
  {'U', 0x06, 0xDF, 0, 0},

  {'\\', 0x07, 0xFE, 0, 0},
  {'6', 0x07, 0xFB, 0, 0},
  {'&', 0x07, 0xFB, 1, 0},
  {'@', 0x07, 0xF7, 0, 0},
  {'J', 0x07, 0xEF, 0, 0},
  {'T', 0x07, 0xDF, 0, 0},

  {']', 0x08, 0xFE, 0, 0},
  {'5', 0x08, 0xFB, 0, 0},
  {'%', 0x08, 0xFB, 1, 0},
  {'/', 0x08, 0xF7, 0, 0},
  {'?', 0x08, 0xF7, 1, 0},
  {'I', 0x08, 0xEF, 0, 0},
  {'S', 0x08, 0xDF, 0, 0},

  {' ', 0x09, 0xFE, 0, 0},
  {'4', 0x09, 0xFB, 0, 0},
  {'$', 0x09, 0xFB, 1, 0},
  {'.', 0x09, 0xF7, 0, 0},
  {'>', 0x09, 0xF7, 1, 0},
  {'H', 0x09, 0xEF, 0, 0},
  {'R', 0x09, 0xDF, 0, 0},

  {0, 0, 0, 0, 0}
};

UInt32 atom_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_VSYNC:
      atom_vsync(arg->a1);
      hardware->eventcount = 0;
      return 0;
    case IO_READB:
      return atom_readb(arg->a1);
    case IO_WRITEB:
      atom_writeb(arg->a1, arg->a2);
      return 0;
  }
  return 0;
}

void atom_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  UInt16 len;

  id = machine->id;
  hardware = _hardware;

  len = sizeof(prefs);
  MemSet(&prefs, len, 0);

  if (PrefGetAppPreferences(AppID, 128 + id, &prefs, &len, true) == noPreferenceFound) {
    prefs.pad = 0;
    PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, len, true);
  }

  keyMap = MemPtrNew(256 * sizeof(KeyMap));
  kbd_initmap(AtomMap, keyMap);

  ctrl = false;
  key = 0;
  kbselect = 0xFF;
  vdg = 0;
  hardware->m1[0xB000 & 0x7FFF] = 0xFF;
  portc = 0xFF;
  ier = ifr = ddra = ddrb = pcr = latch1 = counter1 = 0;

  hardware->ramsize = ramsize * 4096 - 1;
}

void atom_init(void) {
  UInt32 t;

  io_setgp(0x8000, 0x9800);
  io_vdg(0);
  io_sam(0);
  hardware->vsync_irq = 0;

  // random seed
  t = TimGetTicks();
  hardware->m0[0x08] = t & 0xFF;
  t >>= 8;
  hardware->m0[0x09] = t & 0xFF;
  t >>= 8;
  hardware->m0[0x0A] = t & 0xFF;
  t >>= 8;
  hardware->m0[0x0B] = t & 0xFF;
  hardware->m0[0x0C] = hardware->m0[0x08];
}

void atom_finish(void) {
  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);

  if (keyMap) {
    MemPtrFree(keyMap);
    keyMap = NULL;
  }
}

void atom_key(UInt16 c) {
  if (c >= 'a' && c <= 'z') {
    c &= 0xDF;
  } else if (c == KEY_CTRL) {
    ctrl = ~ctrl;
    c = 0;
  }

  key = c;
  hardware->key = c ? 1 : 0;
}

static void via_clearint(Int16 i, UInt8 b) {
  ifr = (ifr & ~b) & 0x7f;

  if (ifr & ier) {
    ifr |= INT_ANY;
  }
}

UInt8 atom_readb(UInt16 a) {
  UInt8 b;
  UInt32 t;

  switch (a) {
    case 0xB000:
      return hardware->m1[a & 0x7FFF];
    case 0xB001:
      if (hardware->key == 0)
        return 0xFF;
      break;
    case 0xB002:
      break;

    case 0xB800:
    case 0xB801:
    case 0xB802:
    case 0xB803:
    case 0xB804:
    case 0xB805:
    case 0xB806:
    case 0xB807:
    case 0xB80C:
    case 0xB80D:
    case 0xB80E:
    case 0xB80F:
      break;

    default:
      return 0xFF;
  }

  switch (a) {
    case 0xB001:
      b = 0xFF;
      if (key) {
        if ((keyMap[key].line) == kbselect)
          b &= keyMap[key].column;

        if (keyMap[key].shift)
          b &= 0x7F;

        if (ctrl || keyMap[key].ctrl)
          b &= 0xBF;
      }
      return b;
    case 0xB002:
      b = (portc & 0x0F) | 0xC0;
      t = hardware->eventcount;
      if (t < 128)
        b &= 0x7F;	// vblank
      return b;

    case 0xB800:	// via 0 port b
      b = 0x00;
      b = (hardware->m0[a] & ddrb) + (b & ~ddrb);
      return b;

    case 0xB801:	// via 0 port a
    case 0xB80F:	// via 0 port a (without handshake)
      b = 0xFF;
      b = (hardware->m1[0xB801 & 0x7FFF] & ddra) + (b & ~ddra);
      return b;

    case 0xB802:	// via 0 data direction b
      return ddrb;

    case 0xB803:	// via 0 data direction a
      return ddra;

    case 0xB804:	// timer 1 counter (low)
      via_clearint(0, INT_T1);
      return counter1 & 0xFF;

    case 0xB805:	// timer 1 counter (high)
      return counter1 >> 8;

    case 0xB806:	// timer 1 latch   (low)
      return latch1 & 0xFF;

    case 0xB807:	// timer 1 latch (high)
      return latch1 >> 8;

    case 0xB80C:	// pcr
      return pcr;

    case 0xB80D:	// interrupt flag register
      return ifr;

    case 0xB80E:	// interrupt enable register
      return ier | 0x80;
  }

  return 0xFF;
}

/*
Port A - #B000
Output bits:    Function:
0 - 3   Keyboard row
4 - 7   Graphics mode
 
Port B - #B001
Input bits:     Function:
0 - 5   Keyboard column
6       CTRL key (low when pressed)
7       SHIFT keys (low when pressed)
 
Port C - #B002
Output bits:    Function:
0       Tape output
1       Enable 2.4 kHz to cassette output
2       Loudspeaker
3       Color set
                 
Input bits:     Function:
4       2.4 kHz input
5       Cassette input
6       REPT key (low when pressed)
7       60 Hz sync signal (low during flyback)
*/

void atom_writeb(UInt16 a, UInt8 b) {
  UInt8 b1;

  switch (a) {
    case 0xB000:
      if (hardware->key == 0 &&
          (hardware->m1[0xB000 & 0x7FFF] & 0xF0) == (b & 0xF0)) {
        hardware->m1[0xB000 & 0x7FFF] = b;
        return;
      }
      break;
    case 0xB001:
      return;
    case 0xB002:
      break;

    case 0xB800:
    case 0xB801:
    case 0xB802:
    case 0xB803:
    case 0xB804:
    case 0xB805:
    case 0xB806:
    case 0xB807:
    case 0xB80C:
    case 0xB80D:
    case 0xB80E:
    case 0xB80F:
      break;

    default:
      return;
  }

  switch (a) {
    case 0xB000:
      kbselect = b & 0x0F;

      b1 = vdg & 0x08;	// CSS
      if (b & 0x20) b1 |= 0x10;	// GM0
      if (b & 0x40) b1 |= 0x20;	// GM1
      if (b & 0x80) b1 |= 0x40;	// GM2
      if (b & 0x10) b1 |= 0x80;	// A/G

      if (b1 != vdg) {
        vdg = b1;
        io_vdg(vdg);

        switch (vdg & 0xF0) {
          case 0x80: b1 = 1; break;
          case 0x90: b1 = 1; break;
          case 0xA0: b1 = 2; break;
          case 0xB0: b1 = 3; break;
          case 0xC0: b1 = 4; break;
          case 0xD0: b1 = 5; break;
          case 0xE0: b1 = 6; break;
          case 0xF0: b1 = 6; break;
          default:   b1 = 0;
        }
        io_sam(b1);
      }
      hardware->m1[0xB000 & 0x7FFF] = b;
      break;
    case 0xB001:
      break;
    case 0xB002:
      snd_output((b & 0x04) ? 0xFF : 0x00);

      b1 = vdg & 0xF0;	// GM0-GM2, A/G
      if (b & 0x08) b1 |= 0x08;

      if (b1 != vdg) {
        vdg = b1;
        io_vdg(vdg);
      }
      portc = b;
      break;

    case 0xB800:	// via 0 port b
      hardware->m1[a & 0x7FFF] = b;
      break;
    case 0xB801:	// via 0 port a
    case 0xB80F:	// via 0 port a (without handshake)
      hardware->m1[0xB801 & 0x7FFF] = b;
      break;
    case 0xB802:	// via 0 data direction b
      ddrb = b;
      break;
    case 0xB803:	// via 0 data direction a
      ddra = b;
      break;
    case 0xB804:	// timer 1 counter (low)
      latch1 = (latch1 & 0xFF00) | b;
      break;
    case 0xB805:	// timer 1 counter (high)
      latch1 = (latch1 & 0x00FF) | (b << 8);
      counter1 = latch1;
      via_clearint(0, INT_T1);
      break;
    case 0xB806:	// timer 1 latch   (low)
      latch1 = (latch1 & 0xFF00) | b;
      break;
    case 0xB807:	// timer 1 latch (high)
      latch1 = (latch1 & 0x00FF) | (b << 8);
      via_clearint(0, INT_T1);
      break;
    case 0xB80C:	// pcr
      pcr = b;
      break;
    case 0xB80D:	// interrupt flag register
      if (b & INT_ANY)
        b = 0x7F;
      via_clearint(0, b);
      break;
    case 0xB80E:	// interrupt enable register
      if (b & 0x80)
        ier |= b & 0x7F;
      else
        ier &= ~(b & 0x7F);

      if (ifr & INT_ANY) {
        if (((ifr & ier) & 0x7F) == 0) {
          ifr &= ~INT_ANY;
        }
      } else {
        if ((ier & ifr) & 0x7F) {
          ifr |= INT_ANY;
          hardware->irq_request = 1;
        }
      }
  }
}

Err atom_readatm(FileRef f, m6502_Regs *m6502, Hardware *hardware) {
  ATM atm;
  UInt32 addr, exec, size, aux, r;
  UInt16 top;
  Err err;

  aux = sizeof(ATM);

  if ((err = VFSFileSize(f, &size)) != 0)
    return -1;

  if (size < aux)
    return -2;

  if ((err = VFSFileRead(f, aux, &atm, &r)) != 0)
    return -3;

  if (r != aux)
    return -4;

  addr = (atm.start) & 0xFFFF;
  exec = (atm.exec) & 0xFFFF;
  size = (atm.size) & 0xFFFF;

  if (addr >= 0x400 && addr < 0x2800)
    return -5;

  if (addr >= 0x3C00 && addr < 0x8000)
    return -6;

  if (addr >= 0x9800)
    return -7;

  aux = size;

  if (addr < 0x8000) {
    if ((err = VFSFileRead(f, aux, &hardware->m0[addr], &r)) != 0)
      return -8;
  } else {
    if ((err = VFSFileRead(f, aux, &hardware->m1[addr & 0x7FFF], &r)) != 0)
      return -9;
  }

  if (r != aux)
    return -10;


  if (addr == 0x2900 && exec == 0xC2B2) {
    // If the program's start address is 2900h and its execution address is
    // C2B2h, a BASIC "LOAD" command will be issued. This means that the file
    // is loaded into the ATOM's text area at 2900h. Then the text area is
    // scanned through to set the value of TOP and the free space pointer.
    // Remember, if the file is not a valid BASIC program, the prompt may not
    // appear.
    // LOAD loads text files to the current text space.

    for (top = addr; top < addr + size; top++) {
      if (hardware->m0[top] != 0x0D)
        continue;
      top++;
      if (hardware->m0[top] == 0xFF)
        break;
    }
    top++;

    hardware->m0[0x000D] = top & 0xFF;
    hardware->m0[0x000E] = top >> 8;

  } else if (addr != 0x2900 && exec == 0xC2B2) {
    // If the program's start address is not 2900h, but the execution address
    // is C2B2h, the COS command "*LOAD" will be issued. The value of TOP will
    // not be set.
    // *LOAD loads a block of memory to a fixed address, or to an address
    // specified in the command.

  } else {
    // In any other case the COS command "*RUN" will be issued. This will load
    // the file at its start address and then execution is transferred to the
    // execution address specified in the file. The value of TOP is not set.

    m6502->pc_reg.w.h = atm.exec;
  }

  return 0;
}

AtomPrefs *AtomGetPrefs(void) {
  return &prefs;
}

/*
Boolean AtomFormHandler(EventPtr event, Boolean *close)
{
  FormPtr frm;
  Boolean handled;

  frm = FrmGetActiveForm();
  handled = false;
  *close = false;

  switch (event->eType) {
    case frmOpenEvent:
      FrmDrawForm(frm);
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case okBtn:
          *close = true;
          handled = true;
      }
  }

  return handled;
}
*/
