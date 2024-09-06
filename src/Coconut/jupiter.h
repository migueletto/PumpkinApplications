typedef struct {
  UInt8 pad;
} JupiterPrefs;

typedef struct {
  UInt32 frame;
  UInt32 border;
  ArmKeyMap keyMap[256];
} JupiterGlobals;

extern ColorType JupiterColor[3];
extern UInt16 JupiterControl[9];
extern ButtonDef JupiterButtonDef[12];

void jupiter_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void jupiter_finish(void) SECTION("machine");
void jupiter_key(UInt16 c) SECTION("machine");
Err jupiter_readdic(FileRef f, z80_Regs *z80, Hardware *hardware) SECTION("machine");
Err jupiter_readace(FileRef f, z80_Regs *z80, Hardware *hardware) SECTION("machine");

JupiterPrefs *JupiterGetPrefs(void) SECTION("machine");
Boolean JupiterFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 jupiter_callback(ArmletCallbackArg *arg);
void jupiter_writeb(UInt16 a, UInt8 b);
UInt8 jupiter_readb(UInt16 a);
