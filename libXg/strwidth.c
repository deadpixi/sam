/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <libg.h>
#include "libgint.h"

long
strwidth(XftFont *f, char *s)
{
    XGlyphInfo extents = {0};
    XftTextExtentsUtf8(_dpy, f, (FcChar8 *)s, strlen(s), &extents);

    return extents.xOff;
}

Point
strsize(XftFont *f, char *s)
{
    XGlyphInfo extents = {0};
    XftTextExtentsUtf8(_dpy, f, (FcChar8 *)s, strlen(s), &extents);

    return Pt(strwidth(f, s), extents.yOff);
}
