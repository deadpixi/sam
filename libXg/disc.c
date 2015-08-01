/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <libg.h>
#include "libgint.h"

void
disc(Bitmap *b, Point p, int r, int v, Fcode f)
{
	unsigned int d;
	int x, y;
	GC g;

	x = p.x - r;
	y = p.y - r;
	if (b->flag&SHIFT){
		x -= b->r.min.x;
		y -= b->r.min.y;
	}
	d = 2*r;
	g = _getfillgc(f, b, v);
	XFillArc(_dpy, (Drawable)b->id, g, x, y, d, d, 0, 23040/* 360 deg */);
}
