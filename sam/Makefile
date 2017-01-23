#	Copyright (c) 1998 Lucent Technologies - All rights reserved.
#
#	Prototype Makefile for sam
#
include ../config.mk

#	add -Iincludedir for any include directories that need to be searched
#	for posix header files (for UMIPS, add -I/usr/include/posix)
INCS=-I../include

#	RSAMNAME and TERMNAME contain the names of the files containing the
#	sam and samterm executables, respectively.  SAMDIR is the directory
#	where sam is to be installed.
RSAMNAME=sam
TERMNAME=samterm
SAMDIR=$(BINDIR)

#	Set SHELLNAME and SHELLPATH to the name of a shell and the pathname
#	of its executable
SHELLNAME=sh
SHELLPATH=/bin/sh

CFLAGS+=$(STANDARDS) $(INCS) $(INCLUDES) -DRXPATH='"$(RXPATH)"'

CC?=c99

OBJ=sam.o address.o buffer.o cmd.o error.o file.o io.o \
	list.o mesg.o moveto.o multi.o rasp.o regexp.o shell.o \
	string.o sys.o unix.o xec.o

all:    sam

sam:	$(OBJ)
	$(CC) -o sam $(OBJ) $(LDFLAGS)

clean:
	rm -f *.o core sam

nuke:	clean
	rm -f sam

install:	sam
	mkdir -p $(SAMDIR)
	cp sam $(SAMDIR)/$(RSAMNAME)
	ln -f $(SAMDIR)/$(RSAMNAME) $(SAMDIR)/B

$(OBJ):	sam.h ../include/u.h errors.h mesg.h

cmd.o:	parse.h
xec.o:	parse.h

unix.o:	sam.h ../include/u.h errors.h mesg.h
	$(CC) -c $(CFLAGS) unix.c
