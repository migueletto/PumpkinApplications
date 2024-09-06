#define CSW_BUFSIZE 1024

typedef struct {
  UInt8 joystick;
  UInt32 button;
} Mc1000Prefs;

typedef struct {
  UInt32 shift, ctrl, kbline, bit;
  UInt32 joystick, button;
  AY8910 ay8910;
  ArmKeyMap keyMap[256];

  UInt32 first, value, len, index, size;
  UInt8 *buffer;

} Mc1000Globals;

extern UInt16 MC1000Control[9];
extern ButtonDef MC1000ButtonDef[12];

void mc1000_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void mc1000_init(void) SECTION("machine");
void mc1000_reset(void) SECTION("machine");
void mc1000_finish(void) SECTION("machine");
void mc1000_key(UInt16 c) SECTION("machine");
void mc1000_joystick(UInt16 x, UInt16 y) SECTION("machine");

Mc1000Prefs *Mc1000GetPrefs(void) SECTION("machine");
Boolean Mc1000FormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 mc1000_callback(ArmletCallbackArg *arg);
UInt8 mc1000_readb(UInt16 a);
void mc1000_writeb(UInt16 a, UInt8 b);
UInt8 mc1000_readb(UInt16 a);
