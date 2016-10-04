/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include "libgint.h"

static void
grabcursor(void)
{
    raisewindow();

    /* Grab X server with an limp wrist. */
    while (XGrabPointer(_dpy, screen.id, False,
            ButtonPressMask|ButtonReleaseMask|
            ButtonMotionMask|StructureNotifyMask,
        GrabModeAsync, GrabModeAsync, None, None, CurrentTime)
            != GrabSuccess)
        sleep(2);

    /* Grab the keyboard too */
    XSetInputFocus(_dpy, screen.id, RevertToParent, CurrentTime);
}

static void
ungrabcursor(void)
{
    XUngrabPointer(_dpy, CurrentTime);
}

Rectangle
getrect(int but, Mouse *m){
    Rectangle r, rc;

    but = 1<<(but-1);
    cursorswitch(SweepCursor);
    while(m->buttons)
        *m = emouse();
    grabcursor();
    while(!(m->buttons & but)){
        *m = emouse();
        if(m->buttons & (7^but))
            goto Return;
    }
    r.min = m->xy;
    r.max = m->xy;
    do{
        rc = rcanon(r);
        border(&screen, rc, 2, F&~D, _bgpixel);
        *m = emouse();
        border(&screen, rc, 2, F&~D, _bgpixel);
        r.max = m->xy;
    }while(m->buttons & but);

    Return:
    cursorswitch(DefaultCursor);
    if(m->buttons & (7^but)){
        rc.min.x = rc.max.x = 0;
        rc.min.y = rc.max.y = 0;
        while(m->buttons)
            *m = emouse();
    }
    ungrabcursor();
    return rc;
}
