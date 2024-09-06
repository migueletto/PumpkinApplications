#include <PalmOS.h>

#include "section.h"
#include "cpu.h"
#include "endian.h"
#include "sn76489.h"
#include "cpu.h"
#include "armlet.h"
#include "io.h"
#include "palm.h"
#include "endian.h"
#include "gui.h"

void sn76489_init(SN76489 *sn76489, Hardware *hardware, UInt32 clock) {
  MemSet(sn76489, sizeof(SN76489), 0);
  sn76489->hardware = hardware;
  sn76489->clock = clock;

  hardware->snd_tonefreq = 0;
  hardware->snd_toneamp = 0;
  hardware->snd_noisefreq = 0;
  hardware->snd_noiseamp = 0;
}

static void sn76489_update(SN76489 *sn76489);

void sn76489_setb(SN76489 *sn76489, UInt8 b) {
  UInt16 data;

  if (b & 0x80) {
    // latch/data byte

    sn76489->channel = (b & 0x60) >> 5;
    sn76489->type = (b & 0x10) >> 4;	// 0=tone, 1=amp
    data = b & 0x0F;

    if (sn76489->type == 0) {	// tone (lower 4 bits)
      switch (sn76489->channel) {
        case 0:	// A
          sn76489->periodA = (sn76489->periodA & 0x3F0) | data;
          sn76489->toneA = sn76489->periodA ?
                           (sn76489->clock / 32) / sn76489->periodA : 0;
          break;
        case 1:	// B
          sn76489->periodB = (sn76489->periodB & 0x3F0) | data;
          sn76489->toneB = sn76489->periodB ?
                           (sn76489->clock / 32) / sn76489->periodB : 0;
          break;
        case 2:	// C
          sn76489->periodC = (sn76489->periodC & 0x3F0) | data;
          sn76489->toneC = sn76489->periodC ?
                           (sn76489->clock / 32) / sn76489->periodC : 0;
          break;
      }
      sn76489_update(sn76489);

    } else {		// amp (4 bits)

      switch (sn76489->channel) {
        case 0:	// A
          sn76489->ampA = 15 - data;
          break;
        case 1:	// B
          sn76489->ampB = 15 - data;
          break;
        case 2:	// C
          sn76489->ampC = 15 - data;
          break;
      }
      sn76489_update(sn76489);
    }

  } else {
    // data byte

    if (sn76489->type == 0) {	// tone (upper 6 bits)
      data = (b & 0x3F) << 4;

      switch (sn76489->channel) {
        case 0:	// A
          sn76489->periodA = (sn76489->periodA & 0x00F) | data;
          sn76489->toneA = sn76489->periodA ?
                           (sn76489->clock / 32) / sn76489->periodA : 0;
          break;
        case 1:	// B
          sn76489->periodB = (sn76489->periodB & 0x00F) | data;
          sn76489->toneB = sn76489->periodB ?
                           (sn76489->clock / 32) / sn76489->periodB : 0;
          break;
        case 2:	// C
          sn76489->periodC = (sn76489->periodC & 0x00F) | data;
          sn76489->toneC = sn76489->periodC ?
                           (sn76489->clock / 32) / sn76489->periodC : 0;
          break;
      }
      sn76489_update(sn76489);

    } else {			// amp (4 bits)
      data = b & 0x0F;

      switch (sn76489->channel) {
        case 0:	// A
          sn76489->ampA = 15 - data;
          break;
        case 1:	// B
          sn76489->ampB = 15 - data;
          break;
        case 2:	// C
          sn76489->ampC = 15 - data;
          break;
      }
      sn76489_update(sn76489);
    }
  }
}

static void sn76489_update(SN76489 *sn76489) {
  UInt32 freq, amp;

  freq = 0;
  amp = 0;

  if (sn76489->ampA > amp) {
    freq = sn76489->toneA;
    amp = sn76489->ampA;
  }
  if (sn76489->ampB > amp) {
    freq = sn76489->toneB;
    amp = sn76489->ampB;
  }
  if (sn76489->ampC > amp) {
    freq = sn76489->toneC;
    amp = sn76489->ampC;
  }
  snd_settone(freq, amp << 2);
  //snd_setnoise(freq, amp << 2);
}
