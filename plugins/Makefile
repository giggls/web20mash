CFLAGS = -g -Wall -W -std=gnu99 -pedantic

SRC = external.c gpio.c onewire.c searchXfile.c

%.o: %.c
	$(CC) -c -o $@ $< -I. -I.. -fPIC $(CFLAGS)
	
ALL: actuator_external.so actuator_onewire.so actuator_gpio.so sensor_onewire.so sensor_temper1.so

actuator_external.so: actuator_external.o searchXfile.o
	$(CC) -shared -o $@ $?
	
actuator_onewire.so: actuator_onewire.o
	$(CC) -shared -o $@ $?
	
actuator_gpio.so: actuator_gpio.o
	$(CC) -shared -o $@ $?
	
sensor_onewire.so: sensor_onewire.o
	$(CC) -shared -o $@ $?
	
sensor_temper1.so: sensor_temper1.o
	$(CC) -lusb -shared -o $@ $?

clean:
	rm -f *.o *.so
