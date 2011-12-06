TARGET = /usr/local/

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


install: mashctld
	cd webdata/; make
	cp -a webdata/ $(TARGET)/share/web20mash/
	cp mashctld.conf /etc
	cp mashctld $(TARGET)/bin