typedef struct {
  UInt8 joystick;
  UInt32 button1, button2;
} VzPrefs;

extern UInt16 VZ300Control[9];
extern ButtonDef VZ300ButtonDef[12];

void vz_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void vz_init(void) SECTION("machine");
void vz_finish(void) SECTION("machine");
void vz_key(UInt16 c) SECTION("machine");
void vz_joystick(UInt16 x, UInt16 y) SECTION("machine");
void vz_writeb(UInt16 a, UInt8 b);
UInt8 vz_readb(UInt16 a);
Err vz_readvz(FileRef f, z80_Regs *z80, Hardware *hardware) SECTION("machine");

VzPrefs *VzGetPrefs(void) SECTION("machine");
Boolean VzFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 vz_callback(ArmletCallbackArg *arg);
