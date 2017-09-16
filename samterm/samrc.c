#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <u.h>
#include <libg.h>
#include <frame.h>
#include "flayer.h"
#include "samterm.h"

extern bool expandtabs;
extern int tabwidth;
extern bool autoindent;

typedef struct Namemapping Namemapping;
struct Namemapping{
    const char *name;
    int value;
};

static Namemapping commandmapping[] ={
    {"none",            Cnone},
    {"default",         Cdefault},
    {"escape",          Cescape},
    {"scrolldown",      Cscrolldown},
    {"scrollup",        Cscrollup},
    {"scrolldownline",  Cscrolldownline},
    {"scrollupline",    Cscrollupline},
    {"jump",            Cjump},
    {"charright",       Ccharright},
    {"charleft",        Ccharleft},
    {"linedown",        Clinedown},
    {"lineup",          Clineup},
    {"delword",         Cdelword},
    {"delbol",          Cdelbol},
    {"delbs",           Cdelbs},
    {"del",             Cdel},
    {"snarf",           Csnarf},
    {"cut",             Ccut},
    {"paste",           Cpaste},
    {"exchange",        Cexchange},
    {"eol",             Ceol},
    {"bol",             Cbol},
    {"tab",             Ctab},
    {"send",            Csend},
    {"look",            Clook},
    {"search",          Csearch},
    {"write",           Cwrite},
    {NULL, 0}
};

static Namemapping targetmapping[] ={
    {"current",     Tcurrent},
    {"mouse",       Tmouse},
    {NULL, 0}
};

static Namemapping buttonmapping[] ={
    {"0",  0}, 
    {"n",  0}, 
#define B1 1
    {"1",  1}, 
#define B2 2
    {"2",  2}, 
#define B3 4
    {"3",  4}, 
#define B4 8
    {"4",  8}, 
#define B5 16
    {"5", 16}, 
    {NULL, 0}
};

static Namemapping modmapping[] ={
    {"*", 0},
    {"c", ControlMask}, 
    {"a", Mod1Mask},
    {"s", ShiftMask},
    {NULL, 0}
};

static int
lookupmapping(const char *n, Namemapping *m)
{
    for (Namemapping *k = m; k->name != NULL; k++){
        if (strcasecmp(k->name, n) == 0)
            return k->value;
    }

    return -1;
}

#define nametocommand(n) lookupmapping(n, commandmapping)
#define nametotarget(n) lookupmapping(n, targetmapping)

typedef struct Defaultbinding Defaultbinding;
struct Defaultbinding{
    int modifiers;
    KeySym keysym;
    int kind;
    int command;
    const char *arg;
};

