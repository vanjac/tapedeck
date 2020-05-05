# tapedeck

Tapedeck is an audio editor for Linux with an
alternative control interface. Instead of a mouse,
keyboard, and screen, it uses a DJ board (the [Hercules
DJControl Instinct](https://support.hercules.com/en/product/djcontrolinstinct-en/))
and a set of LED graph displays (not currently
implemented, simulated with terminal graphics).

## Building/Running

Required components: ALSA

Required development libraries: PulseAudio, ncurses

Build with `make`. Compiles to `tapedeck.out`. Run in a
terminal. To run you must have the DJ board connected
as a MIDI device, one audio input device, and two audio
output devices (one of which is defined in `audio.c`).

Tapes are read and stored as `.wav` files in the current
directory. They are named with a letter (L,E,S,C) and a
number (1 through 4), like `L1.wav`. Generally tapedeck
can only read `.wav` files that it has written.

## Interface Components

### Tape Decks

The audio editing capabilities are built around two
virtual tape decks, each of which is loaded with one
virtual tape at a time. A tape has boundaries which can
expand, an In point, and an Out point. The controls for
each tape mirror each other on the left and right sides
of the console.

### Mixer

The controls for the mixer are in the center section of
the console.

The mixer has 3 inputs: both tape decks and the external
audio input. Each has a volume control, and their
signals are summed together. All mixer inputs are sent
to any tape deck with recording enabled.

The mixer has two outputs: Monitor and Aux-Send.
External input is sent to monitor. Tape output can be
toggled between monitor and aux-send output (see below).

It is often useful to connect a loopback cable from the
aux-send to input. This can be used to copy/mix data
between tapes.

## DJ Board Controls

Per tape:

- Jog wheel: Jog tape
- Vertical slider: Tape volume
- Play/pause button: Play/pause tape
- Pitch bend: Fast-forward/rewind
- Cue button: Record switch. Tape will only record while
it is playing. Recorded audio overwrites existing audio.
To mix audio instead, enable Loopback.

    While recording is enabled, the tape's boundaries
    can expand if you jog/play past them.

- Headphone button: Toggle tape output to aux-send. See
Mixer section above.
- -/+ buttons: Set in/out point. Hold to cut the
beginning/end of the tape off at this point.
- RW button: Jump backwards to out point, in point, or
start of tape.
- FF button: Jump forwards to in point, out point, or
end of tape.
- Sync button: Toggle action when out point is reached
while playing.
    - Off: Continue
    - On: Loop to in point
    - Flashing: Stop
- Number keypad: Swap tapes. There are 16 tapes in a
library, split across 4 tabs. Use the Mode button to
switch between the tabs.

Other controls:

- Headphone -/+ buttons: Monitor volume (doesn't affect
Loopback)
- Horizontal slider: Audio input level
- Vinyl button: Toggle linked tapes. When enabled,
moving either jog wheel or pressing either play/pause
button will control both tapes.
- Down arrow button: Exit

## LED graphs

(Currently there's no support for real LEDs. They are
simulated with terminal graphics.)

There is a row of 20 LEDs for the Monitor VU meter.

Each tape has 2 rows of 10 LEDs: the position graph and
the in/out graph.

Position graph:

```
..........  At start
#.........  Just past start
#####.....  Middle
#########.  Just before end
##########  At end
.........#  Tape has no data
```

In/out graph:

```
..........  Before in point
#.........  At in point
##........  Just past in point
#####.....  Between in/out points
#########.  Just before out point
##########  At out point
.........#  After out point
#........#  At in and out point (they are the same)
```

