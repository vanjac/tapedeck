#include "interface.h"
#include <stdio.h>

void button_pressed(int button) {
    printf("Note on %d\n", button);
}

void button_released(int button) {
    printf("Note off %d\n", button);
}

void control_changed(int control, int value) {
    printf("Control %d set to %d\n", control, value);
}

