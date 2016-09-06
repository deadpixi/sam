/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <libg.h>
#include "libgint.h"

/*
 * m should be a 1-bit-deep bitmap with origin (0,0) and the
 * same extents as r.  s should have the same depth as d.
 * Rectangle r of s is copied to d wherever corresponding
 * bits of m are 1
 */
void
copymasked(Bitmap *d, Point p, Bitmap *s, Bitmap *m, Rectangle r)
{
    int sx, sy, dx, dy;
    XGCValues gcv;
    GC g;

    if(Dx(r)<=0 || Dy(r)<=0)
        return;
    sx = r.min.x;
    sy = r.min.y;
    if(s->flag&SHIFT){
        sx -= s->r.min.x;
        sy -= s->r.min.y;
    }
    dx = p.x;
    dy = p.y;
    if(d->flag&SHIFT){
        dx -= d->r.min.x;
        dy -= d->r.min.y;
    }
    gcv.fill_style = FillStippled;
    gcv.stipple = (Pixmap)m->id;
    gcv.function = GXclear;
    gcv.ts_x_origin = dx;
    gcv.ts_y_origin = dy;
    gcv.fill_style = FillStippled;
    g = _getgc(d, GCFunction|GCStipple|GCTileStipXOrigin
        |GCTileStipYOrigin|GCFillStyle, &gcv);
    XFillRectangle(_dpy, (Drawable)d->id, g,
            dx, dy, Dx(r), Dy(r));
    gcv.function = GXor;
    gcv.fill_style = FillSolid;
    g = _getgc(d, GCFunction|GCFillStyle, &gcv);
    XCopyArea(_dpy, (Drawable)s->id, (Drawable)d->id, g,
            sx, sy, Dx(r), Dy(r), dx, dy);
}
