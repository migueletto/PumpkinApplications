#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "armlet.h"
#include "z80.h"
#include "endian.h"
#include "snapshot.h"
#include "machine.h"
#include "snapz80.h"
#include "io.h"

static UInt8 buf[256];
static UInt32 ibuf, buflen;

static Err buf_next(FileRef f, UInt8 *b) SECTION("aux");
static Err z80_decompress_block(Hardware *hardware, FileRef f, UInt16 dest, UInt32 size, Int16 marker) SECTION("aux");

static Err buf_next(FileRef f, UInt8 *b) {
  Err err;

  if (ibuf == buflen) {
    if (VFSFileEOF(f))
      return vfsErrFileEOF;
    err = VFSFileRead(f, sizeof(buf), buf, &buflen);
    if (err && err != vfsErrFileEOF)
      return err;
    if (err == vfsErrFileEOF && buflen == 0)
      return err;
    ibuf = 0;
  }
  *b = buf[ibuf++];

  return 0;
}

static Err z80_decompress_block(Hardware *hardware, FileRef f, UInt16 dest, UInt32 size, Int16 marker) {
  UInt8 ch, count, data;
  UInt16 k, i, total;
  Err err;

  for (k = 0, total = 0; k < size;) {
    if ((err = buf_next(f, &ch)) != 0)
      return err;
    k++;

    // either start of ED ED xx yy or single ED
    if (ch == 0xed) {
      if ((err = buf_next(f, &ch)) != 0)
        return err;
      k++;

      if (ch == 0xed) {
        // ED ED xx yy
        // repetition

        if ((err = buf_next(f, &count)) != 0)
          return err;
        k++;

        if ((err = buf_next(f, &data)) != 0)
          return err;
        k++;

        for (i = 0; i < count; i++, total++) {
          if (dest < 0x8000)
            hardware->bank1[dest] = data;
          else
            hardware->bank0[dest - 0x8000] = data;
          dest++;
        }
      } else {
        // single ED

        if (dest < 0x8000)
          hardware->bank1[dest] = 0xed;
        else
          hardware->bank0[dest - 0x8000] = 0xed;
        dest++;
        total++;

        if (dest < 0x8000)
          hardware->bank1[dest] = ch;
        else
          hardware->bank0[dest - 0x8000] = ch;
        dest++;
        total++;
      }
    } else {
      // not ED
      if (dest < 0x8000)
        hardware->bank1[dest] = ch;
      else
        hardware->bank0[dest - 0x8000] = ch;
      dest++;
      total++;
    }
  }

  return 0;
}

