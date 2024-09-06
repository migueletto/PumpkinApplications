#define ESC 27
#define DEL 127

typedef enum { upArrow, downArrow, leftArrow, rightArrow } CursorKey;

void InitTerminal(Int16, Int16) SECTION("aux");
void EmitTerminal(char *, Int16, Int16) SECTION("aux");
void EmitChar(UInt8) SECTION("aux");
void EmitString(char *) SECTION("aux");
char *GetKeySeq(CursorKey) SECTION("aux");
