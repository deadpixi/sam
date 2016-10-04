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

CFLAGS=-c $(INCS) $(STANDARDS) $(INCLUDES)

LIB=libframe.a
CC?=c99

OBJ=frbox.o frdelete.o frdraw.o frinit.o frinsert.o frptofchar.o\
	frselect.o frstr.o frutil.o

all:	$(LIB)

$(LIB):	$(OBJ)
	$(AR) rv $(LIB) $(OBJ)
	$(RANLIB) $(LIB)

clean:
	rm -f *.o *.a

nuke:	clean
	rm -f $(LIB)

install:	$(LIB)

$(OBJ):	../include/u.h ../include/frame.h
