#define SNAP_REGS	0x0001
#define SNAP_STATE	0x0002
#define SNAP_RAM	0x0003
#define SNAP_ROM	0x0004
#define SNAP_IO		0x0005

#define NUM_REGS	20

enum {REG_PC, REG_D, REG_X, REG_Y, REG_U, REG_S, REG_DPCC};

typedef struct {
  UInt16 r[NUM_REGS];
} SnapRegs;

typedef struct {
  UInt8 machine, ramsize;
  UInt16 samreg;
  UInt8 irq_state0, irq_state1, int_state, nmi_state;
  UInt8 border;
} SnapState;

Err write_snap(FileRef f, SnapState *state, SnapRegs *regs, Hardware *hardware) SECTION("aux");
Err read_snap(FileRef f, SnapState *state, SnapRegs *regs, Hardware *hardware) SECTION("aux");
