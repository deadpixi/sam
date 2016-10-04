/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <u.h>
#include <libg.h>
#include <frame.h>

/*
 * The code here and elsewhere requires that strings not be gcalloc()ed
 */

#define CHUNK   16
#define ROUNDUP(n)  ((n+CHUNK)&~(CHUNK-1))

uint8_t *
_frallocstr(unsigned n)
{
    uint8_t *p;

    p = malloc(ROUNDUP(n));
    if(p == 0)
        berror("out of memory");
    return p;
}

void
_frinsure(Frame *f, int bn, unsigned n)
{
    Frbox *b;
    uint8_t *p;

    b = &f->box[bn];
    if(b->nrune < 0)
        berror("_frinsure");
    if(ROUNDUP(b->nrune) > n)   /* > guarantees room for terminal NUL */
        return;
    p = _frallocstr(n);
    b = &f->box[bn];
    memmove(p, b->a.ptr, NBYTE(b)+1);
    free(b->a.ptr);
    b->a.ptr = p;
}
