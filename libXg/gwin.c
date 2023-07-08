/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

#include "GwinP.h"
#include "libgint.h"

const char *clipatom = "PRIMARY";

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
        Offset(forwardr), XtRImmediate, (XtPointer)true},
    {XtNreshaped, XtCReshaped, XtRFunction, sizeof(Reshapefunc),
        Offset(reshaped), XtRFunction, (XtPointer) NULL},
    {XtNgotchar, XtCGotchar, XtRFunction, sizeof(Charfunc),
        Offset(gotchar), XtRFunction, (XtPointer) NULL},
    {XtNgotmouse, XtCGotmouse, XtRFunction, sizeof(Mousefunc),
        Offset(gotmouse), XtRFunction, (XtPointer) NULL},
    {XtNselection, XtCSelection, XtRString, sizeof(String),
        Offset(selection), XtRString, (XtPointer) NULL},
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
    /* class_inited       */    false,
    /* initialize         */    NULL,
    /* initialize_hook    */    NULL,
    /* realize            */    Realize,
    /* actions            */    actions,
    /* num_actions        */    XtNumber(actions),
    /* resources          */    resources,
    /* num_resources      */    XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    true,
    /* compress_exposure  */    XtExposeCompressMultiple,
    /* compress_enterleave*/    true,
    /* visible_interest   */    false,
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
extern XIC xic;
extern XIM xim;

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

    xic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                    XNClientWindow, XtWindow(w), XNFocusWindow, XtWindow(w), NULL);
    if (!xic){
        fprintf(stderr, "could not create input context\n");
        exit(EXIT_FAILURE);
    }
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

typedef struct Unikeysym Unikeysym;
struct Unikeysym{
    KeySym keysym;
    uint16_t value;
};

Unikeysym unikeysyms[] ={
    #include "unikeysyms.h"
    {0, 0}
};

uint16_t
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
    char a[];
};

static Keymapping *keymappings = NULL;

int
installbinding(int m, KeySym s, int k, int c, const char *a)
{
    if (m < 0 || s == NoSymbol || k < 0 || c < 0)
        return -1;

    a = a ? a : "";
    Keymapping *km = calloc(1, sizeof(Keymapping) + strlen(a) + 1);
    if (!km)
        return -1;

    km->m = m;
    km->s = s;
    km->k = k;
    km->c = c;
    strcpy(km->a, a);
    km->next = keymappings;
    keymappings = km;

    return 0;
}

int
removebinding(int m, KeySym s)
{
    if (m < 0 || s == NoSymbol)
        return -1;

    for (Keymapping *km = keymappings; km; km = km->next){
        if (km->m == m && km->s == s)
            km->c = Cdefault;
    }

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
    extern XIC xic;
    int kind = Kraw;

    int c, len, minmod;
    KeySym k, mk;
    Charfunc f;
    Modifiers md;
    Status s;
    wchar_t buf[32] = {0};

    c = 0;
    len = 0;

    /* Translate the keycode into a key symbol. */
    if(e->xany.type != KeyPress)
        return;

    len = XwcLookupString(xic, &e->xkey, buf, 32, &k, &s);
    if (IsModifierKey(k))
        return;

    /* Check to see if it's a specially-handled key first. */
    for (Keymapping *m = keymappings; m; m = m->next){
        KeySym u = NoSymbol;
        KeySym l = NoSymbol;
        XConvertCase(k, &l, &u);

        /* Note that magic bit manipulation here - we want to check that the
         * modifiers that are specified for the binding are all pressed, but
         * we allow other modifiers to be as well. This is because when NumLock
         * is on, it's always added to the modifier mask.
         */
        if (l == m->s || m->s == XK_VoidSymbol){
            if (m->m == 0 || (m->m & ~e->xkey.state) == 0){
                switch (m->c){
                    case Cnone:
                        return;

                    case Cdefault:
                        continue;

                    default:
                        f = ((GwinWidget)w)->gwin.gotchar;
                        if (f)
                            (*f)(m->c, m->k, Tcurrent, 0, 0, m->a);
                        return;
                }
            }
        }
    }

    c = keysymtoshort(k);
    f = ((GwinWidget)w)->gwin.gotchar;
    if(f && c)
        (*f)(c? c : buf[0], kind, Tcurrent, 0, 0, NULL);
}

