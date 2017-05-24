/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include <frame.h>
#include "flayer.h"
#include "samterm.h"

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>
#include <signal.h>

static char *fallbacks[] = {
    "*scrollForwardR: true",
    "*geometry: 740x780",
    NULL
};

void
getscreen(int argc, char **argv)
{
    Rectangle r;

    signal(SIGINT, SIG_IGN);
    xtbinit(0, "Sam", &argc, argv, fallbacks);
    r = inset(screen.r, 4);
    bitblt(&screen, r.min, &screen, r, 0);
}

int
screensize(int *w, int *h)
{
    return scrpix(w,h);
}

void
dumperrmsg(int count, int type, int count0, int c)
{
    uint8_t *cp;
    int i;

    cp = (uint8_t *) rcvstring();
    fprintf(stderr, "samterm: host mesg: count %d %ux %ux %ux %s...ignored\n",
        count, type, count0, c, cp);
    i = 0;
    while (*cp) {
        fprintf(stderr, "%x ", *cp);
        if (i++ >= 20) {
            fprintf(stderr, "\n");
            i = 0;
        }
        cp++;
    }
}
