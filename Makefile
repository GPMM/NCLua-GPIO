CC=gcc
FLAGS=-Wall
CFLAGS=-I./lib -I/usr/include/lua5.2
LIBS=-llua5.2 -lpthread

all: lgpio.so

lgpio.so: lgpio.o
	$(CC) $(FLAGS) -fPIC -shared -Wl,-soname,lgpio.so -o lgpio.so ./lib/gpio_lib.o ./lib/gpio_event.o lgpio.o $(LIBS)
	strip lgpio.so

lgpio.o:
	$(MAKE) -C ./lib
	$(CC) $(FLAGS) -o lgpio.o -c lgpio.c $(CFLAGS) -lgpio_lib.o -lgpio_event.o $(LIBS)

clean:
	$(MAKE) clean -C ./lib
	rm *.o
	rm *.so
