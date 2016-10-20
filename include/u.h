#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <stdbool.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>

#define utflen(s)        (mbstowcs(NULL, (s), 0))
#define fullrune(s, n)   (mbtowc(NULL, (s), (n)) >= 0)
#define runetochar(s, r) (wctomb((s), (r)))
#define runelen(r)       (wctomb(NULL, (r)))
#define UNICODE_REPLACEMENT_CHAR 0xfffd

int chartorune(wchar_t *, char *);
