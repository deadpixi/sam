/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libc.h>
#include <libg.h>
#include <frame.h>
#include "flayer.h"
#include "samterm.h"

#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

#ifdef APOLLO
#define O_NONBLOCK  O_NDELAY
#endif

#if defined(UMIPS) || defined(SUNOS)
#define atexit(p)       /* sigh */
#endif

char *exname = NULL;
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
    if (exname) {
        (void)unlink(exname);
        exname = 0;
    }
}
/*
 *  some systems do not support non-blocking i/o on named pipes
 *  or do not provide working POSIX interfaces to the pipes.
 *  in that case, add the name of the system to the 'ifdef' that
 *  disables the code at the beginning of the function.
 *  The external 'B' command will not work.
 */

void
extstart(void)
{
    if (nofifo)
        return;

#ifndef NOFIFO
    extern char *machine;
    char    *user;
    char    *home;
    int fd;
    int flags;

    user = getenv("LOGNAME") ? getenv("LOGNAME") : getenv("USER") ? getenv("USER") : "unknown";
    home = getenv("HOME");

    if (home == NULL)
        return;

    exname = (char *)alloc(4 + 6 + strlen(home) + 1 + strlen(user) + 1 + strlen(machine) + 100);
    sprint(exname, "%s/.sam.%s", home, machine);

    /* Make the named pipe. */
    if (mkfifo(exname, 0600) == -1) {
        struct stat statb;
        extern int  errno;

        if (errno != EEXIST || stat(exname, &statb) == -1)
            return;

        if (!S_ISFIFO(statb.st_mode)) {
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
            || open(exname, O_WRONLY) == -1) {
        (void)close(fd);
        removeextern();
        return;
    }

    estart(Eextern, fd, 8192);
    atexit(removeextern);
#endif
}

/*
 *  we have to supply a dummy exit function, because some vendors can't be
 *  bothered to provide atexit().  we clean up the named pipes on a normal
 *  exit, but leave them laying around on abnormal exits.
 */
void
exits(char *message)
{
    if (exname) {
        unlink(exname);
        exname = 0;
    }
    if (message == 0)
        exit (0);
    else
        exit(1);
}
