#define PRG_BANKS	64
#define CHR_BANKS	16

#define NES_SCANLINES	262

#define PRG_NAME  "prg.nes"
#define PRG_TYPE  'PRGr'

#define CHR_NAME  "chr.nes"
#define CHR_TYPE  'CHRr'

#define MIRROR_4  0
#define MIRROR_2H 1
#define MIRROR_2V 2
#define MIRROR_1L 3
#define MIRROR_1U 4

typedef struct {
  UInt32 mono, lumi, double_sp, pattern_sp, pattern_bg;
  UInt32 addr_incr, show_sp, show_bg, clip_sp, clip_bg;
  UInt32 mirror, chr_idx[8], chr0, chr1, tmp_addr, dx;
} NesVar;

typedef struct {
  Hardware *hardware;
  UInt32 frame, dirty, cart, line, vblank, mapper;
  UInt32 intenable, phase, vram_addr, sp_addr, hit;
  UInt32 joystick, button1, button2, joy_strobe, joy_state;
  UInt32 prg_count, prg_size, prg_addr, chr_count, chr_size, prg_bank, prg_idx;
  UInt32 mmc1_mode, mmc1_count, mmc1_value, mmc1_f1, mmc1_f2, mmc1_f3;
  NesVar v, a[NES_SCANLINES];
  UInt8 sp_ram[256], *vram, *ntram[4], *vrom[8];
  UInt8 *prg[PRG_BANKS], *chr[CHR_BANKS];
  void *prgRef, *chrRef;
  UInt8 *buffer, *abuffer;
} NesGlobals;

typedef struct {
  UInt8 joystick, button1, button2;
} NesPrefs;

extern ColorType NesColor[65];
extern UInt16 NesControl[9];
extern ButtonDef NesButtonDef[12];

void nes_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void nes_init(void) SECTION("machine");
void nes_reset(void) SECTION("machine");
void nes_finish(void) SECTION("machine");
void nes_osd(UInt32 delay) SECTION("machine");
void nes_dump(void) SECTION("machine");
void nes_key(UInt16 c) SECTION("machine");
void nes_joystick(UInt16 x, UInt16 y) SECTION("machine");
Err nes_readcart(FileRef f, m6502_Regs *m6502, Hardware *hardware, char *name) SECTION("machine");
void nes_debug(void) SECTION("machine");

NesPrefs *NesGetPrefs(void) SECTION("machine");
Boolean NesFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 nes_callback(ArmletCallbackArg *arg);
UInt8 nes_readb(UInt16 a);
void nes_writeb(UInt16 a, UInt8 b);