Err read_z80(FileRef f, SnapState *state, z80_Regs *z80, Hardware *hardware) {
  Int32 i;
  UInt16 w;
  UInt8 lo, hi, data;
  UInt8 pSnapshot[87];
  UInt32 hsize, r;
  Err err;

  ibuf = 0;
  buflen = 0;
  hsize = 30;

  if ((err = VFSFileRead(f, hsize, pSnapshot, &r)) != 0)
    return -2;

  if (r != hsize)
    return -3;

  z80->sela = 0;
  z80->selr = 0;

  w = (pSnapshot[0] << 8) | pSnapshot[1];
  z80->af[z80->sela] = LITTLE_ENDIANIZE_INT32(w);

  w = (pSnapshot[3] << 8) | pSnapshot[2];
  z80->bc[z80->selr] = LITTLE_ENDIANIZE_INT32(w);

  w = (pSnapshot[5] << 8) | pSnapshot[4];
  z80->hl[z80->selr] = LITTLE_ENDIANIZE_INT32(w);

  w = (pSnapshot[9] << 8) | pSnapshot[8];
  z80->sp = LITTLE_ENDIANIZE_INT32(w);

  hi = pSnapshot[10];
  lo = (pSnapshot[11] & 0x7f) | ((pSnapshot[12] & 0x01) << 7);
  w = (hi << 8) | lo;
  z80->ir = LITTLE_ENDIANIZE_INT32(w);

  w = (pSnapshot[14] << 8) | pSnapshot[13];
  z80->de[z80->selr] = LITTLE_ENDIANIZE_INT32(w);

  w = (pSnapshot[16] << 8) | pSnapshot[15];
  z80->bc[1 - z80->selr] = LITTLE_ENDIANIZE_INT32(w);

  w = (pSnapshot[18] << 8) | pSnapshot[17];
  z80->de[1 - z80->selr] = LITTLE_ENDIANIZE_INT32(w);

  w = (pSnapshot[20] << 8) | pSnapshot[19];
  z80->hl[1 - z80->selr] = LITTLE_ENDIANIZE_INT32(w);

  w = (pSnapshot[21] << 8) | pSnapshot[22];
  z80->af[1 - z80->sela] = LITTLE_ENDIANIZE_INT32(w);

  w = (pSnapshot[24] << 8) | pSnapshot[23];
  z80->iy = LITTLE_ENDIANIZE_INT32(w);

  w = (pSnapshot[26] << 8) | pSnapshot[25];
  z80->ix = LITTLE_ENDIANIZE_INT32(w);

  w = pSnapshot[27] != 0;
  z80->iff1 = LITTLE_ENDIANIZE_INT32(w);

  w = pSnapshot[28] != 0;
  z80->iff2 = LITTLE_ENDIANIZE_INT32(w);

  z80->irq_state = 0;
  z80->nmi_state = 0;

  w = pSnapshot[29] & 0x03;
  z80->im = LITTLE_ENDIANIZE_INT32(w);

  if (pSnapshot[6] || pSnapshot[7]) {
    // arquivo versao 1
    // SPECTRUM_Z80_SNAPSHOT_48K_OLD

    w = (pSnapshot[7] << 8) | pSnapshot[6];
    z80->pc = LITTLE_ENDIANIZE_INT32(w);

    if ((pSnapshot[12] & 0x20) == 0) {
      // uncompressed

      if ((err = VFSFileRead(f, 0x4000, &hardware->bank1[0x4000], &r)) != 0)
        return -4;

      if (r != 0x4000)
        return -5;

      if ((err = VFSFileRead(f, 0x8000, hardware->bank0, &r)) != 0 &&
           err != vfsErrFileEOF)
        return -6;

      if (r != 0x8000)
        return -7;
    } else
      // compressed

      if ((err = z80_decompress_block(hardware, f, 0x4000, -1, 1)) != 0 &&
           err != vfsErrFileEOF)
        return -8;

  } else {
    hsize = 5;

    if ((err = VFSFileRead(f, hsize, pSnapshot+30, &r)) != 0)
      return -9;

    if (r != hsize)
      return -10;

    lo = pSnapshot[30];
    hi = pSnapshot[31];
    data = pSnapshot[34];      // Hardware mode

    if (hi == 0 && lo == 23) {
      // arquivo versao 2

      if (data > 1)
        return -11;
    } else if (hi == 0 && (lo == 54 || lo == 55)) {
      // arquivo versao 3

      if (data == 2 || data > 3)
        return -12;
    } else
      return -13;

    hsize = ((hi << 8) | lo) - 3;

    if ((err = VFSFileRead(f, hsize, pSnapshot+35, &r)) != 0)
      return -14;

    if (r != hsize)
      return -15;

    // SPECTRUM_Z80_SNAPSHOT_48K

    w = (pSnapshot[33] << 8) | pSnapshot[32];
    z80->pc = LITTLE_ENDIANIZE_INT32(w);

    for (;;) {
      UInt16 length;
      UInt8 page, ch;
      Int32 dest;

      if ((err = buf_next(f, &lo)) != 0)
        break;
      if ((err = buf_next(f, &hi)) != 0)
        return -16;
      if ((err = buf_next(f, &page)) != 0)
        return -17;

      length = (hi << 8) | lo;

      switch (page) {
        case 4:  dest = 0x8000; break;
        case 5:  dest = 0xc000; break;
        case 8:  dest = 0x4000; break;
        default: return -18;
      }

      if (length == 0xffff) {
        // uncompressed

        for (i = 0; i < 16384; i++) {
          if ((err = buf_next(f, &ch)) != 0)
            return -19;
          if ((i + dest) < 0x8000)
            hardware->bank1[i + dest] = ch;
          else
            hardware->bank0[i + dest - 0x8000] = ch;
        }
      } else {
        // compressed

        if ((err = z80_decompress_block(hardware, f, dest, length, 0)) != 0)
          return -20;
      }
    }
  }

  state->border = (pSnapshot[12] & 0x0e) >> 1;

  return 0;
}
