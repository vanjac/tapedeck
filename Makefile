HFILES = instinct.h interface.h tape.h
CFILES = instinct.c interface.c main.c

all: $(HFILES) $(CFILES)
	gcc -o tapedeck.out $(CFILES) -lpulse-simple -lpulse

