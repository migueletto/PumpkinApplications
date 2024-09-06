#include <PalmOS.h>
#include <VFSMgr.h>

#include "section.h"
#include "palm.h"
#include "cpu.h"
#include "video.h"
#include "kbd.h"
#include "dac.h"
#include "misc.h"
#include "machine.h"
#include "m6845.h"
#include "sn76489.h"
#include "ay8910.h"
#include "ti9918.h"
#include "gui.h"

#include "z80.h"
#include "m6502.h"

#include "coco.h"
#include "mc10.h"
#include "mc1000.h"
#include "vz.h"
#include "spectrum.h"
#include "msx.h"
#include "coleco.h"
#include "cpm.h"
#include "apple.h"
#include "vic20.h"
#include "oric.h"
#include "atom.h"
#include "cgenie.h"
#include "jupiter.h"
#include "aquarius.h"
#include "nes.h"

#define MAX_FAMILIES 32
#define MAX_MACHINES 64

#define CO1_RAM		(RAM_4K|RAM_16K|RAM_32K|RAM_64K)
#define COE_RAM		(RAM_16K|RAM_32K|RAM_64K)
#define CO2_RAM		(RAM_16K|RAM_64K)
#define D32_RAM		RAM_32K
#define D64_RAM		RAM_64K
#define MC10_RAM	(RAM_4K|RAM_16K)
#define SPE_RAM		RAM_64K
#define CPM_RAM		RAM_64K
#define A2P_RAM		RAM_64K
#define MC1000_RAM	(RAM_16K|RAM_32K)
#define VZ300_RAM	RAM_16K
#define VIC_RAM		RAM_32K
#define ORIC_RAM	RAM_64K
#define ATOM_RAM	RAM_16K
#define CGENIE_RAM	(RAM_16K|RAM_32K)
#define JUPITER_RAM	RAM_64K
#define AQUARIUS_RAM	(RAM_4K|RAM_16K)
#define MSX_RAM		RAM_64K
#define COLECO_RAM	RAM_4K
#define NES_RAM		RAM_4K

#define COCO_CLOCK	 894886
#define SPECTRUM_CLOCK	3500000
#define CPM_CLOCK	3500000
#define MC1000_CLOCK	3572100
#define APPLE_CLOCK	1021800
#define VZ300_CLOCK	3546900
#define VIC_CLOCK	1022727
#define ORIC_CLOCK	1000000
#define ATOM_CLOCK	1000000
#define CGENIE_CLOCK	2216800
#define JUPITER_CLOCK	3250000
#define AQUARIUS_CLOCK	3500000
#define MSX_CLOCK	3579545
#define COLECO_CLOCK	3579545
#define NES_CLOCK	1789772		// NTSC

static char *family[MAX_FAMILIES] = {
  "", "coco", "dragon", "mc10", "spectrum", "cpm", "mc1000", "apple", "vz",
  "vic20", "oric", "atom", "cgenie", "jupiter", "aquarius", "msx", "coleco",
  "nes", NULL
};

