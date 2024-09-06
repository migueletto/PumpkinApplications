#ifndef _ARMLET_H
#define _ARMLET_H

#define CMD_INIT     1
#define CMD_RESET    2
#define CMD_EXECUTE  3
#define CMD_EXIT     4

typedef struct {
  UInt32 cmd;
  UInt32 a1, a2;
  Hardware *hardware;
} ArmletCallbackArg;

typedef UInt32 ArmletCallback(ArmletCallbackArg *arg);

typedef struct {
  UInt32 cmd;
  Int32 a1, a2;
  Hardware *hardware;
  ArmletCallback *callback;
} ArmletArg;

#endif
