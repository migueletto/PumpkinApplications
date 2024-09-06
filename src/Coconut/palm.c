#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "gui.h"
#include "misc.h"
#include "cdebug.h"
#include "debug.h"

#include "cpu.h"
#include "armlet.h"
#include "m6809.h"
#include "m6803.h"
#include "m6502.h"
#include "z80.h"
#include "io.h"
#include "endian.h"

#include "snapshot.h"
#include "kbd.h"
#include "video.h"
#include "dsk.h"
#include "cgdsk.h"
#include "bin.h"
#include "cas.h"
#include "snapz80.h"
#include "dac.h"
#include "machine.h"
#include "m6845.h"
#include "m6847.h"
#include "sn76489.h"
#include "ay8910.h"
#include "ti9918.h"

#include "coco.h"
#include "mc10.h"
#include "spectrum.h"
#include "mc1000.h"
#include "vz.h"
#include "apple.h"
#include "vic20.h"
#include "oric.h"
#include "atom.h"
#include "cgenie.h"
#include "jupiter.h"
#include "aquarius.h"
#include "msx.h"
#include "coleco.h"
#include "nes.h"

#include "cpm.h"
#include "console.h"
#include "auxp.h"
#include "lpt.h"
#include "screen.h"
#include "vt100.h"
#include "cpmdsk.h"

static m6809_Regs m6809;
static m6803_Regs m6803;
static m6502_Regs m6502;
static z80_Regs z80;
static Hardware *hardware;

enum {PROCESSOR_ARM, PROCESSOR_X86};

static uint32_t (*cpuFunc)(ArmletArg *arg);
static ArmletArg arg;
static UInt32 processor = 0;
static MemHandle ifh;
static UInt16 display_x0 = 0, display_y0 = 0;
static UInt32 font_width = 0, font_height = 0, row_height = 0;
static UInt8 vdg = 0;
static WinHandle vdg_wh = NULL;
static BitmapType *vdg_bmp = NULL;
static UInt8 *vdg_ptr = NULL;
static Boolean vdg_filled = false;
static UInt16 samreg = 0;
static UInt8 sam = 0;
static Boolean border = true;

static MemHandle fh = NULL;
static UInt32 keyDelay = 0;
static UInt8 *rom2 = NULL;
static UInt8 d64mode = 1;

static UInt8 da_value = 63;
static UInt8 kbcolumn = 0xFF;
static UInt32 keydown = 0;
static UInt8 key = 0;
static Int8 joy_coord[2];
static UInt8 joy_axis = JOY_XAXIS;
static UInt8 joy_control = JOY_RIGHT;

#define MAX_SAMPLES 2048

static UInt8 snd_enable = 0;
static UInt8 snd_mux = 0;
static UInt8 snd_1bit = 0;
static UInt32 snd_count = 0;
static UInt32 snd_samples = 0;
static UInt32 snd_tonefreq = 0;
static UInt32 snd_toneamp = 0;
static UInt32 snd_noisefreq = 0;
static UInt32 snd_noiseamp = 0;
static SoundSample *snd_buffer = NULL;

static DskInfo dsk;
static FileRef fdsk = NULL;
static char dskname[128];

static FileRef cassette = NULL;

static KeyMap *keyMap = NULL;
static UInt16 *controlMap = NULL;
static ButtonDef *buttonDef = NULL;
static Boolean keyboardOpen;

typedef struct {
  char title[64];
  UInt16 form, size, index;
  char **entry;
  char buf[256], *last;
  void (*callback)(char *);
} FileListType;

static FileListType *list = NULL;

static Boolean developer = true;
static char *devstring = "developer";

static Boolean started;
static Boolean running = false;
static Boolean iowait = false;
static Int16 wait = evtWaitForever;
static UInt16 waitTicks = 0;
static UInt32 maxVsyncs = 0;
static UInt32 numVsyncs = 0;
static UInt32 lastTicks = 0;
static UInt16 ticksPerSecond = 0;
static UInt16 *ticksPerVsync = NULL;


static Boolean InterceptEvent(EventPtr event);
static Boolean InputEvent(EventPtr event);
static void KeyboardEvent(void);
static void CasseteCallback(char *name);
static void LoadSnapCallback(char *name);
static void LoadCocoSnapCallback(char *name);
#ifdef F_CPM
static void LoadCpmDskCallback(UInt8 drive, char *name);
static void LoadCpmDsk0Callback(char *name);
static void LoadCpmDsk1Callback(char *name);
#endif
static void LoadDskCallback(char *name);
static void LoadDskBinCallback(char *name);
static void LoadBinCallback(char *name);
static void CloseList(void);
static FileRef CheckROM(RomType *rom);
static Err LoadROM(RomType *rom);
static Err LoadCocoSnap(char *name);
static Err SaveCocoSnap(char *name);
static Err LoadBin(char *name);
static Err LoadDskBin(char *name);
static Err LoadSnap(char *name);
static Err OpenDskList(char *title, char *file, void (*callback)(char *));
static Err OpenList(char *title, char *dir, char *dir1, Boolean showdirs,
                    char *ext, void (*callback)(char *)) SECTION("aux");

Boolean ControlGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param);
Boolean ButtonGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) SECTION("aux");
static void RomDrawCell(void *tbl, Int16 row, Int16 col, RectangleType *rect) SECTION("aux");
static void ListDrawCell(void *tbl, Int16 row, Int16 col, RectangleType *rect) SECTION("aux");
static Int16 compare(void *e1, void *e2, Int32 other) SECTION("aux");
static Err SaveScreen(char *name, Boolean full) SECTION("aux");

static void coco_d64mode(UInt8 mode);
static void coco_samupdate(void);
static void io_vsync(void);
static void io_sample(void);
static void io_sample2(void);
static void io_tone(void);
static void io_tone2(void);

Err AppInit(void *_param) {
  UInt32 version, attr, depth; 
  Boolean highDensity, hasVFS;
  Err err;

  FtrGet(sysFileCSystem, sysFtrNumProcessorID, &processor);
  if (processor >= sysFtrNumProcessorARM720T &&
      processor <= sysFtrNumProcessorARM710A) {
    processor = PROCESSOR_ARM;
  } else if (processor == sysFtrNumProcessorx86) {
    processor = PROCESSOR_X86;
  } else {
    return -1;
  }

  hasVFS = FtrGet(sysFileCVFSMgr, vfsFtrIDVersion, &version) == 0;
  if (!hasVFS) {
    return -1;
  }

  WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &depth, NULL);
  if (!(depth & 0x8000) && !(depth & 0x80)) {
    return -1;
  }

  depth = 8;
  WinScreenMode(winScreenModeSet, NULL, NULL, &depth, NULL);

  highDensity = false;

  if (FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version) == 0 &&
      version >= 4) {
    WinScreenGetAttribute(winScreenDensity, &attr);
    if (attr == kDensityDouble)
      highDensity = true;
  }

  if (!highDensity) {
    return -1;
  }

  hardware = sys_calloc(1, sizeof(Hardware));
  MemSet(hardware, sizeof(Hardware), 0);

  WinSetCoordinateSystem(kCoordinatesDouble);
  WinScreenMode(winScreenModeGet, &hardware->display_width, &hardware->display_height, NULL, NULL);
  font_width = 8;
  font_height = row_height = 12;

  display_x0 = (hardware->display_width - 256) / 2;
  display_y0 = (hardware->display_height - 192 - lowBorder*2) / 2;

  vdg_wh = WinCreateOffscreenWindow(256*font_width, font_height, nativeFormat, &err);
  WinSetCoordinateSystem(kCoordinatesStandard);
  if (vdg_wh == NULL) {
    return -1;
  }

  vdg_bmp = WinGetBitmap(vdg_wh);
  vdg_ptr = BmpGetBits(vdg_bmp);

  if ((hardware->bank0 = sys_calloc(0x8000, 1)) == NULL) {
    return -1;
  }
  if ((hardware->bank1 = sys_calloc(0x8000, 1)) == NULL) {
    sys_free(hardware->bank0);
    return -1;
  }
  if ((hardware->bank2 = sys_calloc(0x8000, 1)) == NULL) {
    sys_free(hardware->bank0);
    sys_free(hardware->bank1);
    return -1;
  }
  hardware->bank3 = NULL;
  hardware->bank4 = NULL;

  rom2 = NULL;

  keyboardOpen = false;
  ticksPerSecond = SysTicksPerSecond();
  keyDelay = ticksPerSecond/10;
  wait = evtWaitForever;

  ticksPerVsync = sys_calloc(100, sizeof(UInt16));
  list = sys_calloc(1, sizeof(FileListType));
  keyMap = sys_calloc(256, sizeof(KeyMap));

  snd_buffer = sys_calloc(MAX_SAMPLES, sizeof(SoundSample));
  hardware->snd_buffer = (void *)snd_buffer;

  MemSet(dskname, sizeof(dskname), 0);
  check_volume();
  cas_init();

  cpuFunc = NULL;

  ifh = DmGet1Resource(fontExtRscType, InternalFontID);
  FntDefineFont(internalFontID, MemHandleLock(ifh));

  hardware->m0 = hardware->bank0;
  hardware->m1 = hardware->bank1;
  hardware->m2 = hardware->bank2;

  hardware->globals = NULL;
  hardware->rnd = NULL;
  hardware->tape = 0;

  started = false;

  return 0;
}

void AppFinish(void) {
  AppPrefs *prefs;
  MachineType *machine;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  if (machine->dac & DAC_SAMPLE)
    dac_finish();
  if (machine->dac & DAC_TONE)
    dac_settone(0, 0);

  if (hardware->rnd)
    sys_free(hardware->rnd);

  if (started)
    if (machine->finishFunction)
      machine->finishFunction();

  mch_finish();

  sys_free(snd_buffer);
  sys_free(ticksPerVsync);
  sys_free(keyMap);

  if (list->last)
    sys_free(list->last);
  sys_free(list);

  sys_free(hardware->bank0);
  sys_free(hardware->bank1);
  sys_free(hardware->bank2);
  if (hardware->bank3)
    sys_free(hardware->bank3);
  if (hardware->bank4)
    sys_free(hardware->bank4);

  if (rom2)
    sys_free(rom2);
  if (cassette)
    VFSFileClose(cassette);

  WinDeleteWindow(vdg_wh, false);
  if (hardware->screen_wh) {
    WinDeleteWindow(hardware->screen_wh, false);
  }

  cpuFunc = NULL;

  FntSetFont(stdFont);

  MemHandleUnlock(ifh);
  DmReleaseResource(ifh);

  if (fh) {
    MemHandleUnlock(fh);
    DmReleaseResource(fh);
  }
  fh = NULL;

  cas_finish();

#ifdef F_CPM
  close_drive(0);
  close_drive(1);
  close_drive(2);
  CloseScreen();
#endif

  sys_free(hardware);
}

void SetEventHandler(FormPtr frm, Int16 formId) {
  switch (formId) {
    case MainForm:
      FrmSetEventHandler(frm, MainFormHandleEvent);
      running = false;
      wait = evtWaitForever;
      break;
    case EmuForm:
      FrmSetEventHandler(frm, EmuFormHandleEvent);
      wait = waitTicks;
      break;
    case ListForm:
      FrmSetEventHandler(frm, ListFormHandleEvent);
      running = false;
      wait = evtWaitForever;
      break;
    case CocoConfigForm:
    case MC10ConfigForm:
    case SpecConfigForm:
    case CpmConfigForm:
    case AppleConfigForm:
    case MC1000ConfigForm:
    case VZ300ConfigForm:
    case VicConfigForm:
    case OricConfigForm:
    case AtomConfigForm:
    case CgenieConfigForm:
    case JupiterConfigForm:
    case AquariusConfigForm:
    case MsxConfigForm:
    case ColecoConfigForm:
    case NesConfigForm:
      FrmSetEventHandler(frm, ConfigFormHandleEvent);
      running = false;
      wait = evtWaitForever;
  }
}

