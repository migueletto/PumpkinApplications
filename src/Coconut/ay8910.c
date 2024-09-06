#include <PalmOS.h>

#include "section.h"
#include "cpu.h"
#include "endian.h"
#include "ay8910.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "palm.h"
#include "endian.h"
#include "gui.h"

void ay8910_init(AY8910 *ay8910, Hardware *hardware, UInt32 type, UInt32 clock) {
  MemSet(ay8910, sizeof(AY8910), 0);
  ay8910->hardware = hardware;
  ay8910->type = type;
  ay8910->clock = clock;

  hardware->snd_tonefreq = 0;
  hardware->snd_toneamp = 0;
  hardware->snd_noisefreq = 0;
  hardware->snd_noiseamp = 0;
}

static void ay8910_update(AY8910 *ay8910);

void ay8910_setindex(AY8910 *ay8910, UInt32 index) {
  ay8910->index = index & 0x0F;
}

UInt32 ay8910_getindex(AY8910 *ay8910) {
  return ay8910->index;
}

static void ay8910_update(AY8910 *ay8910) {
  UInt32 freq, amp;

  freq = 0;
  amp = 0;

  if (ay8910->mixer_toneA && ay8910->ampA > amp) {
    freq = ay8910->toneA;
    amp = ay8910->ampA;
  }
  if (ay8910->mixer_toneB && ay8910->ampB > amp) {
    freq = ay8910->toneB;
    amp = ay8910->ampB;
  }
  if (ay8910->mixer_toneC && ay8910->ampC > amp) {
    freq = ay8910->toneC;
    amp = ay8910->ampC;
  }
  snd_settone(freq, amp << 2);

  freq = 0;
  amp = 0;

  if (ay8910->mixer_noiseA && ay8910->ampA > amp) {
    freq = ay8910->noise;
    amp = ay8910->ampA;
  }
  if (ay8910->mixer_noiseB && ay8910->ampB > amp) {
    freq = ay8910->noise;
    amp = ay8910->ampB;
  }
  if (ay8910->mixer_noiseC && ay8910->ampC > amp) {
    freq = ay8910->noise;
    amp = ay8910->ampB;
  }

  snd_setnoise(freq, amp << 2);
}

void ay8910_setvalue(AY8910 *ay8910, UInt32 value) {
  UInt8 old;
  UInt32 aux;

  value &= 0xFF;
  old = ay8910->r[ay8910->index];

  if (value == old)
    return;

  ay8910->r[ay8910->index] = value;

  switch (ay8910->index) {
    case 0:
      aux = (((UInt32)ay8910->r[1] & 0x0F) << 8) | value;
      ay8910->toneA = aux ? (ay8910->clock / 16) / aux : 0;
      ay8910_update(ay8910);
      break;
    case 1:
      aux = ((value & 0x0F) << 8) | ay8910->r[0];
      ay8910->toneA = aux ? (ay8910->clock / 16) / aux : 0;
      ay8910_update(ay8910);
      break;
    case 2:
      aux = (((UInt32)ay8910->r[3] & 0x0F) << 8) | value;
      ay8910->toneB = aux ? (ay8910->clock / 16) / aux : 0;
      ay8910_update(ay8910);
      break;
    case 3:
      aux = ((value & 0x0F) << 8) | ay8910->r[2];
      ay8910->toneB = aux ? (ay8910->clock / 16) / aux : 0;
      ay8910_update(ay8910);
      break;
    case 4:
      aux = (((UInt32)ay8910->r[5] & 0x0F) << 8) | value;
      ay8910->toneC = aux ? (ay8910->clock / 16) / aux : 0;
      ay8910_update(ay8910);
      break;
    case 5:
      aux = ((value & 0x0F) << 8) | ay8910->r[4];
      ay8910->toneC = aux ? (ay8910->clock / 16) / aux : 0;
      ay8910_update(ay8910);
      break;
    case 6:
      aux = value & 0x1F;
      ay8910->noise = aux ? (ay8910->clock / 16) / aux : 0;
      ay8910_update(ay8910);
      break;
    case 7:	// mixer
      ay8910->mixer_toneA = (value & 0x01) ? 0 : 1;
      ay8910->mixer_toneB = (value & 0x02) ? 0 : 1;
      ay8910->mixer_toneC = (value & 0x04) ? 0 : 1;
      ay8910->mixer_noiseA = (value & 0x08) ? 0 : 1;
      ay8910->mixer_noiseB = (value & 0x10) ? 0 : 1;
      ay8910->mixer_noiseC = (value & 0x20) ? 0 : 1;
      ay8910->portA_input = (value & 0x40) ? 0 : 1;
      ay8910->portB_input = (value & 0x80) ? 0 : 1;
      ay8910_update(ay8910);
      break;
    case 8:	// channel A amplitude
      ay8910->ampA = (value & 0x10) ? 0 : value & 0x0F;
      ay8910_update(ay8910);
      break;
    case 9:	// channel B amplitude
      ay8910->ampB = (value & 0x10) ? 0 : value & 0x0F;
      ay8910_update(ay8910);
      break;
    case 10:	// channel B amplitude
      ay8910->ampC = (value & 0x10) ? 0 : value & 0x0F;
      ay8910_update(ay8910);
      break;
    case 11:	// envelope fine
      aux = (ay8910->r[12] << 8) | value;
      ay8910->envelope = aux ? (ay8910->clock / 256) / aux : 0;
      ay8910_update(ay8910);
      break;
    case 12:	// envelope coarse
      aux = (value << 8) | ay8910->r[11];
      ay8910->envelope = aux ? (ay8910->clock / 256) / aux : 0;
      ay8910_update(ay8910);
      break;
    case 13:
      break;
    case 14:
      break;
    case 15:
      break;
  }
}

UInt32 ay8910_getvalue(AY8910 *ay8910) {
  return ay8910->r[ay8910->index];
}
