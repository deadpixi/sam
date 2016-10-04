/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include <frame.h>
#include "flayer.h"
#include "samterm.h"

#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

char exname[PATH_MAX + 1] = {0};
static char *fallbacks[] = {
    "*scrollForwardR: true",
    "*geometry: 740x780",
    NULL
};

extern int nofifo;

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

void
removeextern(void)
{
    unlink(exname);
    exname[0] = 0;
}

void
extstart(void)
{
    extern char *machine;
    int fd;
    int flags;

    if (nofifo || !getenv("HOME"))
        return;

    snprintf(exname, PATH_MAX, "%s/.sam.%s", getenv("HOME"), machine);

    /* Make the named pipe. */
    if (mkfifo(exname, 0600) == -1){
        struct stat statb;

        if (errno != EEXIST || stat(exname, &statb) == -1)
            return;

        if (!S_ISFIFO(statb.st_mode)){
            removeextern();
            if (mkfifo(exname, 0600) == -1)
                return;
        }
    }

    fd = open(exname, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        removeextern();
        return;
    }

    /*
     * Turn off no-delay and provide ourselves as a lingering
     * writer so as not to get end of file on read.
     */ 
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1 || fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) == -1
            || open(exname, O_WRONLY) == -1){
        (void)close(fd);
        removeextern();
        return;
    }

    estart(Eextern, fd, 8192);
    atexit(removeextern);
}
