#ifndef _COMMANDS_H
#define _COMMANDS_H

enum{
    Knone,
    Kraw,
    Kcomposed,
    Kcommand
};

enum{
    Cescape,
    Cscrolldown,
    Cscrollup,
    Cjump,
    Ccharright,
    Ccharleft,
    Clinedown,
    Clineup,
    Cdelword,
    Cdelbol,
    Cdel,
    Cmax
}; /* virtual command keystrokes */

#endif
