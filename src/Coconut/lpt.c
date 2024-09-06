#include <PalmOS.h>

#include "section.h"
#include "lpt.h"

Err init_lpt(void)
{
  return 0;
}

void close_lpt(void)
{
}

UInt8 stat_lpt(void)
{
  return 1;
}

UInt8 read_lpt(void)
{
  return 0;
}
 
UInt8 write_lpt(UInt8 b)
{
  return 0;
}
