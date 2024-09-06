typedef struct {
  Hardware *hardware;
  UInt32 clock;
  UInt32 channel, type;
  UInt32 periodA, periodB, periodC, periodN;
  UInt32 ampA, ampB, ampC, ampN;
  UInt32 toneA, toneB, toneC, noise;
} SN76489;

void sn76489_init(SN76489 *sn76489, Hardware *hardware, UInt32 clock) SECTION("aux");

void sn76489_setb(SN76489 *sn76489, UInt8 b);
