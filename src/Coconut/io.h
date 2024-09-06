#define IO_GETB    1
#define IO_PUTB    2
#define IO_READB   3
#define IO_WRITEB  4
#define IO_EVENT   5
#define IO_VSYNC   6
#define IO_CART    7
#define IO_SAM     8
#define IO_HALT    9
#define IO_CHECK   10
#define ILLEGAL0   11
#define ILLEGAL1   12
#define ILLEGAL2   13
#define IO_CASBUF  14
#define IO_DEBUG   128

void io_kbselect(UInt8);
void io_daset(UInt8 b);
void io_motor(UInt8 b);
void io_sel2(UInt8 b);
void io_sel1(UInt8 b);
void io_snd(UInt8 b);
void io_snd1bit(UInt8 b);
void io_vdg(UInt8 b);
void io_sam(UInt8 b);
void io_samreg(UInt8 b);
void io_bank(UInt8 b);
void io_hsync(UInt8 b);
void io_cart(UInt8 b);
UInt8 io_casin(void);
UInt8 io_serin(void);
void io_setvdg(void);
void io_setgp(UInt32 b, UInt32 e);
void io_halt(void);
void io_run(void);

void coco_writeb(UInt16 a, UInt8 b);
UInt8 coco_readb(UInt16 a);
UInt8 coco_kbline(void);
void coco_vsync(UInt8 b);

void apple_vsync(UInt8 b);
void mc10_vsync(UInt8 b);
void vz_vsync(UInt8 b);
void atom_vsync(UInt8 b);

void spectrum_vsync(UInt8 b);
void vic_vsync(UInt8 b);
void oric_vsync(UInt8 b);
void cgenie_vsync(UInt8 b);
void jupiter_vsync(UInt8 b);
void aquarius_vsync(UInt8 b);
void msx_vsync(UInt8 b);
void coleco_vsync(UInt8 b);
void mc1000_vsync(UInt8 b);
void nes_vsync(UInt8 b);

UInt8 io_readb(ArmletArg *arg, UInt16 a);
void io_writeb(ArmletArg *arg, UInt16 a, UInt8 b);
void io_setb(ArmletArg *arg, UInt32, UInt32, UInt32);

#define IO_readb(p,b) io_readb(p->arg, (b))
#define IO_writeb(p,b1,b2) io_writeb(p->arg, (b1), (b2))

#define IO_event(p,a1,a2) io_setb(p->arg, IO_EVENT, a1, a2)
#define IO_vsync(p,b) io_setb(p->arg, IO_VSYNC, b, 0)
#define IO_cart(p,b) io_setb(p->arg, IO_CART, b, 0)
#define IO_sam(p,b) io_setb(p->arg, IO_SAM, b, 0)
#define IO_bank(p,b) io_setb(p->arg, IO_BANK, b, 0)
#define IO_halt(p,b) io_setb(p->arg, IO_HALT, b, 0)
#define ILLEGAL(p,t,b) io_setb(p->arg, t, b, 0)
#define IO_debug(p,b) io_setb(p->arg, IO_DEBUG, b, 0)
#define IO_check(p,b) io_readb(p->arg, 0xFFFF)
