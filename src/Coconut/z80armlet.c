#include <PalmOS.h>

#include "cpu.h"
#include "armlet.h"
#include "z80.h"
#include "io.h"
#include "endian.h"

uint32_t z80_ArmletStart(ArmletArg *arg) {
  uint32_t r = 0;

  switch (arg->cmd) {
    case CMD_INIT:
      z80_init(arg->hardware->cpu);
      break;

    case CMD_RESET:
      z80_reset(arg->hardware->cpu);
      break;

    case CMD_EXECUTE:
      r = z80_execute(arg->hardware->cpu, arg->a1);
      break;

    case CMD_EXIT:
      z80_exit(arg->hardware->cpu);
      break;
  }

  return r;
}
