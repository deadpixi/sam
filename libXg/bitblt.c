/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include "libgint.h"

void
bitblt(Bitmap *d, Point p, Bitmap *s, Rectangle r, Fcode f)
{
    bitblt2(d, p, s, r, f, _fgpixel, _bgpixel);
}

void
bitblt2(Bitmap *d, Point p, Bitmap *s, Rectangle r, Fcode f, uint64_t fg, uint64_t bg)
{
    int sx, sy, dx, dy, bfunc;
    GC g;
    uint64_t plane;
    Bitmap *btmp;

    if (fg == 0)
        fg = _fgpixel;

    if (bg == 0)
        bg = _bgpixel;

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
    g = _getcopygc2(f, d, s, &bfunc, fg, bg);
    if(bfunc == UseCopyArea)
        XCopyArea(_dpy, (Drawable)s->id, (Drawable)d->id, g,
            sx, sy, Dx(r), Dy(r), dx, dy);
    else if(bfunc == UseFillRectangle){
        XFillRectangle(_dpy, (Drawable)d->id, g,
            dx, dy, Dx(r), Dy(r));
    }else{
        /* bfunc == UseCopyPlane */
        plane = _ld2dmask[s->ldepth];
        plane &= ~(plane>>1);
        if(0/*f == S*/)
            XCopyPlane(_dpy, (Drawable)s->id, (Drawable)d->id, g,
                sx, sy, Dx(r), Dy(r), dx, dy, plane);
        else {
            /*
             * CopyPlane can only do func code S,
             * so copy src rect into a bitmap with the same depth
             * as the dest, then do the bitblt from the tmp.
             * This won't recurse again because we only get
             * UseCopyPlane with differing bitmap depths
             */
            btmp = _balloc(Rect(0,0,Dx(r),Dy(r)), d->ldepth);
            XCopyPlane(_dpy, (Drawable)s->id, (Drawable)btmp->id, g,
                sx, sy, Dx(r), Dy(r), 0, 0, plane);
            bitblt(d, p, btmp, btmp->r, f);
            bfree(btmp);
        }
    }
    XFlush(_dpy);
}
