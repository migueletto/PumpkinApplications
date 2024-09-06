typedef struct {
  UInt8 pad;
} AquariusPrefs;

typedef struct {
  UInt32 frame;
  UInt32 ctrl;
  UInt8 *font;
  ArmKeyMap keyMap[256];
} AquariusGlobals;

extern ColorType AquariusColor[17];
extern UInt16 AquariusControl[9];
extern ButtonDef AquariusButtonDef[12];

void aquarius_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void aquarius_init(void) SECTION("machine");
void aquarius_finish(void) SECTION("machine");
void aquarius_key(UInt16 c) SECTION("machine");
Err aquarius_readbin(FileRef f, z80_Regs *z80, Hardware *hardware) SECTION("machine");

AquariusPrefs *AquariusGetPrefs(void) SECTION("machine");
Boolean AquariusFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 aquarius_callback(ArmletCallbackArg *arg);
UInt8 aquarius_readb(UInt16 a);
void aquarius_writeb(UInt16 a, UInt8 b);
