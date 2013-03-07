CFLAGS = -g -Wall -W -std=gnu99 -pedantic

SRC = external.c gpio.c onewire.c searchXfile.c

%.o: %.c
	$(CC) -c -o $@ $< -I. -I.. -fPIC $(CFLAGS)
	
ALL: actuator_external.so actuator_onewire.so actuator_gpio.so

actuator_external.so: external.o searchXfile.o
	$(CC) -lm -shared -o $@ $?
	
actuator_onewire.so: onewire.o
	$(CC) -lm -shared -o $@ $?
	
actuator_gpio.so: gpio.o
	$(CC) -lm -shared -o $@ $?

clean:
	rm -f *.o *.so
