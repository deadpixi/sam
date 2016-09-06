/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <libg.h>
#include "libgint.h"

void
rdbitmap(Bitmap *b, int miny, int maxy, unsigned char *data)
{
    XImage *gim, *eim;
    int x, y, w, h, pix, l, offset, px;
    int inld, outld;
    char *tdata;

    /*
     * The XGetImage returned image may be wrong in a number of ways:
     * wrong bit order, byte order, bit pad, scanline pad,
     * and constant shift.
     * So use a SLOW loop, for now
     */
    w = Dx(b->r);
    h = maxy - miny;
    outld = b->ldepth;
    inld = (b->ldepth == 0) ? 0 : screen.ldepth;
    gim = XGetImage(_dpy, (Drawable)b->id, 0, miny - b->r.min.y,
            w, h, ~0, ZPixmap);
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
    if(l <= 0)
        return;
    tdata = (char *)malloc(l);
    if (tdata == (char *) 0)
        berror("rdbitmap malloc");
    eim = XCreateImage(_dpy, 0, 1 << inld, ZPixmap, 0, tdata,
            w+offset, h, 8, 0);
    eim->bitmap_pad = 8;
    eim->bitmap_bit_order = MSBFirst;
    eim->byte_order = MSBFirst;

    for(y = 0; y < h; y++)
        for(x = 0; x < w; x++) {
            pix = XGetPixel(gim, x, y);
            XPutPixel(eim, x+offset, y, pix);
        }

    if (inld == outld)
        memcpy((char *)data, tdata, l);
    else
        _ldconvert(tdata, inld, (char*)data, outld, w, h);

    XDestroyImage(gim);
    XDestroyImage(eim);
}
