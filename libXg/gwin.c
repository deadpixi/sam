/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#ifndef XtSpecificationRelease
#define R3
#define XtPointer caddr_t
#define XtOffsetOf(s_type,field) XtOffset(s_type*,field)
#define XtExposeCompressMultiple TRUE
#endif

#include "GwinP.h"
#include "../config.h"
#include <commands.h>

/* Forward declarations */
static void Realize(Widget, XtValueMask *, XSetWindowAttributes *);
static void Resize(Widget);
static void Redraw(Widget, XEvent *, Region);
static void Mappingaction(Widget, XEvent *, String *, Cardinal*);
static void Keyaction(Widget, XEvent *, String *, Cardinal*);
static void Mouseaction(Widget, XEvent *, String *, Cardinal*);
static String SelectSwap(Widget, String);

/* Data */

#define Offset(field) XtOffsetOf(GwinRec, gwin.field)

static XtResource resources[] = {
    {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
        Offset(foreground), XtRString, (XtPointer)XtDefaultForeground},
    {XtNscrollForwardR, XtCScrollForwardR, XtRBoolean, sizeof(Boolean),
        Offset(forwardr), XtRImmediate, (XtPointer)TRUE},
    {XtNreshaped, XtCReshaped, XtRFunction, sizeof(Reshapefunc),
        Offset(reshaped), XtRFunction, (XtPointer) NULL},
    {XtNgotchar, XtCGotchar, XtRFunction, sizeof(Charfunc),
        Offset(gotchar), XtRFunction, (XtPointer) NULL},
    {XtNgotmouse, XtCGotmouse, XtRFunction, sizeof(Mousefunc),
        Offset(gotmouse), XtRFunction, (XtPointer) NULL},
    {XtNselection, XtCSelection, XtRString, sizeof(String),
        Offset(selection), XtRString, (XtPointer) NULL},
    {XtNcomposeMod, XtCComposeMod, XtRInt, sizeof(int),
        Offset(compose), XtRImmediate, (XtPointer) 0}
};
#undef Offset

static XtActionsRec actions[] = {
    {"key", Keyaction},
    {"mouse", Mouseaction},
    {"mapping", Mappingaction}
};

static char tms[] =
    "<Key> : key() \n\
    <Motion> : mouse() \n\
    <BtnDown> : mouse() \n\
    <BtnUp> : mouse() \n\
    <Mapping> : mapping() \n";

/* Class record declaration */

GwinClassRec gwinClassRec = {
  /* Core class part */
   {
    /* superclass         */    (WidgetClass)&widgetClassRec,
    /* class_name         */    "Gwin",
    /* widget_size        */    sizeof(GwinRec),
    /* class_initialize   */    NULL,
    /* class_part_initialize*/  NULL,
    /* class_inited       */    FALSE,
    /* initialize         */    NULL,
    /* initialize_hook    */    NULL,
    /* realize            */    Realize,
    /* actions            */    actions,
    /* num_actions        */    XtNumber(actions),
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    TRUE,
    /* compress_exposure  */    XtExposeCompressMultiple,
    /* compress_enterleave*/    TRUE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    Resize,
    /* expose             */    Redraw,
    /* set_values         */    NULL,
    /* set_values_hook    */    NULL,
    /* set_values_almost  */    XtInheritSetValuesAlmost,
    /* get_values_hook    */    NULL,
    /* accept_focus       */    XtInheritAcceptFocus,
    /* version            */    XtVersion,
    /* callback_offsets   */    NULL,
    /* tm_table           */    tms,
    /* query_geometry       */  XtInheritQueryGeometry,
    /* display_accelerator  */  NULL,
    /* extension            */  NULL
   },
  /* Gwin class part */
   {
    /* select_swap    */    SelectSwap,
   }
};

/* Class record pointer */
WidgetClass gwinWidgetClass = (WidgetClass) &gwinClassRec;

static XModifierKeymap *modmap;
static int keypermod;

