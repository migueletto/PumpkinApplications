Err init_drive(UInt8 drive, Int16 vol, char *name) SECTION("aux");
void close_drive(UInt8 drive) SECTION("aux");

Err seek_sector(UInt8 drive, UInt8 track, UInt8 sector) SECTION("aux");
UInt8 read_sector(UInt8, UInt8, UInt8, UInt8 *) SECTION("aux");
UInt8 write_sector(UInt8, UInt8, UInt8, UInt8 *) SECTION("aux");
