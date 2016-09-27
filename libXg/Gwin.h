/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#ifndef GWIN_H
#define GWIN_H

/* New resource names */

#define XtNscrollForwardR "scrollForwardR"
#define XtCScrollForwardR "ScrollForwardR"
#define XtNreshaped "reshaped"
#define XtCReshaped "Reshaped"
#define XtNgotchar "gotchar"
#define XtCGotchar "Gotchar"
#define XtNgotmouse "gotmouse"
#define XtCGotmouse "Gotmouse"
#define XtNp9font   "p9font"
#define XtCP9font   "P9font"
#define XtNcomposeMod   "composeMod"
#define XtCComposeMod   "ComposeMod"

/* External reference to the class record pointer */
extern WidgetClass gwinWidgetClass;

/* Type definition for gwin widgets */
typedef struct _GwinRec *GwinWidget;

/* Type definition for gwin resources */
typedef struct {
        int buttons;
        struct {
            int x;
            int y;
        } xy;
        uint64_t msec;
    } Gwinmouse;

typedef void (*Reshapefunc)(int, int, int, int);
typedef void (*Charfunc)(int, int, int, int, int, const char *);
typedef void (*Mousefunc)(Gwinmouse*);

/* Method declarations */
extern String GwinSelectionSwap(Widget, String);

#endif /* GWIN_H */
