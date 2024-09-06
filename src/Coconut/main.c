#include <PalmOS.h>

#include "palm.h"
#include "gui.h"
#include "debug.h"

static Err StartApplication(void *);
static void StopApplication(void);

static void RetrieveVersion(char *);

static AppPrefs prefs, prefs_aux;

static UInt16 ROMVerMajor;
static UInt16 ROMVerMinor;
static UInt16 ROMVerFix;
static Int16 ROMNumber;
static char appVersion[8];
static char romVersion[8];
static Boolean hasBacklight;

UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  if (cmd == sysAppLaunchCmdNormalLaunch) {
    if (StartApplication((void *)cmdPBP) == -1)
      return 0;

    EventLoop();
    StopApplication();
  }
  return 0;
}

static Err StartApplication(void *param) {
  UInt32 dw;
  UInt16 len;

  RetrieveVersion(appVersion);
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &dw);
  ROMVerMajor = sysGetROMVerMajor(dw);
  ROMVerMinor = sysGetROMVerMinor(dw);
  ROMVerFix = sysGetROMVerFix(dw);
  ROMNumber = ROMVerMajor*10 + ROMVerMinor;

  if (ROMVerFix) {
    StrPrintF(romVersion, "%d.%d.%d", ROMVerMajor, ROMVerMinor, ROMVerFix);
  } else {
    StrPrintF(romVersion, "%d.%d", ROMVerMajor, ROMVerMinor);
  }

  if (ROMNumber < 50) {
    FrmCustomAlert(ErrorAlert, "This program requires at least PalmOS 4.0", "", "");
    return -1;
  }

  WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &dw, NULL);
  if (!(dw & 0x80) && !(dw & 0x08)) {
    FrmCustomAlert(ErrorAlert, "This program requires at least a 16 colors display", "", "");
    return -1;
  }

  FtrGet(sysFtrCreator, sysFtrNumBacklight, &dw);
  hasBacklight = dw & 1;

  len = sizeof(prefs);
  MemSet(&prefs, len, 0);
  InitPrefs(&prefs);
  if (PrefGetAppPreferences(AppID, 1, &prefs, &len, true) == noPreferenceFound) {
    PrefSetAppPreferences(AppID, 1, 1, &prefs, len, true);
  }
  LoadPrefs();

  if (AppInit(param) != 0) {
    debug(1, "XXX", "passo 8a");
    return -1;
  }

  FrmGotoForm(MainForm);

  return 0;
}

static void StopApplication(void) {
  FrmCloseAllForms();
  PrefSetAppPreferences(AppID, 1, 1, &prefs, sizeof(prefs), true);
  WinScreenMode(winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);
  AppFinish();
}

Boolean ApplicationHandleEvent(EventPtr event) {
  FormPtr frm;
  UInt16 formId;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      formId = event->data.frmLoad.formID;
      frm = FrmInitForm(formId);
      FrmSetActiveForm(frm);
      SetEventHandler(frm, formId);
      handled = true;
      break;
    case frmCloseEvent:
      switch (formId = event->data.frmClose.formID) {
        case MainForm:
        case EmuForm:
          frm = FrmGetFormPtr(formId);
          FrmEraseForm(frm);
          FrmDeleteForm(frm);
          handled = true;
      }
    default:
      break;
  }

  return handled;
}

void SavePrefs(void) {
  MemMove(&prefs, &prefs_aux, sizeof(prefs));
}

AppPrefs *LoadPrefs(void) {
  MemMove(&prefs_aux, &prefs, sizeof(prefs));
  return &prefs_aux;
}

AppPrefs *GetPrefs(void) {
  return &prefs;
}

static void RetrieveVersion(char *appVersion) {
  MemHandle h;
  char *s;

  if ((h = DmGetResource(verRsc, appVersionID)) != NULL) {
    if ((s = MemHandleLock(h)) != NULL) {
      StrCopy(appVersion, s);
      MemHandleUnlock(h);
    }
    else
      StrCopy(appVersion, "?.?");
    DmReleaseResource(h);
  } 
  else
    StrCopy(appVersion, "?.?");
}
