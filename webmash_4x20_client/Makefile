DESTDIR = /usr/local
PROG=webmash_4x20_client
DESTDIR = /usr/local
PREFIX = $(DESTDIR)

CFLAGS = -g -Wall -W -std=gnu99 -pedantic
LDLIBS = -lcurl -lwiringPi -lwiringPiDev

OBJ = 4x20_client.o jsmn.o cmdline.o action_functions.o

all: l10n/de/LC_MESSAGES/$(PROG).mo $(PROG)

%.o: %.c
	$(CC) $(EXTRAFLAGS) $(CFLAGS) -c $<

cmdline.c: cmdline.cli
	clig $<

$(PROG):$(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ) $(LDLIBS)
	
# l10n
l10n/de/LC_MESSAGES/$(PROG).mo: l10n/de.po
	mkdir -p l10n/de/LC_MESSAGES/
	msgfmt -c -v -o l10n/de/LC_MESSAGES/$(PROG).mo $<

clean:
	rm -f *.o $(PROG) *.1 l10n/de/LC_MESSAGES/*.mo

install: all
	mkdir -p $(DESTDIR)/share/locale/de/LC_MESSAGES/
	cp l10n/de.po $(DESTDIR)/share/locale/de/LC_MESSAGES/
	chmod 644 $(DESTDIR)/share/locale/de/LC_MESSAGES/de.po
	mkdir -p $(DESTDIR)/bin
	cp $(PROG) $(DESTDIR)/bin
	chmod 755 $(DESTDIR)/bin/$(PROG)