static MachineType machine[] = {
{fcoco, coco, CPU_M6809, COCO_CLOCK, CO1_RAM, 0, 60,
 CocoColor, CocoControl, CocoButtonDef, CocoMap,
 NtscSquareID, CocoConfigForm, DAC_SAMPLE,
 coco_select, NULL, NULL, coco_finish, NULL, NULL,
 coco_key, coco_joystick, CocoFormHandler, coco_callback,
 "1980", "coco", "Color Computer", "Tandy Radio Shack", 0},

#if 0
{fcoco, cocoe, CPU_M6809, COCO_CLOCK, COE_RAM, 0, 60,
 CocoColor, CocoControl, CocoButtonDef, CocoMap,
 NtscSquareID, CocoConfigForm, DAC_SAMPLE,
 coco_select, NULL, NULL, coco_finish, NULL, NULL,
 coco_key, coco_joystick, CocoFormHandler, coco_callback,
 "1981", "cocoe", "Color Computer Ext.", "Tandy Radio Shack", 0},

{fcoco, coco2, CPU_M6809, COCO_CLOCK, CO2_RAM, 0, 60,
 CocoColor, CocoControl, CocoButtonDef, CocoMap,
 NtscSquareID, CocoConfigForm, DAC_SAMPLE,
 coco_select, NULL, NULL, coco_finish, NULL, NULL,
 coco_key, coco_joystick, CocoFormHandler, coco_callback,
 "1983", "coco2", "Color Computer 2", "Tandy Radio Shack", 0},
#endif

{fcoco, coco2b, CPU_M6809, COCO_CLOCK, CO2_RAM, 0, 60,
 CocoColor, CocoControl, CocoButtonDef, CocoMap,
 NtscRoundID, CocoConfigForm, DAC_SAMPLE,
 coco_select, NULL, NULL, coco_finish,
 NULL, NULL,
 coco_key, coco_joystick, CocoFormHandler, coco_callback,
 "1985", "coco2b", "Color Computer 2B", "Tandy Radio Shack", 0},

{fcoco, cp400, CPU_M6809, COCO_CLOCK, CO2_RAM, 0, 60,
 Cp400Color, CocoControl, CocoButtonDef, CocoMap,
 NtscSquareID, CocoConfigForm, DAC_SAMPLE,
 coco_select, NULL, NULL, coco_finish, NULL, NULL,
 coco_key, coco_joystick, CocoFormHandler, coco_callback,
 "1984", "cp400", "CP400", "Prologica", 0},

{fdragon, dragon32, CPU_M6809, COCO_CLOCK, D32_RAM, 0, 60,
 CocoColor, CocoControl, CocoButtonDef, DragonMap,
 PalSquareID, CocoConfigForm, DAC_SAMPLE,
 coco_select, NULL, NULL, coco_finish, NULL, NULL,
 coco_key, coco_joystick, CocoFormHandler, coco_callback,
 "1982", "dragon32", "Dragon 32", "Dragon Data Ltd", 0},

{fdragon, dragon64, CPU_M6809, COCO_CLOCK, D64_RAM, 0, 60,
 CocoColor, CocoControl, CocoButtonDef, DragonMap,
 PalSquareID, CocoConfigForm, DAC_SAMPLE,
 coco_select, NULL, NULL, coco_finish, NULL, NULL,
 coco_key, coco_joystick, CocoFormHandler, coco_callback,
 "1983", "dragon64", "Dragon 64", "Dragon Data Ltd", 0},

{fmc10, mc10, CPU_M6803, COCO_CLOCK, MC10_RAM, 0, 60,
 CocoColor, MC10Control, MC10ButtonDef, NULL,
 NtscSquareID, MC10ConfigForm, DAC_SAMPLE,
 mc10_select, mc10_init, NULL, mc10_finish, NULL, NULL,
 mc10_key, mc10_joystick, Mc10FormHandler, mc10_callback,
 "1983", "mc10", "MC-10", "Tandy Radio Shack", 0},

{fmc1000, mc1000, CPU_Z80, MC1000_CLOCK, MC1000_RAM, 3, 50,
 CocoColor, MC1000Control, MC1000ButtonDef, NULL,
 NtscSquareID, MC1000ConfigForm, DAC_SAMPLE|DAC_TONE,
 mc1000_select, mc1000_init, mc1000_reset, mc1000_finish, NULL, NULL,
 mc1000_key, mc1000_joystick, Mc1000FormHandler, mc1000_callback,
 "1985", "mc1000", "MC-1000", "CCE", 0},

{fvz, vz300, CPU_Z80, VZ300_CLOCK, VZ300_RAM, 4, 50,
 CocoColor, VZ300Control, VZ300ButtonDef, NULL,
 NtscSquareID, VZ300ConfigForm, DAC_SAMPLE,
 vz_select, vz_init, NULL, vz_finish, NULL, NULL,
 vz_key, vz_joystick, VzFormHandler, vz_callback,
 "1985", "vz300", "VZ300", "Dick Smith", 0},

{fspec, spectrum, CPU_Z80, SPECTRUM_CLOCK, SPE_RAM, 1, 50,
 SpectrumColor, SpectrumControl, SpectrumButtonDef, NULL,
 NtscSquareID, SpecConfigForm, DAC_SAMPLE,
 spectrum_select, spectrum_init, NULL, spectrum_finish, NULL, NULL,
 spectrum_key, spectrum_joystick, SpecFormHandler, spectrum_callback,
 "1982", "spectrum", "ZX Spectrum", "Sinclair Research", 0},

#ifdef F_CGENIE
{fcgenie, cgenie, CPU_Z80, CGENIE_CLOCK, CGENIE_RAM, 5, 60,
 CgenieColor, CgenieControl, CgenieButtonDef, NULL,
 NtscSquareID, 0, DAC_SAMPLE|DAC_TONE,
 cgenie_select, cgenie_init, NULL, cgenie_finish, NULL, NULL,
 cgenie_key, cgenie_joystick, NULL, cgenie_callback,
 "1982", "cgenie", "Colour Genie", "EACA Computers", 0},
#endif

#ifdef F_AQUARIUS
{faquarius, aquarius, CPU_Z80, AQUARIUS_CLOCK, AQUARIUS_RAM, 7, 60,
 AquariusColor, AquariusControl, AquariusButtonDef, NULL,
 NtscSquareID, 0, DAC_SAMPLE,
 aquarius_select, aquarius_init, NULL, aquarius_finish, NULL, NULL,
 aquarius_key, NULL, NULL, aquarius_callback,
 "1983", "aquarius", "Aquarius", "Mattel", 0},
#endif

#ifdef F_JUPITER
{fjupiter, jupiter, CPU_Z80, JUPITER_CLOCK, JUPITER_RAM, 6, 50,
 JupiterColor, JupiterControl, JupiterButtonDef, NULL,
 NtscSquareID, 0, DAC_SAMPLE,
 jupiter_select, NULL, NULL, jupiter_finish, NULL, NULL,
 jupiter_key, NULL, NULL, jupiter_callback,
 "1981", "jupiter", "Jupiter Ace", "Jupiter Cantab", 0},
#endif

#ifdef F_CPM
{fcpm, cpm, CPU_Z80, CPM_CLOCK, CPM_RAM, 2, 0,
 CpmColor, CpmControl, CpmButtonDef, NULL,
 Cpm6x10ID, CpmConfigForm, DAC_NONE,
 cpm_select, NULL, NULL, cpm_finish, NULL, NULL,
 cpm_key, NULL, CpmFormHandler, cpm_callback,
 "2003", "cpm", "CP/M", "Coconut", 0},
#endif

#ifdef F_APPLE
{fapple, apple2p, CPU_M6502, APPLE_CLOCK, A2P_RAM, 1, 60,
 AppleColor, AppleControl, AppleButtonDef, NULL,
 AppleID, 0, DAC_NONE,
 apple_select, apple_init, apple_reset, apple_finish, NULL, NULL,
 apple_key, apple_joystick, NULL, apple_callback,
 "1979", "apple2p", "Apple][+", "Apple Computer", 0},
#endif

#ifdef F_VIC20
{fvic20, vic20, CPU_M6502, VIC_CLOCK, VIC_RAM, 2, 60,
 VicColor, VicControl, VicButtonDef, NULL,
 NtscSquareID, VicConfigForm, DAC_TONE,
 vic_select, NULL, NULL, vic_finish, NULL, NULL,
 vic_key, vic_joystick, VicFormHandler, vic_callback,
 "1981", "vic20", "VIC-20", "Commodore", 0},
#endif

#ifdef F_ORIC
{foric, orica, CPU_M6502, ORIC_CLOCK, ORIC_RAM, 3, 50,
 OricColor, OricControl, OricButtonDef, NULL,
 NtscSquareID, OricConfigForm, DAC_SAMPLE|DAC_TONE,
 oric_select, NULL, NULL, oric_finish, NULL, NULL,
 oric_key, oric_joystick, OricFormHandler, oric_callback,
 "1984", "orica", "Oric Atmos", "Tangerine", 0},
#endif

#ifdef F_ATOM
{fatom, atom, CPU_M6502, ATOM_CLOCK, ATOM_RAM, 4, 60,
 CocoColor, AtomControl, AtomButtonDef, NULL,
 AtomID, 0, DAC_SAMPLE,
 atom_select, atom_init, NULL, atom_finish, NULL, NULL,
 atom_key, NULL, NULL, atom_callback,
 "1980", "atom", "Atom", "Acorn", 0},
#endif

#ifdef F_NES
{fnes, nes, CPU_M6502, NES_CLOCK, NES_RAM, 5, 60,
 NesColor, NesControl, NesButtonDef, NULL,
 NtscSquareID, NesConfigForm, DAC_NONE,
 nes_select, nes_init, nes_reset, nes_finish, nes_osd, nes_debug,
 nes_key, nes_joystick, NesFormHandler, nes_callback,
 "1985", "nes", "NES (NTSC)", "Nintendo", 0},
#endif

#ifdef F_MSX
{fmsx, expert11, CPU_Z80, MSX_CLOCK, MSX_RAM, 8, 50,
 Ti9918Color, MsxControl, MsxButtonDef, NULL,
 NtscSquareID, MsxConfigForm, DAC_SAMPLE|DAC_TONE,
 msx_select, NULL, msx_reset, msx_finish, NULL, NULL,
 msx_key, msx_joystick, MsxFormHandler, msx_callback,
 "1986", "expert11", "Expert 1.1", "Gradiente", 0},

{fmsx, hotbit12, CPU_Z80, MSX_CLOCK, MSX_RAM, 8, 50,
 Ti9918Color, MsxControl, MsxButtonDef, NULL,
 NtscSquareID, MsxConfigForm, DAC_SAMPLE|DAC_TONE,
 msx_select, NULL, msx_reset, msx_finish, NULL, NULL,
 msx_key, msx_joystick, MsxFormHandler, msx_callback,
 "1986", "hotbit12", "Hotbit 1.2", "Epcom/Sharp", 0},
#endif

#ifdef F_COLECO
{fcoleco, coleco, CPU_Z80, COLECO_CLOCK, COLECO_RAM, 9, 60,
 Ti9918Color, ColecoControl, ColecoButtonDef, NULL,
 NtscSquareID, ColecoConfigForm, DAC_TONE,
 coleco_select, coleco_init, coleco_reset, coleco_finish, NULL, NULL,
 coleco_key, coleco_joystick, ColecoFormHandler, coleco_callback,
 "1982", "coleco", "Colecovision", "Coleco", 0},
#endif

{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

static RomType rom[] = {
  {coco,     0x2000, 0x2000, 0x00b50aaa, 0, 1, "bas10.rom"},
  {cocoe,    0x2000, 0x2000, 0x6270955a, 0, 1, "bas11.rom"},
  {cocoe,    0x0000, 0x2000, 0x6111a086, 0, 1, "extbas10.rom"},
  {coco2,    0x2000, 0x2000, 0x54368805, 0, 1, "bas12.rom"},
  {coco2,    0x0000, 0x2000, 0xa82a6254, 0, 1, "extbas11.rom"},
  {coco2b,   0x2000, 0x2000, 0xd8f4d15e, 0, 1, "bas13.rom"},
  {coco2b,   0x0000, 0x2000, 0xa82a6254, 0, 1, "extbas11.rom"},
  {cp400,    0x0000, 0x4000, 0x878396a5, 0, 1, "cp400bas.rom"},
  {dragon32, 0x0000, 0x4000, 0xe3879310, 0, 1, "d32.rom"},
  {dragon64, 0x0000, 0x4000, 0x60a4634c, 0, 1, "d64_1.rom"},
  {dragon64, 0x0000, 0x4000, 0x17893a42, 0, 0, "d64_2.rom"},
  {mc10,     0x6000, 0x2000, 0x11fda97e, 0, 1, "mc10.rom"},
  {mc1000,   0x4000, 0x3570, 0x00000000, 0, 1, "mc1000.rom"},
  {vz300,    0x0000, 0x4000, 0x00000000, 0, 1, "vtechv20.rom"},
  {spectrum, 0x0000, 0x4000, 0xddee531f, 0, 1, "tk95.rom"},
  {expert11, 0x0000, 0x8000, 0x00000000, 0, 1, "expbios11.rom"},
  {hotbit12, 0x0000, 0x8000, 0x00000000, 0, 1, "hotbit12.rom"},
  {coleco,   0x0000, 0x2000, 0x00000000, 0, 1, "coleco.rom"},
  {vic20,    0x0000, 0x1000, 0x00000000, 0, 1, "901460.03"},
  {vic20,    0x4000, 0x2000, 0x00000000, 0, 1, "901486.01"},
  {vic20,    0x6000, 0x2000, 0x00000000, 0, 1, "901486.06"},
  {orica,    0x4000, 0x4000, 0x00000000, 0, 1, "basic11b.rom"},
  {atom,     0x4000, 0x1000, 0x00000000, 0, 1, "abasic.rom"},
  {atom,     0x5000, 0x1000, 0x00000000, 0, 1, "afloat.rom"},
  {atom,     0x7000, 0x1000, 0x00000000, 0, 1, "akernel.rom"},
  {cgenie,   0x0000, 0x4000, 0x00000000, 0, 1, "cgenie.rom"},
  {cgenie,   0xC000, 0x2000, 0x00000000, 0, 1, "cgdos.rom"},
  {jupiter,  0x0000, 0x1000, 0x00000000, 0, 1, "jupiter.lo"},
  {jupiter,  0x1000, 0x1000, 0x00000000, 0, 1, "jupiter.hi"},
  {aquarius, 0x0000, 0x2000, 0x00000000, 0, 1, "aq2.rom"},
  {apple2p,  0x5000, 0x3000, 0x00000000, 0, 1, "apple2p.rom"},
  {0, 0, 0, 0, 0, 0, NULL}
};

//{cocoe,    0x4000, 0x2000, 0xb4f9968e, 1, 1, "disk10.rom"},
//{coco2,    0x4000, 0x2000, 0x0b9c5415, 1, 1, "disk11.rom"},
//{coco2b,   0x4000, 0x2000, 0x0b9c5415, 1, 1, "disk11.rom"},
//{cp400,    0x4000, 0x2000, 0xe9ad60a0, 0, 1, "cp400dsk.rom"},
//{dragon32, 0x4000, 0x2000, 0xb44536f6, 0, 1, "ddos10.rom"},
//{dragon64, 0x4000, 0x2000, 0xb44536f6, 0, 1, "ddos10.rom"},

static Int16 num = 0;
static char *name[MAX_MACHINES];
static Int16 romindex[MAX_MACHINES];
static Int16 numroms[MAX_MACHINES];

void mch_init(void)
{
  Int16 i;

  for (num = 0; machine[num].name; num++) {
    machine[num].fname = family[machine[num].family];
    name[num] = machine[num].name;

    numroms[num] = 0;
    romindex[num] = 0;

    for (i = 0; rom[i].id; i++) {
      if (rom[i].id == machine[num].id) {
        if (numroms[num] == 0)
          romindex[num] = i;
        numroms[num]++;
      }
    }
  }
}

void mch_finish(void)
{
}

MachineType *mch_getmachine(Int16 index)
{
  return (index >= 0 && index < num) ? &machine[index] : NULL;
}

char **mch_getfamilies(void)
{
  return family;
}

Int16 mch_getnummachines(void)
{
  return num;
}

char **mch_getnames(void)
{
  return name;
}

Int16 mch_getnumroms(Int16 index)
{
  if (index < 0 || index >= num)
    return 0;

  return numroms[index];
}

RomType *mch_getrom(Int16 index, Int16 r)
{
  if (index < 0 || index >= num) 
    return NULL;

  if (r < 0 || r >= numroms[index])
    return NULL;

  return &rom[romindex[index]+r];
}

void mch_createdirs(void)
{
  mch_mkdir(fcoco, DIR_ROM);
  mch_mkdir(fcoco, DIR_DISK);
  mch_mkdir(fcoco, DIR_CASSETTE);
  mch_mkdir(fcoco, DIR_SNAPSHOT);
  mch_mkdir(fcoco, DIR_SCREENSHOT);

  mch_mkdir(fdragon, DIR_ROM);
  mch_mkdir(fdragon, DIR_DISK);
  mch_mkdir(fdragon, DIR_CASSETTE);
  mch_mkdir(fdragon, DIR_SNAPSHOT);
  mch_mkdir(fdragon, DIR_SCREENSHOT);

  mch_mkdir(fmc10, DIR_ROM);
  mch_mkdir(fmc10, DIR_CASSETTE);
  mch_mkdir(fmc10, DIR_SCREENSHOT);

  mch_mkdir(fmc1000, DIR_ROM);
  mch_mkdir(fmc1000, DIR_CASSETTE);
  mch_mkdir(fmc1000, DIR_SCREENSHOT);

  mch_mkdir(fvz, DIR_ROM);
  mch_mkdir(fvz, DIR_SNAPSHOT);
  mch_mkdir(fvz, DIR_SCREENSHOT);

  mch_mkdir(fspec, DIR_ROM);
  mch_mkdir(fspec, DIR_SNAPSHOT);
  mch_mkdir(fspec, DIR_SCREENSHOT);

#ifdef F_CGENIE
  mch_mkdir(fcgenie, DIR_ROM);
  mch_mkdir(fcgenie, DIR_DISK);
  mch_mkdir(fcgenie, DIR_SNAPSHOT);
  mch_mkdir(fcgenie, DIR_SCREENSHOT);
#endif

#ifdef F_AQUARIUS
  mch_mkdir(faquarius, DIR_ROM);
  mch_mkdir(faquarius, DIR_SNAPSHOT);
  mch_mkdir(faquarius, DIR_SCREENSHOT);
#endif

#ifdef F_JUPITER
  mch_mkdir(fjupiter, DIR_ROM);
  mch_mkdir(fjupiter, DIR_SNAPSHOT);
  mch_mkdir(fjupiter, DIR_SCREENSHOT);
#endif

#ifdef F_CPM
  mch_mkdir(fcpm, DIR_DISK);
  mch_mkdir(fcpm, DIR_SCREENSHOT);
#endif

#ifdef F_APPLE
  mch_mkdir(fapple, DIR_ROM);
  mch_mkdir(fapple, DIR_SCREENSHOT);
#endif

#ifdef F_VIC20
  mch_mkdir(fvic20, DIR_ROM);
  mch_mkdir(fvic20, DIR_SNAPSHOT);
  mch_mkdir(fvic20, DIR_SCREENSHOT);
#endif

#ifdef F_ORIC
  mch_mkdir(foric, DIR_ROM);
  mch_mkdir(foric, DIR_SNAPSHOT);
  mch_mkdir(foric, DIR_SCREENSHOT);
#endif

#ifdef F_ATOM
  mch_mkdir(fatom, DIR_ROM);
  mch_mkdir(fatom, DIR_SNAPSHOT);
  mch_mkdir(fatom, DIR_SCREENSHOT);
#endif

#ifdef F_NES
  mch_mkdir(fnes, DIR_ROM);
  mch_mkdir(fnes, DIR_SNAPSHOT);
  mch_mkdir(fnes, DIR_SCREENSHOT);
#endif

#ifdef F_MSX
  mch_mkdir(fmsx, DIR_ROM);
  mch_mkdir(fmsx, DIR_SNAPSHOT);
  mch_mkdir(fmsx, DIR_SCREENSHOT);
#endif

#ifdef F_COLECO
  mch_mkdir(fcoleco, DIR_ROM);
  mch_mkdir(fcoleco, DIR_SNAPSHOT);
  mch_mkdir(fcoleco, DIR_SCREENSHOT);
#endif
}

void mch_mkdir(Int16 f, Int16 dir)
{
  Int16 vol;
  static char path[128];

  if ((vol = check_volume()) == -1)
    return;

  MemSet(path, sizeof(path), 0);
  StrCopy(path, AppDir);

  switch (dir) {
    case DIR_ROM:
      StrCat(path, "/ROM/");
      break;
    case DIR_DISK:
      StrCat(path, "/Disk/");
      break;
    case DIR_CASSETTE:
      StrCat(path, "/Cassette/");
      break;
    case DIR_SNAPSHOT:
      StrCat(path, "/Snapshot/");
      break;
    case DIR_SCREENSHOT:
      StrCat(path, "/Screenshot/");
      break;
    default:
      return;
  }

  StrCat(path, family[f]);
  VFSDirCreate(vol, path);
}

UInt8 getramsize(UInt8 ramsizes, UInt16 index)
{
  UInt16 i = 0;

  if ((ramsizes & RAM_4K)  && (index == i++)) return RAM_4K;
  if ((ramsizes & RAM_16K) && (index == i++)) return RAM_16K;
  if ((ramsizes & RAM_32K) && (index == i++)) return RAM_32K;
  if ((ramsizes & RAM_64K) && (index == i++)) return RAM_64K;

  return 0;
}

UInt8 getmaxramsize(UInt8 ramsizes)
{
  if (ramsizes & RAM_64K) return RAM_64K;
  if (ramsizes & RAM_32K) return RAM_32K;
  if (ramsizes & RAM_16K) return RAM_16K;
  if (ramsizes & RAM_4K)  return RAM_4K;
  return 0;
}

char **buildramsizes(UInt16 id, UInt8 ramsizes, UInt8 ramsize,
                     UInt16 *num, UInt16 *index)
{
  static char *list0[4] = {"4K","16K","32K","64K"};
  static char *list1[4] = {"16K","32K","64K","X"};
  static char *list2[4] = {"16K","64K","X","X"};
  static char *list3[4] = {"32K","X","X","X"};
  static char *list4[4] = {"64K","X","X","X"};
  static char *list5[4] = {"4K","20K","X","X"};
  static char *list6[4] = {"48K","X","X","X"};
  static char *list7[4] = {"16K","X","X","X"};
  static char *list8[4] = {"12K","X","X","X"};
  static char *list9[4] = {"16K","32K","X","X"};
  static char *list10[4] = {"49K","X","X","X"};
  static char *list11[4] = {"1K","X","X","X"};
  static char *list12[4] = {"2K","X","X","X"};
  static char *list13[4] = {"16K","48K","X","X"};
  char **list;

  switch (ramsizes) {
    case RAM_4K|RAM_16K|RAM_32K|RAM_64K:
      list = list0;
      *num = 4;
      switch (ramsize) {
        case RAM_4K:  *index = 0; break;
        case RAM_16K: *index = 1; break;
        case RAM_32K: *index = 2; break;
        case RAM_64K: *index = 3; break;
        default: *index = 0;
      }
      break;
    case RAM_16K|RAM_32K|RAM_64K:
      list = list1;
      *num = 3;
      switch (ramsize) {
        case RAM_16K: *index = 0; break;
        case RAM_32K: *index = 1; break;
        case RAM_64K: *index = 2; break;
        default: *index = 0;
      }
      break;
    case RAM_16K|RAM_64K:
      list = list2;
      *num = 2;
      switch (ramsize) {
        case RAM_16K: *index = 0; break;
        case RAM_64K: *index = 1; break;
        default: *index = 0;
      }
      break;
    case RAM_32K:
      list = list3;
      *num = 1;
      *index = 0;
      break;
    case RAM_64K:
      switch (id) {
        case spectrum:
        case apple2p:
        case orica:
          list = list6;
          break;
        case jupiter:
          list = list10;
          break;
        default:
          list = list4;
      }
      *num = 1;
      *index = 0;
      break;
    case RAM_4K|RAM_16K:
      list = list5;
      *num = 2;
      switch (ramsize) {
        case RAM_4K:  *index = 0; break;
        case RAM_16K: *index = 1; break;
        default: *index = 0;
      }
      break;
    case RAM_16K:
      list = (id == atom) ? list8 : list7;
      *num = 1;
      *index = 0;
      break;
    case RAM_16K|RAM_32K:
      list = (id == mc1000) ? list13: list9;
      *num = 2;
      switch (ramsize) {
        case RAM_16K: *index = 0; break;
        case RAM_32K: *index = 1; break;
        default: *index = 0;
      }
      break;
    case RAM_4K:
      list = (id == coleco) ? list11 : list12;
      *num = 1;
      *index = 0;
      break;
    default:
      list = list1;
      *num = 3;
      *index = 0;
  }

  return list;
}
