#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "machine.h"
#include "kbd.h"
#include "endian.h"
#include "m6502.h"
#include "nes.h"
#include "misc.h"
#include "db.h"
#include "cdebug.h"
#include "gui.h"

/*
Real               Emulador
         NMI
0..19  : vblank    241..260
20     : dummy     261
21..260: screen    0..239
261    : nothing   240

Nintencer:

0          : nothing
1 (0..11)  : vblank
             NMI
1 (12..340): vblank
2..20      : vblank
21         : dummy
22..261    : screen

*/

#define LINE_SCREEN0  0
#define LINE_SCREEN1  239
#define LINE_NOTHING  240
#define LINE_VBLANK0  241
#define LINE_VBLANK1  260
#define LINE_DUMMY  261

#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 240

static UInt8 id;
static NesPrefs prefs;
static Hardware *hardware;
static char cartname[64];
static char buf[256];

ColorType NesColor[65] = {
  {0x00, 0x75, 0x75, 0x75},
  {0x01, 0x27, 0x1B, 0x8F},
  {0x02, 0x00, 0x00, 0xAB},
  {0x03, 0x47, 0x00, 0x9F},
  {0x04, 0x8F, 0x00, 0x77},
  {0x05, 0xAB, 0x00, 0x13},
  {0x06, 0xA7, 0x00, 0x00},
  {0x07, 0x7F, 0x0B, 0x00},
  {0x08, 0x43, 0x2F, 0x00},
  {0x09, 0x00, 0x47, 0x00},
  {0x0A, 0x00, 0x51, 0x00},
  {0x0B, 0x00, 0x3F, 0x17},
  {0x0C, 0x1B, 0x3F, 0x5F},
  {0x0D, 0x00, 0x00, 0x00},
  {0x0E, 0x00, 0x00, 0x00},
  {0x0F, 0x00, 0x00, 0x00},
  {0x10, 0xBC, 0xBC, 0xBC},
  {0x11, 0x00, 0x73, 0xEF},
  {0x12, 0x23, 0x3B, 0xEF},
  {0x13, 0x83, 0x00, 0xF3},
  {0x14, 0xBF, 0x00, 0xBF},
  {0x15, 0xE7, 0x00, 0x5B},
  {0x16, 0xDB, 0x2B, 0x00},
  {0x17, 0xCB, 0x4F, 0x0F},
  {0x18, 0x8B, 0x73, 0x00},
  {0x19, 0x00, 0x97, 0x00},
  {0x1A, 0x00, 0xAB, 0x00},
  {0x1B, 0x00, 0x93, 0x3B},
  {0x1C, 0x00, 0x83, 0x8B},
  {0x1D, 0x00, 0x00, 0x00},
  {0x1E, 0x00, 0x00, 0x00},
  {0x1F, 0x00, 0x00, 0x00},
  {0x20, 0xFF, 0xFF, 0xFF},
  {0x21, 0x3F, 0xBF, 0xFF},
  {0x22, 0x5F, 0x97, 0xFF},
  {0x23, 0xA7, 0x8B, 0xFD},
  {0x24, 0xF7, 0x7B, 0xFF},
  {0x25, 0xFF, 0x77, 0xB7},
  {0x26, 0xFF, 0x77, 0x63},
  {0x27, 0xFF, 0x9B, 0x3B},
  {0x28, 0xF3, 0xBF, 0x3F},
  {0x29, 0x83, 0xD3, 0x13},
  {0x2A, 0x4F, 0xDF, 0x4B},
  {0x2B, 0x58, 0xF8, 0x98},
  {0x2C, 0x00, 0xEB, 0xDB},
  {0x2D, 0x00, 0x00, 0x00},
  {0x2E, 0x00, 0x00, 0x00},
  {0x2F, 0x00, 0x00, 0x00},
  {0x30, 0xFF, 0xFF, 0xFF},
  {0x31, 0xAB, 0xE7, 0xFF},
  {0x32, 0xC7, 0xD7, 0xFF},
  {0x33, 0xD7, 0xCB, 0xFF},
  {0x34, 0xFF, 0xC7, 0xFF},
  {0x35, 0xFF, 0xC7, 0xDB},
  {0x36, 0xFF, 0xBF, 0xB3},
  {0x37, 0xFF, 0xDB, 0xAB},
  {0x38, 0xFF, 0xE7, 0xA3},
  {0x39, 0xE3, 0xFF, 0xA3},
  {0x3A, 0xAB, 0xF3, 0xBF},
  {0x3B, 0xB3, 0xFF, 0xCF},
  {0x3C, 0x9F, 0xFF, 0xF3},
  {0x3D, 0x00, 0x00, 0x00},
  {0x3E, 0x00, 0x00, 0x00},
  {0x3F, 0x00, 0x00, 0x00},

  {-1, 0, 0, 0}
};

