#include <PalmOS.h>
#include <CPMLib.h>

#include "resource.h"
#include "cpm.h"

static void SetEventHandler(FormType *frm, Int16 form) {
  switch(form) {
    case ProvidersForm:
      FrmSetEventHandler(frm, ProvidersFormHandleEvent);
      break;
    case HashForm:
    case EncryptForm:
    case DecryptForm:
      FrmSetEventHandler(frm, CpmFormHandleEvent);
      break;
  }
}

static Boolean ApplicationHandleEvent(EventType *event) {
  FormType *frm;
  UInt16 form;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      form = event->data.frmLoad.formID;
      frm = FrmInitForm(form);
      FrmSetActiveForm(frm);
      SetEventHandler(frm, form);
      handled = true;
      break;
    default:
      break;
  }

  return handled;
}

static void EventLoop() {
  EventType event;
  Err err;
 
  do {
    EvtGetEvent(&event, evtWaitForever);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);
  } while (event.eType != appStopEvent);
}


static Err StartApplication(UInt16 *refnum) {
  UInt16 ROMVerMajor, ROMVerMinor, numProviders;
  UInt32 dw;

  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &dw);
  ROMVerMajor = sysGetROMVerMajor(dw);
  ROMVerMinor = sysGetROMVerMinor(dw);

  if ((10*ROMVerMajor + ROMVerMinor) < 50) {
    FrmCustomAlert(ErrorAlert, "CPMTest requires at least PalmOS 5.0", "", "");
    return -1;
  }

  if (SysLibLoad(sysFileTLibrary, cpmCreator, refnum) != errNone) {
    FrmCustomAlert(ErrorAlert, "Can not load CPMLib", "", "");
    return -1;
  }

  if (CPMLibOpen(*refnum, &numProviders) != errNone) {
    FrmCustomAlert(ErrorAlert, "Can not open CPMLib", "", "");
    return -1;
  }

  CpmInit(*refnum, numProviders);
  FrmGotoForm(ProvidersForm);

  return 0;
}

static void StopApplication(UInt16 refnum) {
  CpmFinish();
  CPMLibClose(refnum);
  FrmCloseAllForms();
}

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  UInt16 refnum;

  if (cmd == sysAppLaunchCmdNormalLaunch) {
    if (StartApplication(&refnum) == 0) {
      EventLoop();
      StopApplication(refnum);
    }
  }
    
  return 0;
}
