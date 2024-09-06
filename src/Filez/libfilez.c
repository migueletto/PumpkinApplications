#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include <PalmOS.h>

#include "mutex.h"
#include "script.h"
#include "pwindow.h"
#include "image.h"
#include "media.h"
#include "sys.h"
#include "vfs.h"
#include "mem.h"
#include "pdb.h"
#include "libpalmos.h"
#include "res.h"
#include "debug.h"
#include "xalloc.h"

#define AppName  "Filez"
#define AppID    'Filz'

extern UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);

extern const uint8_t _binary_resources_bin_start[];
extern const uint8_t _binary_resources_bin_end[];

static mutex_t *mutex;

static int libfilez_extract_resources(void) {
  uint32_t size, offset, rtype, p;
  uint8_t *bin;
  mem_chunk_t *chunk;
  pdb_t *pdb;
  res_t *res;
  int r = -1;

  if ((pdb = pdb_new(STORAGE_RES, AppName, AppID, sysFileTApplication)) != NULL) {
    size = _binary_resources_bin_end - _binary_resources_bin_start;

    for (offset = 0; offset < size;) {
      res = (res_t *)&_binary_resources_bin_start[offset];
      offset += sizeof(res_t);

      bin = (uint8_t *)&_binary_resources_bin_start[offset];
      offset += res->size;
      if ((offset % 4) != 0) {
        p = 4 - (offset % 4);
        offset += p;
      }

      libpalmos_s2id(&rtype, (char *)res->type);
      chunk = libpalmos_chunk_new(bin, res->size, bin);
      pdb_add_res(pdb, chunk, rtype, res->id);
    }
    r = libpalmos_add_prc(pdb);
    pdb_destroy(pdb);
  }

  return r;
}

static void pilot_main(uint16_t code, void *param, uint16_t flags) {
  if (mutex_lock(mutex) == 0) {
    libpalmos_run(AppName, PilotMain, code, param, flags);
    mutex_unlock(mutex);
  }
}

static int libfilez_deploy(int pe) {
  int r;

  r = libfilez_extract_resources();
  script_set_pointer(pe, AppName, pilot_main);

  return script_push_boolean(pe, r == 0);
}

int libfilez_load(void) {
  mutex = mutex_create(AppName);
  return mutex ? 0 : -1;
}

int libfilez_unload(void) {
  if (mutex) {
    mutex_destroy(mutex);
  }

  return 0;
}

int libfilez_init(int pe, script_ref_t obj) {
  script_add_function(pe, obj, "deploy", libfilez_deploy);

  return 0;
}
