#define D_INFO		0
#define D_WARNING	1
#define D_ERROR		2
#define D_FILE		3

void InfoDialog(int type, const char *fmt, ...);

void DebugInit(char *name, Boolean overwrite);
void DebugFinish(void);
void DebugByte(UInt8 b);
void DebugBuffer(UInt8 *b, UInt32 n);
