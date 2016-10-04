/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include "libgint.h"

Bitmap*
balloc(Rectangle r, int ldepth)
{
    Bitmap *b;

    b = _balloc(r, ldepth);
    bitblt(b, r.min, b, r, Zero);
    return b;
}

Bitmap*
_balloc(Rectangle r, int ldepth)
{
    int id;
    Bitmap *b;
    int ld;
    Rectangle rx;

    b = (Bitmap *)calloc(1, sizeof(Bitmap));
    if(b == 0)
        berror("balloc malloc");
    if (ldepth == 0)
        ld = 0;
    else
        ld = screen.ldepth;
    rx = r;
    if (Dx(rx) == 0)
        rx.max.x++;
    if (Dy(rx) == 0)
        rx.max.y++;
    id = (int) XCreatePixmap(_dpy, (Drawable)screen.id,
            Dx(rx), Dy(rx), _ld2d[ld]);
    b->ldepth = ldepth;
    b->r = r;
    b->clipr = r;
    b->id = id;
    b->cache = 0;
    if(ldepth == 0)
        b->flag = DP1|BL1;
    else
        b->flag = screen.flag&BL1;
    if(r.min.x==0 && r.min.y ==0)
        b->flag |= ZORG;
    else
        b->flag |= SHIFT;
    return b;
}

void
bfree(Bitmap *b)
{
    if (b->fd)
        XftDrawDestroy(b->fd);
    XFreePixmap(_dpy, (Pixmap)b->id);
    free(b);
}
