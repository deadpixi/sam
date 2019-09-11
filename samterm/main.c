
/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include <frame.h>
#include <unistd.h>
#include "flayer.h"
#include "samterm.h"

extern uint64_t _bgpixel;
extern void hmoveto(int, int64_t, Flayer *);

Text    cmd;
wchar_t    *scratch;
int64_t    nscralloc;
extern Bitmap   screen;
unsigned int cursor;
Mouse   mouse;
Flayer  *which = NULL;
Flayer  *flast = NULL;
Flayer  *work = NULL;
int64_t    snarflen;
int64_t    typestart = -1;
int64_t    typeend = -1;
int64_t    typeesc = -1;
bool    modified = false;       /* strange lookahead for menus */
char    lock = 1;
bool    hasunlocked = false;
bool expandtabs = false;
bool autoindent = false;
char *machine = "localhost";
int exfd = -1;
const char *exname;
bool followfocus = false;

void
removeext(void)
{
    if (exname){
        unlink(exname);

        char lockpath[FILENAME_MAX + 1] = {0};
        const char *path = getenv("SAMSOCKPATH")? getenv("SAMSOCKPATH") : getenv("HOME");
        snprintf(lockpath, PATH_MAX, "%s/.sam.%s.lock", path, machine);
        unlink(lockpath);
    }
}

int
main(int argc, char *argv[])
{
    int i, got, scr, opt;
    Text *t;
    Rectangle r;
    Flayer *nwhich;
    char rcpath[PATH_MAX + 1] = {0};
    FILE *rc = NULL;

    setlocale(LC_ALL, "");
    installdefaultbindings();
    installdefaultchords();

    if (getenv("SAMRC"))
        strncpy(rcpath, getenv("SAMRC"), PATH_MAX);
    else
        snprintf(rcpath, PATH_MAX, "%s/.samrc", getenv("HOME") ? getenv("HOME") : ".");

    while ((opt = getopt(argc, argv, "ef:n:r:")) != -1){
        switch (opt){
            case 'r':
                machine = optarg;
                break;

            case 'f':
                exfd = atoi(optarg);
                break;

            case 'n':
                exname = optarg;
                atexit(removeext);
                break;
        }
    }

    rc = fopen(rcpath, "r");
    if (rc){
        loadrcfile(rc);
        fclose(rc);
    }

    getscreen(argc, argv);
    initio();
    scratch = alloc(100*RUNESIZE);
    nscralloc = 100;
    r = screen.r;
    r.max.y = r.min.y+Dy(r)/5;
    flstart(screen.clipr);
    rinit(&cmd.rasp);
    flnew(&cmd.l[0], stgettext, 1, &cmd);
    cmd.l[0].bg = getbg();
    flinit(&cmd.l[0], r, font, cmd.l[0].bg);
    cmd.nwin = 1;
    which = &cmd.l[0];
    cmd.tag = Untagged;
    outTs(Tversion, VERSION);
    startnewfile(Tstartcmdfile, &cmd);

    got = 0;
    for(;;got = waitforio()){
        if(hasunlocked && RESHAPED())
            reshape();
        if(got&RHost)
            rcv();
        if(got&RExtern){
            for(i=0; cmd.l[i].textfn==0; i++)
                ;
            current(&cmd.l[i]);
            flsetselect(which, cmd.rasp.nrunes, cmd.rasp.nrunes);
            type(which);
        }
        if(got&RKeyboard){
            if(which)
                type(which);
            else
                kbdblock();
        }
        if(got&RMouse){
            if(lock==2 || !ptinrect(mouse.xy, screen.r)){
                mouseunblock();
                continue;
            }
            nwhich = flwhich(mouse.xy);
            scr = which && ptinrect(mouse.xy, which->scroll);
            if(mouse.buttons)
                flushtyping(true);
            if(mouse.buttons&1){
                if(nwhich){
                    if(nwhich!=which)
                        current(nwhich);
                    else if(scr)
                        scroll(which, 1, 1);
                    else{
                        t=(Text *)which->user1;
                        if(flselect(which)){
                            outTsl(Tdclick, t->tag, which->p0);
                            t->lock++;
                        }else if(t!=&cmd)
                            outcmd();
                    }
                }
            }else if((mouse.buttons&2) && which){
                if(scr)
                    scroll(which, 2, 2);
                else
                    menu2hit();
            }else if((mouse.buttons&4)){
                if(scr)
                    scroll(which, 3, 3);
                else
                    menu3hit();
            }else if(followfocus && nwhich && nwhich!=which){
                current(nwhich);
            }
            mouseunblock();
        }
    }

    return EXIT_SUCCESS;
}


