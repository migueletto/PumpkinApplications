#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "gui.h"
#include "machine.h"
#include "misc.h"
#include "vararg.h"

static char buffer[256];

void SelectMachine(UInt8 index, UInt8 r) { 
  FormPtr frm;
  ListType *lst;
  MachineType *machine;
  UInt16 num, i;
  char **list;
  
  frm = FrmGetActiveForm();
  machine = mch_getmachine(index);
  FldInsertStr(frm, descrFld, machine->description);
  FldInsertStr(frm, companyFld, machine->company);
  FldInsertStr(frm, yearFld, machine->year);
  
  list = buildramsizes(machine->id, machine->ramsizes, r, &num, &i);
  lst = (ListPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, ramList));
  LstSetHeight(lst, num);
  LstSetListChoices(lst, list, num);
  LstSetSelection(lst, i);
  CtlSetLabel((ControlPtr)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, ramCtl)), list[i]);
}

void FldInsertStr(FormPtr frm, UInt16 id, char *str) {
  FieldPtr fld;
  UInt16 index, len;
  FieldAttrType attr;
  Boolean old;

  index = FrmGetObjectIndex(frm, id);
  fld = (FieldPtr)FrmGetObjectPtr(frm, index);
  if (fld == NULL)
    return;

  FldGetAttributes(fld, &attr);
  old = attr.editable;
  attr.editable = true; 
  FldSetAttributes(fld, &attr);
  
  len = FldGetTextLength(fld);
  if (len)
    FldDelete(fld, 0, len);
  if (str && str[0])
    FldInsert(fld, str, StrLen(str));
  else
    FldInsert(fld, "", 0);
  
  attr.editable = old;
  FldSetAttributes(fld, &attr);
}

char *DoInput(char *msg, char *fill) { 
  static char buf[64];
  FieldType *fld;
  FormType *frm;
  UInt16 r, index;

  frm = FrmInitForm(InputForm);
  index = FrmGetObjectIndex(frm, inputFld);

  if (msg && msg[0])
    FrmSetTitle(frm, msg);
  if (fill && fill[0]) {
    fld = (FieldPtr)FrmGetObjectPtr(frm, index);
    FldInsert(fld, fill, StrLen(fill));
  }
  
  FrmSetFocus(frm, index);
  r = FrmDoDialog(frm);

  if (r == okBtn) {   
    char *s;
    fld = (FieldPtr)FrmGetObjectPtr(frm, index);
    s = FldGetTextPtr(fld);
    if (s) {
      MemSet(buf, sizeof(buf), 0);
      StrNCopy(buf, s, sizeof(buf)-1);
      FrmDeleteForm(frm);
      return buf;
    }
  }

  FrmDeleteForm(frm);
  return NULL;
}

Err check_dir(Int16 vol, char *path) { 
  FileRef dir;
  Err err;
  
  if ((err = VFSFileOpen(vol, path, vfsModeRead, &dir)) != 0) {
    if ((err = VFSDirCreate(vol, path)) != 0)
      return err;
    if ((err = VFSFileOpen(vol, path, vfsModeRead, &dir)) != 0)
      return err;
  }
  
  VFSFileClose(dir);
  return 0;
}

Int16 check_volume(void) {
  UInt32 iterator;
  Int16 i, vol;
  char path[64];
  Err err;

  vol = -1;

  for (iterator = vfsIteratorStart; iterator != vfsIteratorStop;) {
    if ((err = VFSVolumeEnumerate((UInt16 *)&i, &iterator)) != 0)
      break;
    if (i > 0) {
      vol = i;
      break;
    }
  }

  if (vol == -1)
    return -1;

  if (check_dir(vol, ProgDir) != 0)
    return -1;

  if (check_dir(vol, AppDir) != 0)
    return -1;

  MemSet(path, sizeof(path), 0);
  StrCopy(path, AppDir);
  StrCat(path, "/ROM");
  if (check_dir(vol, path) != 0)
    return -1;

  MemSet(path, sizeof(path), 0);
  StrCopy(path, AppDir);
  StrCat(path, "/Cassette");
  if (check_dir(vol, path) != 0)
    return -1;

  MemSet(path, sizeof(path), 0);
  StrCopy(path, AppDir);
  StrCat(path, "/Snapshot");
  if (check_dir(vol, path) != 0)
    return -1;

  MemSet(path, sizeof(path), 0);
  StrCopy(path, AppDir);
  StrCat(path, "/Screenshot");
  if (check_dir(vol, path) != 0)
    return -1;

  MemSet(path, sizeof(path), 0);
  StrCopy(path, AppDir);
  StrCat(path, "/Disk");
  if (check_dir(vol, path) != 0)
    return -1;

  return vol;
}

Boolean JoystickGadgetCallback(FormGadgetTypeInCallback *gad, UInt16 cmd, void *param) {
  if (cmd == formGadgetDrawCmd) {
    RGBColorType rgb;

    rgb.r = 224; rgb.g = 224; rgb.b = 224;
    WinSetForeColor(WinRGBToIndex(&rgb));
    WinDrawRectangle(&gad->rect, 0);

    rgb.r = 128; rgb.g = 128; rgb.b = 128;
    WinSetForeColor(WinRGBToIndex(&rgb));

    WinDrawLine(gad->rect.topLeft.x, gad->rect.topLeft.y,
      gad->rect.topLeft.x+gad->rect.extent.x-1, gad->rect.topLeft.y);
    WinDrawLine(gad->rect.topLeft.x, gad->rect.topLeft.y,
      gad->rect.topLeft.x, gad->rect.topLeft.y+gad->rect.extent.y-1);
    WinDrawLine(gad->rect.topLeft.x, gad->rect.topLeft.y+gad->rect.extent.y-1,
      gad->rect.topLeft.x+gad->rect.extent.x-1,
      gad->rect.topLeft.y+gad->rect.extent.y-1);
    WinDrawLine(gad->rect.topLeft.x+gad->rect.extent.x-1, gad->rect.topLeft.y,
      gad->rect.topLeft.x+gad->rect.extent.x-1,
      gad->rect.topLeft.y+gad->rect.extent.y-1);
  }

  return true;
}

void hide_control(FormPtr frm, UInt16 id) {
  UInt16 index = FrmGetObjectIndex(frm, id);
  FrmHideObject(frm, index);
}

void change_control(FormPtr frm, UInt16 id, UInt16 newid) {
  UInt16 index = FrmGetObjectIndex(frm, id);
  ControlType *ctl = (ControlType *)FrmGetObjectPtr(frm, index);
  CtlSetGraphics(ctl, newid, newid);
}

void change_label(FormPtr frm, UInt16 id, char *label) {
  UInt16 index = FrmGetObjectIndex(frm, id);
  ControlType *ctl = (ControlType *)FrmGetObjectPtr(frm, index);
  CtlSetLabel(ctl, label);
}

void status(Int16 x, Int16 y, const char *fmt, ...) {
  va_list arg;

  va_start(arg, fmt);
  StrVPrintF(buffer, fmt, arg);
  va_end(arg);

  WinPushDrawState();
  FntSetFont(internalFontID);
  WinSetTextColor(0);
  WinSetBackColor(255);
  WinSetCoordinateSystem(kCoordinatesDouble);
  WinPaintChars(buffer, StrLen(buffer), x*FntCharWidth('a'), y*FntCharHeight());
  WinSetCoordinateSystem(kCoordinatesStandard);
  WinPopDrawState();
}
