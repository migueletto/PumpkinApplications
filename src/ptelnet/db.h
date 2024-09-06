DmOpenRef DbOpen(LocalID dbID, UInt mode);
DmOpenRef DbOpenByName(CharPtr name, UInt mode);
void DbClose(DmOpenRef);
CharPtr DbOpenRec(DmOpenRef dbRef, UInt index);
void DbCloseRec(DmOpenRef dbRef, UInt index, CharPtr rec);
CharPtr DbCreateRec(DmOpenRef dbRef, UInt16 *index, UShort size, Word category);