static void
Realize(Widget w, XtValueMask *valueMask, XSetWindowAttributes *attrs)
{
    *valueMask |= CWBackingStore;
    attrs->backing_store = Always;

    XtCreateWindow(w, InputOutput, (Visual *)0, *valueMask, attrs);
    XtSetKeyboardFocus(w->core.parent, w);
    if ((modmap = XGetModifierMapping(XtDisplay(w))))
        keypermod = modmap->max_keypermod;

    Resize(w);
}

static void
Resize(Widget w)
{
    if(XtIsRealized(w))
        (*(XtClass(w)->core_class.expose))(w, (XEvent *)NULL, (Region)NULL);
}

static void
Redraw(Widget w, XEvent *e, Region r)
{
    Reshapefunc f;

    f = ((GwinWidget)w)->gwin.reshaped;
    if(f)
        (*f)(w->core.x, w->core.y,
            w->core.x+w->core.width, w->core.y+w->core.height);
}

static void
Mappingaction(Widget w, XEvent *e, String *p, Cardinal *np)
{
    if (modmap)
        XFreeModifiermap(modmap);
    modmap = XGetModifierMapping(e->xany.display);
    if (modmap)
        keypermod = modmap->max_keypermod;
}

#define STUFFCOMPOSE() \
                f = ((GwinWidget)w)->gwin.gotchar; \
                if (f) \
                    for (c = 0; c < composing; c++) \
                        (*f)(compose[c], 0, Tcurrent, 0, 0)

typedef struct Unikeysym Unikeysym;
struct Unikeysym{
    KeySym keysym;
    unsigned short value;
};

Unikeysym unikeysyms[] ={
    #include "unikeysyms.h"
    {0, 0}
};

unsigned short
keysymtoshort(KeySym k)
{
    for (Unikeysym *ks = unikeysyms; ks->keysym != 0; ks++){
        if (k == ks->keysym)
            return ks->value;
    }

    return k;
}

typedef struct Keymapping Keymapping;
struct Keymapping{
    Keymapping *next;
    int m;
    KeySym s;
    int k;
    int c;
};

static Keymapping *keymappings = NULL;

int
installbinding(int m, KeySym s, int k, int c)
{
    if (m < 0 || s == NoSymbol || k < 0 || c < 0)
        return -1;

    Keymapping *km = calloc(1, sizeof(Keymapping));
    if (!km)
        return -1;

    km->m = m;
    km->s = s;
    km->k = k;
    km->c = c;

    km->next = keymappings;
    keymappings = km;

    return 0;
}

void
freebindings(void)
{
    Keymapping *m = keymappings;
    while (m){
        Keymapping *n = m->next;
        free(m);
        m = n;
    }
}

