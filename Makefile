HFILES = instinct.h interface.h tape.h main.h
CFILES = instinct.c interface.c tape.c main.c

all: $(HFILES) $(CFILES)
	gcc -o tapedeck.out $(CFILES) -lpulse-simple -lpulse

