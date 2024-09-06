typedef struct {
  UInt8 joystick, button;
} ColecoPrefs;

typedef struct {
  UInt32 joymode, joystick, button;
  SN76489 sn76489;
  TI9918 ti9918;
  ArmKeyMap keyMap[256];
} ColecoGlobals;

extern UInt16 ColecoControl[9];
extern ButtonDef ColecoButtonDef[12];

void coleco_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void coleco_init(void) SECTION("machine");
void coleco_reset(void) SECTION("machine");
void coleco_finish(void) SECTION("machine");
void coleco_key(UInt16 c) SECTION("machine");
void coleco_joystick(UInt16 x, UInt16 y) SECTION("machine");
Err coleco_readcart(FileRef f, z80_Regs *z80, Hardware *hardware) SECTION("machine");

ColecoPrefs *ColecoGetPrefs(void) SECTION("machine");
Boolean ColecoFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 coleco_callback(ArmletCallbackArg *arg);
UInt8 coleco_readb(UInt16 a);
void coleco_writeb(UInt16 a, UInt8 b);
