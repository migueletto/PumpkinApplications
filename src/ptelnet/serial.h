Err SerialList(char deviceNames[][MAX_DEVICENAME], UInt32 *deviceCreators, UInt16 *numDevices);
Err SerialOnline(UShort *, UInt32, UShort, UShort, UShort, UShort, UShort, UInt16 port);
void SerialOffline(UShort);
Int SerialReceive(UShort, UChar *, Int, Err *);
Int SerialSend(UShort, UChar *, Int, Err *);
Word SerialGetStatus(UShort);
Err SerialBreak(UShort);
