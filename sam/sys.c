/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */

#include <errno.h>
#include <stdbool.h>

#include "sam.h"

static bool inerror = false;

#define ERRLEN 63

/*
 * A reasonable interface to the system calls
 */

void
resetsys(void)
{
    inerror = false;
}

void
syserror(char *a)
{
    char buf[ERRLEN + 1] = {0};

    if(!inerror){
        inerror = true;
        strncpy(buf, strerror(errno), ERRLEN);
        dprint(L"%s: ", a);
        error_s(Eio, buf);
    }
}

int
Read(int f, void *a, int n)
{
    char buf[ERRLEN + 1] = {0};

    if(read(f, (char *)a, n)!=n) {
        if (lastfile)
            lastfile->state = Readerr;
        strncpy(buf, strerror(errno), ERRLEN);
        if (downloaded)
            fprintf(stderr, "read error: %s\n", buf);
        rescue();
        exit(EXIT_FAILURE);
    }
    return n;
}

int
Write(int f, void *a, int n)
{
    int m;

    if((m=write(f, (char *)a, n))!=n)
        syserror("write");
    return m;
}

void
Seek(int f, int64_t n, int w)
{
    if(lseek(f, n, w)==-1)
        syserror("seek");
}
