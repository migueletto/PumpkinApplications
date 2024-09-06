#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "endian.h"
#include "snapshot.h"
#include "machine.h"
#include "misc.h"
#include "dsk.h"

static Err dsk_putsector(SnapRegs *regs, Hardware *hardware, UInt16 i, UInt8 *buf, UInt16 size) SECTION("aux");
static Err dsk_seeksector(FileRef d, DskInfo *dsk, UInt16 head, UInt16 track, UInt16 sector) SECTION("aux");
static Err dsk_readsector(FileRef f, DskInfo *dsk, UInt16 head, UInt16 track, UInt16 sector, UInt8 *buf) SECTION("aux");
static Err dsk_direntry(UInt8 *buf, UInt16 index, DskDirEntry *e) SECTION("aux");

Err open_dsk(FileRef f, DskInfo *dsk)
{
  UInt8 header[8];
  UInt32 dsksize, r;
  Int16 i, j, k, n;

  dsk->heads = 1;
  dsk->tracks = 35;
  dsk->sectors = 18;
  dsk->sectorsize = 256;

  if (VFSFileSize(f, &dsksize) != 0)
    return -1;

  dsk->headersize = dsksize % 256;
  if (dsk->headersize > 5)
    return -2;

  if (dsk->headersize)
    if (VFSFileRead(f, dsk->headersize, header, &r) != 0 ||
        r != dsk->headersize)
      return -3;
  
  switch (dsk->headersize) {
    case 5:
      if (header[4] != 0)
        return -4;
    case 4: 
    case 3:
      if (header[2] > 3)
        return -5;
      dsk->sectorsize = 128 << header[2];
    case 2:
      if (header[1] != 1)
        return -6;
      dsk->heads = header[1];
    case 1:
      dsk->sectors = header[0];
  }

  if (dsk->heads != 1 || dsk->tracks != 35 || dsk->sectors != 18)
    return -6;

  dsk->granules = (dsk->tracks - 1) * 2;

  if ((dsk->fat = sys_calloc(dsk->sectorsize, 1)) == NULL)
    return -7;

  if ((dsk->buf = sys_calloc(dsk->sectorsize, 1)) == NULL) {
    sys_free(dsk->fat);
    dsk->fat = NULL;
    return -8;
  }

  n = dsk->sectorsize/32;

  if ((dsk->dir = sys_calloc(9*n, sizeof(DskDirEntry))) == NULL) {
    sys_free(dsk->fat);
    dsk->fat = NULL;
    sys_free(dsk->buf);
    dsk->buf = NULL;
    return -9;
  }

  if (dsk_readsector(f, dsk, 0, 17, 1, dsk->fat) != 0)
    return -10;

  for (k = 0, i = 2; i < 11; i++) {
    if (dsk_readsector(f, dsk, 0, 17, i, dsk->buf) != 0)
      return -11;
    for (j = 0; j < n; j++)
      if (dsk_direntry(dsk->buf, j, &dsk->dir[k]) == 0)
        k++;
  }

  dsk->numentries = k;

  return 0;
}

void close_dsk(DskInfo *dsk)
{
  if (dsk->fat) sys_free(dsk->fat);
  if (dsk->buf) sys_free(dsk->buf);
  if (dsk->dir) sys_free(dsk->dir);
  dsk->fat = NULL;
  dsk->buf = NULL;
  dsk->dir = NULL;
}

Err read_dsk(FileRef f, DskInfo *dsk, UInt16 index,
             SnapState *state, SnapRegs *regs, Hardware *hardware)
{
  DskDirEntry *e;
  Boolean lastgranule;
  UInt8 granule, track, sector, first, last;
  UInt16 i, n;
  Err err;

  if (index >= dsk->numentries)
    return -12;

  e = &dsk->dir[index];

  if (e->type != 0x02)
    return -13;

  granule = e->granule;

  for (i = 0;;) {
    track = granule/2;
    if (track >= 17) track++;
    first = (granule%2) ? dsk->sectors/2 : 0;

    granule = dsk->fat[granule];

    if (granule >= 0xC0 && granule <= (0xC0 + dsk->sectors/2)) {
      lastgranule = true;
      last = first+(granule & 0x0F);
    } else if (granule < dsk->granules) {
      lastgranule = false;
      last = first+dsk->sectors/2;
    } else
      return -14;

    for (sector = first; sector < last; sector++) {
      if (dsk_readsector(f, dsk, 0, track, sector, dsk->buf) != 0)
        return -15;
      n = lastgranule && sector == (last-1) ? e->lastbytes : dsk->sectorsize;
      if ((err = dsk_putsector(regs, hardware, i++, dsk->buf, n)) != 0)
        return err;
    }
    if (lastgranule)
      break;
  }

  return 0;
}

