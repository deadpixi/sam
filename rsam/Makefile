# Copyright (C) 2013-2015 Rob King <jking@deadpixi.com
# This file may be redistributed and modified for any purpose.
# No warranty is expressed or implied; use at your own risk.

include ../config.mk

LDFLAGS=

all: rsam

rsam: rsam.o

clean:
	rm -f *.o rsam

install: rsam
	cp rsam $(BINDIR)
