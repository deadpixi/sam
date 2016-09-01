/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <libg.h>
#include <frame.h>
#include <unistd.h>
#include "flayer.h"
#include "samterm.h"
#include <commands.h>

extern unsigned long _bgpixel;
extern void hmoveto(int, long);

Text	cmd;
Rune	*scratch;
long	nscralloc;
Cursor	*cursor;
extern Bitmap	screen;
Mouse	mouse;
Flayer	*which = 0;
Flayer  *flast = 0;
Flayer	*work = 0;
long	snarflen;
long	typestart = -1;
long	typeend = -1;
long	typeesc = -1;
long	modified = 0;		/* strange lookahead for menus */
char	lock = 1;
char	hasunlocked = 0;
int expandtabs = 0;
char *machine = "localhost";
int nofifo = 0;

void
main(int argc, char *argv[])
{
	int i, got, scr, opt;
	Text *t;
	Rectangle r;
	Flayer *nwhich;
	int fwdbut;

    while ((opt = getopt(argc, argv, "efr:")) != -1){
        switch (opt){
            case 'r':
                machine = optarg;
                break;

            case 'e':
                expandtabs = 1;
                break;

            case 'f':
                nofifo = 1;
                break;
        }
    }

	getscreen(argc, argv);
	fwdbut = scrollfwdbut();
	iconinit();
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
			type(which, RExtern);
		}
		if(got&RKeyboard)
			if(which)
				type(which, RKeyboard);
			else
				kbdblock();
		if(got&RMouse){
			if(lock==2 || !ptinrect(mouse.xy, screen.r)){
				mouseunblock();
				continue;
			}
			nwhich = flwhich(mouse.xy);
			scr = which && ptinrect(mouse.xy, which->scroll);
			if(mouse.buttons)
				flushtyping(1);
            if(mouse.buttons&1){
				if(nwhich){
					if(nwhich!=which)
						current(nwhich);
					else if(scr)
						scroll(which, 1, fwdbut == 3 ? 1 : 3);
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
					scroll(which, 3, fwdbut == 3 ? 3 : 1);
				else
					menu3hit();
			}
			mouseunblock();
		}
	}
}


