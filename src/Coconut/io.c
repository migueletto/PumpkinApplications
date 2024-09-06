#include <PalmOS.h>

#include "cpu.h"
#include "armlet.h"
#include "io.h"

void io_setb(ArmletArg *aarg, UInt32 cmd, UInt32 a1, UInt32 a2) {
  ArmletCallbackArg arg;

  arg.cmd = cmd;
  arg.a1 = a1;
  arg.a2 = a2;
  arg.hardware = aarg->hardware;
  aarg->callback(&arg);
}

UInt8 io_readb(ArmletArg *aarg, UInt16 a) {
  ArmletCallbackArg arg;
  UInt32 r;

  arg.cmd = IO_READB;
  arg.a1 = a;
  arg.hardware = aarg->hardware;
  r = aarg->callback(&arg);

  return (UInt8)r;
}

void io_writeb(ArmletArg *aarg, UInt16 a, UInt8 b) {
  ArmletCallbackArg arg;

  arg.cmd = IO_WRITEB;
  arg.a1 = a;
  arg.a2 = b;
  arg.hardware = aarg->hardware;
  aarg->callback(&arg);
}
