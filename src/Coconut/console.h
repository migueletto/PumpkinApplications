UInt8 stat_console(void) SECTION("aux");
UInt8 read_console(void) SECTION("aux");
UInt8 write_console(UInt8) SECTION("aux");
Err init_console(void) SECTION("aux");
void close_console(void) SECTION("aux");
void to_console(UInt8) SECTION("aux");
