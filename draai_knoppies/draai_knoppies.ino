
#define BEEPER 1

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

Beeper beeper;
#endif

enum {
    kStateInitial,
    kStateFinished,
};

int led_pin = 12;
int input_pin = 11;
int beeper_pin = 10;
int reset_pin = 9;
int state = kStateInitial;
uint32_t last_time = 0;
uint32_t led_disable_timer = 0;
bool led_state = false;

void setup() {
    // put your setup code here, to run once:

    pinMode(input_pin, INPUT_PULLUP);
    pinMode(reset_pin, INPUT_PULLUP);
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);

#if BEEPER
    setupBeeper(beeper_pin, COUNT_OF(beeper_frequencies), beeper_frequencies, &beeper);
#endif
}

void loop() {
    // put your main code here, to run repeatedly:

    if (digitalRead(reset_pin) == LOW) {
        state = kStateInitial;
        led_disable_timer = millis() + 2000;
        return;
    }

    if (state == kStateInitial) {
#if BEEPER
        updateBeeper(&beeper);
#endif
        uint32_t frequency = 500;
        uint32_t now = millis();
        if ((now - last_time) > frequency && now > led_disable_timer) {
            last_time = now;
            led_state = !led_state;
            digitalWrite(led_pin, led_state ? HIGH : LOW);
        }

        if (digitalRead(input_pin) == LOW) {
            state = kStateFinished;
        }

    } else {
        digitalWrite(led_pin, LOW);
#if BEEPER
        stopBeeper(&beeper);
#endif
    }
}
