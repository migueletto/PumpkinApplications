#ifndef __ENDIANUTILS_H__
#define __ENDIANUTILS_H__

// Big-endian: M68K, M6809
// Little-endian: ARM, Z80

// handy macros for handling endian and alignment issues

#define ByteSwap16(n) ( ((((unsigned short)(n)) << 8) & 0xFF00) | \
                        ((((unsigned short)(n)) >> 8) & 0x00FF) )

#define ByteSwap32(n) ( ((((unsigned long)(n)) << 24) & 0xFF000000) |	\
                        ((((unsigned long)(n)) <<  8) & 0x00FF0000) |	\
                        ((((unsigned long)(n)) >>  8) & 0x0000FF00) |	\
                        ((((unsigned long)(n)) >> 24) & 0x000000FF) )

#define ReadUnaligned32(addr)  \
	( ((((unsigned char *)(addr))[0]) << 24) | \
	  ((((unsigned char *)(addr))[1]) << 16) | \
	  ((((unsigned char *)(addr))[2]) <<  8) | \
	  ((((unsigned char *)(addr))[3])) )

#define WriteUnaligned32(addr, value) \
	( ((unsigned char *)(addr))[0] = (unsigned char)((unsigned long)(value) >> 24), \
	  ((unsigned char *)(addr))[1] = (unsigned char)((unsigned long)(value) >> 16), \
	  ((unsigned char *)(addr))[2] = (unsigned char)((unsigned long)(value) >>  8), \
	  ((unsigned char *)(addr))[3] = (unsigned char)((unsigned long)(value)) )

#ifdef LSB_FIRST
#define BIG_ENDIANIZE_INT16(x)          (ByteSwap16(x))
#define BIG_ENDIANIZE_INT32(x)          (ByteSwap32(x))
#define LITTLE_ENDIANIZE_INT16(x)       (x)
#define LITTLE_ENDIANIZE_INT32(x)       (x)
#else
#define BIG_ENDIANIZE_INT16(x)          (x)
#define BIG_ENDIANIZE_INT32(x)          (x)
#define LITTLE_ENDIANIZE_INT16(x)       (ByteSwap16(x))
#define LITTLE_ENDIANIZE_INT32(x)       (ByteSwap32(x))
#endif

#endif // __ENDIANUTILS_H__
