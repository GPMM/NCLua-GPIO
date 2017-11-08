CC=gcc
FLAGS=-Wall

all: gpio_lib.o gpio_event.o

gpio_lib.o: 
	$(CC) $(FLAGS) -o gpio_lib.o -c gpio_lib.c

gpio_event.o:
	$(CC) $(FLAGS) -o gpio_event.o -c gpio_event.c

clean:
	rm *.o
