#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "endian.h"
#include "snapshot.h"
#include "machine.h"
#include "z80.h"
#include "misc.h"
#include "dsk.h"
#include "cgdsk.h"

// GPL  Granules Per Lump
// GAT  Granule Allocation Table
// GATL GAT Length
// GATM GAT Mask
// DDGA Disk Directory Granule Allocation

// 1 sector = 256 bytes
// 1 granule = 5 sectors
// Floppy disk format  Granules per lump
// A,E,I  (SS,SD)      2
// B,F,J  (DS,SD)      4
// C,G,K  (SS,DD)      3
// D,H,L  (DS,DD)      6

// trilha 0 e ignorada
// granule 0 comeca na trilha 1

// Arquivo de sistema NCW1983/JHL ocupa o granule 0 (5 setores)
// byte 3 do setor 0 deste arquivo contem o lump do inicio do diretorio

// Arquivo de sistema DIR/SYS contem o diretorio
// setor 0 contem informacao sobre o disco
//   bytes 0x00 a 0x05: indicam os free granules no disco (1 byte = 1 lump)
//   bytes 0xD0 a 0xD7: nome do disco
//   bytes 0xD8 a 0xDF: data do disco
// setor 1 contem o hash table
//   bytes 0x00 a 0x1E (?) : 1 byte por entrada no diretorio (0 = vazia)
//   byte  0x1F: tamanho do DIR/SYS - 10 (em setores)
// setores 2 ao ultimo contem as entradas do diretorio (32 bytes por entrada)

// formato de uma entrada de diretorio:
// Byte 0:  Bit  Meaning when set
//            7  Always 0
//            6  System file
//            5  No meaning
//            4  Entry in use, contains a file
//            3  Hidden file, not listed using CMD"I"
//            2-0  Always 0 on the Colour Genie
// Byte 1:  Bit  Meaning when set
//            7  Always 0
//            6  System file
//            5-0  No meaning
// Byte 2:  No meaning
// Byte 3:  Indicates which byte in the last sector of the file does not belong to the file anymore. Here the rule is 0=256.
// Byte 4:  Not used by DOS
// Bytes 5-12:  Contain the file name, padded with spaces.
// Bytes 13-15  Contain the file extension.
// Bytes 16-19:  Always 0 on the Colour Genie
// Bytes 20-21  Indicate how many sectors are used by this file.

/*
The remaining bytes indicate the location of the data. It is possible that the
file is divided into several blocks. For this reason, 4 pairs of bytes are
reserved. If they are not sufficient, the file will obtain a second entry.
When the first byte of a pair equals FFH, the file has no more following
blocks. The remaining byte pairs are meaningless. 

When the first byte of a pair equals FEH, the file has an additional entry in
the directory. The second byte indicates the location of the entry;
the bits 0-4 show in which sector, and the bits 5-7 indicate which entry within
the sector.

Otherwise, the first byte shows in which lump the data block belonging to the
file starts. Bits 5-7 in the second byte then show, in which granule of this
lump the data block starts, and the bits 0-4 indicate the length of the data
block (in granules).

An additional entry of a file can be recognised by bit 7 of byte 0. Whenever
this bit is set to 1, then byte 1 indicates the location of the previous entry
(see above). Bytes 2-12 stay unused, and the bytes 23-31 have the same meaning
as described above.
*/


// dir_sector = DDSL * GATM * GPL + SPT
// dir_length = DDGA * GPL

// Para geometria 2:
// dir_sector = 24 * 3 * 5 + 18 = 378
// dir_length = 3 * 5 = 15