static Defaultbinding defaultbindings[] ={    
    /* Suppress control key combinations unless explicitly bound. */
    {ControlMask, XK_VoidSymbol,    Kcommand, Cnone,        NULL},

    /* Motion commands following the WordStar diamond. */
    {ControlMask, XK_e,             Kcommand,  Clineup,     NULL},
    {ControlMask, XK_x,             Kcommand,  Clinedown,   NULL},
    {ControlMask, XK_d,             Kcommand,  Ccharright,  NULL},
    {ControlMask, XK_s,             Kcommand,  Ccharleft,   NULL},
    {ControlMask, XK_u,             Kcommand,  Cdelbol,     NULL},
    {ControlMask, XK_w,             Kcommand,  Cdelword,    NULL},
    {ControlMask, XK_k,             Kcommand,  Cjump,       NULL},
    {ControlMask, XK_BackSpace,     Kcommand,  Cdelword,    NULL},
    {ControlMask, XK_y,             Kcommand,  Ccut,        NULL},
    {ControlMask, XK_c,             Kcommand,  Csnarf,      NULL},
    {ControlMask, XK_v,             Kcommand,  Cpaste,      NULL},
    {ControlMask, XK_q,             Kcommand,  Cexchange,   NULL},

    /* Handle arrow keys, page up/down, and escape. */
    {0,           XK_Up,            Kcommand, Cscrollup,    NULL},
    {0,           XK_Prior,         Kcommand, Cscrollup,    NULL},
    {0,           XK_Left,          Kcommand, Cscrollup,    NULL},
    {0,           XK_Down,          Kcommand, Cscrolldown,  NULL},
    {0,           XK_Next,          Kcommand, Cscrolldown,  NULL},
    {0,           XK_Right,         Kcommand, Cscrolldown,  NULL},
    {0,           XK_Escape,        Kcommand, Cescape,      NULL},
    
    /* More fundamental stuff: backspace, delete, etc. */
    {0,           XK_BackSpace,     Kcommand, Cdelbs,   NULL},
    {0,           XK_Delete,        Kcommand, Cdel,     NULL},
    {0,           XK_Tab,           Kcommand, Ctab,     NULL},
    {0,           XK_Return,        Kraw,     '\n',     NULL},
    {0,           XK_KP_Enter,      Kraw,     '\n',     NULL},
    {0,           XK_Linefeed,      Kraw,     '\r',     NULL},
    {0,           XK_KP_0,          Kraw,     '0',      NULL},
    {0,           XK_KP_1,          Kraw,     '1',      NULL},
    {0,           XK_KP_2,          Kraw,     '2',      NULL},
    {0,           XK_KP_3,          Kraw,     '3',      NULL},
    {0,           XK_KP_4,          Kraw,     '4',      NULL},
    {0,           XK_KP_5,          Kraw,     '5',      NULL},
    {0,           XK_KP_6,          Kraw,     '6',      NULL},
    {0,           XK_KP_7,          Kraw,     '7',      NULL},
    {0,           XK_KP_8,          Kraw,     '8',      NULL},
    {0,           XK_KP_9,          Kraw,     '9',      NULL},
    {0,           XK_KP_Divide,     Kraw,     '/',      NULL},
    {0,           XK_KP_Multiply,   Kraw,     '*',      NULL},
    {0,           XK_KP_Subtract,   Kraw,     '-',      NULL},
    {0,           XK_KP_Add,        Kraw,     '+',      NULL},
    {0,           XK_KP_Decimal,    Kraw,     '.',      NULL},
    {0,           XK_hyphen,        Kraw,     '-',      NULL},

    /* Support traditional control sequences. */
    {ControlMask, XK_bracketleft,   Kcommand, Cescape,  NULL},
    {ControlMask, XK_h,             Kcommand, Cdelbs,   NULL},
    {ControlMask, XK_Delete,        Kcommand, Cdel,     NULL},
    {ControlMask, XK_i,             Kcommand, Ctab,     NULL},
    {ControlMask, XK_j,             Kraw,     '\n',     NULL},
    {ControlMask, XK_m,             Kraw,     '\r',     NULL},

    /* Use Control-Tab to insert a literal tab when tab expansion is enabled. */
    {ControlMask, XK_Tab,           Kraw,     '\t',     NULL},

    {0,           0,                Kend,     0,        NULL}
};

void
installdefaultbindings(void)
{
    for (Defaultbinding *b = defaultbindings; b->kind != Kend; b++)
        installbinding(b->modifiers, b->keysym, b->kind, b->command, b->arg);
}

typedef struct Defaultchord Defaultchord;
struct Defaultchord{
    int state1;
    int state2;
    int command;
    int target;
    const char *arg;
};

static Defaultchord defaultchords[] ={
    {B1, B1|B2,  Ccut,   Tcurrent, NULL},
    {B1, B1|B3,  Cpaste, Tcurrent, NULL},
    {B1|B2, B1,  Cnone,  Tcurrent, NULL},
    {B1|B3, B1,  Cnone,  Tcurrent, NULL},

    {B4, 0,  Cscrollupline,   Tmouse, NULL},
    {B5, 0,  Cscrolldownline, Tmouse, NULL},

    {0, 0, Kend, 0, NULL}
};

void
installdefaultchords(void)
{
    for (Defaultchord *c = defaultchords; c->state1 != 0; c++)
        installchord(c->state1, c->state2, c->command, c->target, c->arg);
}

static int
statetomask(const char *n, Namemapping *m)
{
    int r = 0;
    for (int i = 0; n[i] != 0; i++){
        char s[2] = {n[i], 0};
        int v = lookupmapping(s, m);
        if (v < 0)
            return -1;
        r |= v;
    }

    return r;
}

#define buttontomask(n) statetomask(n, buttonmapping)
#define modtomask(n) statetomask(n, modmapping)

static KeySym
nametokeysym(const char *n)
{
    KeySym k, l, u;

    k = XStringToKeysym(n);
    XConvertCase(k, &l, &u);
    return l;
}

static int
dirfollowfocus(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    if (strcasecmp(s1, "true") != 0 && strcasecmp(s1, "false") != 0)
        return -1;

    followfocus = (strcasecmp(s1, "true") == 0);
    return 0;
}

static int
dirsnarfselection(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    extern const char *clipatom;

    if (strcasecmp(s1, "primary") == 0)
        clipatom = "PRIMARY";
    else if (strcasecmp(s1, "secondary") == 0)
        clipatom = "SECONDARY";
    else if (strcasecmp(s1, "clipboard") == 0)
        clipatom = "CLIPBOARD";
    else
        return -1;

    return 0;
}

static int
dirchord(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    return installchord(buttontomask(s1), buttontomask(s2), nametocommand(s3), nametotarget(s4), s5);
}