void
reshape(void)
{
    int i;

    flreshape(screen.clipr);
    for(i = 0; i<nname; i++)
        if(text[i])
            hcheck(text[i]->tag);
}

void
current(Flayer *nw)
{
    Text *t;

    if(which)
        flborder(which, false);
    if(nw){
        flushtyping(true);
        if (!followfocus)
            flupfront(nw);
        flborder(nw, true);
        buttons(Up);
        t = (Text *)nw->user1;
        t->front = nw-&t->l[0];
        if(t != &cmd)
            work = nw;
    }
    which = nw;
}

void
closeup(Flayer *l)
{
    Text *t=(Text *)l->user1;
    int m;

    m = whichmenu(t->tag);
    if(m < 0)
        return;
    flclose(l);
    if(l == which){
        which = 0;
        current(flwhich(Pt(0, 0)));
    }
    if(l == flast)
        flast = 0;
    if(l == work)
        work = 0;
    if(--t->nwin == 0){
        rclear(&t->rasp);
        free(t);
        text[m] = 0;
    }else if(l == &t->l[t->front]){
        for(m=0; m<NL; m++) /* find one; any one will do */
            if(t->l[m].textfn){
                t->front = m;
                return;
            }
        panic("close");
    }
}

Flayer *
findl(Text *t)
{
    int i;
    for(i = 0; i<NL; i++)
        if(t->l[i].textfn==0)
            return &t->l[i];
    return 0;
}

void
duplicate(Flayer *l, Rectangle r, XftFont *f, int close)
{
    Text *t=(Text *)l->user1;
    Flayer *nl = findl(t);
    wchar_t *rp;
    uint64_t n;

    if(nl){
        flnew(nl, stgettext, l->user0, (char *)t);
        flinit(nl, r, f, l->bg);
        nl->origin = l->origin;
        rp = (*l->textfn)(l, l->f.nchars, &n);
        flinsert(nl, rp, rp+n, l->origin);
        flsetselect(nl, l->p0, l->p1);
        if(close){
            flclose(l);
            if(l==which)
                which = 0;
        }else
            t->nwin++;
        current(nl);
        hcheck(t->tag);
    }
    cursorswitch(cursor);
}

void
buttons(int updown)
{
    while(((mouse.buttons&7)!=0) != updown)
        frgetmouse();
}

int
getr(Rectangle *rp)
{
    Point p;
    Rectangle r;

    *rp = getrect(3, &mouse);
    if(rp->max.x && rp->max.x-rp->min.x<=5 && rp->max.y-rp->min.y<=5){
        p = rp->min;
        r = cmd.l[cmd.front].entire;
        *rp = screen.r;
        if(cmd.nwin==1){
            if (p.y <= r.min.y)
                rp->max.y = r.min.y;
            else if (p.y >= r.max.y)
                rp->min.y = r.max.y;
            if (p.x <= r.min.x)
                rp->max.x = r.min.x;
            else if (p.x >= r.max.x)
                rp->min.x = r.max.x;
        }
    }
    return rectclip(rp, screen.r) &&
       rp->max.x-rp->min.x>100 && rp->max.y-rp->min.y>40;
}

void
snarf(Text *t, int w)
{
    Flayer *l = &t->l[w];

    if(l->p1>l->p0){
        snarflen = l->p1-l->p0;
        outTsll(Tsnarf, t->tag, l->p0, l->p1);
    }
}

