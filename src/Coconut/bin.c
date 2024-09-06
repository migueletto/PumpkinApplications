#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "endian.h"
#include "snapshot.h"
#include "machine.h"
#include "bin.h"

Err read_bin(FileRef f, SnapState *state, SnapRegs *regs, Hardware *hardware) {
  UInt16 start, len, exec;
  UInt8 buf[16];
  UInt32 r;
  Err err;

  for (;;) {
    if ((err = VFSFileRead(f, 5, buf, &r)) != 0 && err != vfsErrFileEOF)
      return err;

    if (r != 5)
      return -1;

    if (buf[0] != 0x00)
      break;

    len = buf[1]*256 + buf[2];
    start = buf[3]*256 + buf[4];

    if (start >= 0x8000 || len > 0x8000)
      return -2;

    if ((start + len) >= 0x8000)
      len = 0x8000 - start;

    if ((err = VFSFileRead(f, len, &hardware->bank0[start], &r)) != 0)
      return err;
 
    if (r != len)
      return -3;
  }

  if (buf[1] != 0x00 || buf[2] != 0x00)
    return -4;

  exec = buf[3]*256 + buf[4];
  regs->r[REG_PC] = exec;

  return 0;
}
