DESTDIR = /usr/local
CFDIR = /etc
PREFIX = $(DESTDIR)

CFLAGS = -g -Wall -W -std=gnu99 -pedantic
LDLIBS = -lowcapi -lmicrohttpd -lmagic -lrt

SRC = cmdline.c mashctld.c owfunc.c minIni.c readcfg.c

OBJ = $(SRC:%.c=%.o)

mashctld:$(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ) $(LDLIBS)


cmdline.c: cmdline.cli
	clig $<

mudflap: CFLAGS += -fmudflap
mudflap: LDLIBS += -lmudflap
mudflap: mashctld

clean:
	rm -f *.o *~ cmdline.c cmdline.h mashctld mashctld.1

debian/copyright:
	cp COPYING debian/copyright

install: debian/copyright mashctld
	cd webdata/; make
	mkdir -p $(DESTDIR)/share/web20mash/images
	mkdir -p $(DESTDIR)/share/web20mash/js
	mkdir -p $(CFDIR)
	mkdir -p $(DESTDIR)/bin
	cp -a webdata/css $(DESTDIR)/share/web20mash/
	cp -a webdata/js/*.js $(DESTDIR)/share/web20mash/js/
	cp webdata/images/*.png $(DESTDIR)/share/web20mash/images/
	cp webdata/images/*.gif $(DESTDIR)/share/web20mash/images/
	sed -e 's;^webroot.*;webroot = $(PREFIX)/share/web20mash;' mashctld.conf.sample >$(CFDIR)/mashctld.conf
	cp mashctld $(DESTDIR)/bin
	cp mps2iConnectLED $(DESTDIR)/bin
	chmod 755 $(DESTDIR)/bin/mashctld
	chmod 755 $(DESTDIR)/share/web20mash $(DESTDIR)/share/web20mash/images $(DESTDIR)/share/web20mash/js $(DESTDIR)/share/web20mash/css
	chmod 644 $(DESTDIR)/share/web20mash/*/*