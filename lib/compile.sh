# TODO Makefile TODO

gcc -o GPIO.o -c GPIO.c
gcc -o lua_gpio.o -c lua_gpio.c -l GPIO.o -I/usr/include/lua5.2 -llua5.2
gcc -fPIC -shared -Wl,-soname,lgpio.so -o lgpio.so GPIO.o lua_gpio.o -llua5.2
