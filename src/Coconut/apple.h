#define APPLE_HCOL	'hcol'
#define APPLE_HCOL_ID	1

#define APPLE_GRAPH	0x01
#define APPLE_HIRES	0x02
#define APPLE_MIXED	0x04
#define APPLE_PAGE2	0x08

typedef struct {
  UInt8 joystick;
  UInt32 button1, button2;
} ApplePrefs;

extern ColorType AppleColor[17];
extern UInt16 AppleControl[9];
extern ButtonDef AppleButtonDef[12];

void apple_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void apple_init(void) SECTION("machine");
void apple_reset(void) SECTION("machine");
void apple_finish(void) SECTION("machine");
void apple_key(UInt16 c) SECTION("machine");
void apple_joystick(UInt16 x, UInt16 y) SECTION("machine");
void apple_writeb(UInt16 a, UInt8 b);
UInt8 apple_readb(UInt16 a);
void apple_video(Boolean dirty) SECTION("machine");

ApplePrefs *AppleGetPrefs(void) SECTION("machine");
Boolean AppleFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 apple_callback(ArmletCallbackArg *arg);
