/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <libg.h>
#include "libgint.h"

void
polysegment(Bitmap *d, int n, Point *pp, int v, Fcode f)
{
	XPoint *xp;
	int i;
	GC g;

	if (!(xp = (XPoint *)calloc(n, sizeof(XPoint))))
		berror("polysegment: could not allocate XPoints");
	for (i = 0; i < n; i++, pp++)
		if(d->flag&SHIFT){
			xp[i].x = pp->x - d->r.min.x;
			xp[i].y = pp->y - d->r.min.y;
		} else {
			xp[i].x = pp->x;
			xp[i].y = pp->y;
		}
	g = _getfillgc(f, d, v);
	XDrawLines(_dpy, (Drawable)d->id, g, xp, n, CoordModeOrigin);
	free(xp);
}
