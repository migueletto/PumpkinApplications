#include <PalmOS.h>
#include <PalmCompatibility.h>

#include "const.h"
#include "ptelnet.h"
#include "serial.h"
#include "error.h"

#define tamSerBuf 4096

static void CreatorToString(UInt32 creator, Char *s) {
  s[0] = '(';
  s[1] = (char)(creator >> 24);
  s[2] = (char)(creator >> 16);
  s[3] = (char)(creator >> 8);
  s[4] = (char)creator;
  s[5] = ')';
  s[6] = '\0';
}

Err SerialList(char deviceNames[][MAX_DEVICENAME], UInt32 *deviceCreators, UInt16 *numDevices) {
  DeviceInfoType dev;
  UInt16 num, i;
  Err err;

  if ((err = SrmGetDeviceCount(&num)) != 0) {
    ErrorDialog("Serial device count", err);
    return err;
  }

  if (num > *numDevices) {
    num = *numDevices;
  }

  for (i = 0; i < num; i++) {
    if ((err = SrmGetDeviceInfo(i, &dev)) != 0) {
      ErrorDialog("Serial device info", err);
      return err;
    }
    if (dev.serDevPortInfoStr[0]) {
      StrNCopy(deviceNames[i], dev.serDevPortInfoStr, MAX_DEVICENAME);
    } else {
      CreatorToString(dev.serDevCreator, deviceNames[i]);
    }
    deviceCreators[i] = dev.serDevCreator;
  }

  *numDevices = num;
  return 0;
}

Err SerialOnline(UShort *AppSerRefnum, UInt32 baud, UShort bits, UShort parity, UShort stopBits, UShort xonXoff, UShort rtsCts, UInt16 port) {
  PtelnetGlobalsType *p = PtelnetGetGlobals();
  ULong flags;
  UShort len;
  Int32 value;
  Err err;

  p->AppSerTimeout = p->ticksPerSecond/4;

  if ((p->BufferH = MemHandleNew(tamSerBuf+64)) == NULL) {
    ErrorDialog("Could not create handle", 0);
    return -1;
  }
 
  if ((p->Buffer = MemHandleLock(p->BufferH)) == NULL) {
    MemHandleFree(p->BufferH);
    ErrorDialog("Could not lock handle", 0);
    return -1;
  }

  err = SrmOpen(port, baud, AppSerRefnum);

  if (err != 0 && err != serErrAlreadyOpen) {
    MemHandleUnlock(p->BufferH);
    MemHandleFree(p->BufferH);
    ErrorDialog("Could not open Serial", err);
    return -1;
  }
 
  flags = 0;
 
  if (xonXoff) {
    flags |= srmSettingsFlagXonXoffM;
  }
 
  if (rtsCts) {
    flags |= srmSettingsFlagRTSAutoM;
    flags |= srmSettingsFlagCTSAutoM;
  }
 
  switch (bits) {
    case 5: flags |= srmSettingsFlagBitsPerChar5; break;
    case 6: flags |= srmSettingsFlagBitsPerChar6; break;
    case 7: flags |= srmSettingsFlagBitsPerChar7; break;
    case 8: flags |= srmSettingsFlagBitsPerChar8; break;
  }

  flags |= srmSettingsFlagParityOnM;
  switch (parity) {
    case 0: flags &= ~srmSettingsFlagParityOnM; break;
    case 1: flags |= srmSettingsFlagParityEvenM; break;
    case 2: flags &= ~srmSettingsFlagParityEvenM; break;
  }
 
  switch (stopBits) {
    case 1: flags |= srmSettingsFlagStopBits1; break;
    case 2: flags |= srmSettingsFlagStopBits2; break;
  }

  len = sizeof(flags);
  err = SrmControl(*AppSerRefnum, srmCtlSetFlags, &flags, &len);

  value = baud;
  len = sizeof(value);
  if (err == 0) {
    err = SrmControl(*AppSerRefnum, srmCtlSetBaudRate, &value, &len);
  }

  value = srmDefaultCTSTimeout;
  len = sizeof(value);
  if (err == 0) {
    err = SrmControl(*AppSerRefnum, srmCtlSetCtsTimeout, &value, &len);
  }

  if (err != 0) {
    MemHandleUnlock(p->BufferH);
    MemHandleFree(p->BufferH);
    SrmClose(*AppSerRefnum);
    ErrorDialog("Could not config serial", err);
    return -1;
  }
 
  err = SrmSetReceiveBuffer(*AppSerRefnum, p->Buffer, tamSerBuf+64);

  if (err != 0) {
    MemHandleUnlock(p->BufferH);
    MemHandleFree(p->BufferH);
    SrmClose(*AppSerRefnum);
    ErrorDialog("Could not set serial buffer", err);
    return -1;
  }
 
  return 0;
}

void SerialOffline(UShort AppSerRefnum) {
  PtelnetGlobalsType *p = PtelnetGetGlobals();

  SrmSetReceiveBuffer(AppSerRefnum, NULL, 0);

  MemHandleUnlock(p->BufferH);
  MemHandleFree(p->BufferH);

  SrmClose(AppSerRefnum);
}

Int SerialReceive(UShort AppSerRefnum, UChar *buf, Int tam, Err *err) {
  PtelnetGlobalsType *p = PtelnetGetGlobals();
  ULong n;

  SrmClearErr(AppSerRefnum);
  if ((*err = SrmReceiveWait(AppSerRefnum, 1, p->AppSerTimeout)) != 0)
    return -1;
 
  SrmClearErr(AppSerRefnum);
  SrmReceiveCheck(AppSerRefnum, &n);
  if (n > tam)
    n = tam;
 
  SrmClearErr(AppSerRefnum);
  return SrmReceive(AppSerRefnum, buf, n, -1, err);
}

Int SerialSend(UShort AppSerRefnum, UChar *buf, Int tam, Err *err) {
  return SrmSend(AppSerRefnum, buf, tam, err);
}

Word SerialGetStatus(UShort AppSerRefnum) {
  UShort lineErrs;
  ULong status;

  return SrmGetStatus(AppSerRefnum, &status, &lineErrs) ? 0 : lineErrs;
}

Err SerialBreak(UShort AppSerRefnum) {
  PtelnetGlobalsType *p = PtelnetGetGlobals();
  SrmControl(AppSerRefnum, srmCtlStartBreak, NULL, NULL);
  SysTaskDelay(3*p->ticksPerSecond/10);	// 300 milissegundos
  SrmControl(AppSerRefnum, srmCtlStopBreak, NULL, NULL);

  return 0;
}
