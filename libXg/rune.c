/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <string.h>

#define UNICODE_REPLACEMENT_CHAR 0xfffd

int
chartorune(wchar_t *rune, char *str)
{
    int r = mbtowc(rune, str, strlen(str));
    if (r < 0){
        *rune = UNICODE_REPLACEMENT_CHAR;
        return 1;
    }
    return r;
}
