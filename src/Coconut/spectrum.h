#define SPECTRUM_SHIFT_LINE     0xFE
#define SPECTRUM_SHIFT_COLUMN   0xFE

#define SPECTRUM_SSHIFT_LINE    0x7F
#define SPECTRUM_SSHIFT_COLUMN  0xFD

typedef struct {
  UInt8 joystick;
  UInt32 button;
} SpecPrefs;

typedef struct {
  UInt32 flash_count;
  UInt32 flash_status;
  UInt32 border;
  UInt32 border_color;
  UInt32 button;
  UInt8 *video;
  ArmKeyMap keyMap[256];
  UInt16 SpectrumVideoAddr[192];
  UInt16 SpectrumVideoCAddr[192];
  UInt8 *SpectrumInk;
  UInt8 *SpectrumPaper;
} SpecGlobals;

extern ColorType SpectrumColor[17];
extern UInt16 SpectrumControl[9];
extern ButtonDef SpectrumButtonDef[12];

void spectrum_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void spectrum_init(void) SECTION("machine");
void spectrum_finish(void) SECTION("machine");
void spectrum_key(UInt16 c) SECTION("machine");
void spectrum_joystick(UInt16 x, UInt16 y) SECTION("machine");

SpecPrefs *SpecGetPrefs(void) SECTION("machine");
Boolean SpecFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 spectrum_callback(ArmletCallbackArg *arg);
UInt8 spectrum_readb(UInt16 a);
void spectrum_writeb(UInt16 a, UInt8 b);
