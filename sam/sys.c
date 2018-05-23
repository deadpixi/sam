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
Read(FILE *f, void *a, int n)
{
    if (fread(a, 1, n, f) != n){
        if (lastfile)
            lastfile->state = Readerr;
        if (downloaded)
            fprintf(stderr, "read error: %s\n", strerror(errno));
        rescue();
        exit(EXIT_FAILURE);
    }
    return n;
}

int
Write(FILE *f, void *a, int n)
{
    size_t m = fwrite(a, 1, n, f);
    if (m != n)
        syserror("write");
    fflush(f);
    return m;
}
