#	Copyright (c) 1998 Lucent Technologies - All rights reserved.
#
#	Prototype Makefile for sam
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

#	If your system has 64-bit addresses, add -DUSE64BITS to $(OS).
OS=-DIRIX5 -DUSE64BITS=$(USE64BITS)

#	add -Iincludedir for any include directories that need to be searched
#	for posix header files (for UMIPS, add -I/usr/include/posix)
INCS=-I../include

#	Set the name of the environment variable containing the user's home directory
HOMEDIR=HOME

#	RSAMNAME and TERMNAME contain the names of the files containing the
#	sam and samterm executables, respectively.  SAMDIR is the directory
#	where sam is to be installed.  SAMSAVEDIR is the name of the directory
#	where the samsave file restoration script is stored.
RSAMNAME=sam
TERMNAME=samterm
SAMDIR=$(BINDIR)
SAMSAVEDIR=$(BINDIR)

#	Set TMP to a good place for tmp files (with lots of room)
TMP=$(TMPDIR)

#	Set SHELLNAME and SHELLPATH to the name of a shell and the pathname
#	of its executable
SHELLNAME=sh
SHELLPATH=/bin/sh

#	Set RXNAME and RXPATHNAME to the name of the remote execution command
#	and the pathname of its executable
RXNAME=ssh
RXPATHNAME=/usr/bin/ssh

#   Set RXSAMNAME to the name of the command to run on the remote host.
RXSAMNAME=rsam

SAMSAVE=/bin/sh\\n$(SAMSAVEDIR)/samsave

CFLAGS=$(OS) -D_LIBXG_EXTENSION $(INCS)\
		-DHOMEDIR=\"$(HOMEDIR)\" -DRSAMNAME=\"$(RSAMNAME)\" \
		-DTERMNAME=\"$(TERMNAME)\" -DTMP=\"$(TMP)\" \
		-DSHELLNAME=\"$(SHELLNAME)\" -DSHELLPATH=\"$(SHELLPATH)\" \
		-DRXNAME=\"$(RXNAME)\" -DRXPATHNAME=\"$(RXPATHNAME)\" \
		-DRXSAMNAME=\"$(RXSAMNAME)\" -DSAMSAVE=\"$(SAMSAVE)\"

LIB=../libframe/libframe.a ../libXg/libXg.a
CC?=gcc 

OBJ=sam.o address.o buffer.o cmd.o disc.o error.o file.o io.o \
	list.o mesg.o moveto.o multi.o rasp.o regexp.o shell.o \
	string.o sys.o unix.o xec.o

all:    sam

sam:	$(OBJ) $(LIB)
	$(CC) -o sam $(OBJ) $(LIB) $(LDFLAGS)

clean:
	rm -f *.o core sam

nuke:	clean
	rm -f sam

install:	sam
	mkdir -p $(SAMDIR)
	mkdir -p $(SAMSAVEDIR)
	cp sam $(SAMDIR)/$(RSAMNAME)
	cp samsave $(SAMSAVEDIR)/samsave
	chmod +x $(SAMSAVEDIR)/samsave

$(OBJ):	sam.h ../include/u.h ../include/libc.h errors.h mesg.h

cmd.o:	parse.h
xec.o:	parse.h

unix.o:	sam.h ../include/u.h ../include/libc.h errors.h mesg.h
	$(CC) -c $(CFLAGS) unix.c
