/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <string.h>
#include <u.h>
#include <libg.h>
#include "libgint.h"

Point
string(Bitmap *b, Point p, XftFont *ft, char *s, Fcode f)
{
    size_t length = strlen(s);
    int x = p.x;
    int y = p.y;

    x = p.x;
    y = p.y;
    if (b->flag & SHIFT){
        x -= b->r.min.x;
        y -= b->r.min.y;
    }
    y += ft->ascent;

    if (!b->fd)
        b->fd = XftDrawCreate(_dpy, (Drawable)(b->id), DefaultVisual(_dpy, DefaultScreen(_dpy)), DefaultColormap(_dpy, DefaultScreen(_dpy)));
    XftDrawStringUtf8(b->fd, &fontcolor, ft, x, y, (FcChar8 *)s, length);

    x += strwidth(ft, s);

    p.x = (b->flag & SHIFT) ? x + b->r.min.x : x;
    p.x = x + b->r.min.x;
    return p;
}


int64_t
strwidth(XftFont *f, char *s)
{
    XGlyphInfo extents = {0};
    XftTextExtentsUtf8(_dpy, f, (FcChar8 *)s, strlen(s), &extents);

    return extents.xOff;
}

int64_t
charwidth(XftFont *f, wchar_t r)
{
    char chars[MB_LEN_MAX + 1] = {0};

    if (runetochar(chars, r) < 0)
        return 0;

    return strwidth(f, chars);
}
