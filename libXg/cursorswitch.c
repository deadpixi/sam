/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include "libgint.h"

#include <X11/cursorfont.h>

extern Window _topwindow;

static Cursor arrow;
static Cursor sweep;
static Cursor crosshair;
static Cursor pirate;
static Cursor watch;
static Cursor defcursor;

void
cursorswitch(unsigned int c)
{
    Cursor i = defcursor;

    switch (c){
        case ArrowCursor:    i = arrow;     break;
        case SweepCursor:    i = sweep;     break;
        case BullseyeCursor: i = crosshair; break;
        case DeadCursor:     i = pirate;    break;
        case LockCursor:     i = watch;     break;
        default:             i = defcursor; break;
    }

    XDefineCursor(_dpy, _topwindow, i);
}

void
initcursors(void)
{
    sweep = XCreateFontCursor(_dpy, XC_sizing);
    crosshair = XCreateFontCursor(_dpy, XC_crosshair);
    pirate = XCreateFontCursor(_dpy, XC_pirate);
    watch = XCreateFontCursor(_dpy, XC_watch);
    arrow = XCreateFontCursor(_dpy, XC_left_ptr);
    defcursor = XCreateFontCursor(_dpy, XC_xterm);
}