static int
dirraw(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    return installbinding(modtomask(s1), nametokeysym(s2), Kraw, strtol(s3, NULL, 16), NULL);
}

static int
dirrawliteral(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    if (strlen(s3) != 1)
        return -1;
    return installbinding(modtomask(s1), nametokeysym(s2), Kraw, s3[0], NULL);
}

static int
dirbind(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    return installbinding(modtomask(s1), nametokeysym(s2), Kcommand, nametocommand(s3), s4);
}

static int
dirunbind(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    return removebinding(modtomask(s1), nametokeysym(s2));
}

static int
dirunchord(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    return removechord(buttontomask(s1), buttontomask(s2));
}

static int
dirforeground(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    if (strlen(s1) == 0)
        return -1;

    strncpy(foregroundspec, s1, sizeof(foregroundspec) - 1);
    return 0;
}

static int
dirbackground(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    if (strlen(s1) == 0)
        return -1;

    strncpy(backgroundspec, s1, sizeof(backgroundspec) - 1);
    return 0;
}

static int
dirborder(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    if (strlen(s1) == 0)
        return -1;

    strncpy(borderspec, s1, sizeof(borderspec) - 1);
    return 0;
}

static int
dirfont(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    if (strlen(s1) == 0)
        return -1;

    strncpy(fontspec, s1, sizeof(fontspec) - 1);
    return 0;
}

static int
dirtabs(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    int i = atoi(s1);
    if (i <= 0 || i > 12)
        return -1;

    tabwidth = i;
    return 0;
}

static int
direxpandtabs(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    if (strcasecmp(s1, "true") != 0 && strcasecmp(s1, "false") != 0)
        return -1;

    expandtabs = (strcasecmp(s1, "true") == 0);
    return 0;
}

static int
dirautoindent(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    if (strcasecmp(s1, "true") != 0 && strcasecmp(s1, "false") != 0)
        return -1;

    autoindent = (strcasecmp(s1, "true") == 0);
    return 0;
}

static int
dircomment(const char *s1, const char *s2, const char *s3, const char *s4, const char *s5)
{
    return 0;
}

typedef struct Directive Directive;
struct Directive{
    const char *format;
    int result;
    int (*action)(const char *, const char *, const char *, const char *, const char *);
};

Directive directives[] ={
    {" chord %5[Nn12345] %5[Nn12345] %99s %99s %1023[^\n]",       5,   dirchord},
    {" chord %5[Nn12345] %5[Nn12345] %99s %99s",                  4,   dirchord},
    {" unchord %5[Nn12345] %5[Nn12345]",                          2,   dirunchord},
    {" bind %5[*camshNCAMSH12345] %99s raw 0x%4[0-9a-fA-F]",      3,   dirraw},
    {" bind %5[*camshNCAMSH12345] %99s raw %1s",                  3,   dirrawliteral},
    {" bind %5[*camshNCAMSH12345] %99s command %99s %1023[^\n]",  4,   dirbind},
    {" bind %5[*camshNCAMSH12345] %99s command %99s",             3,   dirbind},
    {" unbind %5[*camshNCAMSH12345] %99s",                        2,   dirunbind},
    {" foreground %1023s",                                        1,   dirforeground},
    {" background %1023s",                                        1,   dirbackground},
    {" border %1023s",                                            1,   dirborder},
    {" font %1023[^\n]",                                          1,   dirfont},
    {" tabs %2[0-9]",                                             1,   dirtabs},
    {" expandtabs %99s",                                          1,   direxpandtabs},
    {" autoindent %99s",                                          1,   dirautoindent},
    {" snarfselection %99s",                                      1,   dirsnarfselection},
    {" followfocus %99s",                                         1,   dirfollowfocus},
    {" %1[#]",                                                    1,   dircomment},
    {" %1[^ ]",                                                   EOF, dircomment},
    {NULL, 0, NULL}
};

void
loadrcfile(FILE *f)
{
    char *l = NULL;
    size_t n = 0;
    ssize_t r = 0;
    size_t ln = 0;

    while ((r = getline(&l, &n, f)) >= 0){
        char s1[1024] = {0};
        char s2[1024] = {0};
        char s3[1024] = {0};
        char s4[1024] = {0};
        char s5[1024] = {0};
        int rc = 0;
        bool found = false;

        ln++;
        if (r == 0)
            continue;

        for (Directive *d = directives; d->format && !found; d++){
            if (sscanf(l, d->format, s1, s2, s3, s4, s5) == d->result){
                rc = d->action(s1, s2, s3, s4, s5);
                found = true;
            }
        }

        if (!found)
            fprintf(stderr, "invalid rc line %zd\n", ln);

        if (rc != 0)
            fprintf(stderr, "invalid chord/binding on rc line %zd\n", ln);
    }

    free(l);
}
