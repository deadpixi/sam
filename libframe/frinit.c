/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include <frame.h>

int tabwidth = 8;
extern bool expandtabs;

void
frinit(Frame *f, Rectangle r, XftFont *ft, Bitmap *b, uint64_t bg)
{
    int tabs = atoi(getenv("TABS") ? getenv("TABS") : "");
    if (tabs < 0){
        tabs = -tabs;
        expandtabs = true;
    }

    if (tabs > 0 && tabs <= 12)
        tabwidth = tabs;

    f->font = ft;
    /* ft->height is NOT CORRECT; we must use ascent + descent to
       clear the lowest edge of characters. - cks */
    f->fheight = ft->ascent + ft->descent;
    f->maxtab = tabwidth*charwidth(ft, '0');
    f->nbox = 0;
    f->nalloc = 0;
    f->nchars = 0;
    f->nlines = 0;
    f->p0 = 0;
    f->p1 = 0;
    f->box = 0;
    f->lastlinefull = 0;
    f->bg = bg;
    frsetrects(f, r, b);
}

void
frsetrects(Frame *f, Rectangle r, Bitmap *b)
{
    f->b = b;
    f->entire = r;
    f->r = r;
    f->r.max.y -= (r.max.y-r.min.y)%f->fheight;
    f->left = r.min.x+1;
    f->maxlines = (r.max.y-r.min.y)/f->fheight;
}

void
frclear(Frame *f)
{
    if(f->nbox)
        _frdelbox(f, 0, f->nbox-1);
    if(f->box)
        free(f->box);
    f->box = 0;
}
