#ifndef _VARARG
#define _VARARG

/*
typedef char * va_list;
#define __va_start(parm)        (va_list) (&parm + 1)
#define va_start(ap, parm)      ap = __va_start(parm)
#define va_end(ap)
#define va_arg(ap, type)        (*(((type *) (ap += sizeof(type))) - 1))
*/
#include <stdarg.h>

#endif
