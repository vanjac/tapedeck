HFILES = audio.h display.h file.h instinct.h interface.h tape.h main.h
CFILES = audio.c display.c file.c instinct.c interface.c tape.c main.c
C_FLAGS = -std=c99 -pedantic -Wall -Wextra

all: $(HFILES) $(CFILES)
	gcc $(C_FLAGS) -o tapedeck.out $(CFILES) -lpulse-simple -lpulse -lncurses -lm

