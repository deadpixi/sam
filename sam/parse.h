#include <u.h>

/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
typedef struct Addr Addr;
typedef struct Cmd Cmd;
struct Addr
{
    char    type;   /* # (char addr), l (line addr), / ? . $ + - , ; */
    union{
        String  *re;
        Addr    *aleft;     /* left side of , and ; */
    } g;
    Posn    num;
    Addr    *next;          /* or right side of , and ; */
};

#define are g.re
#define left    g.aleft

struct Cmd
{
    Addr    *addr;          /* address (range of text) */
    String  *re;            /* regular expression for e.g. 'x' */
    union{
        Cmd *cmd;       /* target of x, g, {, etc. */
        String  *text;      /* text of a, c, i; rhs of s */
        Addr    *addr;      /* address for m, t */
    } g;
    Cmd *next;          /* pointer to next element in {} */
    int16_t   num;
    uint16_t  flag;           /* whatever */
    uint16_t  cmdc;           /* command character; 'x' etc. */
};

#define ccmd    g.cmd
#define ctext   g.text
#define caddr   g.addr

extern struct cmdtab{
    uint16_t  cmdc;       /* command character */
    uint8_t   text;       /* takes a textual argument? */
    uint8_t   regexp;     /* takes a regular expression? */
    uint8_t   addr;       /* takes an address (m or t)? */
    uint8_t   defcmd;     /* default command; 0==>none */
    uint8_t   defaddr;    /* default address */
    uint8_t   count;      /* takes a count e.g. s2/// */
    wchar_t *token;     /* takes text terminated by one of these */
    bool      keepslash;   /* pass slashes in unchanged */
    bool (*fn)(File*, Cmd*); /* function to call with parse tree */
}cmdtab[];

enum Defaddr{   /* default addresses */
    aNo,
    aDot,
    aAll
};

bool nl_cmd(File*, Cmd*), a_cmd(File*, Cmd*), b_cmd(File*, Cmd*);
bool c_cmd(File*, Cmd*), cd_cmd(File*, Cmd*), d_cmd(File*, Cmd*);
bool D_cmd(File*, Cmd*), e_cmd(File*, Cmd*);
bool f_cmd(File*, Cmd*), g_cmd(File*, Cmd*), i_cmd(File*, Cmd*);
bool k_cmd(File*, Cmd*), m_cmd(File*, Cmd*), n_cmd(File*, Cmd*);
bool p_cmd(File*, Cmd*), q_cmd(File*, Cmd*);
bool P_cmd(File*, Cmd*), P_cmd(File*, Cmd*);
bool s_cmd(File*, Cmd*), u_cmd(File*, Cmd*), w_cmd(File*, Cmd*);
bool x_cmd(File*, Cmd*), X_cmd(File*, Cmd*), plan9_cmd(File*, Cmd*);
bool eq_cmd(File*, Cmd*);


String  *getregexp(int);
Addr    *newaddr(void);
Address address(Addr*, Address, int);
int cmdexec(File*, Cmd*);