UInt16 NesControl[9] = {stopCmd, restartCmd, configCmd,
                        loadSnapCmd, 0, 0,
                        0, 0, 0};

#define C1 {0, 0x00, 0x00, 0x00}
#define C2 {0, 0xE0, 0xE0, 0xE0}
#define C3 {0, 0x80, 0x80, 0x80}

ButtonDef NesButtonDef[12] = {
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {KEY_SEL, internalFontID, "SEL",  C1, C2, C3},
  {KEY_START, internalFontID, "STA",  C1, C2, C3},

  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},

  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1},
  {0, 0, 0, C1, C1, C1}
};

static char *Mirroring[5] = {"4", "2H", "2V", "1L", "1U"};

static void nes_closecart(NesGlobals *g) SECTION("machine");
static FileRef nes_opensram(char *name, UInt32 mode) SECTION("machine");

UInt32 nes_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_VSYNC:
      nes_vsync(arg->a1);
  }
  return 0;
}

void nes_select(MachineType *machine, Hardware *_hardware, UInt8 ramsize) {
  NesGlobals *g;
  UInt16 len, i, index;
  UInt32 aux;
  DmOpenRef dbRef;
  Err err;

  id = machine->id;
  hardware = _hardware;

  len = sizeof(prefs);
  MemSet(&prefs, len, 0);

  if (PrefGetAppPreferences(AppID, 128 + id, &prefs, &len, true) ==
         noPreferenceFound) {
    prefs.joystick = 0;
    prefs.button1 = keyBitHard1;
    prefs.button2 = keyBitHard2;
    PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, len, true);
  }

  hardware->key = 0;
  hardware->joyx = hardware->joyy = 32;

  if (!hardware->globals) {
    hardware->globals = MemPtrNew(sizeof(NesGlobals));
    MemSet(hardware->globals, sizeof(NesGlobals), 0);
  }
  g = hardware->globals;

  g->hardware = hardware;
  g->joystick = prefs.joystick;
  g->button1 = prefs.button1;
  g->button2 = prefs.button2;
  g->cart = 0;
  g->line = 0;
  g->buffer = MemPtrNew(256L*240);
  g->abuffer = g->buffer;
  g->vram = hardware->m1;

  hardware->x0 = (hardware->display_width - 256) / 2;
  hardware->y0 = (hardware->display_height - 224 - lowBorder*2) / 2;
  hardware->dx = 256;
  hardware->dy = 224;

  for (i = 0; i < 263; i++) {   // 1 a mais
    // aux = totalcycles no inicio da linha i
    aux = ((i * machine->clock) / machine->vsync) / NES_SCANLINES;
    hardware->ecycle[i] = aux;
  }
  hardware->nevents = NES_SCANLINES;

  cartname[0] = 0;

  DbDeleteByName(PRG_NAME);
  if (DbCreate(PRG_NAME, PRG_TYPE, AppID) == 0) {
    if ((dbRef = DbOpenByName(PRG_NAME, dmModeWrite, &err)) != NULL) {
      for (i = 0; i < PRG_BANKS; i++)
        DbCreateRec(dbRef, &index, 0x4000, 0);
      DbClose(dbRef);
    }
  }

  DbDeleteByName(CHR_NAME);
  if (DbCreate(CHR_NAME, CHR_TYPE, AppID) == 0) {
    if ((dbRef = DbOpenByName(CHR_NAME, dmModeWrite, &err)) != NULL) {
      for (i = 0; i < CHR_BANKS; i++)
        DbCreateRec(dbRef, &index, 0x2000, 0);
      DbClose(dbRef);
    }
  }
}

