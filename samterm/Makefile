#	Copyright (c) 1998 Lucent Technologies - All rights reserved.
#
#	Prototype Makefile for samterm
#

include ../config.mk

#	add -Iincludedir for any include directories that need to be searched
#	for posix header files (for UMIPS, add -I/usr/include/posix)
INCS=-I../include

#	SAMTERM contains the name of the file containing the samterm
#	executable.
SAMTERM=$(BINDIR)/samterm

#	set this if your X libraries are in different locations
#	or if you need extra libraries to load with X11 applications
XLIBS=-lXt -lX11 -lXft

CFLAGS=$(INCS) $(STANDARDS) $(INCLUDES)

LIBS=../libframe/libframe.a ../libXg/libXg.a
CC?=c99

OBJ=main.o flayer.o  io.o menu.o mesg.o rasp.o samrc.o scroll.o unix.o

all:	samterm

samterm:	$(OBJ) $(LIBS)
	$(CC)  -o samterm $(OBJ) $(LIBS) $(XLIBS) $(LDFLAGS)

clean:
	rm -f *.o core samterm

install:	samterm
	cp samterm $(SAMTERM)

$(OBJ):	samterm.h flayer.h ../include/frame.h ../include/libg.h ../include/u.h ../sam/mesg.h
