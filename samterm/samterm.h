/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#define SAMTERM

#define RUNESIZE    sizeof(wchar_t)
#define MAXFILES    256
#define NL  5

enum{
    Cescape = Csysmax + 1, /* highlight recently typed text */
    Cscrolldown,           /* scroll file down by screen */
    Cscrollup,             /* scroll file up by screen */
    Cscrolldownline,       /* scroll file down by line */
    Cscrollupline,         /* scroll file up by line */
    Cjump,                 /* jump to/from command file */
    Ccharright,            /* move dot right by character */
    Ccharleft,             /* move dot left by character */
    Clinedown,             /* move dot down by line */
    Clineup,               /* move dot up by line */
    Cdelword,              /* delete word to left of dot */
    Cdelbol,               /* delete to beginning of line */
    Cdelbs,                /* delete character to left of dot */
    Cdel,                  /* delete character to right of dot */
    Csnarf,                /* snarf dot */
    Ccut,                  /* cut dot */
    Cpaste,                /* paste from snarf buffer */
    Cexchange,             /* exchange snarf buffer with OS */
    Ceol,                  /* move to beginning of line */
    Cbol,                  /* move to end of line */
    Ctab,                  /* insert a possibly expanded tab */
    Csend,                 /* send a command to the editor */
    Cwrite,                /* write the current file */
    Clook,                 /* literal search */
    Csearch,               /* search for regex again */
    Cmax                   /* invalid command */
};

enum{
    Up,
    Down
};

typedef struct Text Text;
typedef struct Section  Section;
typedef struct Rasp Rasp;

struct Section
{
    int64_t    nrunes;
    wchar_t    *text;      /* if null, we haven't got it */
    Section *next;
};

struct Rasp
{
    int64_t    nrunes;
    Section *sect;
};

#define Untagged    ((uint16_t)65535)

struct Text
{
    Rasp    rasp;
    int16_t   nwin;
    int16_t   front;      /* input window */
    uint16_t  tag;
    char    lock;
    Flayer  l[NL];      /* screen storage */
};

enum Resource
{
    Eextern     = 0x08,
    Ehost       = 0x04,
    RHost       = Ehost,
    RExtern     = Eextern,
    RKeyboard   = Ekeyboard,
    RMouse      = Emouse
};

extern Text *text[];
extern uint8_t    *name[];
extern uint16_t   tag[];
extern int  nname;
extern unsigned int cursor;
extern Flayer   *which;
extern Flayer   *work;
extern Text cmd;
extern wchar_t *scratch;
extern int64_t nscralloc;
extern char lock;
extern bool hasunlocked;
extern int64_t snarflen;
extern Mouse    mouse;
extern bool modified;
extern bool followfocus;

wchar_t    *stgettext(Flayer*, int64_t, uint64_t*);
void    *alloc(uint64_t n);

void    iconinit(void);
void    getscreen(int, char**);
void    initio(void);
void    setlock(void);
void    outcmd(void);
void    rinit(Rasp*);
void    startnewfile(int, Text*);
void    cursorset(Point);
void    getmouse(void);
void    mouseunblock(void);
void    kbdblock(void);
int button(int but);
int waitforio(void);
int rcvchar(void);
int getch(void);
Keystroke qpeekc(void);
Keystroke   kbdchar(void);
void    mouseexit(void);
void    cut(Text*, int, bool, bool);
void    paste(Text*, int);
void    snarf(Text*, int);
int center(Flayer*, int64_t);
int xmenuhit(int, Menu*);
void    buttons(int);
int getr(Rectangle*);
void    current(Flayer*);
void    duplicate(Flayer*, Rectangle, XftFont*, int);
void    startfile(Text*);
void    panic(char*);
void    closeup(Flayer*);
void    Strgrow(wchar_t**, int64_t*, int);
int RESHAPED(void);
void    reshape(void);
void    rcv(void);
void    type(Flayer*);
void    menu2hit(void);
void    menu3hit(void);
void    scroll(Flayer*, int, int);
void    hcheck(int);
void    rclear(Rasp*);
int whichmenu(int);
void    hcut(int, int64_t, int64_t);
void    horigin(int, int64_t, Flayer *);
void    hgrow(int, int64_t, int64_t, bool);
int hdata(int, int64_t, uint8_t*, int);
int hdatarune(int, int64_t, wchar_t*, int);
wchar_t    *rload(Rasp*, uint64_t, uint64_t, uint64_t*);
void    menuins(int, uint8_t*, Text*, int, int);
void    menudel(int);
Text    *sweeptext(int, int);
void    setpat(char*);
bool    haspat(void);
void    scrdraw(Flayer*, int64_t tot);
int rcontig(Rasp*, uint64_t, uint64_t, bool);
int rmissing(Rasp*, uint64_t, uint64_t);
void    rresize(Rasp *, int64_t, int64_t, int64_t);
void    rdata(Rasp*, int64_t, int64_t, wchar_t*);
void    rclean(Rasp*);
void    scrorigin(Flayer*, int, int64_t);
int64_t    scrtotal(Flayer*);
void    flnewlyvisible(Flayer*);
char    *rcvstring(void);
void    Strcpy(wchar_t*, wchar_t*);
void    Strncpy(wchar_t*, wchar_t*, int64_t);
void    flushtyping(bool);
void    dumperrmsg(int, int, int, int);
int screensize(int*,int*);

#include "../sam/mesg.h"

void    outTs(Tmesg, int);
void    outT0(Tmesg);
void    outTl(Tmesg, int64_t);
void    outTslS(Tmesg, int, int64_t, wchar_t*);
void    outTslll(Tmesg, int, int64_t, int64_t, int64_t);
void    outTsll(Tmesg, int, int64_t, int64_t);
void    outTsl(Tmesg, int, int64_t);
void    outTv(Tmesg, void*);
void    outstart(Tmesg);
void    outcopy(int, uint8_t*);
void    outshort(int);
void    outlong(int64_t);
void    outsend(void);
int getlayer(const Flayer *l, const Text *t);
void loadrcfile(FILE *);
void installdefaultbindings(void);
void installdefaultchords(void);
