#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "misc.h"
#include "cdebug.h"
#include "vararg.h"
#include "gui.h"

static UInt32 i;
static FileRef f = NULL;
static UInt8 buffer[4096];

void DebugInit(char *name, Boolean overwrite)
{
  Int16 vol;
  UInt32 mode;
  Err err;

  if ((vol = check_volume()) == -1)
    return;

  if (overwrite)
    mode = vfsModeCreate|vfsModeTruncate|vfsModeWrite;
  else
    mode = vfsModeCreate|vfsModeWrite;

  if (f)
    VFSFileClose(f);

  StrCopy((char *)buffer, AppDir);
  StrCat((char *)buffer, "/");
  StrCat((char *)buffer, name);

  if ((err = VFSFileOpen(vol, (char *)buffer, mode, &f)) != 0)
    return;

  i = 0;
}

void DebugFinish(void)
{
  UInt32 r;

  if (f) {
    if (i)
      VFSFileWrite(f, i, buffer, &r);
    VFSFileClose(f);
    f = NULL;
  }
}

void DebugByte(UInt8 b)
{
  UInt32 r;

  if (f) {
    buffer[i++] = b;
    if (i == 4096) {
      VFSFileWrite(f, 4096, buffer, &r);
      i = 0;
    }
  }
}

void DebugBuffer(UInt8 *b, UInt32 n)
{
  UInt32 r;

  if (f)
    VFSFileWrite(f, n, b, &r);
}

void InfoDialog(int type, const char *fmt, ...)
{
  va_list arg;
  static char buf[256];

  va_start(arg, fmt);
  StrVPrintF(buf, fmt, arg);
  va_end(arg);

  switch (type) {
    case D_INFO:
      FrmCustomAlert(InfoAlert, buf, "", "");
      break;
    case D_WARNING:
      FrmCustomAlert(WarningAlert, buf, "", "");
      break;
    case D_ERROR:
      FrmCustomAlert(ErrorAlert, buf, "", "");
      break;
    case D_FILE:
      DebugBuffer((UInt8 *)buf, StrLen(buf));
  }
}
