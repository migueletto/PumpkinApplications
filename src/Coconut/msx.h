typedef struct {
  UInt8 joystick, button1, button2;
} MsxPrefs;

typedef struct {
  UInt32 ctrl;
  UInt32 kbselect;
  UInt32 joyselect;
  UInt32 joystick, button1, button2;
  AY8910 ay8910;
  TI9918 ti9918;
  ArmKeyMap keyMap[256];
} MsxGlobals;

extern UInt16 MsxControl[9];
extern ButtonDef MsxButtonDef[12];

void msx_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void msx_reset(void) SECTION("machine");
void msx_finish(void) SECTION("machine");
void msx_osd(UInt32 delay) SECTION("machine");
void msx_debug(void) SECTION("machine");
void msx_key(UInt16 c) SECTION("machine");
void msx_joystick(UInt16 x, UInt16 y) SECTION("machine");
Err msx_readcart(FileRef f, z80_Regs *z80, Hardware *hardware) SECTION("machine");

MsxPrefs *MsxGetPrefs(void) SECTION("machine");
Boolean MsxFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 msx_callback(ArmletCallbackArg *arg);
UInt8 msx_readb(UInt16 a);
void msx_writeb(UInt16 a, UInt8 b);
