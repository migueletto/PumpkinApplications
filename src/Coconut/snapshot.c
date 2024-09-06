#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "endian.h"
#include "snapshot.h"
#include "misc.h"

static Err write_block(FileRef f, UInt8 *p, UInt16 type, UInt16 start, UInt16 size) SECTION("aux");
static Err read_header(FileRef f, UInt16 *type, UInt16 *start, UInt16 *size) SECTION("aux");
static Err read_block(FileRef f, UInt8 *p, UInt16 size) SECTION("aux");

Err write_snap(FileRef f, SnapState *state, SnapRegs *regs, Hardware *hardware)
{
  Err err;

  if ((err = write_block(f, (UInt8 *)regs, SNAP_REGS, 0, sizeof(SnapRegs))) != 0)
    return err;
  if ((err = write_block(f, (UInt8 *)state, SNAP_STATE,0, sizeof(SnapState))) != 0)
    return err;
  if ((err = write_block(f, hardware->bank0, SNAP_RAM, 0, 0x8000)) != 0)
    return err;
  if (hardware->bank2 &&
        (err = write_block(f, hardware->bank2, SNAP_RAM, 0x8000, 0x7F00)) != 0)
    return err;
  if ((err = write_block(f, &hardware->bank1[0x7F00], SNAP_IO, 0xFF00, 0x100)) != 0)
    return err;

  return 0;
}

Err read_snap(FileRef f, SnapState *state, SnapRegs *regs, Hardware *hardware)
{
  UInt16 type, start, size;
  Boolean cart;
  Err err;

  cart = true;

  for (err = 0; !err;) {
    if (VFSFileEOF(f))
      break;

    if ((err = read_header(f, &type, &start, &size)) != 0)
      return err;

    switch (type) {
      case SNAP_REGS:
        cart = false;

        if (start > (sizeof(SnapRegs)-2) || size < 2 ||
            size > sizeof(SnapRegs) || (start+size) > sizeof(SnapRegs) ||
            (size % 1))
          return -1;

        if ((err = read_block(f, ((UInt8 *)regs)+start, size)) != 0)
          return -2;

        break;
      case SNAP_STATE:
        cart = false;

        if (start > (sizeof(SnapState)-2) || size < 2 ||
            size > sizeof(SnapState) || (start+size) > sizeof(SnapState) ||
            (size % 1))
          return -3;

        err = read_block(f, ((UInt8 *)state)+start, size);
        break;
      case SNAP_RAM:
        cart = false;

        if (start < 0x8000) {
          if (size > 0x8000 || (start+size) > 0x8000)
            return -4;

          err = read_block(f, hardware->bank0+start, size);
        } else if (start < 0xFF00) {
          if (size > 0x7F00 || (0xFF00-start) < size)
            return -5;

          if (hardware->bank2 == NULL)
            hardware->bank2 = sys_calloc(0x8000, 1);
          if (hardware->bank2 == NULL)
            return -6;

          err = read_block(f, hardware->bank2+start-0x8000, size);
        } else
          return -7;
        break;
      case SNAP_ROM:
        if (start < 0xC000 || size > 0x4000 || (0xFFFF-start+1) < size)
          return -8;

        err = read_block(f, hardware->bank1+start-0x8000, size);

        if (start != 0xC000 || (size != 2048 && size != 4096 &&
                                size != 8192 && size != 16384))
          cart = false;
        break;
      case SNAP_IO:
        cart = false;

        if (start < 0xFF00 || size > 0x100 || (0xFFFF-start+1) < size)
          return -9;

        err = read_block(f, hardware->bank1+start-0x8000, size);
    }

    if (err != 0)
      return err;
  }

  if (cart) {
    UInt8 *p;
    Int16 len;

    p = hardware->bank1 + 0xC000 + size;
    len = 0x4000 - size;

    while (len > 0) {
      if (size > len)
        size = len;
      MemMove(p, hardware->bank1 + 0xC000, size);
      p += size;
      len -= size;
    }

    hardware->firq_request = 1;
  }

  return 0;
}

static Err write_block(FileRef f, UInt8 *p, UInt16 type, UInt16 start, UInt16 size)
{
  UInt32 r;
  Err err;

  if ((err = VFSFileWrite(f, sizeof(type), &type, &r)) != 0)
    return err;
  if ((err = VFSFileWrite(f, sizeof(start), &start, &r)) != 0)
    return err;
  if ((err = VFSFileWrite(f, sizeof(size), &size, &r)) != 0)
    return err;
  if ((err = VFSFileWrite(f, size, p, &r)) != 0)
    return err;

  return 0;
}

static Err read_header(FileRef f, UInt16 *type, UInt16 *start, UInt16 *size)
{
  UInt32 r;
  Err err;
  
  if ((err = VFSFileRead(f, sizeof(UInt16), type, &r)) != 0 ||
      r != sizeof(UInt16))
    return err;

  if ((err = VFSFileRead(f, sizeof(UInt16), start, &r)) != 0 ||
      r != sizeof(UInt16))
    return err;

  if ((err = VFSFileRead(f, sizeof(UInt16), size, &r)) != 0 ||
      r != sizeof(UInt16))
    return err;

  return 0;
}

static Err read_block(FileRef f, UInt8 *p, UInt16 size)
{
  UInt32 r;
  Err err;

  if ((err = VFSFileRead(f, size, p, &r)) != 0 || r != size)
    return err;

  return 0;
}
