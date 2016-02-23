PROG=toHD44780
LIB=libutf8tolcd.a

CFLAGS = -g -Wall -W -std=gnu99
LDLIBS=$(LIB)
OBJ = utf8tohd44780.o toHD44780.o

all: $(LIB) $(PROG)

%.o: %.c
	$(CC) $(EXTRAFLAGS) $(CFLAGS) -l$(LIB) -L. -c $<

$(LIB): utf8tohd44780.o
	ar rcs $@ $^
	
clean:
	rm -f *.a *.o $(PROG)

