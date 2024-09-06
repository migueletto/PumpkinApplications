typedef struct {
  UInt8 joystick;
  UInt32 button;
} CgeniePrefs;

typedef struct {
  m6845_Regs m6845;
  UInt32 init;
  UInt32 ctrl;
  UInt32 port_ff;
  AY8910 ay8910;
  ArmKeyMap keyMap[256];
} CgenieGlobals;

extern ColorType CgenieColor[22];
extern UInt16 CgenieControl[9];
extern ButtonDef CgenieButtonDef[12];

void cgenie_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void cgenie_init(void) SECTION("machine");
void cgenie_finish(void) SECTION("machine");
void cgenie_key(UInt16 c) SECTION("machine");
void cgenie_joystick(UInt16 x, UInt16 y) SECTION("machine");

CgeniePrefs *CgenieGetPrefs(void) SECTION("machine");
Boolean CgenieFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 cgenie_callback(ArmletCallbackArg *arg);
UInt8 cgenie_readb(UInt16 a);
void cgenie_writeb(UInt16 a, UInt8 b);
