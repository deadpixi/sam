#ifndef _COMMANDS_H
#define _COMMANDS_H

enum{
    Knone,
    Kraw,
    Kcomposed,
    Kcommand,
    Kend
};

enum{
    Cnone,
    Cescape,
    Cscrolldown,
    Cscrollup,
    Cscrolldownline,
    Cscrollupline,
    Cjump,
    Ccharright,
    Ccharleft,
    Clinedown,
    Clineup,
    Cdelword,
    Cdelbol,
    Cdel,
    Csnarf,
    Ccut,
    Cpaste,
    Cexchange,
    Cwrite,
    Ceol,
    Cbol,
    Cmax
}; /* virtual command keystrokes */

#endif
