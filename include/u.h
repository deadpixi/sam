/* Copyright (c) 1998 Lucent Technologies - All rights reserved. */
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <setjmp.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

typedef unsigned short      ushort;
typedef unsigned long       ulong;
typedef unsigned char		uchar;
typedef	unsigned short		Rune;

	/* System configuration parameters */

#ifdef	SYSVR3
#include	<malloc.h>
typedef	unsigned short	ushort;
typedef unsigned long	ulong;
#define	remove(v)			unlink(v)
#define	WEXITSTATUS(s)			(((s)>>8)&0xFF)
extern	char *getenv(char*);
extern	char *getlogin(void);
extern	char *strerror(int);
extern	void *memmove(void*, const void*, size_t);
#define	NEEDMEMMOVE
#define	NEEDSTRERROR
#define	NEEDVARARG
#endif	/* SYSVR3 */

#ifdef	IRIX5
typedef	unsigned short	ushort;
typedef unsigned long	ulong;
#endif	/* IRIX5 */

#ifdef	IRIX
extern	void *memmove(void*, const void*, size_t);
#define	NEEDMEMMOVE
#endif	/* IRIX */

#ifdef	UMIPS
typedef unsigned long	ulong;
typedef	unsigned short	ushort;
#define	const			/* mips compiler doesn't support const */
extern	char *strerror(int);
extern	void *memmove(void*, const void*, size_t);
#define	NEEDMEMMOVE
#define	NEEDSTRERROR
#define	NEEDVARARG
#define	NOFIFO			/* turn off exstart in samterm/unix.c */
#endif	/* UMIPS */

#ifdef	SUNOS
typedef	unsigned short	ushort;
typedef unsigned long	ulong;
extern	char *strerror(int);
extern	void *memmove(void*, const void*, size_t);
extern	void *memcpy(void*, const void*, size_t);
#define	NEEDMEMMOVE
#define	NEEDSTRERROR
#endif	/* SUNOS */

#ifdef	SOLARIS
typedef	unsigned short	ushort;
typedef unsigned long	ulong;
#endif

#ifdef	AIX
typedef	unsigned short	ushort;
typedef unsigned long	ulong;
#endif	/* AIX */

#ifdef	OSF1
typedef	unsigned short	ushort;
typedef unsigned long	ulong;
extern	void *memmove(void*, const void*, size_t);
#endif	/* OSF1 */

#ifdef  HPUX
typedef	unsigned short	ushort;
typedef unsigned long	ulong;
#define	NEEDSTRERROR
#endif  /* HPUX */

#ifdef  APOLLO
typedef	unsigned short	ushort;
typedef unsigned long	ulong;
#endif  /* APOLLO */

#ifdef  CONVEX
typedef unsigned long	ulong;
#endif  /* CONVEX */

#ifdef  DYNIX
#define	SIG_ERR		BADSIG
#define	NEEDMEMMOVE
#define	remove(v)			unlink(v)
#define	WEXITSTATUS(s)			(((s)>>8)&0xFF)
#define	NEEDMEMMOVE
#define	NOFIFO			/* turn off exstart in samterm/unix.c */
#endif  /* DYNIX */

#ifdef	PTX
typedef	unsigned short	ushort;
typedef unsigned long	ulong;
#endif	/* PTX */

#ifdef	BSDi
typedef unsigned long   ulong;
#endif	/* BSDi */

#ifdef	v10
typedef	unsigned short	ushort;
typedef unsigned long	ulong;
#endif