static void
Keyaction(Widget w, XEvent *e, String *p, Cardinal *np)
{
    static unsigned char compose[5];
    static int composing = -2;
    int kind = Kraw;

    int c, minmod;
    KeySym k, mk;
    Charfunc f;
    Modifiers md;

    c = 0;

    /*
     * I tried using XtGetActionKeysym, but it didn't seem to
     * do case conversion properly
     * (at least, with Xterminal servers and R4 intrinsics)
     */
    if(e->xany.type != KeyPress)
        return;
    XtTranslateKeycode(e->xany.display, (KeyCode)e->xkey.keycode, e->xkey.state, &md, &k);

    /* Check to see if it's a specially-handled key first. */
    for (Keymapping *m = keymappings; m; m = m->next){
        if (k == m->s){
            if (m->m == 0 || (m->m & ~e->xkey.state) == 0){
                f = ((GwinWidget)w)->gwin.gotchar;
                if (f)
                    (*f)(m->c, m->k, Tcurrent, 0, 0);
                return;
            }
        }
    }

    /*
     * The following song and dance is so we can have our chosen
     * modifier key behave like a compose key, i.e, press and release
     * and then type the compose sequence, like Plan 9.  We have
     * to find out which key is the compose key first though.
     */
    if (IsModifierKey(k) && ((GwinWidget)w)->gwin.compose
            && composing == -2 && modmap) {
        minmod = (((GwinWidget)w)->gwin.compose+2)*keypermod;
        for (c = minmod; c < minmod+keypermod; c++) {
            XtTranslateKeycode(e->xany.display,
                    modmap->modifiermap[c],
                        e->xkey.state, &md, &mk);
            if (k == mk) {
                composing = -1;
                break;
            }
        }
        return;
    }

    /* Handle Multi_key separately, since it isn't a modifier */
    if(k == XK_Multi_key) {
        composing = -1;
        return;
    }

    if(k == NoSymbol || k > 0xff00)
        return;

    /* Check to see if we are in a composition sequence */
    if (!((GwinWidget)w)->gwin.compose && (e->xkey.state & Mod1Mask)
            && composing == -2)
        composing = -1;
    if (composing > -2) {
        compose[++composing] = k;
        if ((*compose == 'X') && (composing > 0)) {
            if ((k < '0') || (k > 'f') ||
                    ((k > '9') && (k < 'a'))) {
                STUFFCOMPOSE();
                c = (unsigned short)k;
                composing = -2;
            } else if (composing == 4) {
                c = unicode(compose);
                if (c == -1) {
                    STUFFCOMPOSE();
                    c = (unsigned short)compose[4];
                }
                composing = -2;
            }
        } else if (composing == 1) {
            c = (int)latin1(compose);
            if (c == -1) {
                STUFFCOMPOSE();
                c = (unsigned short)compose[1];
            }
            composing = -2;
        }
    } else {
        if (composing >= 0) {
            composing++;
            STUFFCOMPOSE();
        }
        c = keysymtoshort(k);
        composing = -2;
    }

    if (composing >= -1)
        return;

    f = ((GwinWidget)w)->gwin.gotchar;
    if(f)
        (*f)(c, kind, Tcurrent, 0, 0);
}

typedef struct Chordmapping Chordmapping;
struct Chordmapping{
    Chordmapping *next;
    int s1;
    int s2;
    int c;
    int t;
};

static Chordmapping *chordmap = NULL;

int
installchord(int s1, int s2, int c, int t)
{
    if (s1 < 0 || s2 < 0 || c < 0 || (t != Tmouse && t != Tcurrent))
        return -1;

    Chordmapping *m = calloc(1, sizeof(Chordmapping));
    if (!m)
        return -1;

    m->s1 = s1;
    m->s2 = s2;
    m->c = c;
    m->t = t;

    m->next = chordmap;
    chordmap = m;
    return 0;
}

void
freechords(void)
{
    Chordmapping *m = chordmap;
    while (m){
        Chordmapping *n = m->next;
        free(m);
        m = n;
    }
}

