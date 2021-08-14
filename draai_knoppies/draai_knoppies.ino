
#define BEEPER 1

#if BEEPER
#define BEEPER_IMPLEMENTATION
#include "../libs/beeper/beeper.h"
Beeper beeper;
#endif

enum {
    kStateInitial,
    kStateFinished,
};

int led_pin = 12;
int input_pin = 11;
int beeper_pin = 10;
int state = kStateInitial;
int last_time = 0;
bool led_state = false;

void setup() {
    // put your setup code here, to run once:

    pinMode(input_pin, INPUT_PULLUP);
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);

#if BEEPER
    setupBeeper(beeper_pin, &beeper);
#endif
}

void loop() {
    // put your main code here, to run repeatedly:

    if (state == kStateInitial) {
#if BEEPER
        updateBeeper(&beeper);
#endif
        int frequency = 500;
        int now = millis();
        if ((now - last_time) > frequency) {
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