void nes_init(void) {
  NesGlobals *g;

  g = hardware->globals;
  g->cart = 0;
  cartname[0] = 0;
  hardware->vsync = 0;
  hardware->useevents = 1;
  hardware->vsync_irq = 0;
}

void nes_reset(void) {
  NesGlobals *g;
  UInt16 i, height;
  UInt8 *s;

  g = hardware->globals;

  g->frame = 0;
  g->dirty = 1;
  g->intenable = 1;
  g->phase = 0;
  g->vram_addr = 0;
  g->sp_addr = 0;
  g->hit = 0;
  g->vblank = 0;
  g->joy_strobe = 0;
  g->joy_state = 0;

  g->prg_addr = (0x8000);
  g->prg_size = (0x4000);
  g->chr_size = (0x2000);
  g->prg_bank = 0;
  g->prg_idx = 0;

  g->mmc1_f1 = 0;
  g->mmc1_f2 = 0;
  g->mmc1_f3 = 0;
  g->mmc1_value = 0;
  g->mmc1_count = 0;

  g->v.mono = 0;
  g->v.lumi = 0;
  g->v.double_sp = 0;
  g->v.pattern_sp = 0;
  g->v.pattern_bg = 0;
  g->v.addr_incr = (1);
  g->v.show_sp = 0;
  g->v.show_bg = 0;
  g->v.clip_sp = 0;
  g->v.clip_bg = 0;
  g->v.chr0 = 0;
  g->v.chr1 = 0;
  g->v.tmp_addr = 0;
  g->v.dx = 0;

  for (i = 0; i < 8; i++) {
    g->v.chr_idx[i] = 0;
  }

  MemSet(g->a, 240 * sizeof(NesVar), 0);
  MemSet(g->sp_ram, 256, 0);
  MemSet(g->buffer, 256L*240, 0);
  MemSet(hardware->m1, 0x4000, 0);	// VRAM

  height = hardware->display_height - lowBorder*2;
  s = WinScreenLock(winLockCopy);

  for (i = 0; i < height; i++, s += hardware->display_width) {
    MemSet(s, hardware->display_width, 255);
  }

  WinScreenUnlock();

  if (!g->cart) {
    g->mapper = 0;
    g->mmc1_mode = 0;
    g->v.mirror = MIRROR_2H;

    hardware->banksw[0] = 0;
    hardware->banksw[1] = 0;
    hardware->banksw[2] = 0;

    nes_closecart(g);

    for (i = 0; i < 16; i++)
      hardware->p[i] = (&hardware->m0[0x5000]);
  }

  switch (g->v.mirror) {
    case MIRROR_4:
      g->ntram[0] = (&hardware->m1[0x2000]);
      g->ntram[1] = (&hardware->m1[0x2400]);
      g->ntram[2] = (&hardware->m1[0x2800]);
      g->ntram[3] = (&hardware->m1[0x2C00]);
      break;
    case MIRROR_2H:
      g->ntram[0] = (&hardware->m1[0x2000]);
      g->ntram[1] = (&hardware->m1[0x2000]);
      g->ntram[2] = (&hardware->m1[0x2400]);
      g->ntram[3] = (&hardware->m1[0x2400]);
      break;
    case MIRROR_2V:
      g->ntram[0] = (&hardware->m1[0x2000]);
      g->ntram[1] = (&hardware->m1[0x2400]);
      g->ntram[2] = (&hardware->m1[0x2000]);
      g->ntram[3] = (&hardware->m1[0x2400]);
      break;
    case MIRROR_1L:
      g->ntram[0] = (&hardware->m1[0x2000]);
      g->ntram[1] = (&hardware->m1[0x2000]);
      g->ntram[2] = (&hardware->m1[0x2000]);
      g->ntram[3] = (&hardware->m1[0x2000]);
      break;
    case MIRROR_1U:
      g->ntram[0] = (&hardware->m1[0x2400]);
      g->ntram[1] = (&hardware->m1[0x2400]);
      g->ntram[2] = (&hardware->m1[0x2400]);
      g->ntram[3] = (&hardware->m1[0x2400]);
  }
}

