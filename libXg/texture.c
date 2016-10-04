/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include "libgint.h"

void
texture(Bitmap *d, Rectangle r, Bitmap *s, Fcode f)
{
    int x, y, w, h, bfunc;
    GC g;

    x = r.min.x;
    y = r.min.y;
    if(d->flag&SHIFT){
        x -= d->r.min.x;
        y -= d->r.min.y;
    }
    g = _getcopygc(f, d, s, &bfunc);
    if(d->flag&SHIFT){
        XSetTSOrigin(_dpy, g, -d->r.min.x, -d->r.min.y);
    }else
        XSetTSOrigin(_dpy, g, 0, 0);
    w = Dx(r);
    h = Dy(r);
    if(bfunc == UseFillRectangle){
        /* source isn't involved at all */
        XFillRectangle(_dpy, (Drawable)d->id, g, x, y, w, h);
    }else if(bfunc == UseCopyArea){
        XSetTile(_dpy, g, (Drawable)s->id);
        XSetFillStyle(_dpy, g, FillTiled);
        XFillRectangle(_dpy, (Drawable)d->id, g, x, y, w, h);
        XSetFillStyle(_dpy, g, FillSolid);
    }else{
        if(s->ldepth != 0)
            berror("unsupported texture");
        XSetStipple(_dpy, g, (Drawable)s->id);
        XSetFillStyle(_dpy, g, FillOpaqueStippled);
        XFillRectangle(_dpy, (Drawable)d->id, g, x, y, w, h);
        XSetFillStyle(_dpy, g, FillSolid);
    }
}
