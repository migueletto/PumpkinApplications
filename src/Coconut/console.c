#include <PalmOS.h>

#include "section.h"
#include "console.h"
#include "vt100.h"

#define BUFFER_LEN 64
#define incp(p) ((p + 1) % BUFFER_LEN)
#define decp(p) (p ? p - 1 : BUFFER_LEN)

static UInt8 buffer[BUFFER_LEN];
static Int16 p_in, p_out;

Err init_console(void)
{
  p_in = 0;
  p_out = 0;
  MemSet(buffer, sizeof(buffer), 0);

  return 0;
}

void close_console(void)
{
}

UInt8 stat_console(void)
{
  if (p_in == p_out)
    return 0;

  return 0xff;
}

UInt8 read_console(void)
{
  UInt8 b;

  if (p_in == p_out)
    return 0;

  b = buffer[p_out];
  p_out = incp(p_out);

  return b;
}
 
UInt8 write_console(UInt8 b)
{
  EmitTerminal((char *)&b, 1, 1);
  return 0;
}

void to_console(UInt8 b)
{
  if (incp(p_in) == p_out)
    return;

  buffer[p_in] = b;
  p_in = incp(p_in);
}