void EventLoop(void) {
  EventType event;
  Err err;

  do {
    if (keyboardOpen) {
      KeyboardEvent();
      keyboardOpen = false;
    }

    if (key && ((TimGetTicks() - keydown) > keyDelay)) {
      AppPrefs *prefs;
      MachineType *machine;

      prefs = GetPrefs();
      machine = mch_getmachine(prefs->machine);

      if (machine->keyHandler)
        machine->keyHandler(key = 0);
    }

    EvtGetEvent(&event, wait);

    if (event.eType == nilEvent && wait != evtWaitForever) {
      FrmDispatchEvent(&event);
      continue;
    }

    if (InterceptEvent(&event))
      continue;

    if (SysHandleEvent(&event))
      continue;

    if (MenuHandleEvent(NULL, &event, &err))
      continue;

    if (InputEvent(&event))
      continue;

    if (ApplicationHandleEvent(&event))
      continue;

    FrmDispatchEvent(&event);

  } while (event.eType != appStopEvent);
}

static Boolean InterceptEvent(EventPtr event) {
  char *name;

  if (event->eType != keyDownEvent) {
    return false;
  }

  if (event->data.keyDown.modifiers & commandKeyMask) {
    switch (event->data.keyDown.chr) {
      case vchrHard1:
      case vchrHard2:
      case vchrHard3:
        return running;

      case pageUpChr:
        if (developer && FrmGetActiveFormID() == EmuForm) {
          AppPrefs *prefs;
          MachineType *machine;

          prefs = GetPrefs();
          machine = mch_getmachine(prefs->machine);

          if (machine->debugFunction)
            machine->debugFunction();
        }
        break;

      case pageDownChr:
        if (developer && FrmGetActiveFormID() == EmuForm) {
          name = DoInput("Save machine screenshot", NULL);
          if (name) {
            if (name[0] == '_')
              SaveScreen(&name[1], true);
            else
              SaveScreen(name, false);
          }
          return true;
        }
        break;
        return true;

      case vchrHard4:
        if (running) {
          event->data.keyDown.chr = vchrKeyboard;
          keyboardOpen = true;
        }
    }
  }

  return false;
}

static Boolean InputEvent(EventPtr event) {
  if (event->eType != keyDownEvent) {
    return false;
  }

  if (event->data.keyDown.modifiers & commandKeyMask) {
    return false;
  }

  if (running || iowait) {
    AppPrefs *prefs;
    MachineType *machine;

    prefs = GetPrefs();
    machine = mch_getmachine(prefs->machine);

    if (machine->keyHandler) {
      machine->keyHandler(key = event->data.keyDown.chr & 0xFF);
      keydown = TimGetTicks();
      return running;
    }
  }

  return running;
}

static void KeyboardEvent(void) {
  FormType *frm;
  UInt16 index;
  FieldPtr fld;
  EventType event;
  char *s;

  if (FrmGetActiveFormID() != EmuForm)
    return;

  frm = FrmGetActiveForm();
  index = FrmGetObjectIndex(frm, hiddenFld);
  fld = FrmGetObjectPtr(frm, index);
  s = FldGetTextPtr(fld);

  if (s && s[0]) {
    event.eType = keyDownEvent;
    event.data.keyDown.chr = s[0];
    event.data.keyDown.modifiers = 0;
    InputEvent(&event);

    FldDelete(fld, 0, 1);
  }
}

void InitPrefs(AppPrefs *prefs) {
  MachineType *machine;

  mch_init();
  prefs->machine = 0;
  machine = mch_getmachine(prefs->machine);
  prefs->ramsize = getmaxramsize(machine->ramsizes);
  mch_createdirs();
}

Boolean MainFormHandleEvent(EventPtr event) {
  FormPtr frm;
  ListPtr lst;
  TableType *tbl;
  Int16 num, row;
  MachineType *machine;
  Boolean handled;
  AppPrefs *prefs;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      prefs = GetPrefs();
      machine = mch_getmachine(prefs->machine);

      lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, machineList));
      num = mch_getnummachines();
      LstSetHeight(lst, num);
      LstSetListChoices(lst, mch_getnames(), num);
      LstSetSelection(lst, prefs->machine);
      CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm,
         FrmGetObjectIndex(frm, machineCtl)),
         LstGetSelectionText(lst, prefs->machine));

      SelectMachine(prefs->machine, prefs->ramsize);

      tbl = (TableType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, romTbl));
      for (row = 0; row < 3; row++) {
        TblSetRowUsable(tbl, row, true);
        TblSetItemStyle(tbl, row, 0, customTableItem);
        TblSetItemStyle(tbl, row, 1, customTableItem);
        TblSetRowSelectable(tbl, row, false);
      }

      TblSetColumnUsable(tbl, 0, true);
      TblSetColumnUsable(tbl, 1, true);
      TblSetCustomDrawProcedure(tbl, 0, RomDrawCell);
      TblSetCustomDrawProcedure(tbl, 1, RomDrawCell);

      FrmDrawForm(frm);
      handled = true;
      break;

    case menuEvent:
      switch (event->data.menu.itemID) {
        case AboutCmd:
          frm = FrmInitForm(AboutForm);
          FrmDoDialog(frm);
          FrmDeleteForm(frm);
          handled = true;
      }
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case startBtn:
          if (EmuSelect() == 0) {
            started = true;
            FrmGotoForm(EmuForm);
          }
          handled = true;
      }
      break;

    case popSelectEvent:
      switch (event->data.popSelect.listID) {
        case machineList:
          prefs = GetPrefs();
          prefs->machine = event->data.popSelect.selection;
          machine = mch_getmachine(prefs->machine);
          prefs->ramsize = getmaxramsize(machine->ramsizes);

          SelectMachine(prefs->machine, prefs->ramsize);

          frm = FrmGetActiveForm();
          tbl = (TableType *)FrmGetObjectPtr(frm,FrmGetObjectIndex(frm,romTbl));
          TblMarkTableInvalid(tbl);
          TblRedrawTable(tbl);
          break;

        case ramList:
          prefs = GetPrefs();
          machine = mch_getmachine(prefs->machine);
          prefs->ramsize = getramsize(machine->ramsizes,
             event->data.popSelect.selection);
      }
      handled = false;

    default:
      break;
  }

  return handled;
}

static void RomDrawCell(void *t, Int16 row, Int16 col, RectangleType *rect) {
  AppPrefs *prefs;
  RomType *rom;
  MemHandle h;
  BitmapType *bmp;
  FileRef f;
  UInt16 id;

  prefs = GetPrefs();
  if ((rom = mch_getrom(prefs->machine, row)) != NULL) {
    if (col == 0) {
      WinPaintChars(rom->name, StrLen(rom->name),
        rect->topLeft.x, rect->topLeft.y);
    } else {
      if ((f = CheckROM(rom)) != NULL) {
        id = okBmp;
        VFSFileClose(f);
      } else
        id = missingBmp;
      if ((h = DmGetResource(bitmapRsc, id)) != NULL) {
        if ((bmp = (BitmapPtr)MemHandleLock(h)) != NULL) {
          WinDrawBitmap(bmp, rect->topLeft.x, rect->topLeft.y);
          MemHandleUnlock(h);
        }
        DmReleaseResource(h);
      }
    }
  }
}

Boolean EmuFormHandleEvent(EventPtr event) {
  FormPtr frm;
  UInt16 i, index;
  AppPrefs *prefs;
  MachineType *machine;
  Int8 x, y;
  Boolean handled;
  static RectangleType joystick;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      prefs = GetPrefs();
      machine = mch_getmachine(prefs->machine);

      frm = FrmGetActiveForm();

      index = FrmGetObjectIndex(frm, joystickCtl);
      FrmGetObjectBounds(frm, index, &joystick);

      if (machine->joystickHandler) {
        FrmSetGadgetHandler(frm, index, JoystickGadgetCallback);
      }

      for (i = 0; i < 9; i++) {
        if (controlMap[i]) {
          index = FrmGetObjectIndex(frm, controlCmd+i);
          FrmSetGadgetData(frm, index, &controlMap[i]);
          FrmSetGadgetHandler(frm, index, ControlGadgetCallback);
        } else {
          hide_control(frm, controlCmd+i);
        }
      }

      for (i = 0; i < 12; i++) {
        if (buttonDef[i].label) {
          index = FrmGetObjectIndex(frm, buttonCmd+i);
          FrmSetGadgetData(frm, index, &buttonDef[i]);
          FrmSetGadgetHandler(frm, index, ButtonGadgetCallback);
        } else {
          hide_control(frm, buttonCmd+i);
        }
      }

      FrmSetFocus(frm, FrmGetObjectIndex(frm, hiddenFld));
      FrmDrawForm(frm);

      if (EmuStart() == 0) {
#ifdef F_CPM
        if (machine->id == cpm)
          cpm_cls();
#endif

        if (machine->dac & DAC_SAMPLE) {
          UInt32 cps;

          snd_samples = SAMPLE_RATE / maxVsyncs;
          hardware->snd_samples = snd_samples;
          dac_init(SAMPLE_RATE, snd_samples, machine->clock / maxVsyncs);

          cps = (machine->clock / maxVsyncs) / snd_samples;
          hardware->rnd = sys_calloc(snd_samples, 1);
          hardware->cycles_per_sample = cps;

          hardware->rnd[0] = (UInt8)SysRandom(TimGetTicks());
          for (i = 1; i < snd_samples; i++)
            hardware->rnd[i] = (UInt8)SysRandom(0);
        }
        if (machine->dac & DAC_TONE)
          dac_settone(0, 0);

      } else {
        started = false;
        running = false;
        FrmGotoForm(MainForm);
      }

      handled = true;
      break;

    case nilEvent:
      if (running) {
        hardware->button = KeyCurrentState();
        arg.cmd = CMD_EXECUTE;
        cpuFunc(&arg);
      } else {
        wait = evtWaitForever;
      }
      handled = true;
      break;

    case penDownEvent:
    case penMoveEvent:
      if (wait != evtWaitForever &&
          event->screenX >= joystick.topLeft.x &&
          event->screenY >= joystick.topLeft.y &&
          event->screenX < (joystick.topLeft.x+joystick.extent.x) &&
          event->screenY < (joystick.topLeft.y+joystick.extent.y)) {

        x = (event->screenX - joystick.topLeft.x - 4) * 2;
        y = (event->screenY - joystick.topLeft.y - 4) * 2;

        if (x < 0) x = 0;
        else if (x > 63) x = 63;
        if (y < 0) y = 0;
        else if (y > 63) y = 63;

        prefs = GetPrefs();
        machine = mch_getmachine(prefs->machine);

        if (machine->joystickHandler)
          machine->joystickHandler(x, y);

        handled = true;
      }
    default:
      break;
  }

  return handled;
}

