CFLAGS = -g -Wall -W -std=gnu99 -pedantic

SRC = external.c gpio.c onewire.c searchXfile.c

%.o: %.c
	$(CC) -c -o $@ $< -I. -I.. -fPIC $(CFLAGS)
	
ALL: actuator_external.so actuator_onewire.so actuator_gpio.so sensor_onewire.so

actuator_external.so: actuator_external.o searchXfile.o
	$(CC) -lm -shared -o $@ $?
	
actuator_onewire.so: actuator_onewire.o
	$(CC) -lm -shared -o $@ $?
	
actuator_gpio.so: actuator_gpio.o
	$(CC) -lm -shared -o $@ $?
	
sensor_onewire.so: sensor_onewire.o
	$(CC) -lm -shared -o $@ $?

clean:
	rm -f *.o *.so
