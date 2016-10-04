/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include "libgint.h"

/*
 * The screen data structure should always be up to date
 * (Not true in the Plan 9 library, which is why this
 * function exists).
 */
Rectangle
bscreenrect(Rectangle *clipr)
{
    if(clipr)
        *clipr = screen.clipr;
    return screen.r;
}