void
cut(Text *t, int w, bool save, bool check)
{
    int64_t p0, p1;
    Flayer *l;

    l = &t->l[w];
    p0 = l->p0;
    p1 = l->p1;
    if(p0 == p1)
        return;
    if(p0 < 0)
        panic("cut");
    if(save)
        snarf(t, w);
    outTsll(Tcut, t->tag, p0, p1);
    flsetselect(l, p0, p0);
    t->lock++;
    hcut(t->tag, p0, p1-p0);
    if(check)
        hcheck(t->tag);
}

void
paste(Text *t, int w)
{
    if(snarflen){
        cut(t, w, false, false);
        t->lock++;
        outTsl(Tpaste, t->tag, t->l[w].p0);
    }
}

void
scrorigin(Flayer *l, int but, int64_t p0)
{
    Text *t=(Text *)l->user1;

    switch(but){
    case 1:
        outTslll(Torigin, t->tag, l->origin, p0, getlayer(l, t));
        break;
    case 2:
        outTslll(Torigin, t->tag, p0, 1L, getlayer(l, t));
        break;
    case 3:
        horigin(t->tag, p0, NULL);
    }
}

int
raspc(Rasp *r, int64_t p)
{
    uint64_t n;
    rload(r, p, p+1, &n);
    if(n)
        return scratch[0];
    return 0;
}

int64_t
ctlw(Rasp *r, int64_t o, int64_t p)
{
    int c;

    if(--p < o)
        return o;
    if(raspc(r, p)=='\n')
        return p;
    for(; p>=o && !iswalnum(c=raspc(r, p)); --p)
        if(c=='\n')
            return p+1;
    for(; p>o && iswalnum(raspc(r, p-1)); --p)
        ;
    return p>=o? p : o;
}

int64_t
ctlu(Rasp *r, int64_t o, int64_t p)
{
    for(; p-1>=o && raspc(r, p-1)!='\n'; --p)
        ;
    return p>=o? p : o;
}

int64_t
indent(Flayer *l, long p)
{
	Text *t = (Text *)l->user1;
	static wchar_t sbuf[7] = {' ',' ',' ',' ',' ',' ',' '};
	static wchar_t tbuf[7] = {'\t','\t','\t','\t','\t','\t','\t'};
	int i, is, it, q, c, space;

	q = p - 1; is = 0; it = 0; space = true;
	while(--q >= l->origin) {
		c = raspc(&t->rasp, q);
		if(c == '\n') {
            break;
		} else if(c == '\t') {
			++it;
		} else if(c == ' ') {
			++is;
		} else {
			it = is = 0; 
			space = false;
		}
	}
    if(space) 
        it = is = 0;

	while(it != 0) {
		i = it>7?7:it;
		hgrow(t->tag, p, i, 0);
		t->lock++;
		hdatarune(t->tag, p, tbuf, i);
		it -= i; p += i;
	}
	while(is != 0) {
		i = is > 7? 7 : is;
		hgrow(t->tag, p, i, 0);
		t->lock++;
		hdatarune(t->tag, p, sbuf, i);
		is -= i; p += i;
	}

	return typeend = l->p0 = l->p1 = p;
}

int
center(Flayer *l, int64_t a)
{
    Text *t = l->user1;

    if (!t->lock && (a < l->origin || l->origin + l->f.nchars < a)){
        a = (a > t->rasp.nrunes) ? t->rasp.nrunes : a;
        outTslll(Torigin, t->tag, a, 2L, getlayer(l, t));
        return 1;
    }

    return 0;
}

int
onethird(Flayer *l, int64_t a)
{
    Text *t;
    Rectangle s;
    int64_t lines;

    t = l->user1;
    if(!t->lock && (a<l->origin || l->origin+l->f.nchars<a)){
        if(a > t->rasp.nrunes)
            a = t->rasp.nrunes;
        s = inset(l->scroll, 1);
        lines = ((s.max.y-s.min.y)/l->f.fheight+1)/3;
        if (lines < 2)
            lines = 2;
        outTslll(Torigin, t->tag, a, lines, getlayer(l, t));
        return 1;
    }
    return 0;
}


