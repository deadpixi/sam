/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include	"sam.h"
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<signal.h>

#ifdef	NEEDVARARG
#include	<varargs.h>
#else
#include	<stdarg.h>
#endif

Rune	samname[] = { '~', '~', 's', 'a', 'm', '~', '~', 0 };

static Rune l1[] = { '{', '[', '(', '<', 0253, 0};
static Rune l2[] = { '\n', 0};
static Rune l3[] = { '\'', '"', '`', 0};
Rune *left[]= { l1, l2, l3, 0};

static Rune r1[] = {'}', ']', ')', '>', 0273, 0};
static Rune r2[] = {'\n', 0};
static Rune r3[] = {'\'', '"', '`', 0};
Rune *right[]= { r1, r2, r3, 0};

void
print_ss(char *s, String *a, String *b)
{
	char *ap, *bp, *cp;
	Rune *rp;

	ap = emalloc(a->n+1);
	for (cp = ap, rp = a->s; *rp; rp++)
		cp += runetochar(cp, rp);
	*cp = 0;
	bp = emalloc(b->n+1);
	for (cp = bp, rp = b->s; *rp; rp++)
		cp += runetochar(cp, rp);
	*cp = 0;
	dprint("?warning: %s `%.*s' and `%.*s'\n", s, a->n, ap, b->n, bp);
	free(ap);
	free(bp);
}

void
print_s(char *s, String *a)
{
	char *ap, *cp;
	Rune *rp;

	ap = emalloc(a->n+1);
	for (cp = ap, rp = a->s; *rp; rp++)
		cp += runetochar(cp, rp);
	*cp = 0;
	dprint("?warning: %s `%.*s'\n", s, a->n, ap);
	free(ap);
}

int
statfile(char *name, ulong *dev, ulong *id, long *time, long *length, long *appendonly)
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
statfd(int fd, ulong *dev, ulong *id, long *time, long *length, long *appendonly)
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

void
hup(int sig)
{
	rescue();
	exit(1);
}

int
notify (void(*f)(void *, char *))
{
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, hup);
	signal(SIGPIPE, SIG_IGN);
#ifdef	v10
	close(3);		/* redirect v10 /dev/tty */
	open("/dev/null", 2);
#endif
	return 1;
}

void
notifyf(void *a, char *b)	/* never called */
{
}

int
newtmp(void)
{
    FILE *f = tmpfile();
    if (f)
        return fileno(f);
    panic("could not create tempfile!");
}

void
samerr(char *buf)
{
	sprint(buf, "%s/sam.err.%.6s", TMPDIR, getuser());
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
emalloc(ulong n)
{
	void *p;

	if (n < sizeof(int))
		n = sizeof(int);
	p = malloc(n);
	if(p == 0)
		panic("malloc fails");
	memset(p, 0, n);
	return p;
}

void*
erealloc(void *p, ulong n)
{
	p = realloc(p, n);
	if(p == 0)
		panic("realloc fails");
	return p;
}

void
exits(char *message)
{

	if (message == 0)
		exit(0);
	else
		exit(1);
}

void
dprint(char *z, ...)
{
	va_list args;
	char buf[BLOCKSIZE];

	va_start(args, z);
	vsprintf(buf, z, args);
	termwrite(buf);
	va_end(args);
}

