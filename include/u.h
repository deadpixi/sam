#include <fcntl.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>

typedef uint16_t ushort;
typedef uint8_t  uchar;
typedef uint16_t  Rune;

#if USE64BITS == 0
typedef uint32_t ulong;
#else
typedef uint64_t ulong;
#endif

