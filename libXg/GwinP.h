/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#ifndef GWINP_H
#define GWINP_H

#include "Gwin.h"

/* Gwin is derived from Core */

/* Gwin instance part */
typedef struct {
    /* New resource fields */
    Pixel       foreground;
    Boolean     forwardr;   /* does right button scroll forward? */
    Reshapefunc reshaped;   /* Notify app of reshape */
    Charfunc    gotchar;    /* Notify app of char arrival */
    Mousefunc   gotmouse;   /* Notify app of mouse change */
    String      selection;  /* Current selection */
    Boolean     selxfered;  /* Has selection been swapped. */
    int     compose;
} GwinPart;

/* Full instance record */
typedef struct _GwinRec {
    CorePart    core;
    GwinPart    gwin;
} GwinRec;

/* New type for class methods */
typedef String (*SelSwapProc)(Widget, String);

/* Class part */
typedef struct {
    SelSwapProc select_swap;
    XtPointer   extension;
} GwinClassPart;

/* Full class record */
typedef struct _GwinClassRec {
    CoreClassPart   core_class;
    GwinClassPart   gwin_class;
} GwinClassRec, *GwinWidgetClass;

/* External definition for class record */
extern GwinClassRec gwinClassRec;

#endif /* GWINP_H */