int
XDisplay(Display *);

extern Display * _dpy;

void
flushtyping(bool clearesc)
{
    Text *t;
    uint64_t n;

    if (clearesc)
        typeesc = -1;   
    if (typestart == typeend){
        modified = false;
        return;
    }
    t = which->user1;
    if(t != &cmd)
        modified = true;
    rload(&t->rasp, typestart, typeend, &n);
    scratch[n] = 0;
    if(t==&cmd && typeend==t->rasp.nrunes && scratch[typeend-typestart-1]=='\n'){
        setlock();
        outcmd();
    }
    outTslS(Ttype, t->tag, typestart, scratch);
    typestart = -1;
    typeend = -1;
    XFlush(_dpy);
}

static int64_t
cmdscrolldown(Flayer *l, int64_t a, Text *t, const char *arg)
{
    flushtyping(false);
    center(l, l->origin + l->f.nchars + 1);
    return a;
}

static int64_t
cmdscrollup(Flayer *l, int64_t a, Text *t, const char *arg)
{
    flushtyping(false);
    outTslll(Torigin, t->tag, l->origin, l->f.maxlines + 1, getlayer(l, t));
    return a;
}

static int64_t
cmdcharleft(Flayer *l, int64_t a, Text *t, const char *arg)
{
    flsetselect(l, a, a);
    flushtyping(false);
    if (a > 0)
        a--;
    flsetselect(l, a, a);
    center(l, a);

    return a;
}

static int64_t
cmdcharright(Flayer *l, int64_t a, Text *t, const char *arg)
{
    flsetselect(l, a, a);
    flushtyping(false);
    if (a < t->rasp.nrunes)
        a++;
    flsetselect(l, a, a);
    center(l, a);

    return a;
}

static int64_t
cmdeol(Flayer *l, int64_t a, Text *t, const char *arg)
{
    flsetselect(l, a, a);
    flushtyping(true);
    while(a < t->rasp.nrunes)
         if(raspc(&t->rasp, a++) == '\n') {
             a--;
             break;
         }

    flsetselect(l, a, a);
    center(l, a);

    return a;
}

static int64_t
cmdbol(Flayer *l, int64_t a, Text *t, const char *arg)
{
    flsetselect(l, a, a);
    flushtyping(true);
    while (a > 0){
        if (raspc(&t->rasp, --a) == '\n'){
            a++;
            break;
        }
    }

    flsetselect(l, a, a);
    center(l, a);

    return a;
}

static int64_t
cmdscrollupline(Flayer *l, int64_t a, Text *t, const char *arg)
{
    if (l->origin > 0)
        hmoveto(t->tag, l->origin - 1, l);
    return a;
}

static int64_t
cmdscrolldownline(Flayer *l, int64_t a, Text *t, const char *arg)
{
    int64_t e = t->rasp.nrunes;

    horigin(t->tag,
            l->origin + frcharofpt(&l->f,Pt(l->f.r.min.x, l->f.r.min.y + l->f.fheight)),
            l);

    return a;
}

static int64_t
cmdlineup(Flayer *l, int64_t a, Text *t, const char *arg)
{
    flsetselect(l, a, a);
    flushtyping(true);
    if (a > 0){
        int64_t n0, n1, count = 0;
        while (a > 0 && raspc(&t->rasp, a - 1) != '\n'){
            a--;
            count++;
        }
        if (a > 0){
            n1 = a;
            a--;
            while (a > 0 && raspc(&t->rasp, a - 1) != '\n')
                a--;
    
            n0 = a;
            a = (n0 + count >= n1) ? n1 - 1 : n0 + count;
            flsetselect(l, a, a);
            center(l, a);
        }
    }

    return a;
}