static Err dsk_putsector(SnapRegs *regs, Hardware *hardware, UInt16 i, UInt8 *buf, UInt16 size)
{
  UInt16 k;
  static UInt16 count = 0;
  static UInt16 s = 0;
  static UInt8 aux[5];
  static UInt16 start = 0, len = 0, exec = 0;

  if (i == 0) {
    s = 0;
    count = 0;
  }

  switch (s) {
    case 0:
      for (k = 0; count < 5 && k < size; k++, count++)
        aux[count] = buf[k];

      if (count == 5) {
multipart:
        len = aux[1]*256 + aux[2];
        start = aux[3]*256 + aux[4];

        if (aux[0] != 0x00)
          return -16;
        if (start >= 0x8000)
          return -17;
        if (len > 0x8000)
          return -18;
        if ((start + len) >= 0x8000)
          len = 0x8000 - start;

        s = 1;
        count = 0;

        for (; count < len && k < size; k++, count++)
          hardware->bank0[start+count] = buf[k];

        if (count == len) {
          s = 2;
          count = 0;

          for (; count < 5 && k < size; k++, count++)
            aux[count] = buf[k];

          if (count == 5) {
            s = 3;

            if (aux[0] != 0xFF || aux[1] != 0x00 || aux[2] != 0x00)
              goto multipart;

            exec = aux[3]*256 + aux[4];
            regs->r[REG_PC] = (exec);
          }
        }
      }
      break;

    case 1:
      for (k = 0; count < len && k < size; k++, count++)
        hardware->bank0[start+count] = buf[k];

      if (count == len) {
        s = 2;
        count = 0;

        for (; count < 5 && k < size; k++, count++)
          aux[count] = buf[k];

        if (count == 5) {
          s = 3;

          if (aux[0] != 0xFF || aux[1] != 0x00 || aux[2] != 0x00)
            goto multipart;

          exec = aux[3]*256 + aux[4];
          regs->r[REG_PC] = (exec);
        }
      }
      break;

    case 2:
      for (k = 0; count < 5 && k < size; k++, count++)
        aux[count] = buf[k];

      if (count == 5) {
        s = 3;

        if (aux[0] != 0xFF || aux[1] != 0x00 || aux[2] != 0x00)
          goto multipart;

        exec = aux[3]*256 + aux[4];
        regs->r[REG_PC] = (exec);
      }
  }

  return 0;
}

static Err dsk_readsector(FileRef f, DskInfo *dsk, UInt16 head, UInt16 track,
                          UInt16 sector, UInt8 *buf)
{
  UInt32 r;

  if (dsk_seeksector(f, dsk, head, track, sector) != 0)
    return -1;

  if (VFSFileRead(f, dsk->sectorsize, buf, &r) != 0 ||
      r != dsk->sectorsize)
    return -1;

  return 0;
}

static Err dsk_seeksector(FileRef f, DskInfo *dsk, UInt16 head, UInt16 track,
                          UInt16 sector)
{
  Int32 offset = (Int32)dsk->headersize +
                 (Int32)track * dsk->sectors * dsk->sectorsize +
                 (Int32)sector * dsk->sectorsize;

  if (VFSFileSeek(f, vfsOriginBeginning, offset) != 0)
    return -1;

  return 0;
}

static Err dsk_direntry(UInt8 *buf, UInt16 index, DskDirEntry *e)
{
  Int16 i, k;

  k = 0;
  buf += index*32;

  if (buf[0] == 0xFF || buf[0] == 0x00)
    return -1;

  for (i = 0; i < 8; i++)
    if (buf[i] != ' ')
      e->name[k++] = buf[i];
    else
      break;

  e->name[k++] = '.';

  for (i = 8; i < 11; i++)
    if (buf[i] != ' ')
      e->name[k++] = buf[i];
    else
      break;

  e->name[k] = 0;
  e->type = buf[11];
  e->flag = buf[12];
  e->granule = buf[13];
  e->lastbytes = buf[14]*256 + buf[15];

  return 0;
}
