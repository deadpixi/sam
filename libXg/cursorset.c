/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include "libgint.h"

/*
 * Only allow cursor to move within screen Bitmap
 */
void
cursorset(Point p)
{
    /* motion will be relative to window origin */
    p = sub(p, screen.r.min);
    XWarpPointer(_dpy, None, (Window)screen.id, 0, 0, 0, 0, p.x, p.y);
}
