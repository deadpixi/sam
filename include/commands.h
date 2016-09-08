#ifndef _COMMANDS_H
#define _COMMANDS_H

enum{
    Knone,      /* invalid command type */
    Kdefault,   /* perform default command action */
    Kraw,       /* insert raw character code, subject to transformation (e.g. tab expansion) */
    Kcomposed,  /* insert composed character code */
    Kcommand,   /* execute command (see below) */
    Kend        /* mark the end of a command list */
};

enum{
    Cnone,              /* invalid command */
    Cescape,            /* highlight recently typed text */
    Cscrolldown,        /* scroll file down by screen */
    Cscrollup,          /* scroll file up by screen */
    Cscrolldownline,    /* scroll file down by line */
    Cscrollupline,      /* scroll file up by line */
    Cjump,              /* jump to/from command file */
    Ccharright,         /* move dot right by character */
    Ccharleft,          /* move dot left by character */
    Clinedown,          /* move dot down by line */
    Clineup,            /* move dot up by line */
    Cdelword,           /* delete word to left of dot */
    Cdelbol,            /* delete to beginning of line */
    Cdel,               /* delete character to left of dot */
    Csnarf,             /* snarf dot */
    Ccut,               /* cut dot */
    Cpaste,             /* paste from snarf buffer */
    Cexchange,          /* exchange snarf buffer with OS */
    Cwrite,             /* write file */
    Ceol,               /* move to beginning of line */
    Cbol,               /* move to end of line */
    Cmax                /* invalid command */
};

enum{
    Tcurrent,   /* command is sent to focused layer */
    Tmouse      /* command is sent to layer containing the mouse */
};

#endif
