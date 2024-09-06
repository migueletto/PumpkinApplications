#include <PalmOS.h>

#include "cpu.h"
#include "armlet.h"
#include "m6502.h"
#include "io.h"
#include "endian.h"
#include "debug.h"

uint32_t m6502_ArmletStart(ArmletArg *arg) {
  uint32_t r = 0;

  switch (arg->cmd) {
    case CMD_INIT:
      m6502_init(arg->hardware->cpu);
      break;

    case CMD_RESET:
      m6502_reset(arg->hardware->cpu);
      break;

    case CMD_EXECUTE:
      r = m6502_execute(arg->hardware->cpu, arg->a1);
      break;

    case CMD_EXIT:
      m6502_exit(arg->hardware->cpu);
      break;
  }

  return r;
}
