DESTDIR = /usr/local
CFDIR = /etc
PREFIX = $(DESTDIR)

CFLAGS = -g -Wall -W -std=gnu99 -pedantic
EXTRAFLAGS =  -DCTLD_PLUGINDIR=\"$(PREFIX)/lib/web20mash/plugins\" -DCTLD_WEBROOT=\"$(PREFIX)/share/web20mash/\"
LDLIBS = -lowcapi -lmicrohttpd -lmagic -lrt -rdynamic -ldl -lmnl

# Use this to generate a simulation only binary without sensor/actor support
# If BINDLOCALHOST is defined build a binary listening on 127.0.0.1 only
#
# In addition just call "make mashctld" because it does not make sense to
# build plugins
#CFLAGS = -g -Wall -W -std=gnu99 -pedantic -D NOSENSACT -D BINDLOCALHOST
#LDLIBS = -lmicrohttpd -lmagic -lrt -lmnl


OBJ = cmdline.o mashctld.o ctrlfunc.o minIni.o readcfg.o myexec.o gen_json_4interfaces.o

%.o: %.c
	$(CC) $(EXTRAFLAGS) $(CFLAGS) -c $<
	
#-DCTLD_PLUGINDIR=\"$(DESTDIR)/lib/web20mash/plugins\" -DCTLD_WEBROOT=\"$(DESTDIR)/share/web20mash/\" $(CFLAGS)

all:mashctld gpio_buzzer plugins find_ow_devs webdata

mashctld:$(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ) $(LDLIBS)

gpio_buzzer:gpio_buzzer.o
	$(CC) -o $@ $(CFLAGS) $<

find_ow_devs:find_ow_devs.o
	 $(CC) -o $@ $(CFLAGS) -lowcapi $<

cmdline.c: cmdline.cli
	clig $<

ifinfo: ifinfo.c gen_json_4interfaces.c
	$(CC) -static -o $@ ifinfo.c gen_json_4interfaces.c -lmnl

mudflap: CFLAGS += -fmudflap
mudflap: LDLIBS += -lmudflap
mudflap: mashctld

.PHONY: plugins
plugins:
	make -C plugins
	
.PHONY: webdata
webdata:
	make -C webdata

clean:
	rm -f *.o *~ mashctld ifinfo
	make -C plugins clean
	make -C webmash_7segm_client clean
	make -C webdata clean

mrproper: clean
	rm -f cmdline.c cmdline.h mashctld.1
	

debian/copyright:
	cp COPYING debian/copyright

install: debian/copyright mashctld gpio_buzzer
	cd webdata/; make
	cd webmash_7segm_client; make
	mkdir -p $(DESTDIR)/share/web20mash/images
	mkdir -p $(DESTDIR)/share/web20mash/js
	mkdir -p $(DESTDIR)/lib/web20mash/plugins
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
		-e 's;^plugin_dir.*;plugin_dir= $(PREFIX)/lib/web20mash/plugins;' -e 's;^port.*;port = 80;' mashctld.conf.sample >$(CFDIR)/mashctld.conf
	install -m 755 mashctld $(DESTDIR)/bin
	install -m 755 gpio_buzzer $(DESTDIR)/bin
	install -m 755 find_ow_devs $(DESTDIR)/bin
	install -m 755 mps2iConnectLED $(DESTDIR)/bin
	install -m 755 webmash_7segm_client/webmash_7segm_client $(DESTDIR)/bin
	install -m 755 mashctld_readonly_root_script.sh $(DESTDIR)/bin
	install -m 755 web20mash.sudo $(CFDIR)/sudoers.d/web20mash
	install -m 755 webmash.init $(CFDIR)/init.d/webmash
	chmod 755 $(DESTDIR)/share/web20mash $(DESTDIR)/share/web20mash/images $(DESTDIR)/share/web20mash/js $(DESTDIR)/share/web20mash/css
	chmod 644 $(DESTDIR)/share/web20mash/*/*
	cp plugins/*.so $(DESTDIR)/lib/web20mash/plugins
	chmod 755 $(DESTDIR)/lib/web20mash/plugins/*.so
	
