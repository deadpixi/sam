/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include	<u.h>
#include	<libc.h>
#include	<pwd.h>
#ifdef	NEEDVARARG
#include	<varargs.h>
#else
#include	<stdarg.h>
#endif
#include <errno.h>

int errstr(char *buf)
{

	strncpy(buf, strerror(errno), ERRLEN);
	return 1;
}

char*
getuser(void)
{
	struct passwd *p;

	static char *user = 0;

	if (!user) {
		p = getpwuid(getuid());
		if (p && p->pw_name) {
			user = malloc(strlen(p->pw_name)+1);
			if (user)
				strcpy(user, p->pw_name);
		}
	}
	if(!user)
		user = "unknown";
	return user;
}

#ifdef NEEDSTRERROR
char *
strerror(int n)
{
	extern char *sys_errlist[];
	return sys_errlist[n];
}
#endif /* NEEDSTRERROR */

#ifdef NEEDMEMMOVE
/*
 * memcpy is probably fast, but may not work with overlap
 */
void*
memmove(void *a1, const void *a2, size_t n)
{
	char *s1;
	const char *s2;

	s1 = a1;
	s2 = a2;
	if(s1 > s2)
		goto back;
	if(s1 + n <= s2)
		return memcpy(a1, a2, n);
	while(n > 0) {
		*s1++ = *s2++;
		n--;
	}
	return a1;

back:
	s2 += n;
	if(s2 <= s1)
		return memcpy(a1, a2, n);
	s1 += n;
	while(n > 0) {
		*--s1 = *--s2;
		n--;
	}
	return a1;
}
#endif /* NEEDMEMMOVE */
