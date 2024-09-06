typedef struct {
  UInt8 joystick;
  UInt32 button;
} OricPrefs;

typedef struct {
  UInt32 frame;
  UInt32 area_height;
  UInt32 border;
  UInt32 text;
  UInt32 blink_status;
  UInt32 ier[1];
  UInt32 ifr[1];
  UInt32 ddra[1];
  UInt32 ddrb[1];
  UInt32 pcr[1];
  UInt32 kbselect1;
  UInt32 kbselect2;
  UInt32 latch1;
  UInt32 counter1;
  UInt32 psg;
  UInt32 motor;
  UInt32 button;
  UInt32 ctrl;
  AY8910 ay8910;
  UInt8 *video;
  ArmKeyMap keyMap[256];
} OricGlobals;

extern ColorType OricColor[9];
extern UInt16 OricControl[9];
extern ButtonDef OricButtonDef[12];

void oric_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void oric_finish(void) SECTION("machine");
void oric_key(UInt16 c) SECTION("machine");
void oric_joystick(UInt16 x, UInt16 y) SECTION("machine");
Err oric_readtap(FileRef f, m6502_Regs *m6502, Hardware *hardware) SECTION("machine");

OricPrefs *OricGetPrefs(void) SECTION("machine");
Boolean OricFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 oric_callback(ArmletCallbackArg *arg);
UInt8 oric_readb(UInt16 a);
void oric_writeb(UInt16 a, UInt8 b);
