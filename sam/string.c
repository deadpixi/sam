/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include "sam.h"

#define MINSIZE 16      /* minimum number of chars allocated */
#define MAXSIZE 256     /* maximum number of chars for an empty string */


void
Strinit(String *p)
{
    p->s = emalloc(MINSIZE*RUNESIZE);
    p->n = 0;
    p->size = MINSIZE;
}

void
Strinit0(String *p)
{
    p->s = emalloc(MINSIZE*RUNESIZE);
    p->s[0] = 0;
    p->n = 1;
    p->size = MINSIZE;
}

void
Strclose(String *p)
{
    free(p->s);
    memset(p, 0, sizeof(String));
}

void
Strzero(String *p)
{
    if(p->size > MAXSIZE){
        p->s = erealloc(p->s, RUNESIZE*MAXSIZE); /* throw away the garbage */
        p->size = MAXSIZE;
    }
    p->n = 0;
}

void
Strdupl(String *p, wchar_t *s) /* copies the null */
{
    p->n = wcslen(s);
    Strinsure(p, p->n + 1);
    wmemmove(p->s, s, p->n);
}

void
Strduplstr(String *p, String *q)    /* will copy the null if there's one there */
{
    Strinsure(p, q->n);
    p->n = q->n;
    wmemmove(p->s, q->s, q->n);
}

void
Straddc(String *p, wchar_t c)
{
    Strinsure(p, p->n + 1);
    p->s[p->n++] = c;
}

void
Strinsure(String *p, uint64_t n)
{
    if(n > STRSIZE)
        error(Etoolong);

    if(p->size < n){    /* p needs to grow */
        n += 100;
        p->s = erealloc(p->s, n * RUNESIZE);
        p->size = n;
    }
}

void
Strinsert(String *p, String *q, Posn p0)
{
    Strinsure(p, p->n+q->n);
    wmemmove(p->s + p0 + q->n, p->s + p0, p->n - p0);
    wmemmove(p->s + p0, q->s, q->n);
    p->n += q->n;
}

void
Strdelete(String *p, Posn p1, Posn p2)
{
    wmemmove(p->s + p1, p->s + p2, p->n - p2);
    p->n -= p2-p1;
}

int
Strcmp(String *a, String *b)
{
    return wcscmp(a->s, b->s);
}

char*
Strtoc(String *s)
{
    size_t l = s->n * MB_LEN_MAX;
    char *c = emalloc(l + 1);
    wchar_t ws[s->n + 1];

    memset(ws, 0, sizeof(ws));
    memset(c, 0, l + 1);
    wmemcpy(ws, s->s, s->n);
    ws[s->n] = 0;

    if (wcstombs(c, ws, l) == (size_t)-1)
        panic("encoding 1");

    return c;
}

/*
 * Build very temporary String from wchar_t*
 */
String*
tmprstr(wchar_t *r, int n)
{
    static String p = {0};

    p.s = r;
    p.n = n;
    p.size = n;
    return &p;
}

/*
 * Convert null-terminated char* into String
 */
String*
tmpcstr(char *s)
{
    String *p = emalloc(sizeof(String));
    p->n = utflen(s);
    p->size = p->n + 1;
    p->s = calloc(p->size, sizeof(wchar_t));
    if (mbstowcs(p->s, s, p->n) == (size_t)-1)
        panic("encoding 2");

    return p;
}

void
freetmpstr(String *s)
{
    free(s->s);
    free(s);
}
