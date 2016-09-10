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

extern int expandtabs;
extern int tabwidth;

typedef struct Namemapping Namemapping;
struct Namemapping{
    const char *name;
    int value;
};

static Namemapping commandmapping[] ={
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
    {"del",             Cdel},
    {"snarf",           Csnarf},
    {"cut",             Ccut},
    {"paste",           Cpaste},
    {"exchange",        Cexchange},
    {"write",           Cwrite},
    {"eol",             Ceol},
    {"bol",             Cbol},
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

static int
nametocommand(const char *n)
{
    return lookupmapping(n, commandmapping);
}

static int
nametotarget(const char *n)
{
    return lookupmapping(n, targetmapping);
}

typedef struct Defaultbinding Defaultbinding;
struct Defaultbinding{
    int modifiers;
    KeySym keysym;
    int kind;
    int command;
};

static Defaultbinding defaultbindings[] ={    
    /* Motion commands. */
    {ControlMask, XK_e,             Kcommand,  Clineup},
    {ControlMask, XK_x,             Kcommand,  Clinedown},
    {ControlMask, XK_d,             Kcommand,  Ccharright},
    {ControlMask, XK_s,             Kcommand,  Ccharleft},
    {ControlMask, XK_u,             Kcommand,  Cdelbol},
    {ControlMask, XK_w,             Kcommand,  Cdelword},
    {ControlMask, XK_k,             Kcommand,  Cjump},
    {ControlMask, XK_BackSpace,     Kcommand,  Cdelword},
    {ControlMask, XK_y,             Kcommand,  Ccut},
    {ControlMask, XK_c,             Kcommand,  Csnarf},
    {ControlMask, XK_v,             Kcommand,  Cpaste},
    {ControlMask, XK_q,             Kcommand,  Cexchange},

    /* Handle arrow keys, page up/down, and escape. */
    {0,           XK_Up,            Kcommand, Cscrollup},
    {0,           XK_Prior,         Kcommand, Cscrollup},
    {0,           XK_Left,          Kcommand, Cscrollup},
    {0,           XK_Down,          Kcommand, Cscrolldown},
    {0,           XK_Next,          Kcommand, Cscrolldown},
    {0,           XK_Right,         Kcommand, Cscrolldown},
    {0,           XK_Escape,        Kcommand, Cescape},
    
    /* More fundamental stuff: backspace, delete, etc. */
    {0,           XK_BackSpace,     Kcommand, Cdel},
    {0,           XK_Delete,        Kcommand, Cdel},
    {0,           XK_Return,        Kraw,     '\n'},
    {0,           XK_KP_Enter,      Kraw,     '\n'},
    {0,           XK_Linefeed,      Kraw,     '\r'},
    {0,           XK_Tab,           Kraw,     '\t'},
    {0,           XK_KP_0,          Kraw,     '0'},
    {0,           XK_KP_1,          Kraw,     '1'},
    {0,           XK_KP_2,          Kraw,     '2'},
    {0,           XK_KP_3,          Kraw,     '3'},
    {0,           XK_KP_4,          Kraw,     '4'},
    {0,           XK_KP_5,          Kraw,     '5'},
    {0,           XK_KP_6,          Kraw,     '6'},
    {0,           XK_KP_7,          Kraw,     '7'},
    {0,           XK_KP_8,          Kraw,     '8'},
    {0,           XK_KP_9,          Kraw,     '9'},
    {0,           XK_KP_Divide,     Kraw,     '/'},
    {0,           XK_KP_Multiply,   Kraw,     '*'},
    {0,           XK_KP_Subtract,   Kraw,     '-'},
    {0,           XK_KP_Add,        Kraw,     '+'},
    {0,           XK_KP_Decimal,    Kraw,     '.'},
    {0,           XK_hyphen,        Kraw,     '-'},

    /* Use Control-Tab to insert a literal tab when tab expansion is enabled. */
    {ControlMask, XK_Tab,           Kcomposed, '\t'},

    {0,           0,                Kend,     0}
};

void
installdefaultbindings(void)
{
    for (Defaultbinding *b = defaultbindings; b->kind != Kend; b++)
        installbinding(b->modifiers, b->keysym, b->kind, b->command);
}

typedef struct Defaultchord Defaultchord;
struct Defaultchord{
    int state1;
    int state2;
    int command;
    int target;
};

static Defaultchord defaultchords[] ={
    {B1, B1|B2,  Ccut,   Tcurrent},
    {B1, B1|B3,  Cpaste, Tcurrent},
    {B1|B2, B1,  Cnone,  Tcurrent},
    {B1|B3, B1,  Cnone,  Tcurrent},

    {B4, 0,  Cscrollupline,   Tmouse},
    {B5, 0,  Cscrolldownline, Tmouse},

    {0, 0, Kend, 0}
};

void
installdefaultchords(void)
{
    for (Defaultchord *c = defaultchords; c->state1 != 0; c++)
        installchord(c->state1, c->state2, c->command, c->target);
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

void
loadrcfile(FILE *f)
{
    char *l = NULL;
    size_t n = 0;
    ssize_t r = 0;
    size_t ln = 0;

    while ((r = getline(&l, &n, f)) >= 0){
        char s1[6] = {0};
        char s2[6] = {0};
        char cname[1024] = {0};
        char tname[1024] = {0};
        char c = 0;
        unsigned short s = 0;
        int rc = 0;
        int i = 0;

        ln++;
        if (r == 0 || l[0] == '\n' || l[0] == 0 || sscanf(l, " %[#]", &c) == 1)
            continue;

        if (sscanf(l, " chord %5[Nn12345] %5[Nn12345] %99s %99s", s1, s2, cname, tname) == 4)
            rc = installchord(statetomask(s1, buttonmapping), statetomask(s2, buttonmapping), nametocommand(cname), nametotarget(tname));
        else if (sscanf(l, " bind %5[*camshNCAMSH12345] %99s raw 0x%hx", s1, s2, &s) == 3)
            rc = installbinding(statetomask(s1, modmapping), XStringToKeysym(s2), Kraw, i);
        else if (sscanf(l, " bind %5[*camshNCAMSH12345] %99s composed 0x%hx", s1, s2, &s) == 3)
            rc = installbinding(statetomask(s1, modmapping), XStringToKeysym(s2), Kcomposed, i);
        else if (sscanf(l, " bind %5[*camshNCAMSH12345] %99s raw %c", s1, s2, &c) == 3)
            rc = installbinding(statetomask(s1, modmapping), XStringToKeysym(s2), Kraw, c);
        else if (sscanf(l, " bind %5[*camshNCAMSH12345] %99s composed %c", s1, s2, &c) == 3)
            rc = installbinding(statetomask(s1, modmapping), XStringToKeysym(s2), Kcomposed, c);
        else if (sscanf(l, " bind %5[*camshNCAMSH12345] %99s command %99s", s1, s2, cname) == 3)
            rc = installbinding(statetomask(s1, modmapping), XStringToKeysym(s2), Kcommand, nametocommand(cname));
        else if (sscanf(l, " foreground %1023s", cname) == 1)
            strncpy(foregroundspec, cname, sizeof(foregroundspec) - 1);
        else if (sscanf(l, " background %1023s", cname) == 1)
            strncpy(backgroundspec, cname, sizeof(backgroundspec) - 1);
        else if (sscanf(l, " border %1023s", cname) == 1)
            strncpy(borderspec, cname, sizeof(borderspec) - 1);
        else if (sscanf(l, " font %1023s", cname) == 1)
            strncpy(fontspec, cname, sizeof(fontspec) - 1);
        else if (sscanf(l, " tabs %hu", &s) == 1 && s < 12 && s > 0)
            tabwidth = s;
        else if (sscanf(l, " expandtabs%n", &i) == 0 && i)
            expandtabs = 1;
        else
            fprintf(stderr, "invalid rc line %zd\n", ln);

        if (rc != 0)
            fprintf(stderr, "invalid chord/binding on rc line %zd\n", ln);
    }

    free(l);
}