Boolean ButtonGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  ButtonDef *button;
  RectangleType rect;
  EventType *event;
  AppPrefs *prefs;
  MachineType *machine;
  UInt16 x, y, len;

  switch (cmd) {
    case formGadgetDrawCmd:
      WinPushDrawState();
      WinSetCoordinateSystem(kCoordinatesDouble);

      button = (ButtonDef *)gad->data;
      len = StrLen(button->label);

      rect.topLeft.x = gad->rect.topLeft.x * 2;
      rect.extent.x = gad->rect.extent.x * 2;
      rect.topLeft.y = gad->rect.topLeft.y * 2;
      rect.extent.y = gad->rect.extent.y * 2;

      WinSetForeColor(WinRGBToIndex(&button->back));
      WinDrawRectangle(&rect, 0);

      FntSetFont(button->font);
      x = rect.topLeft.x + (rect.extent.x - FntCharsWidth(button->label, len)) / 2;
      y = rect.topLeft.y + (rect.extent.y - FntCharHeight()) / 2;

      WinSetForeColor(WinRGBToIndex(&button->fore));
      WinSetBackColor(WinRGBToIndex(&button->back));
      WinPaintChars(button->label, len, x, y);

      WinSetBackColor(WinRGBToIndex(&button->border));
      WinDrawLine(rect.topLeft.x, rect.topLeft.y, rect.topLeft.x+rect.extent.x-1, rect.topLeft.y);
      WinDrawLine(rect.topLeft.x, rect.topLeft.y, rect.topLeft.x, rect.topLeft.y+rect.extent.y-1);
      WinDrawLine(rect.topLeft.x, rect.topLeft.y+rect.extent.y-1, rect.topLeft.x+rect.extent.x-1, rect.topLeft.y+rect.extent.y-1);
      WinDrawLine(rect.topLeft.x+rect.extent.x-1, rect.topLeft.y, rect.topLeft.x+rect.extent.x-1, rect.topLeft.y+rect.extent.y-1);

      WinSetCoordinateSystem(kCoordinatesStandard);
      WinPopDrawState();
      break;

    case formGadgetHandleEventCmd:
      event = (EventType *)param;

      switch (event->eType) {
        case frmGadgetEnterEvent:
          button = (ButtonDef *)gad->data;
          WinInvertRectangle(&gad->rect, 0);

          if (button->key == KEY_RESET) {
            EmuReset();
            prefs = GetPrefs();
            machine = mch_getmachine(prefs->machine);
            if (machine->family == fcoco) {
              CocoPrefs *cocoPrefs = CocoGetPrefs();
              if (cocoPrefs->artifacting > 0) {
                cocoPrefs->artifacting = 3 - cocoPrefs->artifacting;
              }
            }
          } else {
            event->eType = keyDownEvent;
            event->data.keyDown.chr = button->key;
            event->data.keyDown.modifiers = 0;
            InputEvent(event);
          }

          WinInvertRectangle(&gad->rect, 0);
        default:
          break;
      }
  }

  return true;
}

Boolean ControlGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  AppPrefs *prefs;
  MachineType *machine;
  RectangleType rect;
  EventType *event;
  MemHandle h;
  BitmapType *bmp;
  Int16 id, x, y, len, width, height;
  char *name;

  switch (cmd) {
    case formGadgetDrawCmd:
      WinPushDrawState();
      WinSetCoordinateSystem(kCoordinatesDouble);

      id = *((UInt16 *)gad->data);

      rect.topLeft.x = gad->rect.topLeft.x * 2;
      rect.extent.x = gad->rect.extent.x * 2;
      rect.topLeft.y = gad->rect.topLeft.y * 2;
      rect.extent.y = gad->rect.extent.y * 2;

      if ((h = DmGetResource(bitmapRsc, id)) != NULL) {
        if ((bmp = (BitmapType *)MemHandleLock(h)) != NULL) {
          BmpGetDimensions(bmp, &width, &height, NULL);
          x = rect.topLeft.x + (rect.extent.x - width*2) / 2;
          y = rect.topLeft.y + (rect.extent.y - height*2) / 2;

          WinSetClip(&rect);
          WinDrawBitmap(bmp, x, y);
          WinResetClip();

          MemHandleUnlock(h);
        }
        DmReleaseResource(h);
      }

      WinSetCoordinateSystem(kCoordinatesStandard);
      WinPopDrawState();
      break;

    case formGadgetHandleEventCmd:
      event = (EventType *)param;
      switch (event->eType) {
        case frmGadgetEnterEvent:
          prefs = GetPrefs();
          machine = mch_getmachine(prefs->machine);
          id = *((UInt16 *)gad->data);

          switch (id) {
            case stopCmd:
              if (machine->dac & DAC_SAMPLE) {
                dac_finish();
              }
              if (machine->dac & DAC_TONE) {
                dac_settone(0, 0);
              }
              if (machine->finishFunction) {
                machine->finishFunction();
              }
              started = false;
#ifdef F_CPM
              close_drive(0);
              close_drive(1);
              close_drive(2);
              close_drive(3);
#endif
              MemSet(dskname, sizeof(dskname), 0);
              FrmGotoForm(MainForm);
              running = false;
              break;
            case restartCmd:
              EmuStart();
#ifdef F_CPM
              if (machine->id == cpm)
                cpm_cls();
#endif
              vdg_filled = false;
              wait = waitTicks;
              running = true;
              break;
            case configCmd:
              if (machine->form) {
                running = false;
                wait = evtWaitForever;
                FrmPopupForm(machine->form);
              }
              break;
            case loadCasCmd:
              OpenList("Insert Cassete", "Cassette", NULL, false, NULL, CasseteCallback);
              running = true;
              wait = waitTicks;
              break;
            case loadDsk1Cmd:
#ifdef F_CPM
              if (machine->family == fcpm)
                OpenList("Drive A:", "Disk", NULL, false, NULL, LoadCpmDsk0Callback);
              else
#endif
                OpenList("Insert Disk", "Disk", NULL, true, NULL, LoadDskCallback);

              running = true;
              wait = waitTicks;
              break;
#ifdef F_CPM
            case loadDsk2Cmd:
              OpenList("Drive B:", "Disk", NULL, false, NULL, LoadCpmDsk1Callback);
              running = true;
              wait = waitTicks;
              break;
#endif
            case loadBinCmd:
              len = StrLen(dskname);
              if (len == 0)
                InfoDialog(D_INFO, "Please insert a disk first");
              else {
                if (dskname[len-1] == '/')
                  OpenList("Load Binary", "Disk", dskname, false, NULL, LoadBinCallback);
                else
                  OpenDskList("Load Binary", dskname, LoadDskBinCallback);
              }
              running = true;
              wait = waitTicks;
              break;
            case loadSnapCmd:
              switch (machine->family) {
                case fcoco:
                case fdragon:
                  OpenList("Load Snapshot", "Snapshot", NULL, false,
                    NULL, LoadCocoSnapCallback);
                  break;
                case fnes:
                  OpenList("Load cart", "Snapshot", NULL, false,
                    ".nes", LoadSnapCallback);
                  break;
                default:
                  OpenList("Load snapshot", "Snapshot", NULL, false,
                    NULL, LoadSnapCallback);
              }
              running = true;
              wait = waitTicks;
              break;
            case saveSnapCmd:
              name = DoInput("Save Snapshot", NULL);
              if (name)
                SaveCocoSnap(name);
              running = true;
              wait = waitTicks;
          }
        default:
          break;
      }
  }

  return true;
}

Boolean ConfigFormHandleEvent(EventPtr event) {
  AppPrefs *prefs;
  MachineType *machine;
  UInt16 old;
  Boolean handled, close;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  old = machine->font;
  handled = machine->formHandler(event, &close);

  if (machine->font != old) {
    if (fh) {
      FntSetFont(stdFont);
      MemHandleUnlock(fh);
      DmReleaseResource(fh);
    }
    fh = DmGet1Resource(fontExtRscType, machine->font);
    FntDefineFont(fontID, MemHandleLock(fh));
  }

  if (close) {
    running = true;
    wait = waitTicks;
    FrmReturnToForm(EmuForm);
  }

  return handled;
}

Boolean ListFormHandleEvent(EventPtr event) {
  FormPtr frm;
  TableType *tbl;
  ScrollBarType *scl;
  UInt16 row;
  char *s;
  Boolean handled, update;

  handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();

      FrmSetTitle(frm, list->title);

      tbl = (TableType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, listTbl));

      for (row = 0; row < tableRows; row++) {
  TblSetRowUsable(tbl, row, true);
  TblSetItemStyle(tbl, row, 0, customTableItem);
      }

      TblSetColumnUsable(tbl, 0, true);
      TblSetCustomDrawProcedure(tbl, 0, ListDrawCell);

      scl = (ScrollBarType *)FrmGetObjectPtr(frm,
   FrmGetObjectIndex(frm, listScl));
      SclSetScrollBar(scl, list->index / tableRows,
        0, (list->size-1)/tableRows, 1);
      TblHasScrollBar(tbl, true);

      FrmDrawForm(frm);
      handled = true;
      break;

    case sclRepeatEvent: 
    case sclExitEvent: 
      list->index = event->data.sclRepeat.newValue * tableRows;

      frm = FrmGetActiveForm();
      tbl = (TableType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, listTbl));
      TblMarkTableInvalid(tbl);
      TblRedrawTable(tbl);
      break;

    case keyDownEvent:
      update = false;

      if (event->data.keyDown.modifiers & commandKeyMask) {
        switch (event->data.keyDown.chr) {
          case pageUpChr:
            if (list->index > 0) {
              update = true;
              if (list->index >= tableRows)
                list->index -= tableRows;
              else
                list->index = 0;
            }
            handled = true;
            break;
          case pageDownChr:
            if (list->index + tableRows < list->size) {
              update = true;
              list->index += tableRows;
            }
            handled = true;
        }
      } else {
        char c = event->data.keyDown.chr;

        if (c >= 'a' && c <= 'z')
          c &= 0xDF;

        if (c >= '0' && c <= 'Z') {
          Int16 i;

          for (i = 0; i < list->size; i++) {
            if ((list->entry[i][0] & 0xDF) == c) {
              update = true;
              list->index = i;
              break;
            }
          }
          handled = true;
        }
      }

      if (update) {
        frm = FrmGetActiveForm();
        tbl = (TableType *)FrmGetObjectPtr(frm,
           FrmGetObjectIndex(frm, listTbl));
        scl = (ScrollBarType *)FrmGetObjectPtr(frm,
     FrmGetObjectIndex(frm, listScl));
        TblMarkTableInvalid(tbl);
        TblRedrawTable(tbl);
        SclSetScrollBar(scl, list->index / tableRows,
          0, (list->size-1)/tableRows, 1);
      }
      break;

    case tblSelectEvent: 
      row = event->data.tblSelect.row;
      if (list->index+row < list->size) {
        s = list->entry[list->index+row];
        list->entry[list->index+row] = NULL;
      } else
        s = NULL;
      CloseList();

      if (s) {
        list->callback(s);
        if (list->last)
          sys_free(list->last);
        list->last = s;
      }

      running = true;
      wait = waitTicks;
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case cancelBtn:
          CloseList();
    list->callback(NULL);
          running = true;
          wait = waitTicks;
          handled = true;
      }
    default:
      break;
  }

  return handled;
}

static void ListDrawCell(void *t, Int16 row, Int16 col, RectangleType *rect) {
  char *s;

  if (list->entry && (list->index+row) < list->size &&
      (s = list->entry[list->index+row]) != NULL) {
    FntSetFont(stdFont);
    WinPaintChars(s, StrLen(s), rect->topLeft.x, rect->topLeft.y);
  }
}

