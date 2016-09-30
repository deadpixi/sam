/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include    <u.h>
#include    <libc.h>
#include    <pwd.h>
#ifdef  NEEDVARARG
#include    <varargs.h>
#else
#include    <stdarg.h>
#endif
#include <errno.h>

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