static int64_t
cmdlinedown(Flayer *l, int64_t a, Text *t, const char *arg)
{
    flsetselect(l, a, a);
    flushtyping(true);
    if (a < t->rasp.nrunes){
        int64_t p0, count = 0;

        p0 = a;
        while (a > 0 && raspc(&t->rasp, a - 1) != '\n'){
            a--;
            count++;
        }

        a = p0;
        while (a < t->rasp.nrunes && raspc(&t->rasp, a) != '\n')
            a++;

        if (a < t->rasp.nrunes){
            a++;
            while (a < t->rasp.nrunes && count > 0 && raspc(&t->rasp, a) != '\n'){
                a++;
                count--;
            }
            if (a != p0){
                flsetselect(l, a, a);
                center(l, a);
            }
        }
    }

    return a;
}

static int64_t
cmdjump(Flayer *l, int64_t a, Text *u, const char *arg)
{
    Text *t = NULL;

    if (which == &cmd.l[cmd.front] && flast)
        current(flast);
    else{
        l = &cmd.l[cmd.front];
        t = (Text *)l->user1;
        flast = which;
        current(l);
        flushtyping(false);
        flsetselect(l, t->rasp.nrunes, t->rasp.nrunes);
        center(l, t->rasp.nrunes);
    }

    return a;
}

static int64_t
cmdlook(Flayer *l, int64_t a, Text *t, const char *arg)
{
    outTsll(Tlook, t->tag, which->p0, which->p1);
    setlock();
    return a;
}

static int64_t
cmdsearch(Flayer *l, int64_t a, Text *t, const char *arg)
{
    if (t != &cmd && haspat()){
        outcmd();
        outT0(Tsearch);
        setlock();
    }
    return a;
}

static int64_t
cmdwrite(Flayer *l, int64_t a, Text *t, const char *arg)
{
    cursorswitch(BullseyeCursor);
    if (t != &cmd){
        outTs(Twrite, t->tag);
        setlock();
    }
    cursorswitch(cursor);
    return a;
}

static int64_t
cmdescape(Flayer *l, int64_t a, Text *t, const char *arg)
{
    if (typeesc >= 0){
        l->p0 = typeesc;
        l->p1 = a;
        flushtyping(true);
    }

    for (l = t->l; l < &t->l[NL]; l++)
        if (l->textfn)
            flsetselect(l, l->p0, l->p1);

    return a;
}

static int64_t
cmddelword(Flayer *l, int64_t a, Text *t, const char *arg)
{
    if (l->f.p0 > 0 && a > 0)
        l->p0 = ctlw(&t->rasp, l->origin, a);

    l->p1 = a;
    if (l->p1 != l->p0){
        if(typestart<=l->p0 && l->p1<=typeend){
            t->lock++;  /* to call hcut */
            hcut(t->tag, l->p0, l->p1-l->p0);
            /* hcheck is local because we know rasp is contiguous */
            hcheck(t->tag);
        }else{
            flushtyping(false);
            cut(t, t->front, false, true);
        }
    }

    return a;
}

static int64_t
cmddelbol(Flayer *l, int64_t a, Text *t, const char *arg)
{
    if (l->f.p0 > 0 && a > 0)
        l->p0 = ctlu(&t->rasp, l->origin, a);

    l->p1 = a;
    if (l->p1 != l->p0){
        if(typestart<=l->p0 && l->p1<=typeend){
            t->lock++;  /* to call hcut */
            hcut(t->tag, l->p0, l->p1-l->p0);
            /* hcheck is local because we know rasp is contiguous */
            hcheck(t->tag);
        }else{
            flushtyping(false);
            cut(t, t->front, false, true);
        }
    }

    return a;
}

static int64_t
cmddelbs(Flayer *l, int64_t a, Text *t, const char *arg)
{
    if (l->f.p0 > 0 && a > 0)
        l->p0 = a - 1;

    l->p1 = a;
    if (l->p1 != l->p0){
        if(typestart <= l->p0 && l->p1 <= typeend){
            t->lock++;  /* to call hcut */
            hcut(t->tag, l->p0, l->p1 - l->p0);
            /* hcheck is local because we know rasp is contiguous */
            hcheck(t->tag);
        }else{
            flushtyping(false);
            cut(t, t->front, false, true);
        }
    }

    return a;
}