static Err SaveScreen(char *name, Boolean full) {
  AppPrefs *prefs;
  MachineType *machine;
  UInt8 *s, header[54];
  Int16 vol, i;
  UInt32 r, aux, *p, x0, y0, dx, dy;
  RGBColorType *rgb, c;
  FileRef f;
  Err err;
  static UInt8 bmp[54] = {0x42, 0x4d, 0x36, 0x94, 0x01, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x36, 0x04, 0x00, 0x00, 0x28, 0x00,
                          0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0x40, 0x01,
                          0x00, 0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x90, 0x01, 0x00, 0xc4, 0x0e,
                          0x00, 0x00, 0xc4, 0x0e, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  if ((vol = check_volume()) == -1) {
    InfoDialog(D_ERROR, "No volume found");
    return -1;
  }

  if (full) {
    x0 = 0;
    y0 = 0;
    dx = hardware->display_width;
    dy = hardware->display_height;

  } else {
    x0 = hardware->x0;
    y0 = hardware->y0;
    dx = hardware->dx;
    dy = hardware->dy;

    if (x0 >= hardware->display_width || y0 >= hardware->display_height - lowBorder*2)
      return -2;

    if ((x0 + dx) > hardware->display_width) {
      dx = hardware->display_width - x0;
    } if ((y0 + dy) > (hardware->display_height - lowBorder*2)) {
      dy = hardware->display_height -lowBorder*2 - y0;
    }
  }

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  MemSet(list->buf, sizeof(list->buf), 0);
  StrCopy(list->buf, AppDir);
  StrCat(list->buf, "/Screenshot/");
  StrCat(list->buf, machine->fname);
  StrCat(list->buf, "/");
  StrCat(list->buf, name);

  if ((err = VFSFileOpen(vol, list->buf,
        vfsModeCreate | vfsModeTruncate | vfsModeWrite, &f)) != 0) {
    InfoDialog(D_ERROR, "Can not save screenshot (%d)", err);
    return -3;
  }
  
  MemMove(header, bmp, sizeof(header));

  // 0x0002: file size: 0xC436 = 50230 = 256*192 + 1078
  p = (UInt32 *)&header[0x0002];
  aux = dx * dy + 1078;
  *p = aux;

  // 0x0012: width    : 256
  p = (UInt32 *)&header[0x0012];
  *p = dx;

  // 0x0016: height   : 192
  p = (UInt32 *)&header[0x0016];
  *p = dy;

  // 0x0022: data size: 0xC000 = 49512 = 256*192
  p = (UInt32 *)&header[0x0022];
  aux = dx * dy;
  *p = aux;

  if ((err = VFSFileWrite(f, sizeof(header), header, &r)) != 0) {
    VFSFileClose(f);
    return -4;
  }

  rgb = sys_calloc(256, sizeof(RGBColorType));
  WinPalette(winPaletteGet, 0, 256, rgb);

  for (i = 0; i < 256; i++) {
    c = rgb[i];
    rgb[i].index = c.b;
    rgb[i].r = c.g;
    rgb[i].g = c.r;
    rgb[i].b = 0;
  }

  if ((err = VFSFileWrite(f, 256*sizeof(RGBColorType), rgb, &r)) != 0) {
    sys_free(rgb);
    VFSFileClose(f);
    return -5;
  }

  sys_free(rgb);

  s = WinScreenLock(winLockCopy);
  s += (y0 + dy) * hardware->display_width + x0;

  for (i = 0; i < dy; i++) {
    s -= hardware->display_width;
    VFSFileWrite(f, dx, s, &r);
  }

  WinScreenUnlock();

  VFSFileClose(f);
  SndPlaySystemSound(sndClick);

  return 0;
}

static void CasseteCallback(char *name) {
  AppPrefs *prefs;
  MachineType *machine;
  Int16 vol;
  Err err;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  if (name) {
    if ((vol = check_volume()) == -1) {
      InfoDialog(D_ERROR, "No volume found");
      return;
    }

    MemSet(list->buf, sizeof(list->buf), 0);
    StrCopy(list->buf, AppDir);
    StrCat(list->buf, "/Cassette/");
    StrCat(list->buf, machine->fname);
    StrCat(list->buf, "/");
    StrCat(list->buf, name);

    if ((err = VFSFileOpen(vol, list->buf, vfsModeRead, &cassette)) != 0) {
      cassette = NULL;
      InfoDialog(D_ERROR, "Can not open cassette (%d)", err);
    }
  } else {
    if (cassette)
      VFSFileClose(cassette);
    cassette = NULL;
  }

  cas_reset();

  if (machine->family == fmc10) {
    cas_motor(0);
    if (cassette)
      cas_motor(1);
  }

  hardware->tape = cassette ? 1 : 0;
  cas_status(cassette);
}

UInt8 cas_input(void) {
  return cas_read(cassette, &hardware->eventcount);
}

UInt32 cas_buffer(UInt32 n, UInt8 *buf) {
  UInt32 r = 0;
  VFSFileRead(cassette, n, buf, &r);
  cas_validate();
  return r;
}

void cas_validate(void) {
  if (VFSFileEOF(cassette)) {
    VFSFileClose(cassette);
    cassette = NULL;
    cas_status(NULL);
  }
  hardware->tape = cassette ? 1 : 0;
}

static void LoadSnapCallback(char *name) {
  if (name) LoadSnap(name);
}

static void LoadCocoSnapCallback(char *name) {
  if (name) LoadCocoSnap(name);
}

static void LoadDskCallback(char *name) {
  if (name) StrCopy(dskname, name);
}

static void LoadDskBinCallback(char *name) {
  AppPrefs *prefs;
  MachineType *machine;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  if (name)
    LoadDskBin(name);

  switch (machine->family) {
#ifdef F_CGENIE
    case fcgenie:
      close_cgdsk(&dsk);
      break;
#endif
    default:
      close_dsk(&dsk);
  }

  VFSFileClose(fdsk);
  fdsk = NULL;
}

static void LoadBinCallback(char *name) {
  if (name)
    LoadBin(name);
}

#ifdef F_CPM
static void LoadCpmDsk0Callback(char *name) {
  LoadCpmDskCallback(0, name);
}

static void LoadCpmDsk1Callback(char *name) {
  LoadCpmDskCallback(1, name);
}

static void LoadCpmDskCallback(UInt8 drive, char *name) {
  AppPrefs *prefs;
  MachineType *machine;
  Int16 vol;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  if (name) {
    if ((vol = check_volume()) != -1) {
      MemSet(dskname, sizeof(dskname), 0);
      StrCopy(dskname, AppDir);
      StrCat(dskname, "/Disk/");
      StrCat(dskname, machine->fname);
      StrCat(dskname, "/");
      StrCat(dskname, name);
      close_drive(drive);
      init_drive(drive, vol, dskname);
    }
  } else
    close_drive(drive);
}
#endif

static Int16 compare(void *e1, void *e2, Int32 other) {
  return StrCompare(*((char **)e1), *((char **)e2));
}

static Err OpenDskList(char *title, char *s, void (*callback)(char *)) {
  AppPrefs *prefs;
  MachineType *machine;
  Int16 vol, i, j;
  Err err;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  if ((vol = check_volume()) == -1) {
    InfoDialog(D_ERROR, "No volume found");
    return -1;
  }

  list->size = 1024;

  if ((list->entry = sys_calloc(list->size, sizeof(char *))) == NULL) {
    InfoDialog(D_ERROR, "sys_calloc");
    list->size = 0;
    return -1;
  }

  MemSet(list->buf, sizeof(list->buf), 0);
  StrCopy(list->buf, AppDir);
  StrCat(list->buf, "/Disk/");
  StrCat(list->buf, machine->fname);
  StrCat(list->buf, "/");
  StrCat(list->buf, s);

  if ((err = VFSFileOpen(vol, list->buf, vfsModeRead, &fdsk)) != 0) {
    sys_free(list->entry);
    list->entry = NULL;
    list->size = 0;
    fdsk = NULL;
    InfoDialog(D_ERROR, "VFSFileOpen");
    return -1;
  }

  switch (machine->family) {
#ifdef F_CGENIE
    case fcgenie:
      err = open_cgdsk(fdsk, &dsk);
      break;
#endif
    default:
      err = open_dsk(fdsk, &dsk);
  }

  if (err != 0) {
    VFSFileClose(fdsk);
    fdsk = NULL;
    InfoDialog(D_ERROR, "Invalid disk image (%d)", err);
    return err;
  }

  if (dsk.numentries == 0) {
    sys_free(list->entry);
    list->entry = NULL;
    list->size = 0; 
    VFSFileClose(fdsk);
    fdsk = NULL;
    InfoDialog(D_INFO, "No file found");
    return 0;
  }

  for (i = 0, j = 0; i < dsk.numentries; i++)
    if (dsk.dir[i].type == 0x02)
      list->entry[j++] = strdup(dsk.dir[i].name);

  list->size = j;
  list->index = 0;
  list->callback = callback;
    
  SysQSort(list->entry, list->size, sizeof(char *), compare, 0);
    
  StrCopy(list->title, title);
  list->form = FrmGetActiveFormID();
  
  if (list->last)
    for (i = 0; i < list->size; i++)
      if (!StrCompare(list->entry[i], list->last)) {
        list->index = (i / tableRows) * tableRows;
        break;
      }

  WinPushDrawState();
    
  FrmPopupForm(ListForm);
  return 0;
}

static Err OpenList(char *title, char *s, char *s1, Boolean showdirs, char *ext, void (*callback)(char *)) {
  AppPrefs *prefs;
  MachineType *machine;
  Int16 vol;
  FileRef dir;
  FileInfoType info;
  UInt32 iterator;
  Int16 i, extlen, namelen;
  Err err;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  if ((vol = check_volume()) == -1) {
    InfoDialog(D_ERROR, "No volume found");
    return -1;
  }

  list->size = 1024;

  if ((list->entry = sys_calloc(list->size, sizeof(char *))) == NULL) {
    InfoDialog(D_ERROR, "sys_calloc");
    list->size = 0;
    return -1;
  }

  MemSet(list->buf, sizeof(list->buf), 0);
  StrCopy(list->buf, AppDir);
  StrCat(list->buf, "/");
  StrCat(list->buf, s);
  StrCat(list->buf, "/");
  StrCat(list->buf, machine->fname);
  if (s1) {
    StrCat(list->buf, "/");
    StrCat(list->buf, s1);
  }

  if ((err = VFSFileOpen(vol, list->buf, vfsModeRead, &dir)) != 0) {
    sys_free(list->entry);
    list->entry = NULL;
    list->size = 0;
    InfoDialog(D_ERROR, "VFSFileOpen %s (%d)", list->buf, err);
    return -1;
  }

  extlen = ext ? StrLen(ext) : 0;

  for (i = 0, iterator = vfsIteratorStart;
       i < list->size && iterator != vfsIteratorStop;) {
    info.nameP = list->buf;
    info.nameBufLen = sizeof(list->buf);

    if (VFSDirEntryEnumerate(dir, &iterator, &info) != 0) {
      break;
    }
    if (info.attributes & vfsFileAttrLink)
      continue;
    if (info.attributes & vfsFileAttrDirectory) {
      if (showdirs)
        StrCat(list->buf, "/");
      else
        continue;
    }

    namelen = StrLen(list->buf);

    if (extlen && namelen > extlen &&
        StrNCaselessCompare(&list->buf[namelen - extlen], ext, extlen) != 0)
      continue;

    list->entry[i++] = strdup(list->buf);
  }

  VFSFileClose(dir);

  if (i == 0) {
    sys_free(list->entry);
    list->entry = NULL;
    list->size = 0;
    InfoDialog(D_INFO, "No file found");
    return 0;
  }

  list->size = i;
  list->index = 0;
  list->callback = callback;

  SysQSort(list->entry, list->size, sizeof(char *), compare, 0);

  StrCopy(list->title, title);
  list->form = FrmGetActiveFormID();

  if (list->last)
    for (i = 0; i < list->size; i++)
      if (!StrCompare(list->entry[i], list->last)) {
        list->index = (i / tableRows) * tableRows;
        break;
      }

  WinPushDrawState();

  FrmPopupForm(ListForm);
  return 0;
}

static void CloseList(void) {
  Int16 i;

  if (list->entry) {
    for (i = 0; i < list->size; i++)
      if (list->entry[i])
        sys_free(list->entry[i]);
    sys_free(list->entry);
    list->entry = NULL;
  }
  list->index = 0;
  list->size = 0;

  FrmReturnToForm(list->form);

  WinPopDrawState();
}

Err EmuSelect(void) {
  AppPrefs *prefs;
  MachineType *machine;
  UInt16 i;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  switch (machine->cpu) {
    case CPU_M6809:
      cpuFunc = m6809_ArmletStart;
      hardware->cpu = &m6809;
      hardware->cycles = m6809_cycles;
      hardware->flags8i = m6809_flags8i;
      hardware->flags8d = m6809_flags8d;
      m6809.hardware = hardware;
      break;
    case CPU_M6803:
      cpuFunc = m6803_ArmletStart;
      hardware->cpu = &m6803;
      hardware->cycles = m6803_cycles;
      hardware->flags8i = m6803_flags8i;
      hardware->flags8d = m6803_flags8d;
      m6803.hardware = hardware;
      break;
    case CPU_Z80:
      cpuFunc = z80_ArmletStart;
      hardware->cpu = &z80;
      hardware->cycles = z80_cycles;
      hardware->partab = z80_partab;
      z80.hardware = hardware;
      break;
    case CPU_M6502:
      cpuFunc = m6502_ArmletStart;
      hardware->cpu = &m6502;
      hardware->cycles = m6502_cycles;
      m6502.hardware = hardware;
      break;
    default:
      return -1;
  }

  hardware->memmode = machine->memmode;
  hardware->vsync_irq = 1;  // IRQ

  controlMap = machine->control;
  buttonDef = machine->buttonDef;

  if (machine->vsync) {
    hardware->vsync = machine->clock / machine->vsync;
  } else {
    hardware->vsync = 0;
  }

  maxVsyncs = machine->vsync;

  hardware->useevents = 0;
  hardware->nevents = 0;
  hardware->event = 0;

  if (machine->selectFunction) {
    machine->selectFunction(machine, hardware, prefs->ramsize);
  }

  if (fh) {
    FntSetFont(stdFont);
    MemHandleUnlock(fh);
    DmReleaseResource(fh);
  }
  fh = DmGet1Resource(fontExtRscType, machine->font);
  FntDefineFont(fontID, MemHandleLock(fh));

  for (i = 0; machine->colortable[i].index >= 0; i++) {
    RGBColorType rgb;
    UInt16 index, c;

    index = machine->colortable[i].index & 0xFF;
    rgb.r = machine->colortable[i].r;
    rgb.g = machine->colortable[i].g;
    rgb.b = machine->colortable[i].b;

    c = WinRGBToIndex(&rgb);
    hardware->color[index] = c;
  }

  kbd_initmap(machine->map, keyMap);

  for (i = 0; i < maxVsyncs; i++) {
    ticksPerVsync[i] = ((i+1) * ticksPerSecond) / maxVsyncs;
  }

  MemSet(&arg, sizeof(ArmletArg), 0);
  arg.a1 = 32768;  // usado somente no CMD_EXECUTE
  arg.hardware = hardware;
  arg.callback = machine->callback;
  hardware->arg = &arg;

  arg.cmd = CMD_INIT;
  cpuFunc(&arg);

  return 0;
}

Err EmuStart(void) {
  Err err;

  if ((err = EmuInit()) != 0) return err;
  if ((err = EmuLoad()) != 0) return err;
  EmuReset();
  return 0;
}

Err EmuInit(void) {
  AppPrefs *prefs;
  MachineType *machine;
#ifdef F_CPM
  CpmPrefs *cpmPrefs;
#endif

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  MemSet(hardware->bank0, 0x8000, 0);
  MemSet(hardware->bank1, 0x8000, 0);
  MemSet(hardware->bank2, 0x8000, 0);
  if (hardware->bank3) {
    MemSet(hardware->bank3, 0x8000, 0);
  }
  if (hardware->bank4) {
    MemSet(hardware->bank4, 0x8000, 0);
  }

  io_bank(0);

  switch (machine->cpu) {
    case CPU_M6809:
      m6809.pc.d = 0;
      m6809.d.d = 0;
      m6809.u.d = 0;
      m6809.s.d = 0;
      m6809.x.d = 0;
      m6809.y.d = 0;
      break;
    case CPU_M6803:
      m6803.pc.d = 0;
      m6803.d.d = 0;
      m6803.s.d = 0;
      m6803.x.d = 0;
      break;
    case CPU_M6502:
      m6502.pc_reg.d = 0;
      m6502.a_reg.d = 0;
      m6502.x_reg.d = 0;
      m6502.y_reg.d = 0;
      m6502.flag_reg.d = 0;
      m6502.s_reg.d = 0;
      break;
    case CPU_Z80:
      z80.af[0] = z80.af[1] = 0;
      z80.bc[0] = z80.bc[1] = 0;
      z80.de[0] = z80.de[1] = 0;
      z80.hl[0] = z80.hl[1] = 0;
      z80.ir = 0;
      z80.ix = 0;
      z80.iy = 0;
      z80.sp = 0;
      z80.pc = 0;
      z80.iff1 = 0;
      z80.iff2 = 0;
      z80.sela = 0;
      z80.selr = 0;
  }

  joy_coord[JOY_XAXIS] = 32;
  joy_coord[JOY_YAXIS] = 32;

  lastTicks = TimGetTicks();
  numVsyncs = 0;
  keyDelay = ticksPerSecond/10;

  border = true;
  vdg = 0;
  vdg_filled = false;

  if (machine->initFunction) {
    machine->initFunction();
  }

  switch (machine->family) {
#ifdef F_CPM
    case fcpm:
      cpmPrefs = CpmGetPrefs();
      InitScreen(25, cpmPrefs->font == Cpm6x10ID ? 64 : 80, 0, 0, hardware->color[cpmPrefs->crt + 1], hardware->color[0]);
      InitTerminal(25, cpmPrefs->font == Cpm6x10ID ? 64 : 80);

      init_console();
      init_aux();
      init_lpt();

      LoadCpmDskCallback(0, "disk0.dsk");
      LoadCpmDskCallback(1, "disk1.dsk");
      break;
#endif
    case fcoco:
      samreg = (0x0400 >> 9) << 3;
      coco_samupdate();
      break;
    case fdragon:
      d64mode = 1;
      samreg = (0x0400 >> 9) << 3;
      coco_samupdate();
    default:
      break;
  }

  hardware->dirty = 1;

  return 0;
}

Err EmuLoad(void) {
  Int16 i, n;
  RomType *rom;
  AppPrefs *prefs;
  MachineType *machine;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  n = mch_getnumroms(prefs->machine);
  
  for (i = 0; i < n; i++){
    if ((rom = mch_getrom(prefs->machine, i)) == NULL)
      break; 

    if (LoadROM(rom) != 0)
      return -1;
  }

  switch (machine->family) {
    case fcoco:
    case fdragon:
      for (i = 0x02; i <= 0x0F; i++)
        hardware->bank1[0x7FF0 + i] = hardware->bank1[0x3FF0 + i];
    default:
      break;
  }

  return 0;
}

void EmuReset(void) {
  AppPrefs *prefs;
  MachineType *machine;

  running = true;

  arg.cmd = CMD_RESET;
  cpuFunc(&arg);

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  if (machine->resetFunction)
    machine->resetFunction();

#ifdef F_CPM
  if (machine->family == fcpm)
    if (read_sector(0, 0, 1, hardware->bank0) != 0)
      running = false;
#endif

  cas_motor(0);

  snd_count = 0;
  hardware->snd_count = 0;

  snd_tonefreq = 0;
  hardware->snd_tonefreq = 0;

  snd_toneamp = 0;
  hardware->snd_toneamp = 0;

  snd_noisefreq = 0;
  hardware->snd_noisefreq = 0;

  snd_noiseamp = 0;
  hardware->snd_noiseamp = 0;
}

static Err LoadCocoSnap(char *name) {
  AppPrefs *prefs;
  MachineType *machine;
  Int16 vol;
  FileRef f;
  SnapState state;
  SnapRegs regs;
  char *s;
  Err err;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  if ((vol = check_volume()) == -1) {
    InfoDialog(D_ERROR, "No volume found");
    return -1;
  }

  MemSet(list->buf, sizeof(list->buf), 0);
  StrCopy(list->buf, AppDir);
  StrCat(list->buf, "/Snapshot/");
  StrCat(list->buf, machine->fname);
  StrCat(list->buf, "/");
  StrCat(list->buf, name);

  s = "snapshot";

  if ((err = VFSFileOpen(vol, list->buf, vfsModeRead, &f)) != 0) {
    InfoDialog(D_ERROR, "Can not load %s (%d)", s, err);
    return -1;
  }

  MemSet(&state, sizeof(state), 0);
  state.machine = prefs->machine;
  state.ramsize = prefs->ramsize;
  state.samreg = samreg;

  state.irq_state0 = m6809.irq_state[0] ? 1 : 0;
  state.irq_state1 = m6809.irq_state[1] ? 1 : 0;
  state.int_state = m6809.int_state ? 1 : 0;
  state.nmi_state = m6809.nmi_state ? 1 : 0;

  MemSet(&regs, sizeof(regs), 0);
  regs.r[REG_PC] = m6809.pc.w.h;
  regs.r[REG_D] = m6809.d.w.h;
  regs.r[REG_X] = m6809.x.w.h;
  regs.r[REG_Y] = m6809.y.w.h;
  regs.r[REG_U] = m6809.u.w.h;
  regs.r[REG_S] = m6809.s.w.h;
  regs.r[REG_DPCC] = (((UInt16)m6809.dp.b.h2) << 8) | (m6809.cc >> 24);

  err = read_snap(f, &state, &regs, hardware);
  VFSFileClose(f);

  if (err != 0) {
    InfoDialog(D_ERROR, "Invalid %s file (%d)", s, err);
    EmuStart();
    return -1;
  }

  if (state.machine != prefs->machine) {
    InfoDialog(D_ERROR, "Machine or RAM size is different");
    EmuStart();
    return -1;
  }

  vdg_filled = false;

  m6809.pc.w.h = regs.r[REG_PC];
  m6809.d.w.h = regs.r[REG_D];
  m6809.x.w.h = regs.r[REG_X];
  m6809.y.w.h = regs.r[REG_Y];
  m6809.u.w.h = regs.r[REG_U];
  m6809.s.w.h = regs.r[REG_S];
  m6809.dp.b.h2 = regs.r[REG_DPCC] >> 8;
  m6809.cc = ((UInt32)(regs.r[REG_DPCC] & 0xFF)) << 24;

  coco_writeb(0x00, hardware->bank1[0x7F00]);
  coco_writeb(0x01, hardware->bank1[0x7F01]);
  coco_writeb(0x02, hardware->bank1[0x7F02]);
  coco_writeb(0x03, hardware->bank1[0x7F03]);
  coco_writeb(0x20, hardware->bank1[0x7F20]);
  coco_writeb(0x21, hardware->bank1[0x7F21]);
  coco_writeb(0x22, hardware->bank1[0x7F22]);
  coco_writeb(0x23, hardware->bank1[0x7F23]);

  samreg = state.samreg;
  coco_samupdate();

  m6809.irq_state[0] = state.irq_state0 ? 1 : 0;
  m6809.irq_state[1] = state.irq_state1 ? 1 : 0;
  m6809.int_state = state.int_state ? 1 : 0;
  m6809.nmi_state = state.nmi_state ? 1 : 0;

  m6809.irq_state[0] = m6809.irq_state[0];
  m6809.irq_state[1] = m6809.irq_state[1];
  m6809.int_state = m6809.int_state;
  m6809.nmi_state = state.nmi_state;

  return 0;
}

static Err SaveCocoSnap(char *name) {
  AppPrefs *prefs;
  MachineType *machine;
  Int16 vol;
  FileRef f;
  SnapState state;
  SnapRegs regs;
  Err err;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  if (name[0] == '$' && !StrCompare(&name[1], devstring)) {
    developer = true;
    return 0;
  }

  if ((vol = check_volume()) == -1) {
    InfoDialog(D_ERROR, "No volume found");
    return -1;
  }

  MemSet(list->buf, sizeof(list->buf), 0);
  StrCopy(list->buf, AppDir);
  StrCat(list->buf, "/Snapshot/");
  StrCat(list->buf, machine->fname);
  StrCat(list->buf, "/");
  StrCat(list->buf, name);

  if ((err = VFSFileOpen(vol, list->buf,
        vfsModeCreate | vfsModeTruncate | vfsModeWrite, &f)) != 0) {
    InfoDialog(D_ERROR, "Can not save snapshot (%d)", err);
    return -1;
  }

  state.machine = prefs->machine;
  state.ramsize = prefs->ramsize;

  state.samreg = samreg;
  state.irq_state0 = m6809.irq_state[0] ? 1 : 0;
  state.irq_state1 = m6809.irq_state[1] ? 1 : 0;
  state.int_state = m6809.int_state ? 1 : 0;
  state.nmi_state = m6809.nmi_state ? 1 : 0;

  regs.r[REG_PC] = m6809.pc.w.h;
  regs.r[REG_D] = m6809.d.w.h;
  regs.r[REG_X] = m6809.x.w.h;
  regs.r[REG_Y] = m6809.y.w.h;
  regs.r[REG_U] = m6809.u.w.h;
  regs.r[REG_S] = m6809.s.w.h;
  regs.r[REG_DPCC] = (((UInt16)m6809.dp.b.h2) << 8) | (m6809.cc >> 24);

  if ((err = write_snap(f, &state, &regs, hardware)) != 0) {
    VFSFileClose(f);
    InfoDialog(D_ERROR, "Can not save snapshot (%d)", err);
    return -1;
  }

  VFSFileClose(f);

  return 0;
}

static Err LoadBin(char *name) {
  AppPrefs *prefs;
  MachineType *machine;
  Int16 vol;
  FileRef f;
  SnapState state;
  SnapRegs regs;
  Err err;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  if ((vol = check_volume()) == -1) {
    InfoDialog(D_ERROR, "No volume found");
    return -1;
  }

  MemSet(list->buf, sizeof(list->buf), 0);
  StrCopy(list->buf, AppDir);
  StrCat(list->buf, "/Disk/");
  StrCat(list->buf, machine->fname);
  StrCat(list->buf, "/");
  StrCat(list->buf, dskname);
  StrCat(list->buf, name);

  if ((err = VFSFileOpen(vol, list->buf, vfsModeRead, &f)) != 0) {
    InfoDialog(D_ERROR, "Can not load Binary (%d)", err);
    return -1;
  }

  if ((err = read_bin(f, &state, &regs, hardware)) != 0) {
    VFSFileClose(f);
    InfoDialog(D_ERROR, "Invalid Binary file (%d)", err);
    EmuStart();
    return -1;
  }

  VFSFileClose(f);
  m6809.pc.w.h = regs.r[REG_PC];

  return 0;
}

static Err LoadDskBin(char *name) {
  AppPrefs *prefs;
  MachineType *machine;
  SnapState state;
  SnapRegs regs;
  UInt16 index;
  Err err;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  for (index = 0; index < dsk.numentries; index++)
    if (!StrCompare(dsk.dir[index].name, name))
      break;

  if (index == dsk.numentries)
    return -1;

  switch (machine->family) {
#ifdef F_CGENIE
    case fcgenie:
      err = read_cgdsk(fdsk, &dsk, index, &z80, hardware);
      break;
#endif
    default:
      err = read_dsk(fdsk, &dsk, index, &state, &regs, hardware);
      m6809.pc.w.h = regs.r[REG_PC];
  }

  if (err != 0) {
    InfoDialog(D_ERROR, "Invalid Disk or Binary file (%d)", err);
    EmuStart();
    return -1;
  }

  return 0;
}

static Err LoadSnap(char *name) {
  AppPrefs *prefs;
  MachineType *machine;
  Int16 vol;
  FileRef f;
  SnapState state;
  Err err;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);

  if ((vol = check_volume()) == -1) {
    InfoDialog(D_ERROR, "No volume found");
    return -1;
  }

  MemSet(list->buf, sizeof(list->buf), 0);
  StrCopy(list->buf, AppDir);
  StrCat(list->buf, "/Snapshot/");
  StrCat(list->buf, machine->fname);
  StrCat(list->buf, "/");
  StrCat(list->buf, name);

  if ((err = VFSFileOpen(vol, list->buf, vfsModeRead, &f)) != 0) {
    InfoDialog(D_ERROR, "Can not load file (%d)", err);
    return -1;
  }

  switch (machine->family) {
    case fspec:
      err = read_z80(f, &state, &z80, hardware);
      break;
    case fvz:
      err = vz_readvz(f, &z80, hardware);
      break;
#ifdef F_VIC20
    case fvic20:
      err = vic_readprg(f);
      break;
#endif
#ifdef F_ORIC
    case foric:
      err = oric_readtap(f, &m6502, hardware);
      break;
#endif
#ifdef F_ATOM
    case fatom:
      err = atom_readatm(f, &m6502, hardware);
      break;
#endif
#ifdef F_JUPITER
    case fjupiter:
      err = jupiter_readdic(f, &z80, hardware);
      break;
#endif
#ifdef F_AQUARIUS
    case faquarius:
      err = aquarius_readbin(f, &z80, hardware);
      break;
#endif
#ifdef F_MSX
    case fmsx:
      err = msx_readcart(f, &z80, hardware);
      break;
#endif
#ifdef F_COLECO
    case fcoleco:
      err = coleco_readcart(f, &z80, hardware);
      break;
#endif
#ifdef F_NES
    case fnes:
      err = nes_readcart(f, &m6502, hardware, name);
      break;
#endif
    default:
      VFSFileClose(f);
      return -1;
  }

  VFSFileClose(f);

  if (err != 0) {
    InfoDialog(D_ERROR, "Invalid or not suported file (%d)", err);
    EmuStart();
    return -1;
  }

  //if (machine->id == spectrum)
    //spectrum_writeb(0, state.border);

  if (machine->family == faquarius || machine->family == fmsx ||
      machine->family == fcoleco || machine->family == fnes)
    EmuReset();

  return 0;
}

