#	Copyright (c) 1998 Lucent Technologies - All rights reserved.
#	Changes Copyright (c) 2014-2015 Rob King
#
#	master makefile for sam.  configure sub-makefiles first.
#

MODE?=user

all:    config.mk lXg lframe samdir samtermdir docdir

config.mk:
	cp config.mk.def config.mk

lXg:
	cd libXg; $(MAKE)

lframe:
	cd libframe; $(MAKE)

docdir:
	cd doc; $(MAKE)

samdir:
	cd sam; $(MAKE)

samtermdir:
	cd samterm; $(MAKE)

install:
	@xdg-desktop-menu install --mode $(MODE) deadpixi-sam.desktop || echo "unable to install desktop entry"
	cd libXg; $(MAKE) install
	cd libframe; $(MAKE) install
	cd sam; $(MAKE) install
	cd samterm; $(MAKE) install
	cd doc; $(MAKE) install
	cp ssam $(BINDIR)
	chmod +x $(BINDIR)/ssam

uninstall:
	@xdg-desktop-menu uninstall --mode $(MODE) deadpixi-sam.desktop || echo "unable to uninstall desktop entry"
	cd libXg; $(MAKE) uninstall
	cd libframe; $(MAKE) uninstall
	cd sam; $(MAKE) uninstall
	cd samterm; $(MAKE) uninstall
	cd doc; $(MAKE) uninstall

clean:
	cd libXg; $(MAKE) clean
	cd libframe; $(MAKE) clean
	cd sam; $(MAKE) clean
	cd samterm; $(MAKE) clean

nuke: clean
	rm -f config.mk