static int64_t
cmddel(Flayer *l, int64_t a, Text *t, const char *arg)
{
    l->p0 = a;
    if (a < t->rasp.nrunes)
        l->p1 = a + 1;
    if (l->p1 != l->p0){
        if(typestart <= l->p0 && l->p1 <= typeend){
            t->lock++;  /* to call hcut */
            hcut(t->tag, l->p0, l->p1 - l->p0);
            /* hcheck is local because we know rasp is contiguous */
            hcheck(t->tag);
        }else{
            flushtyping(false);
            cut(t, t->front, false, true);
        }
    }

    return a;
}

int
getlayer(const Flayer *l, const Text *t)
{
    int i;
    for (i = 0; i < NL; i++){
        if (&t->l[i] == l)
            return i;
    }

    return -1;
}

static int64_t
cmdexchange(Flayer *l, int64_t a, Text *t, const char *arg)
{
    int w = getlayer(l, t);
    if (w >= 0){
        snarf(t, w);
        outT0(Tstartsnarf);
        setlock();
    }

    return a;
}

static int64_t
cmdsnarf(Flayer *l, int64_t a, Text *t, const char *arg)
{
    flushtyping(false);

    int w = getlayer(l, t);
    if (w >= 0)
        snarf(t, w);

    return a;
}

static int64_t
cmdcut(Flayer *l, int64_t a, Text *t, const char *arg)
{
    flushtyping(false);

    int w = getlayer(l, t);
    if (w >= 0)
        cut(t, w, true, true);

    return a;
}

static int64_t
cmdpaste(Flayer *l, int64_t a, Text *t, const char *arg)
{
    flushtyping(false);

    int w = getlayer(l, t);
    if (w >= 0)
        paste(t, w);

    return a;
}

static int64_t
cmdtab(Flayer *l, int64_t a, Text *t, const char *arg)
{
    flushtyping(false);

    if (!expandtabs)
        pushkbd('\t');
    else{
        int col = 0, nspaces = 8, off = a;
        int i;
        while (off > 0 && raspc(&t->rasp, off - 1) != '\n')
            off--, col++;

        nspaces = tabwidth - col % tabwidth;
        for (i = 0; i < nspaces; i++)
            pushkbd(' ');
    }

    return a;
}

static int64_t
cmdsend(Flayer *l, int64_t a, Text *t, const char *arg)
{
    bool dojump = (t != &cmd);

    flushtyping(false);
    if (dojump)
        cmdjump(l, a, t, NULL);

    for (const char *c = arg; *c; c++){
        pushkbd(*c);
        type(&cmd.l[cmd.front]);
        flushtyping(false);
    }
    pushkbd('\n');
    type(&cmd.l[cmd.front]);
    flushtyping(false);

    if (dojump)
        cmdjump(l, a, t, NULL);

    return a;
}

static int64_t
cmdnone(Flayer *l, int64_t a, Text *t, const char *arg)
{
    return a;
}

typedef int64_t (*Commandfunc)(Flayer *, int64_t, Text *, const char *);
typedef struct CommandEntry CommandEntry;
struct CommandEntry{
    Commandfunc f;
    bool unlocked;
    bool docut;
};