static FileRef CheckROM(RomType *rom) {
  AppPrefs *prefs;
  MachineType *machine;
  Int16 vol;
  UInt32 size;
  FileRef f;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);
  
  if ((vol = check_volume()) == -1)
    return NULL;

  MemSet(list->buf, sizeof(list->buf), 0);
  StrCopy(list->buf, AppDir);
  StrCat(list->buf, "/ROM/");
  StrCat(list->buf, machine->fname);
  StrCat(list->buf, "/");
  StrCat(list->buf, rom->name);

  if (VFSFileOpen(vol, list->buf, vfsModeRead, &f) != 0)
    return NULL;
  
  if (VFSFileSize(f, &size) != 0 || size != rom->size) {
    VFSFileClose(f); 
    return NULL;
  }
    
  return f;
}

static Err LoadROM(RomType *rom) {
  UInt32 size, r;
  FileRef f;
  Int16 vol;
  UInt8 *addr;
  Err err;

  if ((vol = check_volume()) == -1) {
    InfoDialog(D_ERROR, "No volume found");
    return -1;
  }

  if ((f = CheckROM(rom)) == NULL) {
    if (rom->optional)
      return 0;
    InfoDialog(D_ERROR, "Invalid or missing ROM file %s", rom->name);
    return -1;
  }

  VFSFileSize(f, &size);

  if (!rom->preload) {
    if (!rom2)
      rom2 = sys_calloc(size, 1);
    addr = rom2;
  } else
    addr = rom->start < 0x8000 ? &hardware->bank1[rom->start] :
                                 &hardware->bank0[rom->start & 0x7FFF];

  if ((err = VFSFileRead(f, size, addr, &r)) != 0 ||
       r != size) {
    VFSFileClose(f);
    InfoDialog(D_ERROR, "Error reading %s (%d)", rom->name, err);
    return -1;
  }

  VFSFileClose(f);

  return 0;
}

