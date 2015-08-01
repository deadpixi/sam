/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <libg.h>
#include "libgint.h"

void
segment(Bitmap *d, Point p1, Point p2, int v, Fcode f)
{
	int x1, y1, x2, y2;
	GC g;

	x1 = p1.x;
	y1 = p1.y;
	x2 = p2.x;
	y2 = p2.y;
	if(d->flag&SHIFT){
		x1 -= d->r.min.x;
		y1 -= d->r.min.y;
		x2 -= d->r.min.x;
		y2 -= d->r.min.y;
	}
	g = _getfillgc(f, d, v);
	XDrawLine(_dpy, (Drawable)d->id, g, x1, y1, x2, y2);
}
