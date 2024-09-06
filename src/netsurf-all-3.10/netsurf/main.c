#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#include "pumpkin.h"
#include "unzip.h"
#include "resource.h"
#include "debug.h"

#define Y0 30

#define RESDIR "PALM/Programs/netsurf/frontends/framebuffer/res/"

extern int ns_main(int argc, char **argv);
extern void ns_finish(void);

static void resize(FormType *frm) {
  WinHandle wh;
  RectangleType rect;
  UInt32 swidth, sheight;

  WinScreenMode(winScreenModeGet, &swidth, &sheight, NULL, NULL);
  wh = FrmGetWindowHandle(frm);
  RctSetRectangle(&rect, 0, 0, swidth, sheight);
  WinSetBounds(wh, &rect);
}

static Boolean MainFormHandleEvent(EventPtr event) {
  FormType *frm;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      resize(frm);
      FrmDrawForm(frm);
      handled = true;
      break;

    case menuEvent:
      if (event->data.menu.itemID == aboutCmd) {
        AbtShowAboutPumpkin('Nsrf');
      }
      handled = true;
      break;

    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventPtr event) {
  FormPtr frm;
  UInt16 form;
  Boolean handled;

  handled = false;

  switch (event->eType) {
    case frmLoadEvent:
      form = event->data.frmLoad.formID;
      frm = FrmInitForm(form);
      FrmSetActiveForm(frm);
      FrmSetEventHandler(frm, MainFormHandleEvent);
      handled = true;
      break;
    case frmCloseEvent:
      break;
    default:
      break;
  }

  return handled;
}

Boolean EvtHandle(UInt32 timeout, EventType *ev) {
  Err err;

  EvtGetEventUs(ev, timeout);
  if (SysHandleEvent(ev)) return true;
  if (MenuHandleEvent(NULL, ev, &err)) return true;
  if (ApplicationHandleEvent(ev)) return true;
  FrmDispatchEvent(ev);

  if (ev->eType == appStopEvent) {
    ns_finish();
  }

  return false;
}

#ifdef __MINGW64__
#define PIT_EXPORT __declspec(dllexport)
#else
#define PIT_EXPORT
#endif

PIT_EXPORT UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags) {
  EventType ev;
  UInt32 swidth, sheight, sdepth;
  char *argv[16], width[32], height[32], depth[32];
  int argc;

  pumpkin_unzip_resource(zipRsc, 1, (char *)RESDIR);

  if (cmd == sysAppLaunchCmdNormalLaunch) {
    FrmCenterDialogs(true);
    FrmGotoForm(MainForm);
    for (;;) {
      EvtHandle(10, &ev);
      if (ev.eType == frmOpenEvent) break;
    }

    WinScreenMode(winScreenModeGet, &swidth, &sheight, &sdepth, NULL);
    swidth  <<= 1;
    sheight <<= 1;
    sheight -= Y0;
    StrPrintF(width,  "%d", swidth);
    StrPrintF(height, "%d", sheight);
    StrPrintF(depth,  "%d", sdepth);

    argc = 0;
    argv[argc++] = (char *)"NetSurf";
    argv[argc++] = (char *)"-f";
    argv[argc++] = (char *)"pit";
    argv[argc++] = (char *)"-b";
    argv[argc++] = depth;
    argv[argc++] = (char *)"-w";
    argv[argc++] = width;
    argv[argc++] = (char *)"-h";
    argv[argc++] = height;
    ns_main(argc, argv);

    FrmCloseAllForms();
  }

  return 0;
}
