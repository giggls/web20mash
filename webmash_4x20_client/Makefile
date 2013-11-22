DESTDIR = /usr/local
PREFIX = $(DESTDIR)

CFLAGS = -g -Wall -W -std=gnu99 -pedantic
LDLIBS = -lcurl -lwiringPi -lwiringPiDev

OBJ = 4x20_client.o jsmn.o cmdline.o action_functions.o

all:webmash_4x20_client

%.o: %.c
	$(CC) $(EXTRAFLAGS) $(CFLAGS) -c $<

cmdline.c: cmdline.cli
	clig $<

webmash_4x20_client:$(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ) $(LDLIBS)

clean:
	rm -f *.o webmash_4x20_client *.1