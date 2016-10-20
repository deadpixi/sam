/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include "sam.h"
#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#ifdef  NEEDVARARG
#include    <varargs.h>
#else
#include    <stdarg.h>
#endif

wchar_t    samname[] = { '~', '~', 's', 'a', 'm', '~', '~', 0 };

static wchar_t l1[] = { '{', '[', '(', '<', 0253, 0};
static wchar_t l2[] = { '\n', 0};
static wchar_t l3[] = { '\'', '"', '`', 0};
wchar_t *left[]= { l1, l2, l3, 0};

static wchar_t r1[] = {'}', ']', ')', '>', 0273, 0};
static wchar_t r2[] = {'\n', 0};
static wchar_t r3[] = {'\'', '"', '`', 0};
wchar_t *right[]= { r1, r2, r3, 0};

void
print_ss(char *s, String *a, String *b)
{
    char *ap, *bp, *cp;
    wchar_t *rp;

    ap = emalloc(a->n+1);
    for (cp = ap, rp = a->s; *rp; rp++)
        cp += runetochar(cp, *rp);
    *cp = 0;
    bp = emalloc(b->n+1);
    for (cp = bp, rp = b->s; *rp; rp++)
        cp += runetochar(cp, *rp);
    *cp = 0;
    dprint(L"?warning: %s `%.*s' and `%.*s'\n", s, a->n, ap, b->n, bp);
    free(ap);
    free(bp);
}

void
print_s(char *s, String *a)
{
    char *ap, *cp;
    wchar_t *rp;

    ap = emalloc(a->n+1);
    for (cp = ap, rp = a->s; *rp; rp++)
        cp += runetochar(cp, *rp);
    *cp = 0;
    dprint(L"?warning: %s `%.*s'\n", s, a->n, ap);
    free(ap);
}

int
statfile(char *name, uint64_t *dev, uint64_t *id, int64_t *time, int64_t *length, int64_t *appendonly)
{
    struct stat dirb;

    if (stat(name, &dirb) == -1)
        return -1;
    if (dev)
        *dev = dirb.st_dev;
    if (id)
        *id = dirb.st_ino;
    if (time)
        *time = dirb.st_mtime;
    if (length)
        *length = dirb.st_size;
    if(appendonly)
        *appendonly = 0;
    return 1;
}

int
statfd(int fd, uint64_t *dev, uint64_t *id, int64_t *time, int64_t *length, int64_t *appendonly)
{
    struct stat dirb;

    if (fstat(fd, &dirb) == -1)
        return -1;
    if (dev)
        *dev = dirb.st_dev;
    if (id)
        *id = dirb.st_ino;
    if (time)
        *time = dirb.st_mtime;
    if (length)
        *length = dirb.st_size;
    if(appendonly)
        *appendonly = 0;
    return 1;
}

int
newtmp(void)
{
    FILE *f = tmpfile();
    if (f)
        return fileno(f);
    panic("could not create tempfile!");
    return -1;
}

void
samerr(char *buf)
{
    snprintf(buf, PATH_MAX, "%s/sam.err", getenv("HOME") ? getenv("HOME") : "/tmp");
}

int
waitfor(int pid)
{
    int wm;
    int rpid;

    do; while((rpid = wait(&wm)) != pid && rpid != -1);
    return (WEXITSTATUS(wm));
}

void*
emalloc(uint64_t n)
{
    void *p = calloc(1, n < sizeof(int)? sizeof(int) : n);
    if (!p)
        panic("malloc failed");
    return p;
}

void*
erealloc(void *p, uint64_t n)
{
    p = realloc(p, n);
    if(!p)
        panic("realloc fails");
    return p;
}

void
dprint(wchar_t *z, ...)
{
    va_list args;
    wchar_t buf[BLOCKSIZE + 1] = {0};

    va_start(args, z);
    vswprintf(buf, BLOCKSIZE, z, args);
    termwrite(buf);
    va_end(args);
}

