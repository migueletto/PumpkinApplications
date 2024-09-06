typedef struct {
  UInt8 id, ramsize;
  UInt8 joystick;
  UInt32 button;
  UInt8 artifacting;
} CocoPrefs;

#define SHIFT_COLUMN    0x7F
#define SHIFT_LINE      0xBF

#define CTRL_COLUMN     0xFE
#define CTRL_LINE       0xBF

enum {COLOR_BLACK, COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE,
      COLOR_RED, COLOR_WHITE, COLOR_CYAN, COLOR_PURPLE,
      COLOR_ORANGE, COLOR_DARKGREEN, COLOR_DARKORANGE, COLOR_BRIGHTORANGE,
      COLOR_GRED};

#define VDG_TEXT        0x00
#define VDG_GRAPHIC     0x80

extern ColorType CocoColor[14];
extern ColorType Cp400Color[14];

extern UInt16 CocoControl[9];
extern ButtonDef CocoButtonDef[12];
extern KeyMap CocoMap[70];
extern KeyMap DragonMap[70];

void coco_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void coco_finish(void) SECTION("machine");
void coco_key(UInt16 c) SECTION("machine");
void coco_joystick(UInt16 x, UInt16 y) SECTION("machine");

CocoPrefs *CocoGetPrefs(void) SECTION("machine");
Boolean CocoFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 coco_callback(ArmletCallbackArg *arg);
