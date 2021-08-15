#define BEEPER 1

#define COUNT_OF(x) (sizeof(x) / sizeof(*x))

#if BEEPER
#define BEEPER_IMPLEMENTATION
#include "beeper.h"

BeeperFreq beeper_frequencies[] = {
    {4800, 200, 60 * 1000L},               // 60
    {800, 200, 60 * 1000L},                // 60
    {400, 100, 30 * 1000L},                // 30
    {100, 100, 10 * 1000L},                // 10
    {0, BEEPER_FREQ_INFINITE, 3 * 1000L},  // 3
    {BEEPER_FREQ_INFINITE, 0, 30 * 1000L}, // 30
};
BeeperFreq beeper_failed_freqs[] = {{100, 100, 0L}};

Beeper beeper;
#endif

enum {
    kStateInitial = 0,
    kStateFailed = 1,
    kStateSuccess = 2,
};
int blink_frequencies[] = {500, 100, 0};
int led_pin = 12;
int beeper_pin = 10;
int reset_pin = 9;
int button_pins[] = {2, 3, 4, 5, 6, 7, 8};
uint32_t blink_disable_timer = 0;

void setup() {
    pinMode(led_pin, OUTPUT);
    pinMode(reset_pin, INPUT_PULLUP);

    for (int i = 0; i < COUNT_OF(button_pins); ++i) {
        pinMode(button_pins[i], INPUT_PULLUP);
    }

#if BEEPER
    setupBeeper(beeper_pin, COUNT_OF(beeper_frequencies), beeper_frequencies, &beeper);
#endif
}

// int sequence[] = { 6, 0, 4, 6, 0, 1, 5, 2, 5, 0, 3 };
int sequence[] = {2, 1, 6, 5, 4, 3, 0};
int playback_counter = 0;
uint32_t last_time = 0;
bool led_state = false;
int state = kStateInitial;

void loop() {

#if BEEPER
    switch (state) {
    case kStateInitial:
    case kStateFailed:
        updateBeeper(&beeper);
        break;
    case kStateSuccess:
        stopBeeper(&beeper);
        break;
    default:
        break;
    }
#endif

    if (digitalRead(reset_pin) == LOW) { // pressed
        if (state == kStateFailed) {
            changeFrequencies(&beeper, COUNT_OF(beeper_frequencies), beeper_frequencies);
        }

        blink_disable_timer = millis() + 2000;
        led_state = false;
        digitalWrite(led_pin, LOW);
        state = kStateInitial;
        playback_counter = 0;
        return;
    }

    // set lights
    int frequency = blink_frequencies[state];
    if (state != kStateSuccess) {
        uint32_t now = millis();
        if ((now - last_time) > frequency && now > blink_disable_timer) {
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
                    changeFrequencies(&beeper, COUNT_OF(beeper_failed_freqs), beeper_failed_freqs);
                    break;
                }
            }
        }
    }
}
