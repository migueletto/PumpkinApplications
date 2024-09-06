#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpmdsk.h"
#include "misc.h"

#define NUM_DRIVES	2
#define TRACK_SECTORS	16
#define SECTOR_LEN	128

static FileRef fd[NUM_DRIVES] = {NULL, NULL};
static UInt16 ntracks[NUM_DRIVES] = {80, 80};
static char dskname[NUM_DRIVES][256];

Err init_drive(UInt8 drive, Int16 vol, char *name)
{
  if (drive >= NUM_DRIVES)
    return -1;

  MemSet(dskname[drive], sizeof(dskname[drive]), 0);
  StrCopy(dskname[drive], name);

  if (fd[drive])
    close_drive(drive);

  return VFSFileOpen(vol, name, vfsModeReadWrite, &fd[drive]);
}

void close_drive(UInt8 drive)
{
  if (drive < NUM_DRIVES && fd[drive]) {
    VFSFileClose(fd[drive]);
    fd[drive] = NULL;
  }
}

Err seek_sector(UInt8 drive, UInt8 track, UInt8 sector)
{
  Int32 offset;

  if (drive >= NUM_DRIVES || fd[drive] == NULL ||
      track >= ntracks[drive] || sector > TRACK_SECTORS)
    return -1;

  offset = (Int32)track * TRACK_SECTORS * SECTOR_LEN +
           (Int32)(sector - 1) * SECTOR_LEN;

  return VFSFileSeek(fd[drive], vfsOriginBeginning, offset);
}

UInt8 read_sector(UInt8 drive, UInt8 track, UInt8 sector, UInt8 *addr)
{
  UInt32 r;

  if (seek_sector(drive, track, sector) != 0)
    return 1;

  if (VFSFileRead(fd[drive], SECTOR_LEN, addr, &r) != 0)
    return 1;

  if (r != SECTOR_LEN)
    return 1;

  return 0;
}
 
UInt8 write_sector(UInt8 drive, UInt8 track, UInt8 sector, UInt8 *addr)
{
  UInt32 r;

  if (seek_sector(drive, track, sector) != 0)
    return 1;

  if (VFSFileWrite(fd[drive], SECTOR_LEN, addr, &r) != 0)
    return 1;

  if (r != SECTOR_LEN)
    return 1;

  return 0;
}
