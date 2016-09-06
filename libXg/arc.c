/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <libg.h>
#include "libgint.h"
#include <math.h>

#define rad2deg(x) 180*((x)/3.1415926535897932384626433832795028841971693993751)

void
arc(Bitmap *b, Point p0, Point p1, Point p2, int v, Fcode f)
{
    unsigned int d;
    int x, y, r, start, end, delta;
    GC g;

    p1.x -= p0.x;
    p1.y -= p0.y;
    p2.x -= p0.x;
    p2.y -= p0.y;
    r = (int)sqrt((double)(p1.x*p1.x + p1.y*p1.y));
    start = (int)(64*rad2deg(atan2(-p2.y, p2.x)));
    end = (int)(64*rad2deg(atan2(-p1.y, p1.x)));
    if(start < 0)
        start += 64*360;
    if(end < 0)
        end += 64*360;
    delta = end - start;
    if(delta < 0)
        delta += 64*360;
    x = p0.x - r;
    y = p0.y - r;
    if(b->flag&SHIFT){
        x -= b->r.min.x;
        y -= b->r.min.y;
    }
    d = 2*r;
    g = _getfillgc(f, b, v);
    /*
     * delta is positive, so this draws counterclockwise arc
     * from start to start+delta
     */
    XDrawArc(_dpy, (Drawable)b->id, g, x, y, d, d, start, delta);
}

