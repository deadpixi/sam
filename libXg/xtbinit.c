/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "libgint.h"

#define Font xFont
#define Event xEvent

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XInput.h>
#include "Gwin.h"

#undef Font
#undef Event

/* default colors */
#ifndef MAX_BACKGROUNDS
    #define MAX_BACKGROUNDS 20
#endif

#ifndef DEFAULT_FOREGROUND
    #define DEFAULT_FOREGROUND "#000000"
#endif

#ifndef DEFAULT_BACKGROUND
    #define DEFAULT_BACKGROUND "#ffffff"
#endif

#ifndef DEFAULT_BORDER
    #define DEFAULT_BORDER "#000000"
#endif

/* libg globals */
XIM xim;
XIC xic;
Bitmap  screen;
XftFont *font;
XftColor fontcolor;
char fontspec[1024] = {0};
char foregroundspec[1024] = {0};
char backgroundspec[1024] = {0};
char borderspec[1024] = {0};

/* implementation globals */
extern char *machine;
Display     *_dpy;
Widget      _toplevel;
Window _topwindow;
uint64_t _bgpixels[MAX_BACKGROUNDS];
int _nbgs;
uint64_t   _fgpixel, _bgpixel, _borderpixel;
XColor      _fgcolor, _bgcolor, _bordercolor;
int     _ld2d[6] = { 1, 2, 4, 8, 16, 24 };
uint64_t   _ld2dmask[6] = { 0x1, 0x3, 0xF, 0xFF, 0xFFFF, 0x00FFFFFF };
Colormap    _libg_cmap;
int     _cmap_installed;

/* xbinit implementation globals */
static XtAppContext app;
static Widget widg;
static bool exposed = 0;
static Atom wm_take_focus;
static Mouse lastmouse;

typedef struct Ebuf {
    struct Ebuf *next;
    int     n;
    unsigned char   buf[4];
} Ebuf;

typedef struct Esrc {
    bool issocket;
    bool inuse;
    int size;
    int count;
    Ebuf    *head;
    Ebuf    *tail;
} Esrc;

#define MAXINPUT    1024        /* number of queued input events */
#define MAXSRC      10

static Esrc esrc[MAXSRC];
static int  nsrc;


static int einitcalled = 0;
static int Smouse = -1;
static int Skeyboard = -1;


static void reshaped(int, int, int, int);
static void gotchar(int, int, int, int, int, const char *);
static void gotmouse(Gwinmouse *);
static int  ilog2(int);

static Ebuf *ebread(Esrc *);
static Ebuf *ebadd(Esrc *, bool);
static void focinit(Widget);
static void wmproto(Widget, XEvent *, String *, Cardinal *);
static void waitevent(void);

static Errfunc  onerr;

String _fallbacks[] = {
    "*gwin.width: 400",
    "*gwin.height: 400",
    NULL
};

static char *shelltrans = 
    "<ClientMessage> WM_PROTOCOLS : WMProtocolAction()";
static XtActionsRec wmpactions[] = {
    {"WMProtocolAction", wmproto}
};

Bitmap  *darkgrey;

static uint8_t darkgreybits[] = {
    0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77,
    0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77,
    0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77,
    0xDD, 0xDD, 0x77, 0x77, 0xDD, 0xDD, 0x77, 0x77,
};

void
freefont(void)
{
    if (font)
        XftFontClose(_dpy, font);
}