CommandEntry commands[Cmax] ={
    [Cnone]           = {cmdnone,           false, false},
    [Cscrolldown]     = {cmdscrolldown,     false, false},
    [Cscrollup]       = {cmdscrollup,       false, false},
    [Cscrolldownline] = {cmdscrolldownline, false, false},
    [Cscrollupline]   = {cmdscrollupline,   false, false},
    [Ccharleft]       = {cmdcharleft,       false, false},
    [Ccharright]      = {cmdcharright,      false, false},
    [Clineup]         = {cmdlineup,         false, false},
    [Clinedown]       = {cmdlinedown,       false, false},
    [Cjump]           = {cmdjump,           false, false},
    [Cescape]         = {cmdescape,         false, false},
    [Csnarf]          = {cmdsnarf,          false, false},
    [Ccut]            = {cmdcut,            false, false},
    [Cpaste]          = {cmdpaste,          false, false},
    [Cexchange]       = {cmdexchange,       false, false},
    [Cdelword]        = {cmddelword,        true,  false},
    [Cdelbol]         = {cmddelbol,         true,  false},
    [Cdelbs]          = {cmddelbs,          true,  true},
    [Cdel]            = {cmddel,            true,  true},
    [Ceol]            = {cmdeol,            false, false},
    [Cbol]            = {cmdbol,            false, false},
    [Ctab]            = {cmdtab,            false, false},
    [Csend]           = {cmdsend,           false, false},
    [Clook]           = {cmdlook,           false, false},
    [Csearch]         = {cmdsearch,         false, false},
    [Cwrite]          = {cmdwrite,          false, false}
};


void
type(Flayer *l)    /* what a bloody mess this is -- but it's getting better! */
{
    Text *t = (Text *)l->user1;
    wchar_t buf[100];
    Keystroke k = {0};
    wchar_t *p = buf;
    int64_t a;

    if(lock || t->lock){
        kbdblock();
        return;
    }

    k = qpeekc();
    a = l->p0;
    if (a != l->p1 && (k.k != Kcommand || commands[k.c].docut)){
        flushtyping(true);
        cut(t, t->front, true, true);
        return; /* it may now be locked */
    }

    while (((k = kbdchar()), k.c) > 0) {
        if (k.k == Kcommand)
            break;

        *p++ = k.c;
        if (k.c == '\n' || p >= buf + sizeof(buf) / sizeof(buf[0]))
            break;
    }

    if (k.k == Kcommand){
        flushtyping(false);
        if (k.c < 0 || k.c >= Cmax || commands[k.c].f == NULL)
            panic("command table miss");

        CommandEntry *e = &commands[k.c];
        if (!e->unlocked || !lock){
            if (k.t == Tcurrent)
                a = e->f(l, a, t, k.a);
            else{
                Flayer *lt = flwhich(k.p);
                if (lt)
                    lt->p0 = e->f(lt, lt->p0, (Text *)lt->user1, k.a);
            }
        }
    }

    if (p > buf){
        if (typestart < 0)
            typestart = a;

        if (typeesc < 0)
            typeesc = a;

        hgrow(t->tag, a, p-buf, 0);
        t->lock++;  /* pretend we Trequest'ed for hdatarune*/
        hdatarune(t->tag, a, buf, p-buf);
        a += p-buf;
        l->p0 = a;
        l->p1 = a;
        typeend = a;
        if (autoindent && k.c == '\n' && t!=&cmd)
            a = indent(l, a);
        if (k.c == '\n' || typeend - typestart > 100)
            flushtyping(false);
        onethird(l, a);
    }

    if (typeesc >= l->p0)
        typeesc = l->p0;

    if (typestart >= 0){
        if(typestart >= l->p0)
            typestart = l->p0;
        typeend = l->p0;
        if (typestart == typeend){
            typestart = -1;
            typeend = -1;
            modified = false;
        }
    }
}

void
outcmd(void)
{
    if(work)
        outTsll(Tworkfile, ((Text *)work->user1)->tag, work->p0, work->p1);
}

void
panic(char *s)
{
    fprintf(stderr, "samterm:panic: ");
    perror(s);
    abort();
}

wchar_t*
stgettext(Flayer *l, int64_t n, uint64_t *np)
{
    Text *t;

    t = l->user1;
    rload(&t->rasp, l->origin, l->origin+n, np);
    return scratch;
}

int64_t
scrtotal(Flayer *l)
{
    return ((Text *)l->user1)->rasp.nrunes;
}

void*
alloc(uint64_t n)
{
    void *p;

    p = malloc(n);
    if(p == 0)
        panic("alloc");
    memset(p, 0, n);
    return p;
}
