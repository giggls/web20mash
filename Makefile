
CFLAGS = -g -Wall -W -std=gnu99 -pedantic
LDLIBS = -lowcapi -lmicrohttpd -lmagic

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