void nes_finish(void) {
  NesGlobals *g;
  FileRef rf;
  UInt32 r;

  PrefSetAppPreferences(AppID, 128 + id, 1, &prefs, sizeof(prefs), true);

  g = hardware->globals;

  if (g) {
    if (g->cart && cartname[0] && hardware->banksw[0]) {
      // Save current cart SRAM

      if ((rf = nes_opensram(cartname,
            vfsModeWrite|vfsModeCreate|vfsModeTruncate)) != NULL) {
        VFSFileWrite(rf, 0x2000, &hardware->m0[0x6000], &r);
        VFSFileClose(rf);
      }
    }

    nes_closecart(g);

    if (g->buffer)
      MemPtrFree(g->buffer);

    MemPtrFree(g);
    hardware->globals = NULL;
    hardware->globals = NULL;
  }
}

void nes_osd(UInt32 delay) {
  NesGlobals *g;
  UInt16 sram, sprite_pattern, bg_pattern, lumi, prg_count, chr_count;
  UInt16 aux, mmc1_mode, prg_bank, prg_size, chr_size;
  char *mirror, *sprite_size, mono, prg_addr, prg_idx;
  UInt32 midframe_changes;

  g = hardware->globals;
  mirror = Mirroring[g->v.mirror];
  prg_count = (g->prg_count);
  chr_count = (g->chr_count);
  sram = hardware->banksw[0] ? 1 : 0;
  sprite_size = g->v.double_sp ? "16" : "8 ";
  sprite_pattern = g->v.pattern_sp ? 1 : 0;
  bg_pattern = g->v.pattern_bg ? 1 : 0;
  mono = g->v.mono ? 'M' : 'C';
  lumi = (g->v.lumi);
  midframe_changes = hardware->banksw[2];

  status(0, 0, "%4ld", delay);
  status(0, 1, "MI=%s", mirror);
  status(0, 2, "MA=%ld", g->mapper);
  status(0, 3, "PR=%d", prg_count);
  status(0, 4, "CH=%d", chr_count);
  status(0, 5, "RA=%d", sram);
  status(0, 6, "SP=%s", sprite_size);
  status(0, 7, "PS=%d", sprite_pattern);
  status(0, 8, "PB=%d", bg_pattern);
  status(0, 9, "MO=%c", mono);
  status(0, 10, "LU=%d", lumi);
  status(0, 11, "%5ld", midframe_changes);

  if (g->mapper == 1) {
    mmc1_mode = (g->mmc1_mode);
    prg_bank = (g->prg_bank);
    prg_idx = (g->prg_idx);
    if (prg_idx < 10)
      prg_idx += '0';
    else
      prg_idx += 'A' - 10;
    aux = (g->prg_addr);
    prg_addr = (aux == 0x8000) ? '8' : 'C';
    aux = (g->prg_size);
    prg_size = (aux == 0x8000) ? 32 : 16;
    aux = (g->chr_size);
    chr_size = (aux == 0x2000) ? 8 : 4;

    status(48, 0, "MO=%d", mmc1_mode);
    status(48, 1, "BA=%d", prg_bank);
    status(48, 2, "PG=%c", prg_idx);
    status(48, 3, "PA=%c", prg_addr);
    status(48, 4, "PS=%d", prg_size);
    status(48, 5, "CS=%d", chr_size);
  }
}

