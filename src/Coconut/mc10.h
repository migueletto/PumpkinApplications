typedef struct {
  UInt8 joystick;
  UInt32 button;
  UInt8 artifacting;
} Mc10Prefs;

extern UInt16 MC10Control[9];
extern ButtonDef MC10ButtonDef[12];

void mc10_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void mc10_init(void) SECTION("machine");
void mc10_finish(void) SECTION("machine");
void mc10_key(UInt16 c) SECTION("machine");
void mc10_joystick(UInt16 x, UInt16 y) SECTION("machine");
void mc10_writeb(UInt16 a, UInt8 b);
UInt8 mc10_readb(UInt16 a);

Mc10Prefs *Mc10GetPrefs(void) SECTION("machine");
Boolean Mc10FormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 mc10_callback(ArmletCallbackArg *arg);
