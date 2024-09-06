void cas_init(void);
void cas_finish(void);
void cas_reset(void);
void cas_motor(UInt8 b);
UInt8 cas_read(FileRef f, UInt32 *eventcount);
void cas_status(FileRef f);