static void
Mouseaction(Widget w, XEvent *e, String *p, Cardinal *np)
{
    int s = 0;
    int ps = 0; /* the previous state */
    int ob = 0;

    XButtonEvent *be = (XButtonEvent *)e;
    XMotionEvent *me = (XMotionEvent *)e;
    Gwinmouse m;
    Mousefunc f;

    switch(e->type){
    case ButtonPress:
        m.xy.x = be->x;
        m.xy.y = be->y;
        m.msec = be->time;
        ps = s = be->state;
        switch(be->button){
        case 1: s |= Button1Mask; break;
        case 2: s |= Button2Mask; break;
        case 3: s |= Button3Mask; break;
        case 4: s |= Button4Mask; break;
        case 5: s |= Button5Mask; break;
        }
        break;
    case ButtonRelease:
        m.xy.x = be->x;
        m.xy.y = be->y;
        m.msec = be->time;
        ps = s = be->state;
        switch(be->button){
        case 1: s &= ~Button1Mask; break;
        case 2: s &= ~Button2Mask; break;
        case 3: s &= ~Button3Mask; break;
        case 4: s &= ~Button4Mask; break;
        case 5: s &= ~Button5Mask; break;
        }
        break;
    case MotionNotify:
        ps = s = me->state;
        m.xy.x = me->x;
        m.xy.y = me->y;
        m.msec = me->time;
        break;
    default:
        return;
    }

    m.buttons = 0;

    if(ps & Button1Mask) ob |= 1;
    if(ps & Button2Mask) ob |= 2;
    if(ps & Button3Mask) ob |= (s & ShiftMask) ? 2 : 4;
    if(ps & Button4Mask) ob |= 8;
    if(ps & Button5Mask) ob |= 16;

    if(s & Button1Mask) m.buttons |= 1;
    if(s & Button2Mask) m.buttons |= 2;
    if(s & Button3Mask) m.buttons |= (s & ShiftMask) ? 2 : 4;
    if(s & Button4Mask) m.buttons |= 8;
    if(s & Button5Mask) m.buttons |= 16;

    /* Check to see if it's a chord first. */
    for (Chordmapping *cm = chordmap; cm; cm = cm->next){
        if (ob == cm->s1 && m.buttons == cm->s2){
            Charfunc kf = ((GwinWidget)w)->gwin.gotchar;
            if (kf)
                (*kf)(cm->c, Kcommand, cm->t, m.xy.x, m.xy.y);

            m.buttons = 0;
        }
    }

    f = ((GwinWidget)w)->gwin.gotmouse;
    if(f)
        (*f)(&m);
}

static void
SelCallback(Widget w, XtPointer cldata, Atom *sel, Atom *seltype,
    XtPointer val, unsigned long *len, int *fmt)
{
    String s;
    int n;
    GwinWidget gw = (GwinWidget)w;

    if(gw->gwin.selection)
        XtFree(gw->gwin.selection);
    if(*seltype != XA_STRING)
        n = 0;
    else
        n = (*len) * (*fmt/8);
    s = (String)XtMalloc(n+1);
    if(n > 0)
        memcpy(s, (char *)val, n);
    s[n] = 0;
    gw->gwin.selection = s;
    XtFree(val);
}

static Boolean
SendSel(Widget w, Atom *sel, Atom *target, Atom *rtype, XtPointer *ans,
        unsigned long *anslen, int *ansfmt)
{
    GwinWidget gw = (GwinWidget)w;
    char *s;

    if(*target == XA_STRING){
        s = gw->gwin.selection;
        if(!s)
            s = "";
        *rtype = XA_STRING;
        *ans = (XtPointer) XtNewString(s);
        *anslen = strlen(*ans);
        *ansfmt = 8;
        return TRUE;
    }

    return FALSE;
}

static String
SelectSwap(Widget w, String s)
{
    GwinWidget gw;
    String ans;

    gw = (GwinWidget)w;
    if(gw->gwin.selection){
        XtFree(gw->gwin.selection);
        gw->gwin.selection = 0;
    }
    XtGetSelectionValue(w, XA_PRIMARY, XA_STRING, SelCallback, 0,
            XtLastTimestampProcessed(XtDisplay(w)));

    while(gw->gwin.selection == 0)
        XtAppProcessEvent(XtWidgetToApplicationContext(w) , XtIMAll);
    ans = gw->gwin.selection;
    gw->gwin.selection = XtMalloc(strlen(s)+1);
    strcpy(gw->gwin.selection, s);

    XtOwnSelection(w, XA_PRIMARY, XtLastTimestampProcessed(XtDisplay(w)),
            SendSel, NULL, NULL);

    return ans;
}

/* The returned answer should be free()ed when no longer needed */
String
GwinSelectionSwap(Widget w, String s)
{
    XtCheckSubclass(w, gwinWidgetClass, NULL);
    return (*((GwinWidgetClass) XtClass(w))->gwin_class.select_swap)(w, s);
}
