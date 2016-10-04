/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include "libgint.h"

int
clipr(Bitmap *d, Rectangle r)
{
    if(rectclip(&r, d->r) == 0)
        return 0;
    d->clipr = r;
    if(r.min.x != d->r.min.x ||
       r.min.y != d->r.min.y ||
       r.max.x != d->r.max.x ||
       r.max.y != d->r.max.y)
        d->flag |= CLIP;
    else
        d->flag &= ~CLIP;
    return 1;
}
