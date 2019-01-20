HFILES = audio.h instinct.h interface.h tape.h main.h
CFILES = audio.c instinct.c interface.c tape.c main.c

all: $(HFILES) $(CFILES)
	gcc -o tapedeck.out $(CFILES) -lpulse-simple -lpulse

