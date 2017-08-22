#!/bin/bash

#	TODO Makefile TODO #
gcc -o test-blink test-blink.c ../lib/PIN.c ../lib/GPIO.c -I../lib/