void
xtbinit(Errfunc f, char *class, int *pargc, char **argv, char **fallbacks)
{
    int n;
    char *p;
    int compose;

    if(!class && argv[0]){
        p = strrchr(argv[0], '/');
        if(p)
            class = XtNewString(p+1);
        else
            class = XtNewString(argv[0]);
        if(class[0] >= 'a' && class[0] <= 'z')
            class[0] += 'A' - 'a';
    }
    onerr = f;
    if (!fallbacks)
        fallbacks = _fallbacks;

    char name[512] = {0};
    snprintf(name, sizeof(name) - 1, "samterm on %s", machine);
    Arg args[] ={
        {XtNinput, true},
        {XtNtitle, (XtArgVal)name},
        {XtNiconName, (XtArgVal)name},
    };
    _toplevel = XtAppInitialize(&app, class,
            NULL, 0,
            pargc, argv, fallbacks, args, XtNumber(args));

    n = 0;
    XtSetArg(args[n], XtNreshaped, reshaped);   n++;
    XtSetArg(args[n], XtNgotchar, gotchar);     n++;
    XtSetArg(args[n], XtNgotmouse, gotmouse);   n++;
    widg = XtCreateManagedWidget("gwin", gwinWidgetClass, _toplevel, args, n);

    _dpy = XtDisplay(widg);
    XAllocNamedColor(_dpy, DefaultColormap(_dpy, DefaultScreen(_dpy)),
                     foregroundspec[0] ? foregroundspec : getenv("FOREGROUND") ? getenv("FOREGROUND") : DEFAULT_FOREGROUND, &_fgcolor, &_fgcolor);
    XAllocNamedColor(_dpy, DefaultColormap(_dpy, DefaultScreen(_dpy)),
                     borderspec[0] ? borderspec : getenv("BORDER") ? getenv("BORDER") : DEFAULT_BORDER, &_bordercolor, &_bordercolor);
    char bgspec[1024] = {0};
    strncpy(bgspec, backgroundspec[0] ? backgroundspec : getenv("BACKGROUND") ? getenv("BACKGROUND") : DEFAULT_BACKGROUND, sizeof(bgspec) - 1);

    char *bgc = NULL;
    for (bgc = strtok(bgspec, ":"); bgc != NULL && _nbgs < MAX_BACKGROUNDS; bgc = strtok(NULL, ":")){
        XColor xc = {0};
        if (XAllocNamedColor(_dpy, DefaultColormap(_dpy, DefaultScreen(_dpy)), bgc, &xc, &xc))
            _bgpixels[_nbgs++] = xc.pixel;
    }

    if (_nbgs == 0)
        _bgpixels[_nbgs++] = ~_fgcolor.pixel;

    _bgpixel = _bgpixels[0];

    n = 0;
    XtSetArg(args[n], XtNcomposeMod, &compose); n++;
    XtGetValues(widg, args, n);

    if (compose < 0 || compose > 5) {
        n = 0;
        XtSetArg(args[n], XtNcomposeMod, 0);    n++;
        XtSetValues(widg, args, n);
    }

    initcursors();
    atexit(freebindings);
    atexit(freechords);

    if ((xim = XOpenIM(_dpy, NULL, NULL, NULL)) == NULL) {
        XSetLocaleModifiers("@im=local");
        if ((xim =  XOpenIM(_dpy, NULL, NULL, NULL)) == NULL) {
            XSetLocaleModifiers("@im=");
            if ((xim = XOpenIM(_dpy, NULL, NULL, NULL)) == NULL) {
                fprintf(stderr, "could not open input method\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    font = XftFontOpenName(_dpy, DefaultScreen(_dpy), fontspec[0] ? fontspec : getenv("FONT") ? getenv("FONT") : "monospace");
    screen.id = 0;
    XtRealizeWidget(_toplevel);
    _topwindow = XtWindow(_toplevel);
    atexit(freefont);

    pid_t pid = getpid();
    XChangeProperty(_dpy, XtWindow(_toplevel), XInternAtom(_dpy, "_NET_WM_PID", False), XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&pid, 1);

    _fgpixel = _fgcolor.pixel;
    _borderpixel = _bordercolor.pixel;

    XRenderColor xrcolor = {0};
    xrcolor.red = _fgcolor.red;
    xrcolor.green = _fgcolor.green;
    xrcolor.blue = _fgcolor.blue;
    xrcolor.alpha = 65535;
    XftColorAllocValue(_dpy, DefaultVisual(_dpy, DefaultScreen(_dpy)), DefaultColormap(_dpy, DefaultScreen(_dpy)), &xrcolor, &fontcolor);

    screen.id = (int) XtWindow(widg);
    screen.ldepth = ilog2(DefaultDepth(_dpy, DefaultScreen(_dpy)));
    screen.flag = SCR;
    if(_fgpixel != 0)
        screen.flag |= BL1;
    /* leave screen rect at all zeros until reshaped() sets it */
    while(!exposed) {
        XFlush(_dpy);
        XtAppProcessEvent(app, XtIMXEvent);
    }

    darkgrey = balloc(Rect(0, 0, 16, 16), 0);
    wrbitmap(darkgrey, 0, 16, darkgreybits);

    XFlush(_dpy);
    focinit(_toplevel);
}

static void
focinit(Widget w)
{
    XrmValue src, dst;

    src.addr = "WM_TAKE_FOCUS";
    src.size = strlen((char *)src.addr)+1;
    dst.addr = (XtPointer) &wm_take_focus;
    dst.size = sizeof(Atom);
    XtConvertAndStore(w, XtRString, &src, XtRAtom, &dst);
    XSetWMProtocols(XtDisplay(w), XtWindow(w), &wm_take_focus, 1);
    XtAppAddActions(app, wmpactions, XtNumber(wmpactions));
    XtAugmentTranslations(w, XtParseTranslationTable(shelltrans));
}

static void
wmproto(Widget w, XEvent *e , String *p, Cardinal *np)
{
    Time t;

    if(e->type == ClientMessage &&
          (Atom)(e->xclient.data.l[0]) == wm_take_focus) {
        t = (Time) e->xclient.data.l[1];
        XtCallAcceptFocus(widg, &t);
    }
}

static void
reshaped(int minx, int miny, int maxx, int maxy)
{
    Ebuf *eb;

    screen.r = Rect(minx, miny, maxx, maxy);
    screen.clipr = screen.r;
    if (screen.id) {
        exposed = true;
        ereshaped(screen.r);
    }
    if(einitcalled){
        /*
         * Cause a mouse event, so programs like sam
         * will get out of eread and REALLY do the reshape
         */
        eb = ebadd(&esrc[Smouse], false);
        if (eb == 0)
            berror("eballoc can't malloc");
        memcpy((void*)eb->buf, (void*)&lastmouse, sizeof lastmouse);
        esrc[Smouse].count++;
    }
}

static void
gotchar(int c, int kind, int target, int x, int y, const char *arg)
{
    Ebuf *eb;
    Keystroke k;

    if(!einitcalled || Skeyboard == -1)
        return;
    eb = ebadd(&esrc[Skeyboard], false);
    if (eb == NULL)
        berror("eballoc can't malloc");
    k.c = c;
    k.k = kind;
    k.t = target;
    k.p = Pt(x, y);
    k.a = arg;
    memcpy(eb->buf, &k, sizeof(Keystroke));
    esrc[Skeyboard].count++;
}

static void
gotmouse(Gwinmouse *gm)
{
    Ebuf *eb;
    Mouse m;

    if(!einitcalled || Smouse == -1)
        return;
    m.buttons = gm->buttons;
    m.xy.x = gm->xy.x;
    m.xy.y = gm->xy.y;
    m.msec = gm->msec;
    lastmouse = m;
    eb = ebadd(&esrc[Smouse], false);
    if (eb == 0)
        berror("eballoc can't malloc");
    memcpy((void*)eb->buf, (void*)&m, sizeof m);
    esrc[Smouse].count++;
}

static void
gotinput(XtPointer cldata, int *pfd, XtInputId *id)
{
    Ebuf *eb, *lasttail, *newe;
    Esrc *es;
    int n;

    if(!einitcalled)
        return;
    es = (Esrc *)cldata;
    if (es->count >= MAXINPUT)
        return;
    lasttail = es->tail;
    eb = ebadd(es, false);
    if (eb == 0)
        return;
    if(es->size){
        if (es->issocket){
            struct sockaddr addr;
            socklen_t len;
            int fd = accept(*pfd, &addr, &len);
            n = read(fd, (char *)eb->buf, es->size);
            close(fd);
        } else
            n = read(*pfd, (char *)eb->buf, es->size);
        if (n < 0)
            n = 0;
        if(n < es->size) {
            newe = realloc(eb, sizeof(Ebuf)+n);
            newe->n = n;
            if (es->head == eb)
                es->head = newe;
            else
                lasttail->next = newe;
            es->tail = newe;
        }
    }
    es->count++;
}

static int
ilog2(int n)
{
    int i, v;

    for(i=0, v=1; i < 6; i++, v<<=1)
        if(n <= v)
            break;
    return i;
}


void
rdcolmap(Bitmap *b, RGB *map)
{
    XColor cols[256];
    int i, n, depth;
    Colormap cmap;
    Arg args[2];

    if (_cmap_installed) {
        cmap = _libg_cmap;
    } else {
        i = 0;
        XtSetArg(args[i], XtNcolormap, &cmap);  i++;
        XtGetValues(_toplevel, args, i);
    }

    depth = _ld2d[screen.ldepth];
    n = 1 << depth;
    if (depth == 1) {
        map[0].red = map[0].green = map[0].blue = ~0;
        map[1].red = map[1].green = map[1].blue = 0;
    }
    else {
        if (n > 256) {
            berror("rdcolmap bitmap too deep");
            return;
        }
        for (i = 0; i < n; i++)
            cols[i].pixel = i;
        XQueryColors(_dpy, cmap, cols, n);
        for (i = 0; i < n; i++) {
            map[i].red = (cols[i].red << 16) | cols[i].red;
            map[i].green = (cols[i].green << 16) | cols[i].green;
            map[i].blue = (cols[i].blue << 16) | cols[i].blue;
        }
    }
}

void
wrcolmap(Bitmap *b, RGB *map)
{
    int i, n, depth;
    Screen *scr;
    XColor cols[256];
    Arg args[2];
    XVisualInfo vi;
    Window w;

    scr = XtScreen(_toplevel);
    depth = _ld2d[screen.ldepth];
    n = 1 << depth;
    if (n > 256) {
        berror("wrcolmap bitmap too deep");
        return;
    } else if (depth > 1) {
        for (i = 0; i < n; i++) {
            cols[i].red = map[i].red >> 16;
            cols[i].green = map[i].green >> 16;
            cols[i].blue = map[i].blue >> 16;
            cols[i].pixel = i;
            cols[i].flags = DoRed|DoGreen|DoBlue;
        }
        if (!XMatchVisualInfo(_dpy, XScreenNumberOfScreen(scr),
                    depth, PseudoColor, &vi)) {
            berror("wrcolmap can't get visual");
            return;
        }
        w = XtWindow(_toplevel);
        _libg_cmap = XCreateColormap(_dpy, w, vi.visual, AllocAll);
        XStoreColors(_dpy, _libg_cmap, cols, n);

        i = 0;
        XtSetArg(args[i], XtNcolormap, _libg_cmap); i++;
        XtSetValues(_toplevel, args, i);
        _cmap_installed = 1;
    }
}

void
einit(uint64_t keys)
{
    /*
     * Make sure Smouse = ilog2(Emouse) and Skeyboard == ilog2(Ekeyboard)
     */
    nsrc = 0;
    if(keys&Emouse){
        Smouse = 0;
        esrc[Smouse].inuse = true;
        esrc[Smouse].size = sizeof(Mouse);
        esrc[Smouse].count = 0;
        nsrc = Smouse+1;
    }
    if(keys&Ekeyboard){
        Skeyboard = 1;
        esrc[Skeyboard].inuse = true;
        esrc[Skeyboard].size = sizeof(Keystroke);
        esrc[Skeyboard].count = 0;
        if(Skeyboard >= nsrc)
            nsrc = Skeyboard+1;
    }
    einitcalled = 1;
}

uint64_t
estart(uint64_t key, int fd, size_t n, bool issocket)
{
    int i;

    if(fd < 0)
        berror("bad fd to estart");
    if(n <= 0 || n > EMAXMSG)
        n = EMAXMSG;
    for(i=0; i<MAXSRC; i++)
        if((key & ~(1<<i)) == 0 && !esrc[i].inuse){
            if(nsrc <= i)
                nsrc = i+1;
            esrc[i].inuse = true;
            esrc[i].issocket = issocket;
            esrc[i].size = n;
            esrc[i].count = 0;
            XtAppAddInput(app, fd, (XtPointer)XtInputReadMask,
                gotinput, (XtPointer) &esrc[i]);
            return 1<<i;
        }
    return 0;
}

uint64_t
event(Event *e)
{
    return eread(~0L, e);
}

uint64_t
eread(uint64_t keys, Event *e)
{
    Ebuf *eb;
    int i;

    if(keys == 0)
        return 0;
        /* Give Priority to X events */
    if (XtAppPending(app) & XtIMXEvent)
        XtAppProcessEvent(app, XtIMXEvent);

    for(;;){
        for(i=0; i<nsrc; i++)
            if((keys & (1<<i)) && esrc[i].head){
                if(i == Smouse)
                    e->mouse = emouse();
                else if(i == Skeyboard)
                    e->keystroke = ekbd();
                else {
                    eb = ebread(&esrc[i]);
                    e->n = eb->n;
                    if(e->n > 0)
                        memcpy((void*)e->data, (void*)eb->buf, e->n);
                    free(eb);
                }
                return 1<<i;
            }
        waitevent();
    }
}

Mouse
emouse(void)
{
    Mouse m;
    Ebuf *eb;

    if(!esrc[Smouse].inuse)
        berror("mouse events not selected");
    eb = ebread(&esrc[Smouse]);
    memcpy((void*)&m, (void*)eb->buf, sizeof(Mouse));
    free(eb);
    return m;
}

Keystroke
ekbd(void)
{
    Ebuf *eb;
    Keystroke k;

    if(!esrc[Skeyboard].inuse)
        berror("keyboard events not selected");
    eb = ebread(&esrc[Skeyboard]);
    memcpy(&k, eb->buf, sizeof(Keystroke));
    free(eb);
    return k;
}

void
pushkbd(int c)
{
    Ebuf *eb;
    Keystroke k;

    if(!einitcalled || Skeyboard == -1)
        return;
    eb = ebadd(&esrc[Skeyboard], true);
    if (eb == 0)
        berror("eballoc can't malloc");
    k.c = c;
    k.k = Kraw;
    memcpy(eb->buf, &k, sizeof(Keystroke));
    esrc[Skeyboard].count++;
}

int
ecanread(uint64_t keys)
{
    int i;

    for(;;){
        for(i=0; i<nsrc; i++){
            if((keys & (1<<i)) && esrc[i].head)
                return 1<<i;
        }
        if(XtAppPending(app))
            waitevent();
        else
            return 0;
    }
}

int
ecanmouse(void)
{
    if(Smouse == -1)
        berror("mouse events not selected");
    return ecanread(Emouse);
}

int
ecankbd(void)
{
    if(Skeyboard == -1)
        berror("keyboard events not selected");
    return ecanread(Ekeyboard);
}

static Ebuf*
ebread(Esrc *s)
{
    Ebuf *eb;

    while(s->head == 0)
        waitevent();
    eb = s->head;
    if(s == &esrc[Smouse]) {
        while(eb->next) {
            s->head = eb->next;
            s->count--;
            free(eb);
            eb = s->head;
        }
    }
    s->head = s->head->next;
    if(s->head == 0) {
        s->tail = 0;
        s->count = 0;
    } else
        s->count--;
    return eb;
}

static inline void
ebappend(Ebuf *b, Esrc *s)
{
    if (s->tail){
        s->tail->next = b;
        s->tail = b;
    } else
        s->head = s->tail = b;
}

static inline void
ebprepend(Ebuf *b, Esrc *s)
{
    b->next = s->head;
    s->head = b;
}

static Ebuf*
ebadd(Esrc *s, bool prepend)
{
    Ebuf *eb;
    int m;

    m = sizeof(Ebuf);
    if(s->size > 1)
        m += (s->size-1);   /* overestimate, because of alignment */
    eb = (Ebuf *)malloc(m);
    if(eb) {
        eb->next = 0;
        eb->n = s->size;
        if (prepend)
            ebprepend(eb, s);
        else
            ebappend(eb, s);
    }
    return eb;
}

void
berror(char *s)
{
    if(onerr)
        (*onerr)(s);
    else{
        fprintf(stderr, "libg error: %s:\n", s);
        exit(1);
    }
}

void
bflush(void)
{
    while(XtAppPending(app) & XtIMXEvent)
        waitevent();
}

static void
waitevent(void)
{
    XFlush(_dpy);
    if (XtAppPending(app) & XtIMXEvent)
        XtAppProcessEvent(app, XtIMXEvent);
    else
        XtAppProcessEvent(app, XtIMAll);
}
        
int
snarfswap(char *s, int n, char **t)
{
    *t = GwinSelectionSwap(widg, s);
    if (*t)
        return strlen(*t);
    return 0;
}

int scrpix(int *w, int *h)
{
    if (w)
        *w = WidthOfScreen(XtScreen(_toplevel));
    if (h)
        *h = HeightOfScreen(XtScreen(_toplevel));
    return 1;
}

#ifdef DEBUG
/* for debugging */
printgc(char *msg, GC g)
{
    XGCValues v;

    XGetGCValues(_dpy, g, GCFunction|GCForeground|GCBackground|GCFont|
            GCTile|GCFillStyle|GCStipple, &v);
    fprintf(stderr, "%s: gc %x\n", msg, g);
    fprintf(stderr, "  fg %d bg %d func %d fillstyle %d font %x tile %x stipple %x\n",
        v.foreground, v.background, v.function, v.fill_style,
        v.font, v.tile, v.stipple);
}
#endif

void
raisewindow(void)
{
    XEvent e;
    Atom a = XInternAtom(_dpy, "_NET_ACTIVE_WINDOW", True);

    XRaiseWindow(_dpy, _topwindow);

    if (a != None){
        memset(&e, 0, sizeof(XEvent));
        e.type = ClientMessage;
        e.xclient.window = _topwindow;
        e.xclient.message_type = a;
        e.xclient.format = 32;
        e.xclient.data.l[0] = 1;
        e.xclient.data.l[1] = CurrentTime;
        e.xclient.data.l[2] = None;
        e.xclient.data.l[3] = 0;
        e.xclient.data.l[4] = 0;

        XSendEvent(_dpy, DefaultRootWindow(_dpy), False,
                   SubstructureRedirectMask | SubstructureNotifyMask, &e);
    }

    XFlush(_dpy);
}

uint64_t
getbg(void)
{
    static int i = 0;

    if (i >= _nbgs)
        i = 0;

    return _bgpixels[i++];
}

