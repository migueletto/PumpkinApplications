typedef struct {
  UInt8 crt;
  UInt16 font;
} CpmPrefs;

extern ColorType CpmColor[5];
extern UInt16 CpmControl[9];
extern ButtonDef CpmButtonDef[12];

void cpm_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void cpm_finish(void) SECTION("machine");
void cpm_key(UInt16 c) SECTION("machine");
UInt8 cpm_readb(UInt8 *r1, UInt8 *r2, UInt16 a);
void cpm_writeb(UInt8 *r1, UInt8 *r2, UInt16 a, UInt8 b);
void cpm_cls(void) SECTION("machine");

CpmPrefs *CpmGetPrefs(void) SECTION("machine");
Boolean CpmFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 cpm_callback(ArmletCallbackArg *arg);