void io_kbselect(UInt8 b) {
  kbcolumn = b;
}

UInt8 coco_kbline(void) {
  UInt8 b = 0x7F;
  UInt32 mask = KeyCurrentState();
  CocoPrefs *prefs = CocoGetPrefs();

  if (mask & prefs->button) {  // joystick button
    b = (prefs->joystick == JOY_RIGHT) ? 0x7E : 0x7D;
  } else if (key && kbcolumn != 0xFF) {
    if ((kbcolumn^0xFF) & (keyMap[key].column^0xFF))
      b = keyMap[key].line & 0x7F;
    if (kbcolumn == SHIFT_COLUMN && keyMap[key].shift)
      b &= SHIFT_LINE;
  }

  if (prefs->joystick == joy_control && joy_coord[joy_axis] > da_value)
    b |= 0x80;

  return b;
}

void snd_output(UInt8 b) {
  if (snd_count < snd_samples) {
    UInt32 cycle = hardware->totalcycles;
    snd_buffer[snd_count].cycle = cycle;
    snd_buffer[snd_count].sample = b;
    snd_count++;
  }
}

void snd_settone(UInt32 freq, UInt32 amp) {
  snd_tonefreq = freq;
  snd_toneamp = amp;
}

void snd_calcnoise(void) {
  SoundSample *snd_buffer;
  UInt32 i, n, freq, amp, cycle, sample, cps;

  freq = hardware->snd_noisefreq;
  amp = hardware->snd_noiseamp;
  n = 0;

  if (freq != 0 && amp != 0) {
    snd_buffer = hardware->snd_buffer;
    n = hardware->snd_samples;
    cps = (hardware->cycles_per_sample * SAMPLE_RATE) / freq;
    cycle = 0;

    for (i = 0; i < n; i++) {
      sample = hardware->rnd[i];
      snd_buffer[hardware->snd_count].cycle = cycle;
      snd_buffer[hardware->snd_count].sample = sample;
      cycle += cps;
    }
  }

  hardware->snd_count = n;
}

void snd_setnoise(UInt32 freq, UInt32 amp) {
  hardware->snd_noisefreq = freq;
  hardware->snd_noiseamp = amp;
}

void io_daset(UInt8 b) {
  da_value = b;
  if (snd_count < snd_samples) {
    snd_buffer[snd_count].cycle = hardware->totalcycles;
    snd_buffer[snd_count].sample =
       (snd_enable && snd_mux == 0) ? da_value * 4 : 0x00;
    snd_count++;
  }
}

void io_snd(UInt8 b) {
  snd_enable = b ? 1 : 0;
}

void io_snd1bit(UInt8 b) {
  snd_1bit = b ? 0x7F : 0;
}

void io_sel1(UInt8 b) {
  joy_axis = b ? JOY_YAXIS : JOY_XAXIS;
  snd_mux = (snd_mux & 0x02) | (b ? 0x01 : 0x00);
}

void io_sel2(UInt8 b) {
  joy_control = b ? JOY_LEFT : JOY_RIGHT;
  snd_mux = (snd_mux & 0x01) | (b ? 0x02 : 0x00);
}

void io_setvdg(void) {
  io_vdg(vdg);
}

