DESTDIR = /usr/local
PREFIX = $(DESTDIR)

CFLAGS = -g -Wall -W -std=gnu99 -pedantic
LDLIBS = -lcurl -li2c

OBJ = 7segm_client.o jsmn.o cmdline.o 7-segm-ht16k33.o

all:webmash_7segm_client

%.o: %.c
	$(CC) $(EXTRAFLAGS) $(CFLAGS) -c $<

cmdline.c:
	clig $<

webmash_7segm_client:$(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ) $(LDLIBS)

clean:
	rm -f *.o webmash_7segm_client *.1