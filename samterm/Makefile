#	Copyright (c) 1998 Lucent Technologies - All rights reserved.
#
#	Prototype Makefile for samterm
#
#	define operating system.  ONE of:
#		-DIRIX -DSUNOS -DUMIPS -DSYSVR3 -DAIX -DOSF1
#		-DHPUX -DAPOLLO -DCONVEX -DDYNIX
#
#	-DIRIX is the default and should actually work on any modern system.
#	Additionally, -D_POSIX_SOURCE (or its equivalent) may be specified
#	if your compiler supports posix-compatible compilation.
#

include ../config.mk

OS=-DIRIX5

#	add -Iincludedir for any include directories that need to be searched
#	for posix header files (for UMIPS, add -I/usr/include/posix)
INCS=-I../include


#	SAMTERM contains the name of the file containing the samterm
#	executable.
SAMTERM=$(BINDIR)/samterm

#	set this if your X libraries are in different locations
#	or if you need extra libraries to load with X11 applications
XLIBS=-lXt -lX11 -lXft

CFLAGS=$(OS) $(INCS) -D_LIBXG_EXTENSION $(INCLUDES)

LIBS=../libframe/libframe.a ../libXg/libXg.a
CC?=gcc

OBJ=main.o flayer.o icons.o io.o menu.o mesg.o rasp.o scroll.o unix.o

all:	samterm

samterm:	$(OBJ) $(LIBS)
	$(CC)  -o samterm $(OBJ) $(LIBS) $(XLIBS) $(LDFLAGS)

clean:
	rm -f *.o core samterm

install:	samterm
	cp samterm $(SAMTERM)

$(OBJ):	samterm.h flayer.h ../include/frame.h ../include/libg.h ../include/u.h ../include/libc.h ../sam/mesg.h
