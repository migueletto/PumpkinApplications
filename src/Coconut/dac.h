#define DAC_NONE	0x00
#define DAC_SAMPLE	0x01
#define DAC_TONE	0x02

#define SAMPLE_RATE	44100

typedef struct SoundSample {
  uint32_t cycle;
  uint32_t sample;
} SoundSample;

typedef struct CallbackArg {
  uint32_t mutex;
  uint32_t samples, samples_arm;
  uint32_t cycles;
  unsigned char buffer[1024], *buffer_arm;
  uint32_t last_sample;
} CallbackArg;

void dac_init(int32_t rate, int32_t samples, int32_t cycles);
void dac_finish(void);

void dac_setamp(uint32_t amp);
void dac_buffer(SoundSample *s, uint32_t n);

void dac_settone(uint32_t freq, uint32_t amp);