void nes_debug(void) {
  NesGlobals *g;

  g = hardware->globals;

  DebugInit("ram.bin", true);
  DebugBuffer(hardware->m0, 0x8000);
  DebugFinish();

  DebugInit("vram.bin", true);
  DebugBuffer(hardware->m1, 0x4000);
  DebugFinish();

  DebugInit("chr.bin", true);
  DebugBuffer(hardware->m2, 0x2000);
  DebugFinish();

  DebugInit("spram.bin", true);
  DebugBuffer(g->sp_ram, 0x100);
  DebugFinish();

  DebugInit("ppu.txt", true);
  InfoDialog(D_FILE, "mirroring=%s\n", Mirroring[g->v.mirror]);
  InfoDialog(D_FILE, "bg pattern=0x%04X\n", g->v.pattern_bg ? 0x1000 : 0);
  InfoDialog(D_FILE, "sprite pattern=0x%04X\n", g->v.pattern_sp ? 0x1000 : 0);
  InfoDialog(D_FILE, "sprite height=%d\n", g->v.double_sp ? 16 : 8);
  DebugFinish();
}

static void nes_closecart(NesGlobals *g) {
  UInt16 index;

  if (g->prgRef) {
    for (index = 0; index < PRG_BANKS; index++)
      if (g->prg[index])
        DbCloseRec(g->prgRef, index, (char *)g->prg[index]);
    DbClose(g->prgRef);
  }

  if (g->chrRef) {
    for (index = 0; index < CHR_BANKS; index++)
      if (g->chr[index])
        DbCloseRec(g->chrRef, index, (char *)g->chr[index]);

    DbClose(g->chrRef);
  }

  for (index = 0; index < PRG_BANKS; index++)
    g->prg[index] = NULL;

  for (index = 0; index < CHR_BANKS; index++)
    g->chr[index] = NULL;

  g->prgRef = NULL;
  g->chrRef = NULL;
}

void nes_key(UInt16 c) {
  hardware->key = c;
}

void nes_joystick(UInt16 x, UInt16 y) {
  hardware->joyx = x;
  hardware->joyy = y;
}

/*
iNES Format (.NES)

+--------+------+------------------------------------------+
| Offset | Size | Content(s)                               |
+--------+------+------------------------------------------+
|   0    |  3   | 'NES'                                    |
|   3    |  1   | $1A                                      |
|   4    |  1   | 16K PRG-ROM page count                   |
|   5    |  1   | 8K CHR-ROM page count                    |
|   6    |  1   | ROM Control Byte #1                      |
|        |      |   %####vTsM                              |
|        |      |    |  ||||+- 0=Horizontal Mirroring      |
|        |      |    |  ||||   1=Vertical Mirroring        |
|        |      |    |  |||+-- 1=SRAM enabled              |
|        |      |    |  ||+--- 1=512-byte trainer present  |
|        |      |    |  |+---- 1=Four-screen VRAM layout   |
|        |      |    |  |                                  |
|        |      |    +--+----- Mapper # (lower 4-bits)     |
|   7    |  1   | ROM Control Byte #2                      |
|        |      |   %####0000                              |
|        |      |    |  |                                  |
|        |      |    +--+----- Mapper # (upper 4-bits)     |
|  8-15  |  8   | $00                                      |
| 16-..  |      | Actual 16K PRG-ROM pages (in linear      |
|  ...   |      | order). If a trainer exists, it precedes |
|  ...   |      | the first PRG-ROM bank.                  |
| ..-EOF |      | CHR-ROM pages (in ascending order).      |
+--------+------+------------------------------------------+
*/

