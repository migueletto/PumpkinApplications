typedef struct {
  UInt8 pad;
} AtomPrefs;

extern UInt16 AtomControl[9];
extern ButtonDef AtomButtonDef[12];

void atom_select(MachineType *machine, Hardware *hardware, UInt8 ramsize) SECTION("machine");
void atom_init(void) SECTION("machine");
void atom_finish(void) SECTION("machine");
void atom_key(UInt16 c) SECTION("machine");
Err atom_readatm(FileRef f, m6502_Regs *m6502, Hardware *hardware) SECTION("machine");

AtomPrefs *AtomGetPrefs(void) SECTION("machine");
Boolean AtomFormHandler(EventPtr event, Boolean *close) SECTION("machine");

UInt32 atom_callback(ArmletCallbackArg *arg);
void atom_writeb(UInt16 a, UInt8 b);
UInt8 atom_readb(UInt16 a);
