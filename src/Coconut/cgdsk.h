Err open_cgdsk(FileRef f, DskInfo *dsk) SECTION("aux");
Err read_cgdsk(FileRef f, DskInfo *dsk, UInt16 index, z80_Regs *z80, Hardware *hardware) SECTION("aux");
void close_cgdsk(DskInfo *dsk) SECTION("aux");
