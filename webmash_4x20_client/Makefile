PROG=webmash_4x20_client
DESTDIR = /usr/local
CFDIR = /etc
SERVICEDIR=/etc/systemd/system
PREFIX = $(DESTDIR)
BOOT = /boot

CFLAGS = -g -Wall -W -std=gnu99 -pedantic -Ilibutf8tolcd/
LDLIBS = -lcurl

OBJ = 4x20_client.o jsmn.o cmdline.o action_functions.o getifinfo.o

all: l10n/de/LC_MESSAGES/$(PROG).mo $(PROG) fangopcb.dtbo

%.o: %.c
	$(CC) $(EXTRAFLAGS) $(CFLAGS) -c $<

cmdline.c: cmdline.cli
	clig $<

$(PROG):$(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ) libutf8tolcd/libutf8tolcd.a $(LDLIBS)

mudflap: CFLAGS += -fmudflap
mudflap: LDLIBS += -lmudflap
mudflap: $(PROG)

# l10n
l10n/de.po: l10n/de.po.in libutf8tolcd/toHD44780
	libutf8tolcd/toHD44780 $< ^msgstr >$@
	
l10n/de/LC_MESSAGES/$(PROG).mo: l10n/de.po
	mkdir -p l10n/de/LC_MESSAGES/
	msgfmt -c -v -o l10n/de/LC_MESSAGES/$(PROG).mo $<

libutf8tolcd/toHD44780:
	cd libutf8tolcd; make

fangopcb.dtbo:	
	dtc -I dts -O dtb -o fangopcb.dtbo fangopcb.dts 

clean:
	rm -f *.o l10n/de/LC_MESSAGES/*.mo l10n/*.po $(PROG)

install: all
	mkdir -p $(DESTDIR)/share/locale/de/LC_MESSAGES/
	cp l10n/de/LC_MESSAGES/*.mo $(DESTDIR)/share/locale/de/LC_MESSAGES/
	chmod 644 $(DESTDIR)/share/locale/de/LC_MESSAGES/*.mo
	mkdir -p $(DESTDIR)/bin
	mkdir -p $(SERVICEDIR)
	cp $(PROG) $(DESTDIR)/bin
	chmod 755 $(DESTDIR)/bin/$(PROG)
	mkdir -p $(DESTDIR)/share/man/man1
	cp $(PROG).1 $(DESTDIR)/share/man/man1
	chmod 644 $(DESTDIR)/share/man/man1/$(PROG).1
	install -m 755 wm4x20c.init $(CFDIR)/init.d/wm4x20c
	install -m 755 wm4x20c.service $(SERVICEDIR)
	mkdir -p $(BOOT)/overlays
	install fangopcb.dtbo $(BOOT)/overlays/
