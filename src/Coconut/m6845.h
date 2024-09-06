typedef struct {
  UInt32 index;
  UInt32 horizontal_total;
  UInt32 horizontal_displayed;
  UInt32 horizontal_sync_pos;
  UInt32 horizontal_length;
  UInt32 vertical_total;
  UInt32 vertical_adjust;
  UInt32 vertical_displayed;
  UInt32 vertical_sync_pos;
  UInt32 crt_mode;
  UInt32 scan_lines;
  UInt32 cursor_top;
  UInt32 cursor_bottom;
  UInt32 screen_address;
  UInt32 cursor_address;
  UInt32 dirty;
  UInt32 frame;
  UInt32 border;
  UInt32 mode;
  UInt32 cursor_visible;
  Int32 off_x, off_y;
  UInt32 bgcolor;
  Int32 fs0, fs1;
  UInt8 *vram;
  UInt8 *cram;
  UInt8 *font;
} m6845_Regs;

void m6845_init(m6845_Regs *m6845, UInt8 *vram, UInt8 *cram, UInt8 *font);

void m6845_setmode(m6845_Regs *m6845, UInt32 mode);
void m6845_setfs(m6845_Regs *m6845, UInt32 fs, UInt32 value);
void m6845_setbg(m6845_Regs *m6845, UInt32 bg);
void m6845_setindex(m6845_Regs *m6845, UInt32 index);
void m6845_setvalue(m6845_Regs *m6845, UInt32 value);
UInt32 m6845_getindex(m6845_Regs *m6845);
UInt32 m6845_getvalue(m6845_Regs *m6845);
void m6845_video(m6845_Regs *m6845, Hardware *hardware);
