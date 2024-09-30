#include <PalmOS.h>

#include "script.h"
#include "thread.h"
#include "mutex.h"
#include "sys.h"
#include "vfs.h"
#include "bytes.h"
#include "rgb.h"
#include "pumpkin.h"
#include "dosbox.h"
#include "debug.h"
#include "xalloc.h"

#include "resource.h"
#include "libretro.h"

#define AppID 'DOSb'

static const char *COREPATH = "/app_card/" DOSBOX_LIBRETRO "/cores/dosbox_libretro" SOEXT;
static const char *GAMEPATH = "vfs/app_card/" DOSBOX_HOME "/dosbox.conf";

#include "Libretro.c"