Err nes_readcart(FileRef f, m6502_Regs *m6502, Hardware *hardware, char *name) {
  NesGlobals *g;
  UInt8 header[16], prg_count, chr_count, ram_count, rom_ctrl1, rom_ctrl2;
  UInt32 size, r;
  UInt16 mapper, index;
  FileRef rf;
  Int16 i;
  Err err;

  g = hardware->globals;

  if (g->cart && cartname[0] && hardware->banksw[0]) {
    // Save current cart SRAM

    if ((rf = nes_opensram(cartname,
          vfsModeWrite|vfsModeCreate|vfsModeTruncate)) != NULL) {
      VFSFileWrite(rf, 0x2000, &hardware->m0[0x6000], &r);
      VFSFileClose(rf);
    }
  }

  cartname[0] = 0;
  nes_closecart(g);
  g->cart = 0;

  for (i = 0; i < 16; i++)
    hardware->p[i] = (&g->hardware->m0[0x5000]);

  if ((err = VFSFileSize(f, &size)) != 0)
    return -1;

  if (size < 16)
    return -2;

  if ((err = VFSFileRead(f, 16, header, &r)) != 0)
    return -3;

  if (r != 16)
    return -4;

  if (header[0] != 'N' || header[1] != 'E' || header[2] != 'S' ||
      header[3] != 0x1A)
    return -5;

  prg_count = header[4];
  chr_count = header[5];
  rom_ctrl1 = header[6];
  rom_ctrl2 = header[7];
  ram_count = header[8];
  mapper = (rom_ctrl1 >> 4) | (rom_ctrl2 & 0xF0);

  if (prg_count == 2 && chr_count == 1 && (rom_ctrl1 & 0xF0) == 0) {
    // alguns arquivos tem lixo do byte 7 ao 15 (Ex.: 1942.zip)
    rom_ctrl2 = 0;
    ram_count = 0;
    mapper = 0;
  }

  switch (mapper) {
    case 0:	// none
      if (prg_count < 1 || prg_count > 2 || chr_count != 1)
        return -6;
      break;
    case 1:	// MMC1 (up to 16 CHR-ROM banks and 64 PRG-ROM banks)
      if (prg_count < 2 || prg_count > 64 || chr_count > 16)
        return -6;

      if (prg_count < 16)		// 256K carts
        g->mmc1_mode = 0;
      else if (prg_count < 32)
        g->mmc1_mode = (1);	// 512K carts
      else
        g->mmc1_mode = (2);	// 1024K carts

      break;
    case 2:	// UNROM (up to 16 PRG-ROM banks)
      if (prg_count < 2 || prg_count > 16 || chr_count != 0)
        return -6;
      break;
    case 3:	// CNROM (up to 4 CHR-ROM banks)
      if (prg_count < 1 || prg_count > 2 || chr_count < 2 || chr_count > 4)
        return -6;
      break;
    case 7:	// AOROM (up to 32 PRG-ROM banks)
      if (prg_count < 2 || prg_count > 32 || chr_count != 0)
        return -6;
      break;
    default:
      return -7;
  }

  if (ram_count != 0)
    return -8;

  if (rom_ctrl1 & 0x04)	{ // 0x200 bytes trainer at 7000-71FF
    if ((err = VFSFileRead(f, 0x200, &hardware->m0[0x7000], &r)) != 0)
      return -9;

    if (r != 0x200)
      return -10;
  }

  if ((g->prgRef = DbOpenByName(PRG_NAME, dmModeReadOnly, &err)) == NULL)
    return -11;

  for (index = 0; index < prg_count; index++) {
    if ((g->prg[index] = (unsigned char *)DbOpenRec(g->prgRef, index, &err)) == NULL) {
      nes_closecart(g);
      return -12;
    }

    if ((err = VFSFileReadData(f, 0x4000, g->prg[index], 0, &r)) != 0 ||
        r != 0x4000) {
      nes_closecart(g);
      return -13;
    }
  }

  if ((g->chrRef = DbOpenByName(CHR_NAME, dmModeReadOnly, &err)) == NULL) {
    nes_closecart(g);
    return -14;
  }

  for (index = 0; index < chr_count; index++) {
    if ((g->chr[index] = (unsigned char *)DbOpenRec(g->chrRef, index, &err)) == NULL) {
      nes_closecart(g);
      return -15;
    }

    if ((err = VFSFileReadData(f, 0x2000, g->chr[index], 0, &r)) != 0 ||
        r != 0x2000) {
      nes_closecart(g);
      return -16;
    }
  }

  MemSet(cartname, sizeof(cartname), 0);
  StrNCopy(cartname, name, sizeof(cartname)-1);

  if (rom_ctrl1 & 0x02) {
    if ((rf = nes_opensram(cartname, vfsModeRead)) != NULL) {
      // Load SRAM at 0x6000-0x8000

      VFSFileRead(rf, 0x2000, &hardware->m0[0x6000], &r);
      VFSFileClose(rf);

    } else {
      // Erase SRAM at 0x6000-0x8000
      MemSet(&hardware->m0[0x6000], 0x2000, 0);

      // Create SRAM filE
      if ((rf = nes_opensram(cartname, vfsModeWrite|vfsModeCreate)) != NULL) {
        VFSFileWrite(rf, 0x2000, &hardware->m0[0x6000], &r);
        VFSFileClose(rf);
      }
    }
  }

  // Map first 16K ROM at 8000
  hardware->p[0x08] = (&g->prg[0][0x0000]);
  hardware->p[0x09] = (&g->prg[0][0x1000]);
  hardware->p[0x0A] = (&g->prg[0][0x2000]);
  hardware->p[0x0B] = (&g->prg[0][0x3000]);

  if (mapper == 7) {
    // Map second 16K ROM at C000
    hardware->p[0x0C] = (&g->prg[1][0x0000]);
    hardware->p[0x0D] = (&g->prg[1][0x1000]);
    hardware->p[0x0E] = (&g->prg[1][0x2000]);
    hardware->p[0x0F] = (&g->prg[1][0x3000]);
  } else {
    // Map last 16K ROM at C000
    hardware->p[0x0C] = (&g->prg[prg_count-1][0x0000]);
    hardware->p[0x0D] = (&g->prg[prg_count-1][0x1000]);
    hardware->p[0x0E] = (&g->prg[prg_count-1][0x2000]);
    hardware->p[0x0F] = (&g->prg[prg_count-1][0x3000]);
  }

  // Map PPU 0000-1FFF

  if (chr_count == 0) {
    hardware->p[0] = (&hardware->m2[0x0000]);
    hardware->p[1] = (&hardware->m2[0x1000]);

    for (i = 0; i < 8; i++)
      g->vrom[i] = (&hardware->m2[i * 0x400]);

  } else {
    hardware->p[0] = (&g->chr[0][0x0000]);
    hardware->p[1] = (&g->chr[0][0x1000]);

    for (i = 0; i < 8; i++)
      g->vrom[i] = (&g->chr[0][i * 0x400]);
  }

  hardware->banksw[0] = (rom_ctrl1 & 0x02) ? 1 : 0;	// SRAM
  hardware->banksw[1] = mapper;
  g->mapper = mapper;

  g->cart = 1;
  g->dirty = 1;

  if (mapper == 7)
    g->v.mirror = MIRROR_1L;
  else {
    if (rom_ctrl1 & 0x08)
      g->v.mirror = MIRROR_4;
    else if (rom_ctrl1 & 0x01)
      g->v.mirror = MIRROR_2V;
    else
      g->v.mirror = MIRROR_2H;
  }

  g->prg_count = prg_count;
  g->chr_count = chr_count;

  return 0;
}

