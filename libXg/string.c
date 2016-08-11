/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <string.h>
#include <u.h>
#include <libc.h>
#include <libg.h>
#include "libgint.h"

enum	{ Max = 128 };

Point
string(Bitmap *b, Point p, XftFont *ft, char *s, Fcode f)
{
	size_t     length  = strlen(s);
	XGlyphInfo extents = {0};
	int        x       = p.x;
	int        y       = p.y;

	XftTextExtentsUtf8(_dpy, ft, s, length, &extents);

	x = p.x;
	y = p.y;
	if (b->flag & SHIFT){
		x -= b->r.min.x;
		y -= b->r.min.y;
	}
	y += ft->ascent;

    if (!b->fd)
	    b->fd = XftDrawCreate(_dpy, (Drawable)(b->id), DefaultVisual(_dpy, DefaultScreen(_dpy)), DefaultColormap(_dpy, DefaultScreen(_dpy)));

	XftDrawStringUtf8(b->fd, &fontcolor, ft, x, y, s, length);

	x += extents.xOff;

	p.x = (b->flag & SHIFT) ? x + b->r.min.x : x;
	p.x = x + b->r.min.x;
	return p;
}
