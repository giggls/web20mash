DESTDIR = /usr/local
CFDIR = /etc
PREFIX = $(DESTDIR)

CFLAGS = -g -Wall -W -std=gnu99 -pedantic
LDLIBS = -lowcapi -lmicrohttpd -lmagic -lrt

# Use this to generate a simulation only binary without 1-wire support
# If BINDLOCALHOST is defined build a binary listening on 127.0.0.1 only
#CFLAGS = -g -Wall -W -std=gnu99 -pedantic -D NO1W -D BINDLOCALHOST
#LDLIBS = -lmicrohttpd -lmagic -lrt

SRC = cmdline.c mashctld.c owfunc.c minIni.c readcfg.c myexec.c

OBJ = $(SRC:%.c=%.o)

all:mashctld gpio_buzzer

mashctld:$(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ) $(LDLIBS)

gpio_buzzer:gpio_buzzer.o
	$(CC) -o $@ $(CFLAGS) gpio_buzzer.o

cmdline.c: cmdline.cli
	clig $<

mudflap: CFLAGS += -fmudflap
mudflap: LDLIBS += -lmudflap
mudflap: mashctld

clean:
	rm -f *.o *~ mashctld

mrproper: clean
	rm -f cmdline.c cmdline.h mashctld.1
	

debian/copyright:
	cp COPYING debian/copyright

install: debian/copyright mashctld gpio_buzzer
	cd webdata/; make
	mkdir -p $(DESTDIR)/share/web20mash/images
	mkdir -p $(DESTDIR)/share/web20mash/js
	mkdir -p $(CFDIR)
	mkdir -p $(CFDIR)/sudoers.d
	mkdir -p $(CFDIR)/init.d
	mkdir -p $(DESTDIR)/bin
	cp -a webdata/*.html.?? $(DESTDIR)/share/web20mash/
	cp -a webdata/css $(DESTDIR)/share/web20mash/
	cp -a webdata/js/*.js $(DESTDIR)/share/web20mash/js/
	cp webdata/images/*.png $(DESTDIR)/share/web20mash/images/
	cp webdata/images/*.gif $(DESTDIR)/share/web20mash/images/
	cp webdata/images/favicon.ico $(DESTDIR)/share/web20mash/images/
	sed -e 's;^webroot.*;webroot = $(PREFIX)/share/web20mash;' \
		-e 's;^port.*;port = 80;' mashctld.conf.sample >$(CFDIR)/mashctld.conf
	cp mashctld $(DESTDIR)/bin
	chmod 755 $(DESTDIR)/bin/mashctld
	cp gpio_buzzer $(DESTDIR)/bin
	cp mps2iConnectLED $(DESTDIR)/bin
	cp mashctld_readonly_root_script.sh $(DESTDIR)/bin
	chmod 755 $(DESTDIR)/bin/mashctld_readonly_root_script.sh
	cp web20mash.sudo $(CFDIR)/sudoers.d/web20mash
	chmod 755 $(CFDIR)/sudoers.d/web20mash
	cp webmash.init $(CFDIR)/init.d/webmash
	chmod 755 $(DESTDIR)/share/web20mash $(DESTDIR)/share/web20mash/images $(DESTDIR)/share/web20mash/js $(DESTDIR)/share/web20mash/css
	chmod 644 $(DESTDIR)/share/web20mash/*/*
	chmod 755 $(DESTDIR)/bin/mashctld_readonly_root_script.sh
