/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include "libgint.h"
#include <X11/Intrinsic.h>

#include <stdio.h>
void
wrbitmap(Bitmap *b, int miny, int maxy, unsigned char *data)
{
    XImage *im;
    int w, h, inld, outld, l, offset, px;
    GC g;
    char *tdata;

    w = Dx(b->r);
    h = maxy - miny;
    inld = b->ldepth;
    outld = (b->ldepth == 0) ? 0 : screen.ldepth;
    px = 1<<(3-outld);  /* pixels per byte */
    /* set l to number of bytes of data per scan line */
    if(b->r.min.x >= 0)
        offset = b->r.min.x % px;
    else
        offset = px - b->r.min.x % px;
    l = (-b->r.min.x+px-1)/px;
    if(b->r.max.x >= 0)
        l += (b->r.max.x+px-1)/px;
    else
        l -= b->r.max.x/px;
    l *= h;

    tdata = (char *)malloc(l);
    if (tdata == (char *) 0)
            berror("wrbitmap malloc");
    if (inld == outld)
        memcpy((void*)tdata, (void*)data, l);
    else
        _ldconvert((char*)data, inld, tdata, outld, w, h);

    im = XCreateImage(_dpy, 0, 1 << outld, ZPixmap, 0, tdata, w, h, 8, 0);

    /* Botched interface to XCreateImage doesn't let you set these: */
    im->bitmap_bit_order = MSBFirst;
    im->byte_order = MSBFirst;

    g = _getfillgc(S, b, ~0);
    XSetBackground(_dpy, g, b->flag&DP1 ? 0 : _bgpixel);
    XPutImage(_dpy, (Drawable)b->id, g, im, offset, 0, 0, miny - b->r.min.y, w-offset, h);
    XDestroyImage(im);
}