static FileRef nes_opensram(char *name, UInt32 mode) {
  Int16 vol, n;
  FileRef f;

  if ((n = StrLen(name)) < 4)
    return NULL;

  if (StrNCaselessCompare(&name[n - 4], ".nes", 4) != 0)
    return NULL;

  if ((vol = check_volume()) == -1)
    return NULL;

  MemSet(buf, sizeof(buf), 0);
  StrCopy(buf, AppDir);
  StrCat(buf, "/Snapshot/");
  StrCat(buf, "nes");
  StrCat(buf, "/");
  StrCat(buf, name);

  n = StrLen(buf);
  StrCopy(&buf[n - 4], ".ram");

  VFSFileOpen(vol, buf, mode, &f);
  return f;
}

NesPrefs *NesGetPrefs(void) {
  return &prefs;
}

Boolean NesFormHandler(EventPtr event, Boolean *close) {
  FormPtr frm;
  ListPtr lst;
  UInt32 button;
  NesGlobals *g;
  Boolean handled;

  frm = FrmGetActiveForm();
  handled = false;
  *close = false;

  switch (event->eType) {
    case frmOpenEvent:
      if (prefs.joystick == 0)
        FrmSetControlValue(frm, FrmGetObjectIndex(frm, leftCtl), 1);
      else
        FrmSetControlValue(frm, FrmGetObjectIndex(frm, rightCtl), 1);

      switch (prefs.button1) {
        case keyBitHard1: button = 0; break;
        case keyBitHard2: button = 1; break;
        case keyBitHard3: button = 2; break;
        case keyBitHard4: button = 3; break;
        //case keyBitJogPress: button = 4; break;
        //case keyBitRockerCenter: button = 5; break;
        default: button = 0;
      }

      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, buttonList));
      LstSetSelection(lst, button);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
         FrmGetObjectIndex(frm, buttonCtl)),
         LstGetSelectionText(lst, button));

      switch (prefs.button2) {
        case keyBitHard1: button = 0; break;
        case keyBitHard2: button = 1; break;
        case keyBitHard3: button = 2; break;
        case keyBitHard4: button = 3; break;
        //case keyBitJogPress: button = 4; break;
        //case keyBitRockerCenter: button = 5; break;
        default: button = 0;
      }

      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, button2List));
      LstSetSelection(lst, button);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
         FrmGetObjectIndex(frm, button2Ctl)),
         LstGetSelectionText(lst, button));

      FrmDrawForm(frm);
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case leftCtl:
          prefs.joystick = 0;
          handled = true;
          break;
        case rightCtl:
          prefs.joystick = 1;
          handled = true;
          break;
        case okBtn:
          *close = true;
          handled = true;
      }
      break;

    case popSelectEvent:
      switch (event->data.popSelect.listID) {
        case buttonList:
          switch (event->data.popSelect.selection) {
            case 0: prefs.button1 = keyBitHard1; break;
            case 1: prefs.button1 = keyBitHard2; break;
            case 2: prefs.button1 = keyBitHard3; break;
            case 3: prefs.button1 = keyBitHard4; break;
            //case 4: prefs.button1 = keyBitJogPress; break;
            //case 5: prefs.button1 = keyBitRockerCenter; break;
            default: prefs.button1 = keyBitHard1;
          }
          g = hardware->globals;
          g->button1 = prefs.button1;
          break;
        case button2List:
          switch (event->data.popSelect.selection) {
            case 0: prefs.button2 = keyBitHard1; break;
            case 1: prefs.button2 = keyBitHard2; break;
            case 2: prefs.button2 = keyBitHard3; break;
            case 3: prefs.button2 = keyBitHard4; break;
            //case 4: prefs.button2 = keyBitJogPress; break;
            //case 5: prefs.button2 = keyBitRockerCenter; break;
            default: prefs.button2 = keyBitHard1;
          }
          g = hardware->globals;
          g->button2 = prefs.button2;
      }

    default:
      break;
  }

  return handled;
}
