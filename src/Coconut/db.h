DmOpenRef DbOpen(LocalID dbID, UInt16 mode, Err *err) SECTION("aux");
DmOpenRef DbOpenByName(char *name, UInt16 mode, Err *err) SECTION("aux");
Err DbClose(DmOpenRef dbRef) SECTION("aux");
Err DbDeleteByName(char *name) SECTION("aux");
Err DbCreate(char *name, UInt32 type, UInt32 creator) SECTION("aux");
Err DbGetTypeCreator(char *name, UInt32 *type, UInt32 *creator) SECTION("aux");
Err DbGetAttributes(DmOpenRef dbRef, UInt16 *attr) SECTION("aux");
Err DbSetAttributes(DmOpenRef dbRef, UInt16 attr) SECTION("aux");

char *DbOpenRec(DmOpenRef dbRef, UInt16 index, Err *err) SECTION("aux");
Err DbCloseRec(DmOpenRef dbRef, UInt16 index, char *rec) SECTION("aux");
Err DbCreateRec(DmOpenRef dbRef, UInt16 *index, UInt16 size, UInt16 category) SECTION("aux");
Err DbDeleteRec(DmOpenRef dbRef, UInt16 index) SECTION("aux");
Err DbResizeRec(DmOpenRef dbRef, UInt16 index, UInt32 size) SECTION("aux");
Err DbGetRecAttributes(DmOpenRef dbRef, UInt16 index, UInt16 *attr) SECTION("aux");
Err DbSetRecAttributes(DmOpenRef dbRef, UInt16 index, UInt16 attr) SECTION("aux");
UInt16 DbNumRecords(DmOpenRef dbRef) SECTION("aux");
