/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */

    /* Plan 9 C library interface */


#include <u.h>

#define utflen(s)        (mbstowcs(NULL, (s), 0))
#define fullrune(s, n)   (mbtowc(NULL, (s), (n)) >= 0)
#define runetochar(s, r) (wctomb((s), (r)))
#define runelen(r)       (wctomb(NULL, (r)))

/*
 * new rune routines
 */
int chartorune(wchar_t*, char*);

/*
 *  Miscellaneous functions
 */
void notify (void);