void
reshape(void){
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
		flborder(which, 0);
	if(nw){
		flushtyping(1);
		flupfront(nw);
		flborder(nw, 1);
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
		free((uchar *)t);
		text[m] = 0;
	}else if(l == &t->l[t->front]){
		for(m=0; m<NL; m++)	/* find one; any one will do */
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
	Rune *rp;
	ulong n;

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
cut(Text *t, int w, int save, int check)
{
	long p0, p1;
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
		cut(t, w, 0, 0);
		t->lock++;
		outTsl(Tpaste, t->tag, t->l[w].p0);
	}
}

void
scrorigin(Flayer *l, int but, long p0)
{
	Text *t=(Text *)l->user1;

	switch(but){
	case 1: case 4:
		outTsll(Torigin, t->tag, l->origin, p0);
		break;
	case 2:
		outTsll(Torigin, t->tag, p0, 1L);
		break;
	case 3: case 5:
		horigin(t->tag,p0);
	}
}

int
alnum(int c)
{
	/*
	 * Hard to get absolutely right.  Use what we know about ASCII
	 * and assume anything above the Latin control characters is
	 * potentially an alphanumeric.
	 */
	if(c<=' ')
		return 0;
	if(0x7F<=c && c<=0xA0)
		return 0;
	if(utfrune("!\"#$%&'()*+,-./:;<=>?@[\\]^`{|}~", c))
		return 0;
	return 1;
}

int
raspc(Rasp *r, long p)
{
	ulong n;
	rload(r, p, p+1, &n);
	if(n)
		return scratch[0];
	return 0;
}

long
ctlw(Rasp *r, long o, long p)
{
	int c;

	if(--p < o)
		return o;
	if(raspc(r, p)=='\n')
		return p;
	for(; p>=o && !alnum(c=raspc(r, p)); --p)
		if(c=='\n')
			return p+1;
	for(; p>o && alnum(raspc(r, p-1)); --p)
		;
	return p>=o? p : o;
}

long
ctlu(Rasp *r, long o, long p)
{
	for(; p-1>=o && raspc(r, p-1)!='\n'; --p)
		;
	return p>=o? p : o;
}

int
center(Flayer *l, long a)
{
    Text *t = l->user1;

    if (!t->lock && (a < l->origin || l->origin + l->f.nchars < a)){
        a = (a > t->rasp.nrunes) ? t->rasp.nrunes : a;
        outTsll(Torigin, t->tag, a, 2L);
        return 1;
    }

    return 0;
}

int
onethird(Flayer *l, long a)
{
	Text *t;
	Rectangle s;
	long lines;

	t = l->user1;
	if(!t->lock && (a<l->origin || l->origin+l->f.nchars<a)){
		if(a > t->rasp.nrunes)
			a = t->rasp.nrunes;
		s = inset(l->scroll, 1);
		lines = ((s.max.y-s.min.y)/l->f.fheight+1)/3;
		if (lines < 2)
			lines = 2;
		outTsll(Torigin, t->tag, a, lines);
		return 1;
	}
	return 0;
}


int
XDisplay(Display *);

extern Display * _dpy;

void
flushtyping(int clearesc)
{
	Text *t;
	ulong n;

	if(clearesc)
		typeesc = -1;	
	if(typestart == typeend) {
		modified = 0;
		return;
	}
	t = which->user1;
	if(t != &cmd)
		modified = 1;
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

static long
cmdscrolldown(Flayer *l, long a, Text *t)
{
    flushtyping(0);
    center(l, l->origin + l->f.nchars + 1);
    return a;
}

static long
cmdscrollup(Flayer *l, long a, Text *t)
{
    flushtyping(0);
    outTsll(Torigin, t->tag, l->origin, l->f.maxlines + 1);
    return a;
}

static long
cmdcharleft(Flayer *l, long a, Text *t)
{
    flsetselect(l, a, a);
    flushtyping(0);
    if (a > 0)
        a--;
    flsetselect(l, a, a);
    center(l, a);

    return a;
}

static long
cmdcharright(Flayer *l, long a, Text *t)
{
    flsetselect(l, a, a);
    flushtyping(0);
    if (a < t->rasp.nrunes)
        a++;
    flsetselect(l, a, a);
    center(l, a);

    return a;
}

static long
cmdeol(Flayer *l, long a, Text *t)
{
    flsetselect(l, a, a);
    flushtyping(1);
    while(a < t->rasp.nrunes)
         if(raspc(&t->rasp, a++) == '\n') {
             a--;
             break;
         }

    flsetselect(l, a, a);
    center(l, a);

    return a;
}

static long
cmdbol(Flayer *l, long a, Text *t)
{
    flsetselect(l, a, a);
    flushtyping(1);
    while(a > 0)
        if(raspc(&t->rasp, --a) == '\n') {
            a++;
            break;
        }

    flsetselect(l, a, a);
    center(l, a);

    return a;
}

static long
cmdscrollupline(Flayer *l, long a, Text *t)
{
    if (l->origin > 0)
        hmoveto(t->tag, l->origin - 1);
    return a;
}

static long
cmdscrolldownline(Flayer *l, long a, Text *t)
{
    long tot = scrtotal(l);
    long p0 = l->origin + frcharofpt(&l->f, Pt(l->f.r.min.x, l->f.r.min.y + l->f.fheight));
    long p1 = l->origin + frcharofpt(&l->f, Pt(l->f.r.min.x, l->f.r.max.y - l->f.fheight / 2));

    if (p0 < tot && p1 < tot)
        horigin(t->tag, p0);

    return a;
}

static long
cmdlineup(Flayer *l, long a, Text *t)
{
    flsetselect(l, a, a);
    flushtyping(1);
    if (a > 0){
        long n0, n1, count = 0;
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

static long
cmdlinedown(Flayer *l, long a, Text *t)
{
    flsetselect(l, a, a);
    flushtyping(1);
    if (a < t->rasp.nrunes){
        long p0, count = 0;

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

static long
cmdjump(Flayer *l, long a, Text *u)
{
    Text *t = NULL;

    if (which == &cmd.l[cmd.front] && flast)
        current(flast);
    else{
        l = &cmd.l[cmd.front];
        t = (Text *)l->user1;
        flast = which;
        current(l);
        flushtyping(0);
        flsetselect(l, t->rasp.nrunes, t->rasp.nrunes);
        center(l, a);
    }

    return a;
}

static long
cmdescape(Flayer *l, long a, Text *t)
{
    if (typeesc >= 0){
        l->p0 = typeesc;
        l->p1 = a;
        flushtyping(1);
    }

    for (l = t->l; l < &t->l[NL]; l++)
        if (l->textfn)
            flsetselect(l, l->p0, l->p1);

    return a;
}

static long
cmddelword(Flayer *l, long a, Text *t)
{
    if (l->f.p0 > 0 && a > 0)
        l->p0 = ctlw(&t->rasp, l->origin, a);

    l->p1 = a;
    if (l->p1 != l->p0){
        if(typestart<=l->p0 && l->p1<=typeend){
            t->lock++;	/* to call hcut */
            hcut(t->tag, l->p0, l->p1-l->p0);
            /* hcheck is local because we know rasp is contiguous */
            hcheck(t->tag);
        }else{
            flushtyping(0);
            cut(t, t->front, 0, 1);
        }
    }

    return a;
}

static long
cmddelbol(Flayer *l, long a, Text *t)
{
    if (l->f.p0 > 0 && a > 0)
        l->p0 = ctlu(&t->rasp, l->origin, a);

    l->p1 = a;
    if (l->p1 != l->p0){
        if(typestart<=l->p0 && l->p1<=typeend){
            t->lock++;	/* to call hcut */
            hcut(t->tag, l->p0, l->p1-l->p0);
            /* hcheck is local because we know rasp is contiguous */
            hcheck(t->tag);
        }else{
            flushtyping(0);
            cut(t, t->front, 0, 1);
        }
    }

    return a;
}

static long
cmddel(Flayer *l, long a, Text *t)
{
    if (l->f.p0 > 0 && a > 0)
        l->p0 = a - 1;

    l->p1 = a;
    if (l->p1 != l->p0){
        if(typestart<=l->p0 && l->p1<=typeend){
            t->lock++;	/* to call hcut */
            hcut(t->tag, l->p0, l->p1-l->p0);
            /* hcheck is local because we know rasp is contiguous */
            hcheck(t->tag);
        }else{
            flushtyping(0);
            cut(t, t->front, 0, 1);
        }
    }

    return a;
}

static inline int
getlayer(const Flayer *l, const Text *t)
{
    int i;
    for (i = 0; i < NL; i++)
        if (&t->l[i] == l)
            return i;

    return -1;
}

static long
cmdexchange(Flayer *l, long a, Text *t)
{
    int w = getlayer(l, t);
    if (w >= 0){
        snarf(t, w);
        outT0(Tstartsnarf);
        setlock();
    }

    return a;
}

static long
cmdsnarf(Flayer *l, long a, Text *t)
{
    flushtyping(0);

    int w = getlayer(l, t);
    if (w >= 0)
        snarf(t, w);

    return a;
}

static long
cmdcut(Flayer *l, long a, Text *t)
{
    flushtyping(0);

    int w = getlayer(l, t);
    if (w >= 0)
        cut(t, w, 1, 1);

    return a;
}

static long
cmdpaste(Flayer *l, long a, Text *t)
{
    flushtyping(0);

    int w = getlayer(l, t);
    if (w >= 0)
        paste(t, w);

    return a;
}

static long
cmdwrite(Flayer *l, long a, Text *t)
{
    flushtyping(0);
    outTs(Twrite, ((Text *)l->user1)->tag);
    setlock();
    return a;
}

static long
cmdnone(Flayer *l, long a, Text *t)
{
    return a;
}

typedef long (*Commandfunc)(Flayer *, long, Text *);
typedef struct CommandEntry CommandEntry;
struct CommandEntry{
    Commandfunc f;
    int unlocked;
};

CommandEntry commands[Cmax] ={
    [Cnone]           = {cmdnone,           0},
    [Cscrolldown]     = {cmdscrolldown,     0},
    [Cscrollup]       = {cmdscrollup,       0},
    [Cscrolldownline] = {cmdscrolldownline, 0},
    [Cscrollupline]   = {cmdscrollupline,   0},
    [Ccharleft]       = {cmdcharleft,       0},
    [Ccharright]      = {cmdcharright,      0},
    [Clineup]         = {cmdlineup,         0},
    [Clinedown]       = {cmdlinedown,       0},
    [Cjump]           = {cmdjump,           0},
    [Cescape]         = {cmdescape,         0},
    [Csnarf]          = {cmdsnarf,          0},
    [Ccut]            = {cmdcut,            0},
    [Cpaste]          = {cmdpaste,          0},
    [Cexchange]       = {cmdexchange,       0},
    [Cdelword]        = {cmddelword,        1},
    [Cdelbol]         = {cmddelbol,         1},
    [Cdel]            = {cmddel,            1},
    [Cwrite]          = {cmdwrite,          1},
    [Ceol]            = {cmdeol,            0},
    [Cbol]            = {cmdbol,            0}
};

void
type(Flayer *l, int res)	/* what a bloody mess this is -- but it's getting better! */
{
	Text *t = (Text *)l->user1;
	Rune buf[100];
    Keystroke k = {0};
	Rune *p = buf;
	int backspacing, moving;
	long a;

	if(lock || t->lock){
		kbdblock();
		return;
	}

    k = qpeekc();
	a = l->p0;
    if (a != l->p1 && k.k != Kcommand){
		flushtyping(1);
		cut(t, t->front, 1, 1);
		return; /* it may now be locked */
	}

	while (((k = kbdchar()), k.c) > 0) {
        if (k.k == Kcommand)
            break;

        if (expandtabs && k.c == '\t' && k.k != Kcomposed){
            int col = 0, nspaces = 8, off = a;
            int i;
            while (off > 0 && raspc(&t->rasp, off - 1) != '\n')
                off--, col++;

            nspaces = tabwidth - col % tabwidth;
            for (i = 0; i < nspaces; i++)
                pushkbd(' ');
            break;
        }

		*p++ = k.c;
		if (k.c == '\n' || p >= buf + sizeof(buf) / sizeof(buf[0]))
			break;
	}

    if (k.k == Kcommand){
        if (k.c < 0 || k.c >= Cmax || commands[k.c].f == NULL)
            panic("command table miss");

        CommandEntry *e = &commands[k.c];
        if (!e->unlocked || !lock){
            if (k.t == Tcurrent)
                a = e->f(l, a, t);
            else{
                Flayer *lt = flwhich(k.p);
                if (lt)
                    lt->p0 = e->f(lt, lt->p0, (Text *)lt->user1);
            }
        }
    }

    if (p > buf){
        if (typestart < 0)
            typestart = a;

        if (typeesc < 0)
            typeesc = a;

        hgrow(t->tag, a, p-buf, 0);
        t->lock++;	/* pretend we Trequest'ed for hdatarune*/
        hdatarune(t->tag, a, buf, p-buf);
        a += p-buf;
        l->p0 = a;
        l->p1 = a;
        typeend = a;
        if (k.c == '\n' || typeend - typestart > 100)
            flushtyping(0);
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
            modified = 0;
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
	fprint(2, "samterm:panic: ");
	perror(s);
	abort();
}

Rune*
stgettext(Flayer *l, long n, ulong *np)
{
	Text *t;

	t = l->user1;
	rload(&t->rasp, l->origin, l->origin+n, np);
	return scratch;
}

long
scrtotal(Flayer *l)
{
	return ((Text *)l->user1)->rasp.nrunes;
}

void*
alloc(ulong n)
{
	void *p;

	p = malloc(n);
	if(p == 0)
		panic("alloc");
	memset(p, 0, n);
	return p;
}