typedef struct Chordmapping Chordmapping;
struct Chordmapping{
    Chordmapping *next;
    int s1;
    int s2;
    int c;
    int t;
    const char *a;
};

static Chordmapping *chordmap = NULL;

int
installchord(int s1, int s2, int c, int t, const char *a)
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
    m->a = a;

    m->next = chordmap;
    chordmap = m;
    return 0;
}

int
removechord(int s1, int s2)
{
    if (s1 < 0 || s2 < 0)
        return -1;

    for (Chordmapping *m = chordmap; m; m = m->next){
        if (m->s1 == s1 && m->s2 == s2)
            m->c = Cdefault;
    }

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
    static bool chording = false;
    Charfunc kf;

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

    if (!m.buttons)
        chording = false;

    /* Check to see if it's a chord first. */
    for (Chordmapping *cm = chordmap; cm; cm = cm->next){
        if (ob == cm->s1 && m.buttons == cm->s2){
            switch (cm->c){
                case Cdefault:
                    continue;

                case Cnone:
                    break;

                default:
                    kf = ((GwinWidget)w)->gwin.gotchar;
                    if (kf)
                        (*kf)(cm->c, Kcommand, cm->t, m.xy.x, m.xy.y, NULL);
        
                    m.buttons = 0;
                    chording = true;
                    break;
            }
        }
    }

    if (chording)
        m.buttons = 0;

    f = ((GwinWidget)w)->gwin.gotmouse;
    if(f)
        (*f)(&m);
}

static void
SelCallback(Widget w, XtPointer cldata, Atom *sel, Atom *seltype,
    XtPointer val, unsigned long *len, int *fmt)
{
    GwinWidget gw = (GwinWidget)w;
    XTextProperty p = {0};
    char *ls[2] = {(char *)val, NULL};

    gw->gwin.selxfered = true;
    if (*seltype == 0){
        if (gw->gwin.selection == NULL)
            gw->gwin.selection = strdup("");
        return;
    }

    if(gw->gwin.selection){
        XtFree(gw->gwin.selection);
        gw->gwin.selection = NULL;
    }

    if(*seltype != XInternAtom(_dpy, "UTF8_STRING", 0))
        return;

    if (XmbTextListToTextProperty(_dpy, ls, 1, XUTF8StringStyle, &p) != Success)
        return;

    gw->gwin.selection = strdup(p.value);
    XtFree(val);
    XFree(p.value);
}

static Boolean
SendSel(Widget w, Atom *sel, Atom *target, Atom *rtype, XtPointer *ans,
        unsigned long *anslen, int *ansfmt)
{
    GwinWidget gw = (GwinWidget)w;
    XTextProperty p = {0};
    char *ls[2] = {NULL, NULL};

    if ((*target == XA_STRING) ||
        (*target == XInternAtom(_dpy, "UTF8_STRING", 0))){
        ls[0] = gw->gwin.selection? gw->gwin.selection : "";
        if (XmbTextListToTextProperty(_dpy, ls, 1, XUTF8StringStyle, &p) != Success)
            return false;

        *rtype = p.encoding;
        *ans = (XtPointer) XtNewString(p.value);
        *anslen = p.nitems;
        *ansfmt = p.format;
        XFree(p.value);
        return true;
    }

    return false;
}

static String
SelectSwap(Widget w, String s)
{
    GwinWidget gw;
    String ans;

    gw = (GwinWidget)w;
    gw->gwin.selxfered = false;
    XtGetSelectionValue(w, XInternAtom(_dpy, clipatom, 0), XInternAtom(_dpy, "UTF8_STRING", 0), SelCallback, 0,
            XtLastTimestampProcessed(XtDisplay(w)));

    while(gw->gwin.selxfered == false)
        XtAppProcessEvent(XtWidgetToApplicationContext(w) , XtIMAll);
    ans = gw->gwin.selection;
    gw->gwin.selection = XtMalloc(strlen(s)+1);
    strcpy(gw->gwin.selection, s);

    XtOwnSelection(w, XInternAtom(_dpy, clipatom, 0), XtLastTimestampProcessed(XtDisplay(w)),
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
