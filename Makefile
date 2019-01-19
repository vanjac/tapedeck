all: main.c
	gcc -o tapedeck.out main.c -lpulse-simple -lpulse

