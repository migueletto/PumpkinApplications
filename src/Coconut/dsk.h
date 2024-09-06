typedef struct {
  UInt8 lump, granule, ngranules;
} DskFileAloc;

typedef struct {
  char name[16];
  UInt8 type, flag, granule;
  UInt16 lastbytes, nallocs;
  UInt32 size;
  DskFileAloc *alloc;
} DskDirEntry;

typedef struct {
  UInt8 heads, tracks, sectors, granules;
  UInt16 headersize, sectorsize, numentries;
  UInt16 sectors_per_granule, granules_per_lump;
  DskDirEntry *dir;
  UInt8 *fat, *buf;
} DskInfo;

Err open_dsk(FileRef f, DskInfo *dsk) SECTION("aux");
Err read_dsk(FileRef f, DskInfo *dsk, UInt16 index,
             SnapState *state, SnapRegs *regs, Hardware *hardware) SECTION("aux");
void close_dsk(DskInfo *dsk) SECTION("aux");
