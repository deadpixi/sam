/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <libg.h>
#include "libgint.h"

/* e(x,y) = b*b*x*x + a*a*y*y - a*a*b*b */

void
ellipse(Bitmap *bp, Point p, int a, int b, int v, Fcode f)
{
    int x, y;
    GC g;

    x = p.x - a;
    y = p.y - b;
    if (bp->flag&SHIFT){
        x -= bp->r.min.x;
        y -= bp->r.min.y;
    }
    g = _getfillgc(f, bp, v);
    XDrawArc(_dpy, (Drawable)bp->id, g, x, y, 2*a, 2*b, 0, 23040/* 360 deg */);
}
