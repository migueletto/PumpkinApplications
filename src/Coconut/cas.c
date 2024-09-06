#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "endian.h"
#include "palm.h"
#include "misc.h"
#include "cas.h"

#define CAS_WAVELOW	0xFF
#define CAS_WAVEHIGH	0x00
#define CAS_WAVENULL	0x00

#define CAS_READBLOCK	1
#define CAS_SYNC	2
#define CAS_BLOCK	3
#define CAS_READBYTE	4

#define CAS_SAMPLES_HEADER 3000
#define CAS_BUFSIZE	8192
#define CAS_BLOCKSIZE	512

static UInt8 *buffer = NULL;
static UInt8 *casblock = NULL;

static UInt8 motor = 0;
static UInt16 blocklen = 0;
static UInt8 lastblocktype;
static UInt16 cas_get;
static UInt16 cas_put;
static UInt16 cas_state;

void cas_init(void) {
  buffer = sys_calloc(1, CAS_BUFSIZE);
  casblock = sys_calloc(1, CAS_BLOCKSIZE);

  cas_reset();
}

void cas_finish(void) {
  if (casblock) sys_free(casblock);
  if (buffer) sys_free(buffer);
  buffer = casblock = NULL;
}

void cas_reset(void) {
  cas_state = CAS_READBLOCK;
  cas_get = 0;
  cas_put = 0;
  lastblocktype = 0;
}

void cas_motor(UInt8 b) {
  motor = b ? 1 : 0;
}

void cas_status(FileRef f) {
  char buf[8];
  UInt32 pos, size;
  UInt16 pct;

  StrCopy(buf, "    ");

  if (f != NULL && VFSFileTell(f, &pos) == 0 && VFSFileSize(f, &size) == 0) {
    pct = (pos * 100L) / size;
    StrPrintF(buf, "%3d", pct);
    StrCat(buf, "%%");
  }

  status(0, 23, buf);
}

static void cas_putwave(UInt8 w) {
  buffer[cas_put++] = w;
  if (cas_put == CAS_BUFSIZE)
    cas_put = 0;
}

static void cas_putbyte(UInt8 b) { 
  Int16 i;
  
  for (i = 0; i < 8; i++) {
    cas_putwave(CAS_WAVEHIGH);
    if (((b >> i) & 0x01) == 0) {
      cas_putwave(CAS_WAVEHIGH);
      cas_putwave(CAS_WAVELOW);
    }
    cas_putwave(CAS_WAVELOW);
  } 
}

static Int16 cas_readblock(FileRef f, UInt8 *block, UInt16 *len) {
  Int16 j, state = 0, phase = 0;
  UInt8 newb, b = 0, block_length = 0, block_checksum = 0;
  UInt32 r;

  for (;;) {
    if (VFSFileRead(f, 1, &newb, &r) != 0 || r != 1)
      return 0;
    cas_status(f);

    for (j = 0; j < 8; j++) {
      b >>= 1;

      if (newb & 1)
        b |= 0x80;
      newb >>= 1;

      if (state == 0) {
        // Searching for a block
        if (b == 0x3c) {
          // Found one!
          phase = j;
          state = 1;
        }
      } else if (j == phase) {
        switch (state) {
          case 1: // Found file type
            block_checksum = b;
            state++;
            break;
          case 2: // Found file size
            block_length = b;
            block_checksum += b;
            *len = ((UInt16)block_length) + 3;
            state++;
            break;
          case 3: // Data byte
            if (block_length) {
              block_length--;
              block_checksum += b;
            } else {
              // End of block! check the checksum
              if (b != block_checksum)
                // Checksum error
                return 0;
              // We got a block! Return new position
              *block = b;
              return 1;
            }
            break;
        }
        *(block++) = b;
      }
    }
  }

  // Couldn't find a block
  return 0;
}

UInt8 cas_read(FileRef f, UInt32 *eventcount) {
  UInt32 r, count;
  UInt16 i, w;
  UInt8 b;

  if (!f || !motor)
    return 1;

  if (cas_get == cas_put) switch (cas_state) {
    case CAS_READBLOCK:
      if (cas_readblock(f, casblock, &blocklen) == 1) {
        if (lastblocktype == 0x00 || lastblocktype == 0xFF) {
          for (i = 0; i < CAS_SAMPLES_HEADER; i++)
            cas_putwave(CAS_WAVENULL);
          cas_state = CAS_SYNC;
        } else {
          cas_putbyte(0x55);
          cas_putbyte(0x3C);
          for (i = 0; i < blocklen; i++)
            cas_putbyte(casblock[i]);
          cas_putbyte(0x55);
        }
        lastblocktype = casblock[0];
      } else
        cas_state = CAS_READBYTE;
      break;

    case CAS_SYNC:
      for (i = 0; i < 128; i++)
        cas_putbyte(0x55);
      cas_state = CAS_BLOCK;
      break;

    case CAS_BLOCK:
      cas_putbyte(0x55);
      cas_putbyte(0x3C);
      for (i = 0; i < blocklen; i++)
        cas_putbyte(casblock[i]);
      cas_putbyte(0x55);
      cas_state = CAS_READBLOCK;
      break;

    case CAS_READBYTE:
      if (VFSFileRead(f, 1, &b, &r) == 0 && r == 1) {
        cas_putbyte(b);
        cas_status(f);
      } else
        cas_validate();
      break;
  }

  if (cas_get == cas_put)
    return 1;

  // (0.894886 * 1000000) / 4800 = 186.4345 cycles

  count = *eventcount;

  if (count >= 186) {
    if (count >= 372)
      count = 0;
    else
      count -= 186;

    cas_get++;
    if (cas_get == CAS_BUFSIZE)
      cas_get = 0;
  }

  *eventcount = count;

  w = buffer[cas_get] & 0x01;
  return (UInt8)w;
}
