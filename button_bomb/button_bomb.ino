#define BEEPER 1

#define COUNT_OF(x) (sizeof(x) / sizeof(*x))

#if BEEPER
#define BEEPER_IMPLEMENTATION
#include "../libs/beeper/beeper.h"
Beeper beeper;
#endif

enum {
    kStateInitial = 0,
    kStateFailed = 1,
    kStateSuccess = 2,
};
int blink_frequencies[] = {500, 100, 0};
int led_pin = 12;
int beeper_pin = 11;
int button_pins[] = {2, 3, 4, 5, 6, 7, 8};

void setup() {
    pinMode(led_pin, OUTPUT);

    for (int i = 0; i < COUNT_OF(button_pins); ++i) {
        pinMode(button_pins[i], INPUT_PULLUP);
    }

#if BEEPER
    setupBeeper(beeper_pin, &beeper);
#endif
}

// int sequence[] = { 6, 0, 4, 6, 0, 1, 5, 2, 5, 0, 3 };
int sequence[] = {2, 1, 6, 5, 4, 3, 0};
int playback_counter = 0;
int last_time = 0;
bool led_state = false;
int state = kStateInitial;

void loop() {

#if BEEPER
    if (state != kStateSuccess) {
        updateBeeper(&beeper);
    } else {
        stopBeeper(&beeper);
    }
#endif

    // set lights
    int frequency = blink_frequencies[state];
    if (state != kStateSuccess) {
        int now = millis();
        if ((now - last_time) > frequency) {
            last_time = now;

            led_state = !led_state;
            digitalWrite(led_pin, led_state ? HIGH : LOW);
        }
    } else {
        digitalWrite(led_pin, LOW);
    }

    // Read input
    if (state == kStateInitial) {
        for (int i = 0; i < COUNT_OF(button_pins); ++i) {
            bool current_state = digitalRead(button_pins[i]) == LOW;

            if (current_state) {
                // Key down
                if (i == sequence[playback_counter]) {
                    // correct
                    playback_counter += 1;

                    if (playback_counter == COUNT_OF(sequence)) {
                        state = kStateSuccess;
                        break;
                    }
                } else if (playback_counter == 0 || i != sequence[playback_counter - 1]) {
                    // incorrect
                    state = kStateFailed;
                    break;
                }
            }
        }
    }
}
