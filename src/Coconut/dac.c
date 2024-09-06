#include <PalmOS.h>

#include "palm.h"
#include "dac.h"
#include "endian.h"
#include "cdebug.h"

static SndStreamRef stream = 0;
static CallbackArg *arg = NULL;
static UInt32 last_freq = 0;
static UInt32 last_amp = 0;

static Err dac_callback(void *a, SndStreamRef channel, void *b, uint32_t *len);

void dac_init(int32_t rate, int32_t samples, int32_t cycles) {
  UInt32 version;
  Err err;

  if ((arg = MemPtrNew(sizeof(CallbackArg))) == NULL)
    return;

  MemSet(arg, sizeof(CallbackArg), 0);
  arg->buffer_arm = arg->buffer;
  arg->samples = samples;
  arg->samples_arm = arg->samples;
  arg->cycles = cycles;
  arg->last_sample = 0;

  if (FtrGet(sysFileCSoundMgr, sndFtrIDVersion, &version) == 0) {
    err = SndStreamCreateExtended(&stream, sndOutput, sndFormatPCM,
             rate, sndUInt8, sndMono,
             dac_callback, arg,
             samples, false);
    if (err)
      InfoDialog(D_ERROR, "SndStreamCreate: %d", err);
    else {
      SndStreamSetVolume(stream, sndGameVolume);
      SndStreamStart(stream);
    }
  }
}

void dac_finish(void) {
  if (arg) {
    if (stream) {
      SndStreamStop(stream);
      SndStreamDelete(stream);
      stream = 0;
    }
    MemPtrFree(arg);
    arg = NULL;
  }
}

// spectrum:

// clock  = 3500000
// vsyncs = 50
// rate   = 44100

// samples = rate  / vsyncs = 44100   / 50 = 882
// cycles  = clock / vsyncs = 3500000 / 50 = 70000
// cps = cycles / samples = 70000 / 882 = 79

// coco:
    
// clock  = 894886
// vsyncs = 60
// rate   = 44100
    
// samples = rate  / vsyncs = 44100  / 60 = 735 
// cycles  = clock / vsyncs = 894886 / 60 = 14914
// cps = cycles / samples = 14914 / 735 = 20

// oric (AY8910 white noise):
    
// clock  = 1000000
// vsyncs = 50
// rate   = 44100
    
// samples = rate  / vsyncs = 44100  / 50 = 882 
// cycles  = clock / vsyncs = 1000000 / 50 = 20000
// cps = cycles / samples = 20000 / 882 = 23

void dac_setamp(uint32_t amp) {
  UInt32 max;

  if (arg) {
    // ajusta volume de acordo com preferencia, saida =  0 - 63
    max = PrefGetPreference(prefGameSoundVolume);
    amp = max ? ((amp * max) / sndMaxAmp) : 0;

    // converte de faixa SndDoCmd (0 - 63) para SndStreamSetVolume (0 - 1023)
    SndStreamSetVolume(stream, amp << 4);
  }
}

void dac_buffer(SoundSample *s, uint32_t n) {
  UInt32 i, j, cycle, cycles_per_sample;
  UInt8 sample;

  if (arg) {
    cycle = 0;
    cycles_per_sample = arg->cycles / arg->samples;
    sample = arg->last_sample;

    if (arg->mutex)
      return;

    for (i = 0, j = 0; i < arg->samples; i++) {
      arg->buffer[i] = sample;
      cycle += cycles_per_sample;
      if (cycle >= s[j].cycle && j < n)
        sample = s[j++].sample;
    }
    arg->last_sample = sample;
  }
}

void dac_settone(uint32_t freq, uint32_t amp) {
  SndCommandType cmd;
  UInt32 max;

  if (freq != last_freq || amp != last_amp) {
    max = PrefGetPreference(prefGameSoundVolume);

    cmd.cmd = freq ? sndCmdFrqOn : sndCmdQuiet;
    cmd.param1 = freq;
    cmd.param2 = 1000;
    cmd.param3 = max ? ((amp * max) / sndMaxAmp) : 0;

    SndDoCmd(NULL, &cmd, true);

    last_freq = freq;
    last_amp = amp;
  }
}

static Err dac_callback(void *a, SndStreamRef channel, void *b, uint32_t *len) {
  CallbackArg *arg;
  unsigned char *buffer;
  uint32_t i, n;

  arg = a;
  buffer = b;
  n = *len;

  if (n > arg->samples)
    n = arg->samples;

  arg->mutex = 0xFFFFFFFF;
  for (i = 0; i < n; i++)
    buffer[i] = arg->buffer[i];
  arg->mutex = 0;

  *len = n;

  return 0;
}
