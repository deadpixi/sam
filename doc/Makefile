# Copyright 2014-2014 Rob King <jking@deadpixi.com>

include ../config.mk

all:

install: sam.1 
	mkdir -p "$(MANDIR)/man1"
	cp sam.1 "$(MANDIR)/man1"
	ln -sf "$(MANDIR)/man1/sam.1" "$(MANDIR)/man1/B.1"
	ln -sf "$(MANDIR)/man1/sam.1" "$(MANDIR)/man1/samterm.1"
	ln -sf "$(MANDIR)/man1/sam.1" "$(MANDIR)/man1/rsam.1"
	ln -sf "$(MANDIR)/man1/sam.1" "$(MANDIR)/man1/sam.save.1"
	mkdir -p "$(MANDIR)/man5"
	cp samrc.5 "$(MANDIR)/man5"

uninstall:
	rm -f "$(MANDIR)/man1/sam.1"
	rm -f "$(MANDIR)/man1/B.1"
	rm -f "$(MANDIR)/man1/samterm.1"
	rm -f "$(MANDIR)/man1/rsam.1"
	rm -f "$(MANDIR)/man1/sam.save.1"
	rm -f "$(MANDIR)/man5/samrc.5"
