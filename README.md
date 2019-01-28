# tapedeck

Tapedeck is an audio editor for Linux with an
alternative control interface. Instead of a mouse,
keyboard, and screen, it uses a DJ board (the Hercules
DJControl Instinct) and a set of LED graph displays
(not currently implemented, simulated with terminal
graphics).

## Building/Running

Required: ALSA

Required development libraries: PulseAudio, ncurses

Build with `make`. Compiles to `tapedeck.out`. Run in a
terminal. To run you must have the DJ board connected
as a MIDI device, and sound input/output devices.

Tapes are currently loaded/saved to `a.wav` and `b.wav`.
Eventually there will be controls to select different
files.

## Interface Components

### Tape Decks

The audio editing capabilities are built around two
virtual tape decks, each of which is loaded with one
virtual tape at a time. A tape has boundaries which can
expand, an In point, and an Out point. The controls for
each tape mirror each other on the left and right sides
of the DJ board.

### Mixer

The mixer has 3 inputs: both tape decks and the external
audio input. Each has a volume control, and their
signals are summed together.

The mixer has two outputs: Monitor and Loopback. The
Monitor output includes all inputs and is sent to
the speakers. The Loopback output is sent to any tapes
with recording enabled. Tapes are only included in the
Loopback output if their Loopback switch is on.

## DJ Board Controls

Per tape:

- Jog wheel: Jog tape
- Vertical slider: Tape volume
- Play/pause button: Play/pause tape
- Cue button: Record switch. Tape will only record while
it is playing. Recorded audio overwrites existing audio.
To mix audio instead, enable Loopback.

    While recording is enabled, the tape's boundaries
    can expand if you jog/play past them.

- Headphone button: Loopback switch. See Mixer section
above.
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

Other controls:

- -/+ buttons: Monitor volume (doesn't affect Loopback)
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
```

