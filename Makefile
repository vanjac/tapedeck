all: main.c instinct.h tape.h
	gcc -o tapedeck.out main.c -lpulse-simple -lpulse

