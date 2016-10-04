/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include "sam.h"

/*
 * Check that list has room for one more element.
 */
void
growlist(List *l)
{
    if(l->listptr==0 || l->nalloc==0){
        l->nalloc = INCR;
        l->listptr = emalloc(INCR*sizeof(l->g));
        l->nused = 0;
    }else if(l->nused == l->nalloc){
        l->listptr = erealloc(l->listptr, (l->nalloc+INCR)*sizeof(l->g));
        memset((void*)(l->longptr+l->nalloc), 0, INCR*sizeof(l->g));
        l->nalloc += INCR;
    }
}

/*
 * Remove the ith element from the list
 */
void
dellist(List *l, int i)
{
    memmove(&l->longptr[i], &l->longptr[i+1], (l->nused-(i+1))*sizeof(l->g));
    l->nused--;
}

/*
 * Add a new element, whose position is i, to the list
 */
void
inslist(List *l, int i, int64_t val)
{
    growlist(l);
    memmove(&l->longptr[i+1], &l->longptr[i], (l->nused-i)*sizeof(l->g));
    l->longptr[i] = val;
    l->nused++;
}

void
listfree(List *l)
{
    free(l->listptr);
    free(l);
}
