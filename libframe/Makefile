#	Copyright (c) 1998 Lucent Technologies - All rights reserved.
#
#	Prototype Makefile for libframe
#
include ../config.mk

#	add -Iincludedir for any include directories that need to be searched
#	for posix header files (for UMIPS, add -I/usr/include/posix)
INCS=-I../include


#	add name of library orderer - use ":" if none exists
RANLIB=:

#	add name of library
AR=ar

CFLAGS=-c $(INCS) -D_POSIX_C_SOURCE=200809L -D_LIBXG_EXTENSION $(INCLUDES)

LIB=libframe.a
CC?=c99

OBJ=frbox.o frdelete.o frdraw.o frinit.o frinsert.o frptofchar.o\
	frselect.o frstr.o frutil.o misc.o

all:	$(LIB)

$(LIB):	$(OBJ)
	$(AR) rv $(LIB) $(OBJ)
	$(RANLIB) $(LIB)

clean:
	rm -f *.o *.a

nuke:	clean
	rm -f $(LIB)

install:	$(LIB)

$(OBJ):	../include/u.h ../include/libc.h ../include/frame.h