void io_vdg(UInt8 b) {
  AppPrefs *prefs;
  MachineType *machine;
  RectangleType rect;
  UInt8 artifacting;

  if ((b & 0x80) == VDG_TEXT) {
    UInt16 c, x;
    WinHandle old;

    if ((vdg & 0x80) != VDG_TEXT) {
      border = true;
    }

    if ((vdg & 0x80) != VDG_TEXT || (vdg & 0x18) != (b & 0x18) || !vdg_filled) {
      WinSetCoordinateSystem(kCoordinatesDouble);
      old = WinSetDrawWindow(vdg_wh);
      WinPushDrawState();
      WinSetTextColor(hardware->color[(b & 0x08) ?  COLOR_DARKORANGE : COLOR_DARKGREEN]);
      WinSetBackColor(hardware->color[(b & 0x08) ?  COLOR_BRIGHTORANGE : COLOR_GREEN]);
      RctSetRectangle(&rect, 0, 0, 256*font_width, font_height);
      WinEraseRectangle(&rect, 0);

      FntSetFont(fontID);

      prefs = GetPrefs();
      machine = mch_getmachine(prefs->machine);

      if (machine->family == fmc1000) {
        for (c = 0, x = 0; c < 64; c++, x += font_width) {
          WinPaintChar(c+64, x, 0);
        }
        for (; c < 128; c++, x += font_width) {
          WinPaintChar(c, x, 0);
        }

        for (; c < 192; c++, x += font_width) {
          WinPaintChar(c-128, x, 0);
        }
        for (; c < 256; c++, x += font_width) {
          WinPaintChar(c-192, x, 0);
        }

      } else if (machine->family == fatom) {
        UInt16 c1, c2;

        c1 = hardware->color[(b & 0x08) ? COLOR_DARKORANGE : COLOR_DARKGREEN];
        c2 = hardware->color[(b & 0x08) ? COLOR_BRIGHTORANGE : COLOR_GREEN];
        WinSetTextColor(c1);
        WinSetBackColor(c2);
        for (c = 0, x = 0; c < 64; c++, x += font_width)
          WinPaintChar(c, x, 0);

        WinSetTextColor(hardware->color[COLOR_BLACK]);
        WinSetBackColor(c2);
        for (; c < 128; c++, x += font_width)
          WinPaintChar(c, x, 0);

        WinSetTextColor(c1);
        WinSetBackColor(c2);
        for (; c < 192; c++, x += font_width)
          WinPaintChar(c, x, 0);

        WinSetTextColor(hardware->color[COLOR_BLACK]);
        WinSetBackColor(hardware->color[COLOR_DARKGREEN]);
        for (; c < 256; c++, x += font_width)
          WinPaintChar(c-128, x, 0);

      } else {
        for (c = 0, x = 0; c < 32; c++, x += font_width) {
          if (machine->id == coco2b && (b & 0x10)) {
            // lower case
            WinPaintChar(c+144, x, 0);
          } else {
            // inverted
            WinPaintChar(c, x, 0);
          }
        }

        for (; c < 128; c++, x += font_width) {
          WinPaintChar(c, x, 0);
        }

        WinSetBackColor(hardware->color[COLOR_BLACK]);
        for (; c < 256; c++, x += font_width) {
          WinSetTextColor(hardware->color[1 + ((c >> 4) & 0x07)]);
          WinPaintChar(128 + (c & 0x0F), x, 0);
        }
      }

      WinPopDrawState();
      WinSetDrawWindow(old);
      WinSetCoordinateSystem(kCoordinatesStandard);

      vdg_filled = true;
    }

  } else {
    UInt16 i, k, pattern, mask;
    UInt8 c, base, ac;

    if ((vdg & 0x80) == VDG_TEXT || (vdg & 0x08) != (b & 0x08)) border = true;
    c = (b & 0x08) ? COLOR_WHITE : COLOR_GREEN;

    switch (b & 0x70) {
      case 0x00:  // 64x64,   4 colors
        for (i = 0, pattern = 0; pattern < 256; pattern++)
          for (mask = 8; mask > 0; mask -= 2) {
            base = hardware->color[((pattern >> (mask-2)) & 0x03) + c];
            vdg_ptr[i++] = base;
            vdg_ptr[i++] = base;
            vdg_ptr[i++] = base;
            vdg_ptr[i++] = base;
          }
        break;

      case 0x10:  // 128x64,  2 colors
      case 0x30:  // 128x96,  2 colors (PMODE 0)
      case 0x50:  // 128x192, 2 colors (PMODE 2)
        for (i = 0, pattern = 0; pattern < 256; pattern++)
          for (mask = 0x80; mask > 0; mask >>= 1) {
            base = hardware->color[(pattern & mask) ? c : COLOR_BLACK];
            vdg_ptr[i++] = base;
            vdg_ptr[i++] = base;
          }
        break;

      case 0x20:  // 128x64,  4 colors
      case 0x40:  // 128x96,  4 colors (PMODE 1)
      case 0x60:  // 128x192, 4 colors (PMODE 3)
        for (i = 0, pattern = 0; pattern < 256; pattern++)
          for (mask = 8; mask > 0; mask -= 2) {
            base = hardware->color[((pattern >> (mask-2)) & 0x03) + c];
            vdg_ptr[i++] = base;
            vdg_ptr[i++] = base;
          }
        break;

      case 0x70:  // 256x192, 2 colors (PMODE 4)
        prefs = GetPrefs();
        machine = mch_getmachine(prefs->machine);

        switch (machine->family) {
          case fcoco:
          case fdragon:
            artifacting = CocoGetPrefs()->artifacting;
            break;
          case fmc10:
            artifacting = Mc10GetPrefs()->artifacting;
            break;
          default:
            artifacting = 0;
        }

        for (i = 0, pattern = 0; pattern < 256; pattern++) {
          k = i;
          for (mask = 0x80; mask > 0; mask >>= 1) {
            vdg_ptr[i++] = hardware->color[(pattern & mask) ? c : COLOR_BLACK];

            if (artifacting && c == COLOR_WHITE && (i-k) > 2) {
              if (!((i-1) % 2) &&
                  vdg_ptr[i-1] == hardware->color[COLOR_BLACK] &&
                  vdg_ptr[i-2] != hardware->color[COLOR_BLACK] &&
                  vdg_ptr[i-3] == hardware->color[COLOR_BLACK]) {
                ac = artifacting == 1 ? COLOR_BLUE : COLOR_GRED;
                vdg_ptr[i-1] = hardware->color[ac];
                vdg_ptr[i-3] = hardware->color[ac];
              } else if (((i-1) % 2) &&
                  vdg_ptr[i-1] == hardware->color[COLOR_BLACK] &&
                  vdg_ptr[i-2] != hardware->color[COLOR_BLACK] &&
                  vdg_ptr[i-3] == hardware->color[COLOR_BLACK]) {
                ac = artifacting == 1 ? COLOR_GRED : COLOR_BLUE;
                vdg_ptr[i-1] = hardware->color[ac];
                vdg_ptr[i-3] = hardware->color[ac];
              }
            }
          }
        }
    }
  }

  vdg = b;
  hardware->dirty = 1;

  hardware->x0 = (hardware->display_width - 256) / 2;
  hardware->y0 = (hardware->display_height - 192 - lowBorder*2) / 2;
  hardware->dx = 256;
  hardware->dy = 192;
}

void io_sam(UInt8 b)
{
  sam = b;
  m6847_gpend();
}

void io_samreg(UInt8 reg)
{
  if (reg & 1)
    samreg |= (1 << (reg >> 1));
  else
    samreg &= ~(1 << (reg >> 1));
}
  
static void coco_samupdate(void)
{
  AppPrefs *prefs;
  UInt32 memory;
  UInt16 oldsam = sam;
  UInt32 oldgpb = hardware->gp_begin;

  prefs = GetPrefs();

  sam = samreg & 0x0007;
  hardware->gp_begin = (samreg & 0x03F8) << 6;
  m6847_gpend();

  switch (sam) {
    case 0: row_height = font_height;    break;
    case 2: row_height = font_height/4;  break;
    case 4: row_height = font_height/6;  break;
    case 6: row_height = font_height/12;
  }

  if (sam != oldsam || hardware->gp_begin != oldgpb)
    hardware->dirty = 1;

  switch (prefs->ramsize) {
    case RAM_4K:
      samreg &= 0x9FFF;
      break;
    case RAM_16K:
      samreg &= 0x9FFF;
      samreg |= 0x2000;
      break;
    case RAM_32K:
    case RAM_64K:
    default:
      samreg &= 0x9FFF;
      samreg |= 0x4000;
  }

  memory = (samreg & 0x6000) >> 13;

  switch (memory) {
    case 0:
      memory = RAM_4K;
      break;
    case 1:
      memory = RAM_16K;
      break;
    default:
      memory = RAM_32K;
  }
  hardware->ramsize = memory * 4096 - 1;

  if (prefs->ramsize == RAM_64K) {
    io_bank((samreg & 0x8000) ? 1 : 0);
  }
}

void m6847_gpend(void)
{
  switch (sam) {
    case 0:
      hardware->gp_end = hardware->gp_begin + 512;
      break;
    case 1:
      hardware->gp_end = hardware->gp_begin + 1 * 1024;
      break;
    case 2:
    case 3:
      if (vdg & 0x80)
        hardware->gp_end = hardware->gp_begin + 1 * 1536;
      else
        hardware->gp_end = hardware->gp_begin + 2 * 1024;
      break;
    case 4:
    case 5:
      hardware->gp_end = hardware->gp_begin + 3 * 1024;
      break;
    case 6:
      hardware->gp_end = hardware->gp_begin + 6 * 1024;
  }
}

void m6847_border(void) {
  RectangleType rect;
  UInt8 c;

  if ((vdg & 0x80) == VDG_TEXT) {
    c = hardware->color[COLOR_BLACK];
  } else {
    c = hardware->color[(vdg & 0x08) ? COLOR_WHITE : COLOR_GREEN];
  }

  WinSetBackColor(c);
  RctSetRectangle(&rect, 0, 0, hardware->display_width, display_y0+192+display_y0);
  WinEraseRectangle(&rect, 0);

/*
  for (i = 0; i < display_y0; i++, s += hardware->display_width) {
    MemSet(s, hardware->display_width, c);
  }

  for (; i < display_y0+192; i++) {
    MemSet(s, display_x0, c);
    s += display_x0+256;
    MemSet(s, display_x0, c);
    s += display_x0;
  }

  for (; i < display_y0+192+display_y0; i++, s += hardware->display_width) {
    MemSet(s, hardware->display_width, c);
  }
*/

  border = false;
}

void m6847_graphic(void) {
  AppPrefs *prefs;
  MachineType *machine;
  RectangleType srcRect;
  UInt16 i, j, addr;
  UInt8 *vram;
  Err err;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);
  WinSetCoordinateSystem(kCoordinatesDouble);

  if (hardware->screen_wh == NULL) {
    hardware->screen_wh = WinCreateOffscreenWindow(256, 192, nativeFormat, &err);
  }

  addr = hardware->gp_begin;

  if (addr >= 0x8000) {
    vram = hardware->m1;
    addr -= 0x8000;
  } else {
    if (machine->family == fvz) {
      vram = hardware->m1;
    } else {
      vram = hardware->m0;
    }
  }

  if (border) {
    m6847_border();
  }

  switch (sam) {
    case 1:  // 64x64x4, 128x64x2
      for (i = 0; i < 192; i += 3) {
        for (j = 0; j < 16; j++) {
          RctSetRectangle(&srcRect, vram[addr++] << 4, 0, 16, 1);
          WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*16, i, winPaint);
          WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*16, i+1, winPaint);
          WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*16, i+2, winPaint);
        }

        if (addr == 0x8000) {
          vram = hardware->m1;
          addr = 0;
        }
      }
      break;
    case 2:  // 128x64x4
      for (i = 0; i < 192; i += 3) {
        for (j = 0; j < 32; j++) {
          RctSetRectangle(&srcRect, vram[addr++] << 3, 0, 8, 1);
          WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*8, i, winPaint);
          WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*8, i+1, winPaint);
          WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*8, i+2, winPaint);
        }

        if (addr == 0x8000) {
          vram = hardware->m1;
          addr = 0;
        }
      }
      break;
    case 3:  // PMODE 0
      for (i = 0; i < 192; i += 2) {
        for (j = 0; j < 16; j++) {
          RctSetRectangle(&srcRect, vram[addr++] << 4, 0, 16, 1);
          WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*16, i, winPaint);
          WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*16, i+1, winPaint);
        }

        if (addr == 0x8000) {
          vram = hardware->m1;
          addr = 0;
        }
      }
      break;
    case 4:  // PMODE 1
      for (i = 0; i < 192; i += 2) {
        for (j = 0; j < 32; j++) {
          RctSetRectangle(&srcRect, vram[addr++] << 3, 0, 8, 1);
          WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*8, i, winPaint);
          WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*8, i+1, winPaint);
        }

        if (addr == 0x8000) {
          vram = hardware->m1;
          addr = 0;
        }
      }
      break;
    case 5:  // PMODE 2
      for (i = 0; i < 192; i++) {
        for (j = 0; j < 16; j++) {
          RctSetRectangle(&srcRect, vram[addr++] << 4, 0, 16, 1);
          WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*16, i, winPaint);
        }

        if (addr == 0x8000) {
          vram = hardware->m1;
          addr = 0;
        }
      }
      break;
    case 6:  // PMODE 3 e PMODE 4
      for (i = 0; i < 192; i++) {
        for (j = 0; j < 32; j++) {
          RctSetRectangle(&srcRect, vram[addr++] << 3, 0, 8, 1);
          WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*8, i, winPaint);
        }

        if (addr == 0x8000) {
          vram = hardware->m1;
          addr = 0;
        }
      }
  }

  RctSetRectangle(&srcRect, 0, 0, 256, 192);
  WinCopyRectangle(hardware->screen_wh, NULL, &srcRect, display_x0, display_y0, winPaint);
  WinSetCoordinateSystem(kCoordinatesStandard);

  hardware->dirty = 0;
}

