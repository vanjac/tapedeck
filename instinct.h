#include <stdbool.h>

// https://www.alsa-project.org/main/index.php/ALSA_Library_API
#define MIDI_DEVICE "/dev/snd/midiC0D0"

/* BUTTONS */
// (also for LED outputs)

#define NUM_BUTTONS         66

// start of buttons for each deck
#define BTNS_DECK_A         1
#define BTNS_DECK_B         27
#define NUM_DECK_NOTES      26

// specific to deck (add BTNS_DECK_A/B)
#define BTN_DECK_FX_KP1     0 // keypad button 1, in Effect mode
#define BTN_DECK_FX_KP2     1
#define BTN_DECK_FX_KP3     2
#define BTN_DECK_FX_KP4     3
#define BTN_DECK_SAMPLE_KP1 4
#define BTN_DECK_SAMPLE_KP2 5
#define BTN_DECK_SAMPLE_KP3 6
#define BTN_DECK_SAMPLE_KP4 7
#define BTN_DECK_LOOP_KP1   8
#define BTN_DECK_LOOP_KP2   9
#define BTN_DECK_LOOP_KP3   10
#define BTN_DECK_LOOP_KP4   11
#define BTN_DECK_CUE_KP1    12
#define BTN_DECK_CUE_KP2    13
#define BTN_DECK_CUE_KP3    14
#define BTN_DECK_CUE_KP4    15
#define BTN_DECK_PBM        16 // pitch bend minus
#define BTN_DECK_PBP        17
#define BTN_DECK_PREV       18
#define BTN_DECK_NEXT       19
#define BTN_DECK_CUE        20
#define BTN_DECK_PLAY       21
#define BTN_DECK_SYNC       22
#define BTN_DECK_LISTEN     23
#define BTN_DECK_LOAD       24
#define BTN_DECK_TOUCH      25 // press jog wheel

#define BTN_SCRATCH         53 // vinyl
#define BTN_UP              54
#define BTN_DOWN            55
#define BTN_FILES           56
#define BTN_FOLDERS         57
#define BTN_HP_VOL_M        64 // headphone volume minus
#define BTN_HP_VOL_P        65

/* CONTROLS */

#define NUM_CONTROLS        64

#define CTL_JOG_DA          48
#define CTL_JOG_DB          49
#define CTL_PITCH_DA        52
#define CTL_PITCH_DB        53
#define CTL_VOL_DA          54
#define CTL_TREBLE_DA       55
#define CTL_MEDIUM_DA       56
#define CTL_BASS_DA         57
#define CTL_XFADER          58
#define CTL_VOL_DB          59
#define CTL_TREBLE_DB       60
#define CTL_MEDIUM_DB       61
#define CTL_BASS_DB         62


// 0 for not currently pressed
// nonzero for time button was pressed
long button_presses[NUM_BUTTONS];

long control_values[NUM_CONTROLS];

int instinct_open(void);
void instinct_close(void);
int instinct_update(void);

void set_led(int led, bool state);

