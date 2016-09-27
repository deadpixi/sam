/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <libg.h>
#include "libgint.h"

#define PJW 0   /* use NUL==pjw for invisible characters */

long
charwidth(XftFont *f, wchar_t r)
{
    
    char chars[UTFmax + 1] = {0};

    runetochar(chars, &r);
    return strwidth(f, chars);
}