typedef struct {
  UInt8 DDSL;  // directory start lump (lump number of GAT)
  UInt8 GATL;  // number of bytes used in the Granule Allocation Table sector
  UInt8 STEPRATE; // step rate and some SD/DD flag
  UInt8 TRK;   // number of tracks
  UInt8 SPT;   // sectors per track (both heads counted)
  UInt8 GATM;  // number of used bits per byte in the GAT sector (GAT mask)
  UInt8 P7;    // ???? always zero
  UInt8 FLAGS; // ???? some flags (SS/DS bit 6)
  UInt8 GPL;   // sectors per granule (always 5 for the Colour Genie)
  UInt8 DDGA;  // directory Granule allocation (number of directory granules)
} CGDRIVE;

static CGDRIVE geometry[] = {
{0x14, 0x28, 0x07, 0x28, 0x0A, 0x02, 0x00, 0x00, 0x05, 0x02}, //  0 A 40 SS SD
{0x14, 0x28, 0x07, 0x28, 0x14, 0x04, 0x00, 0x40, 0x05, 0x04}, //  1 B 40 DS SD
{0x18, 0x30, 0x53, 0x28, 0x12, 0x03, 0x00, 0x03, 0x05, 0x03}, //  2 C 40 SS DD
{0x18, 0x30, 0x53, 0x28, 0x24, 0x06, 0x00, 0x43, 0x05, 0x06}, //  3 D 40 DS DD
{0x14, 0x28, 0x07, 0x28, 0x0A, 0x02, 0x00, 0x04, 0x05, 0x02}, //  4 E 40 SS SD
{0x14, 0x28, 0x07, 0x28, 0x14, 0x04, 0x00, 0x44, 0x05, 0x04}, //  5 F 40 DS SD
{0x18, 0x30, 0x53, 0x28, 0x12, 0x03, 0x00, 0x07, 0x05, 0x03}, //  6 G 40 SS DD
{0x18, 0x30, 0x53, 0x28, 0x24, 0x06, 0x00, 0x47, 0x05, 0x06}, //  7 H 40 DS DD
{0x28, 0x50, 0x07, 0x50, 0x0A, 0x02, 0x00, 0x00, 0x05, 0x02}, //  8 I 80 SS SD
{0x28, 0x50, 0x07, 0x50, 0x14, 0x04, 0x00, 0x40, 0x05, 0x04}, //  9 J 80 DS SD
{0x30, 0x60, 0x53, 0x50, 0x12, 0x03, 0x00, 0x03, 0x05, 0x03}, // 10 K 80 SS DD
{0x30, 0x60, 0x53, 0x50, 0x24, 0x06, 0x00, 0x43, 0x05, 0x06}, // 11 L 80 DS DD
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

// comando: CMD "<0=A"

static Err cgdsk_putsector(z80_Regs *z80, Hardware *hardware, UInt16 i, UInt8 *buf, UInt16 size) SECTION("aux");
static Err cgdsk_direntry(UInt8 *buf, UInt16 index, DskDirEntry *e) SECTION("aux");

Err open_cgdsk(FileRef f, DskInfo *dsk) {
  Int16 i, j, k;
  UInt8 buf[16];
  UInt32 r, offset, dir_sector, dir_length;
  Err err;

  dir_sector = 0;
  dir_length = 0;

  for (i = 0; geometry[i].DDSL; i++) {

    if ((err = VFSFileSeek(f, vfsOriginBeginning, geometry[i].SPT * 256)) != 0)
      continue;

    if ((err = VFSFileRead(f, 16, buf, &r)) != 0 || r != 16)
      continue;

    // find an entry with matching DDSL

    if (buf[0] != 0x00 || buf[1] != 0xfe || buf[2] != geometry[i].DDSL)
      continue;

    dir_sector = (UInt32)geometry[i].DDSL * geometry[i].GATM * geometry[i].GPL +
                 geometry[i].SPT;
    dir_length = (UInt32)geometry[i].DDGA * geometry[i].GPL;

    // scan directory for DIR/SYS or NCW1983/JHL files
    // look into sector 2 and 3 first entry relative to DDSL

    for (j = 16; j < 32; j += 8) {

      offset = dir_sector * 256 + j * 32;

      if ((err = VFSFileSeek(f, vfsOriginBeginning, offset)) != 0)
        continue;

      if ((err = VFSFileRead(f, 16, buf, &r)) != 0 || r != 16)
        continue;

      if (!StrNCompare((char *)&buf[5], "DIR     SYS", 11) ||
          !StrNCompare((char *)&buf[5], "NCW1983 JHL", 11))
        break;
    }

    if (j < 32)
      break;
  }

  if (!geometry[i].DDSL)
    return -1;

  if ((dsk->buf = sys_calloc(256, 1)) == NULL)
    return -2;

  dsk->numentries = (dir_length - 2) * 8;
  if ((dsk->dir = sys_calloc(dsk->numentries, sizeof(DskDirEntry))) == NULL) {
    sys_free(dsk->buf);
    dsk->buf = NULL;
    return -3;
  }

  dsk->heads = (geometry[i].SPT > 18) ? 2 : 1;
  dsk->tracks = geometry[i].TRK;
  dsk->sectors = geometry[i].SPT / dsk->heads;
  dsk->sectors_per_granule = 5;
  dsk->granules_per_lump = geometry[i].GATM;

  for (i = 2, k = 0; i < dir_length; i++) {
    offset = (dir_sector + i) * 256;

    if (VFSFileSeek(f, vfsOriginBeginning, offset) != 0)
      break;

    if (VFSFileRead(f, 256, dsk->buf, &r) != 0 || r != 256)
      break;

    for (j = 0; j < 8; j++)
      if (cgdsk_direntry(dsk->buf, j, &dsk->dir[k]) == 0)
        k++;
  }

  dsk->numentries = k;

  return 0;
}

Err read_cgdsk(FileRef f, DskInfo *dsk, UInt16 index, z80_Regs *z80, Hardware *hardware) {
  DskDirEntry *e;
  UInt32 r, offset;
  UInt16 i, j, k, n;
  Err err;

  if (index >= dsk->numentries)
    return -1;

  e = &dsk->dir[index];
  k = 0;
  //fprintf(stderr, "nallocs=%d\n", e->nallocs);

  for (i = 0; i < e->nallocs; i++) {
    //fprintf(stderr, "alloc %d\n", i);
    offset = dsk->heads * dsk->sectors * 256;	// trilha 0

    offset += ((e->alloc[i].lump * dsk->granules_per_lump + e->alloc[i].granule) * dsk->sectors_per_granule) * 256;

    if (VFSFileSeek(f, vfsOriginBeginning, offset) != 0)
      return -2;

    n = e->alloc[i].ngranules * dsk->sectors_per_granule;

    for (j = 0; j < n; j++) {
      if (VFSFileRead(f, 256, dsk->buf, &r) != 0 || r != 256)
        return -3;

      if ((err = cgdsk_putsector(z80, hardware, k++, dsk->buf, 256)) < 0)
        return err;

      if (err == 1)
        break;
    }
  }

  return 0;
}

Err cgdsk_putsector(z80_Regs *z80, Hardware *hardware, UInt16 i, UInt8 *buf, UInt16 size) {
  UInt16 k;
  static UInt16 count = 0;
  static UInt16 s = 0;
  static UInt8 aux[4];
  static UInt16 load = 0, len = 0, exec = 0;
  static UInt8 block_type = 1;
  static UInt32 total = 0;
  static UInt32 total2 = 0;

  if (i == 0) {
    s = 0;
    count = 0;
    total = 0;
    total2 = 0;
  }

  switch (s) {
    case 0:	// no inicio do bloco
      k = 0;

nextblock:
      for (; count < 4 && k < size; k++, count++)
        aux[count] = buf[k];

      if (count == 4) {
        total2 += 4;
        switch (aux[1]) {
          case 0x00: len = 254; break;
          case 0x01: len = 255; break;
          case 0x02: len = 256; break;
          default  : len = aux[1] - 2;
        }

        block_type = aux[0];

        switch (block_type) {
          case 0x01:	// load block
            load = aux[3] * 256 + aux[2];
            if (load < 0x4000)
              return -4;
            //fprintf(stderr, "load=%04X, len=%d\n", load, len);
            break;
          case 0x02:	// load last block
            exec = aux[3] * 256 + aux[2];
            z80->pc = LITTLE_ENDIANIZE_INT32(exec);
            //fprintf(stderr, "total=%d (%d+%d), exec=%04X\n", total+total2, total, total2, exec);
            return 1;
          case 0x03:	// ignore block
            break;
          default:
            //fprintf(stderr, "type=%02X\n", block_type);
            return -5;
        }

        for (count = 0; count < len && k < size; k++, count++) {
          if (block_type != 0x03) {
            if ((load+count) < 0x8000)
              hardware->m1[load+count] = buf[k];
            else
              hardware->m0[load+count-0x8000] = buf[k];
            total++;
          }
        }

        if (count == len) {
          count = 0;
          goto nextblock;
        }
        s = 1;
      }
      break;

    case 1:	// aguardando fim do bloco
      for (k = 0; count < len && k < size; k++, count++) {
        if (block_type != 0x03) {
          if ((load+count) < 0x8000)
            hardware->m1[load+count] = buf[k];
          else
            hardware->m0[load+count-0x8000] = buf[k];
          total++;
        }
      }

      if (count == len) {
        s = 0;
        count = 0;
        goto nextblock;
      }
  }

  return 0;
}

void close_cgdsk(DskInfo *dsk) {
  Int16 i;

  for (i = 0; i < dsk->numentries; i++) {
    if (dsk->dir[i].alloc) {
      sys_free(dsk->dir[i].alloc);
      dsk->dir[i].alloc = NULL;
    }
  }
  dsk->numentries = 0;

  if (dsk->buf) sys_free(dsk->buf);
  if (dsk->dir) sys_free(dsk->dir);
  dsk->buf = NULL;
  dsk->dir = NULL;
}

static Err cgdsk_direntry(UInt8 *buf, UInt16 index, DskDirEntry *e) {
  UInt16 i, k;
  UInt32 nsectors;

  k = 0;
  buf += index*32;

  if (!(buf[0] & 0x10))	// vazio
    return -1;

  if (buf[0] & 0x80)	// continuacao
    return -1;

  if (buf[0] & 0x40)	// arquivo de sistema
    return -1;

  if (buf[0] & 0x08)	// arquivo oculto
    return -1;

  for (i = 0; i < 8; i++)
    if (buf[i + 5] != ' ')
      e->name[k++] = buf[i + 5];
    else
      break;

  e->name[k++] = '/';

  for (i = 8; i < 11; i++)
    if (buf[i + 5] != ' ')
      e->name[k++] = buf[i + 5];
    else
      break;

  e->name[k] = 0;
  e->type = 0x02;	// igual ao dsk do coco

  nsectors = buf[21] * 256 + buf[20];
  e->size = nsectors * 256;
  if (buf[3] && nsectors)
    e->size = e->size - 256 + buf[3] - 1;

  if ((e->alloc = sys_calloc(5, sizeof(DskFileAloc))) == NULL)
    return -1;

  e->nallocs = 0;

  for (k = 22; k < 32; k += 2) {
    if (buf[k] == 0xFF)	// fim
      break;

    if (buf[k] == 0xFE)	{ // continua em outra entrada
      sys_free(e->alloc);
      e->alloc = NULL;
      return -1;
    }

    e->alloc[e->nallocs].lump = buf[k];
    e->alloc[e->nallocs].granule = buf[k+1] >> 5;
    e->alloc[e->nallocs].ngranules = (buf[k+1] & 0x1F) + 1;
    e->nallocs++;
  }

  return 0;
}
