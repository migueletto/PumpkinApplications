typedef struct {
  UInt8 joystick;
  UInt32 button;
} VicPrefs;

typedef struct {
  UInt32 frame;
  UInt32 dirty;
  UInt32 x0;
  UInt32 y0;
  UInt32 width;
  UInt32 height;
  UInt32 area_height;
  UInt32 border;
  UInt32 border_color;
  UInt32 aux_color;
  UInt32 backg_color;
  UInt32 double_char;
  UInt32 char_height;
  UInt32 num_cols;
  UInt32 num_lines;
  UInt32 normal;
  UInt32 char_map;
  UInt32 vic_char_map;
  UInt32 color_ram;
  UInt32 video_color_bit;
  UInt32 video_ram;
  UInt32 video_size;
  UInt32 ier[2];
  UInt32 ifr[2];
  UInt32 ddra[2];
  UInt32 ddrb[2];
  UInt32 kbselect1;
  UInt32 kbselect2;
  UInt32 button;
  UInt32 ctrl;
  UInt32 sound[4];
  UInt8 *video;
  ArmKeyMap keyMap[256];
} VicGlobals;

extern ColorType VicColor[17];
extern UInt16 VicControl[9];
extern ButtonDef VicButtonDef[12];

void vic_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void vic_finish(void) SECTION("machine");
void vic_key(UInt16 c) SECTION("machine");
void vic_joystick(UInt16 x, UInt16 y) SECTION("machine");
Err vic_readprg(FileRef f) SECTION("machine");

VicPrefs *VicGetPrefs(void) SECTION("machine");
Boolean VicFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 vic_callback(ArmletCallbackArg *arg);
UInt8 vic_readb(UInt16 a);
void vic_writeb(UInt16 a, UInt8 b);
