#include <PalmOS.h>

#define m_reverse	0x1
#define m_underscore	0x2

void InitScreen(Int16 r, Int16 c, Int16 minx, Int16 miny, UInt8 fg, UInt8 bg) SECTION("aux");
void CloseScreen(void) SECTION("aux");
void Cursor(Int16) SECTION("aux");
void ScrollUp(void) SECTION("aux");
void ScrollDown(void) SECTION("aux");
void SaveCursor(void) SECTION("aux");
void RestoreCursor(void) SECTION("aux");
Int16 GetX(void) SECTION("aux");
Int16 GetY(void) SECTION("aux");
void SetX(Int16) SECTION("aux");
void SetY(Int16) SECTION("aux");
void IncY(void) SECTION("aux");
void DecY(void) SECTION("aux");
void IncX(Int16) SECTION("aux");
void DecX(void) SECTION("aux");
void Home(void) SECTION("aux");
void ClearScreen(void) SECTION("aux");
void ClearScreenToEnd(void) SECTION("aux");
void ClearScreenFromBegin(void) SECTION("aux");
void ClearLine(void) SECTION("aux");
void ClearLineToEnd(void) SECTION("aux");
void ClearLineFromBegin(void) SECTION("aux");
void Underscore(Int16) SECTION("aux");
void Reverse(Int16) SECTION("aux");
void ChangeScrollingRegion(Int16, Int16) SECTION("aux");
Int16 IsUnderscore(void) SECTION("aux");
Int16 IsReverse(void) SECTION("aux");
Int16 GetRows(void) SECTION("aux");
Int16 GetCols(void) SECTION("aux");
void DeleteChar(void) SECTION("aux");
void DrawChar(Int16, UInt16, Int16, Int16) SECTION("aux");
