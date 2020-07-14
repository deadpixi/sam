include ../config.mk

all:    ssam

ssam:	ssam.sh
	cp $< $@

clean:
	rm -f ssam

nuke:	clean
	

install:	ssam
	mkdir -p $(BINDIR)
	cp ssam $(BINDIR)
	chmod +x $(BINDIR)/ssam


uninstall:
	rm -f $(BINDIR)/ssam