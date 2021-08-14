#define COUNT_OF(x) (sizeof(x) / sizeof(*x))
#define TANAGRAM 0
#define DEBUGGING 0
#define LED_PATTERN 0
#define BEEPER 1

#if BEEPER
#define BEEPER_IMPLEMENTATION
#include "beeper.h"
Beeper beeper;
#endif

enum {
    kStateInitial = 0,
    kStateFinished = 1,
};

#if LED_PATTERN
int led_pins[][3] = {{2, 0, 6}, {7, 4, 8}, {3, 5, 9}};
#endif

int led_pin = 12;
int beeper_pin = 10;
int reset_pin = 9;
int input_pins[] = {A0, A1, A2, A3, A4};

byte histories[COUNT_OF(input_pins)][32];
int history_index = 0;
void setup() {
#if DEBUGGING
    Serial.begin(9600);
#endif
#if LED_PATTERN
    for (int i = 0; i < COUNT_OF(led_pins); ++i) {
        for (int j = 0; j < COUNT_OF(led_pins[i]); ++j) {
            pinMode(led_pins[i][j], OUTPUT);
            digitalWrite(led_pins[i][j], LOW);
        }
    }
#endif
    pinMode(reset_pin, INPUT_PULLUP);
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);

    for (int i = 0; i < COUNT_OF(input_pins); ++i) {
        for (int j = 0; j < COUNT_OF(histories[i]); ++j) {
            histories[i][j] = 0xff;
        }
    }

    setupBeeper(beeper_pin, &beeper);
}

int state = kStateInitial;
int last_time = 0;
bool led_state = 0;
#if LED_PATTERN
int last_time_pattern = 0;
int pin_index = 0;
#endif
void loop() {
    if (digitalRead(reset_pin) == LOW) {
        state = kStateInitial;
        history_index = 0;
        memset(histories, 0, sizeof(histories));
        return;
    }

    if (state == kStateInitial) {
        {
            int frequency = 500;
            int now = millis();
            if ((now - last_time) > frequency) {
                last_time = now;
                led_state = !led_state;
                digitalWrite(led_pin, led_state ? HIGH : LOW);
            }
        }

        updateBeeper(&beeper);

#if DEBUGGING
        long averages[5] = {};
#endif
        for (int i = 0; i < COUNT_OF(input_pins); ++i) {
            int reading = analogRead(input_pins[i]);
            histories[i][history_index] = reading;

#if DEBUGGING
            for (int j = 0; j < 32; ++j) {
                averages[i] += histories[i][j];
            }
#endif
        }

#if DEBUGGING
        for (int i = 0; i < COUNT_OF(input_pins); ++i) {
            averages[i] = averages[i] / 32;
        }
        char str[256];
        sprintf(str, "%d, %d, %d, %d, %d", (int)averages[0], (int)averages[1], (int)averages[2],
                (int)averages[3], (int)averages[4]);
        Serial.println(str);
#endif
        history_index = (history_index + 1) % COUNT_OF(histories[0]);

        bool all_success = true;
        bool success_array[5] = {};
        for (int i = 0; i < COUNT_OF(histories); ++i) {
            int success = 0;
            for (int j = 0; j < COUNT_OF(histories[i]); ++j) {
                if (histories[i][j] < 10) {
                    success += 1;
                }
            }
            if (success < 24) {
                success_array[i] = true;
                all_success = false;
            }
        }

#if LED_PATTERN
        {
            int frequency = 100;
            int now = millis();
            if ((now - last_time_pattern) > frequency) {
                last_time_pattern = now;

                for (int j = 0; j < 3; ++j) {
                    digitalWrite(led_pins[pin_index][j], LOW);
                }
                pin_index = (pin_index + 1) % COUNT_OF(led_pins);
                for (int j = 0; j < 3; ++j) {
                    int i = pin_index;
                    if ((i == 0 && j == 0) || (i == 1 && j == 0) || (i == 2 && j == 0) ||
                        (i == 0 && j == 2)) {
                        digitalWrite(led_pins[pin_index][j], success_array[i] ? HIGH : LOW);
                    } else {
                        digitalWrite(led_pins[pin_index][j], HIGH);
                    }
                }
            }
        }
#endif

        if (all_success) {
            state = kStateFinished;
        }
    } else {
#if LED_PATTERN
        for (int i = 0; i < COUNT_OF(led_pins); ++i) {
            for (int j = 0; j < COUNT_OF(led_pins[i]); ++j) {
                digitalWrite(led_pins[i][j], LOW);
            }
        }
#endif
        digitalWrite(led_pin, LOW);
        stopBeeper(&beeper);
    }
}
