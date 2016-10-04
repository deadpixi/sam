/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>

extern uint64_t _borderpixel;

void
border(Bitmap *l, Rectangle r, int i, Fcode c, uint64_t bg)
{
    if(i > 0){
        bitblt2(l, r.min,
            l, Rect(r.min.x, r.min.y, r.max.x, r.min.y+i), c, _borderpixel, bg);
        bitblt2(l, Pt(r.min.x, r.max.y-i),
            l, Rect(r.min.x, r.max.y-i, r.max.x, r.max.y), c, _borderpixel, bg);
        bitblt2(l, Pt(r.min.x, r.min.y+i),
            l, Rect(r.min.x, r.min.y+i, r.min.x+i, r.max.y-i), c, _borderpixel, bg);
        bitblt2(l, Pt(r.max.x-i, r.min.y+i),
            l, Rect(r.max.x-i, r.min.y+i, r.max.x, r.max.y-i), c, _borderpixel, bg);
    }else if(i < 0){
        bitblt2(l, Pt(r.min.x, r.min.y+i),
            l, Rect(r.min.x, r.min.y+i, r.max.x, r.min.y), c, _borderpixel, bg);
        bitblt2(l, Pt(r.min.x, r.max.y),
            l, Rect(r.min.x, r.max.y, r.max.x, r.max.y-i), c, _borderpixel, bg);
        bitblt2(l, Pt(r.min.x+i, r.min.y+i),
            l, Rect(r.min.x+i, r.min.y+i, r.min.x, r.max.y-i), c, _borderpixel, bg);
        bitblt2(l, Pt(r.max.x, r.min.y+i),
            l, Rect(r.max.x, r.min.y+i, r.max.x-i, r.max.y-i), c, _borderpixel, bg);
    }
}
