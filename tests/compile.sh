#!/bin/bash

# TODO Makefile TODO #
gcc -o test-blink test-blink.c ../lib/PIN.c ../lib/GPIO.c -I../lib/
gcc -o test-button test-button.c ../lib/PIN.c ../lib/GPIO.c -I../lib/