void m6847_text(void) {
  AppPrefs *prefs;
  MachineType *machine;
  RectangleType srcRect;
  UInt16 i, j, r, f, addr;
  UInt8 *vram;
  Err err;

  prefs = GetPrefs();
  machine = mch_getmachine(prefs->machine);
  WinSetCoordinateSystem(kCoordinatesDouble);

  if (hardware->screen_wh == NULL) {
    hardware->screen_wh = WinCreateOffscreenWindow(256, 192, nativeFormat, &err);
  }

  addr = hardware->gp_begin;

  if (addr >= 0x8000) {
    vram = hardware->m1;
    addr -= 0x8000;
  } else {
    if (machine->family == fvz) {
      vram = hardware->m1;
    } else {
      vram = hardware->m0;
    }
  }

  if (border) {
    m6847_border();
  }

  r = f = 0;

  for (i = 0; i < 192; i++) {
    for (j = 0; j < 32; j++) {
       RctSetRectangle(&srcRect, vram[addr+j] << 3, f, 8, 1);
       WinCopyRectangle(vdg_wh, hardware->screen_wh, &srcRect, j*8, i, winPaint);
    }

    if (addr == 0x8000) {
      vram = hardware->m1;
      addr = 0;
    }

    r++;
    if (r == row_height) {
      addr += 32;
      r = 0;
    }
    f++;
    if (f == font_height) {
      f = 0;
    }
  }

  RctSetRectangle(&srcRect, 0, 0, 256, 192);
  WinCopyRectangle(hardware->screen_wh, NULL, &srcRect, display_x0, display_y0, winPaint);
  WinSetCoordinateSystem(kCoordinatesStandard);

  hardware->dirty = 0;
}

static void coco_d64mode(UInt8 mode) {
  UInt16 i, pc;
  UInt8 b;

  // 1=32K, 0=64K

  pc = m6809.pc.w.l;
  if (pc >= 0x024D) {
    return;
  }

  if (mode != d64mode) {
    for (i = 0; i < 0x4000; i++) {
      b = hardware->bank1[i];
      hardware->bank1[i] = rom2[i];
      rom2[i] = b;
    }

    for (i = 0x02; i <= 0x0F; i++) {
      hardware->bank1[0x7FF0 + i] = hardware->bank1[0x3FF0 + i];
    }

    d64mode = mode;
  }
}

void io_halt(void) {
  running = false;
  wait = evtWaitForever;
}

void io_run(void) {
  if (iowait) {
    iowait = false;
    running = true;
    wait = waitTicks;
  }
}

void io_setgp(UInt32 begin, UInt32 end) {
  hardware->gp_begin = begin;
  hardware->gp_end = end;
  hardware->dirty = 1;
}

void io_bank(UInt8 b) {
  hardware->m1 = b ? hardware->bank2 : hardware->bank1;
  hardware->m1w = b;
}

void io_hsync(UInt8 b) {
  hardware->bank1[0x7F01] |= 0x80;
}

void m6847_video(void) {
  if ((hardware->dirty || border) && (numVsyncs & 1)) {
    if ((vdg & 0x80) == VDG_GRAPHIC) {
      m6847_graphic();
    } else {
      m6847_text();
    }
  }
}

void coco_vsync(UInt8 b) {
  hardware->bank1[0x7F03] |= 0x80;
  m6847_video();
  io_sample();
  io_vsync();
}

void mc10_vsync(UInt8 b) {
  m6847_video();
  io_sample();
  io_vsync();
}

void spectrum_vsync(UInt8 b) {
  io_sample2();
  io_vsync();
}

void apple_vsync(UInt8 b) {
  apple_video(hardware->dirty != 0);
  io_vsync();
  hardware->dirty = 0;
}

void mc1000_vsync(UInt8 b) {
  UInt32 amp = hardware->snd_noiseamp;
  dac_setamp(amp);
  m6847_video();
  io_sample2();
  io_tone2();
  io_vsync();
}

void vz_vsync(UInt8 b) {   
  m6847_video();
  io_sample();
  io_vsync();
}

void vic_vsync(UInt8 b) {
  io_tone2();
  io_vsync();
}

void oric_vsync(UInt8 b) {
  UInt32 amp = hardware->snd_noiseamp;
  dac_setamp(amp);
  io_sample2();
  io_tone2();
  io_vsync();
}

void atom_vsync(UInt8 b) {
  m6847_video();
  io_sample();
  io_vsync();
}

void cgenie_vsync(UInt8 b) {
  UInt32 amp = hardware->snd_noiseamp;
  dac_setamp(amp);
  io_sample2();
  io_tone2();
  io_vsync();
}

void jupiter_vsync(UInt8 b) {
  io_sample2();
  io_vsync();
}

void aquarius_vsync(UInt8 b) {
  io_sample2();
  io_vsync();
}

void msx_vsync(UInt8 b) {
  UInt32 amp = hardware->snd_noiseamp;
  dac_setamp(amp);
  io_sample2();
  io_tone2();
  io_vsync();
}

void coleco_vsync(UInt8 b) {
  io_tone2();
  io_vsync();
}

void nes_vsync(UInt8 b) {
  //io_tone2();
  io_vsync();
}

static void io_sample(void) {
  dac_buffer(snd_buffer, snd_count);
  snd_count = 0;
}

static void io_sample2(void) {
  snd_count = hardware->snd_count;
  io_sample();
  hardware->snd_count = 0;
}

static void io_tone(void) {
  dac_settone(snd_tonefreq, snd_toneamp);
}

static void io_tone2(void) {
  snd_tonefreq = hardware->snd_tonefreq;
  snd_toneamp = hardware->snd_toneamp;
  io_tone();
}

static void io_vsync(void) {
  UInt32 now, delay;

  now = TimGetTicks();
  delay = now - lastTicks;

  if (delay < ticksPerVsync[numVsyncs]) {
    waitTicks = ticksPerVsync[numVsyncs] - delay;
  } else {
    waitTicks = 0;
  }

  numVsyncs++;
  wait = waitTicks;

  if (numVsyncs == maxVsyncs) {
    if (developer) {
      AppPrefs *prefs;
      MachineType *machine;

      prefs = GetPrefs();
      machine = mch_getmachine(prefs->machine);

      if (machine->osdFunction) {
        machine->osdFunction(delay);
      }
    }

    numVsyncs = 0;
    lastTicks = now;
  }
}

void io_cart(UInt8 b) {
  hardware->bank1[0x7F23] |= 0x80;
}

void coco_key(UInt16 c) {
  if (c >= 'a' && c <= 'z') {
    c &= 0xDF;
  }

  key = c;
}

void coco_joystick(UInt16 x, UInt16 y) {
  joy_coord[JOY_XAXIS] = x;
  joy_coord[JOY_YAXIS] = y;
}

UInt8 coco_readb(UInt16 a) {
  CocoPrefs *prefs;
  UInt8 b;

  switch (a) {
    case 0x00:
      return coco_kbline();
    case 0x01:
      b = hardware->bank1[0x7F01];
      hardware->bank1[0x7F01] &= 0x7F;
      return b;
    case 0x03:
      b = hardware->bank1[0x7F03];
      hardware->bank1[0x7F03] &= 0x7F;
      return b;
    case 0x20:
      return (hardware->bank1[0x7F20] & 0xFE) | cas_input();
    case 0x22:
      prefs = CocoGetPrefs();
      switch (prefs->id) {
        case coco:
        case cocoe:
        case coco2:
        case coco2b:
        case cp400:
          b = prefs->ramsize == RAM_4K ? 0x00 : 0x04;
          b |= (hardware->bank1[0x7F22] & 0xFA);
          // b |= io_serin(&hardware->eventcount);
          return b;
        case dragon32:
          return hardware->bank1[0x7F22] & 0xFB;  // bit2 0 = 32K
        case dragon64:
          return hardware->bank1[0x7F22];
      }
      break;
    case 0x23:
      b = hardware->bank1[0x7F23];
      hardware->bank1[0x7F23] &= 0x7F;
      return b;
  }

  return hardware->bank1[0x7F00 + a];
}

void coco_writeb(UInt16 a, UInt8 b) {
  CocoPrefs *prefs;

  switch (a) {
    case 0x01:
      io_sel1(b & 0x08);
      break;
    case 0x02:
      io_kbselect(b);
      break;
    case 0x03:
      io_sel2(b & 0x08);
      break;
    case 0x20:
      io_daset(b >> 2);
      break;
    case 0x21:
      cas_motor(b & 0x08);
      break;
    case 0x22:
      if ((b & 0xF8) != vdg)
        io_vdg(b & 0xF8);
      io_snd1bit(b & 0x02);
      prefs = CocoGetPrefs();
      if (prefs->id == dragon64) {
        coco_d64mode(b & 0x04 ? 1 : 0);
      }
      break;
    case 0x23:
      io_snd(b & 0x08);
  }

  hardware->bank1[0x7F00 + a] = b;
}

UInt32 coco_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_SAM:
      io_samreg(arg->a1);
      coco_samupdate();
      return 0;
    case IO_EVENT:
      io_hsync(arg->a1);
      return 0;
    case IO_VSYNC:
      coco_vsync(arg->a1);
      return 0;
    case IO_CART:
      io_cart(arg->a1);
      return 0;
    case IO_READB:
      return coco_readb(arg->a1);
    case IO_WRITEB:
      coco_writeb(arg->a1, arg->a2);
      return 0;
    case ILLEGAL0:
      if (developer)
        status(0, 1, "ILLEGAL OP %3d ", (UInt16)arg->a1);
      return 0;
    case ILLEGAL1:
      if (developer)
        status(0, 1, "ILLEGAL OP 16 %3d ", (UInt16)arg->a1);
      return 0;
    case ILLEGAL2:
      if (developer)
        status(0, 1, "ILLEGAL OP 17 %3d ", (UInt16)arg->a1);
      return 0;
  }
  return 0;
}

#ifdef F_CPM
UInt32 cpm_callback(ArmletCallbackArg *arg) {
  switch (arg->cmd) {
    case IO_READB:
      return cpm_readb(hardware->bank0, hardware->bank1, arg->a1);
    case IO_WRITEB:
      FntSetFont(fontID);
      cpm_writeb(hardware->bank0, hardware->bank1, arg->a1, arg->a2);
      return 0;
    case IO_HALT:
      running = false;
      wait = evtWaitForever;
      iowait = (arg->a1 != 0);
      return 0;
  }
  return 0;
}
#endif
