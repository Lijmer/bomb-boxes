#define COUNT_OF(x) (sizeof(x) / sizeof(*x))
#define DEBUGGING 0
#define BEEPER 0

#if BEEPER
#define BEEPER_IMPLEMENTATION
#include "beeper.h"

BeeperFreq beeper_frequencies[] = {
    {4800, 200, 60 * 1000L},               // 60
    {800, 200, 60 * 1000L},                // 60
    {400, 100, 30 * 1000L},                // 30
    {100, 100, 10 * 1000L},                // 10
    {0, BEEPER_FREQ_INFINITE, 3 * 1000L},  // 3
    {BEEPER_FREQ_INFINITE, 0, 13 * 1000L}, // 13
    {100, 100, 600L},                      // 0.6
    {BEEPER_FREQ_INFINITE, 0, 17 * 1000L}, // 17
};

Beeper beeper;
#endif

enum {
    kStateInitial = 0,
    kStateFinished = 1,
};

int led_pins[][3] = {{2, 0, 6}, {7, 4, 8}, {3, 5, 9}};
int led_transl[][3] = {{2, 0 /*unused*/, 5}, {4, 1, 7}, {6, 3, 0}};
int reset_pin = 11;

int led_pin = 12;
int beeper_pin = 10;
int input_pins[] = {A0, A1, A2, A3, A4, A5, 1};
void setup() {
#if DEBUGGING
    Serial.begin(9600);
#endif
    for (int i = 0; i < COUNT_OF(led_pins); ++i) {
        for (int j = 0; j < COUNT_OF(led_pins[i]); ++j) {
            pinMode(led_pins[i][j], OUTPUT);
            digitalWrite(led_pins[i][j], LOW);
        }
    }

    pinMode(reset_pin, INPUT_PULLUP);
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);

    for (int i = 0; i < COUNT_OF(input_pins); ++i) {
        pinMode(input_pins[i], INPUT);
    }
#if BEEPER
    setupBeeper(beeper_pin, COUNT_OF(beeper_frequencies), beeper_frequencies, &beeper);
#endif
}

int state = kStateInitial;
uint32_t last_time = 0;
bool led_state = 0;
uint32_t blink_disable_timer = 0;
uint32_t last_time_pattern = 0;
int pin_index = 0;

void loop() {
    if (digitalRead(reset_pin) == LOW) {
        state = kStateInitial;

        blink_disable_timer = millis() + 2000;
        led_state = false;
        digitalWrite(led_pin, LOW);

        for (int i = 0; i < COUNT_OF(led_pins); ++i) {
            for (int j = 0; j < COUNT_OF(led_pins[i]); ++j) {
                digitalWrite(led_pins[j][i], LOW);
            }
        }
        return;
    }

    if (state == kStateInitial) {
        {
            int frequency = 500;
            uint32_t now = millis();
            if ((now - last_time) > frequency && now > blink_disable_timer) {
                last_time = now;
                led_state = !led_state;
                digitalWrite(led_pin, led_state ? HIGH : LOW);
            }
        }

#if BEEPER
        updateBeeper(&beeper);
#endif

        bool all_success = true;
        bool readings[COUNT_OF(input_pins)];
        for (int i = 0; i < COUNT_OF(input_pins); ++i) {
            bool reading = digitalRead(input_pins[i]) == LOW;
            if (!reading) {
                all_success = false;
            }
            readings[i] = reading;
        }

#if DEBUGGING
        for (int i = 0; i < COUNT_OF(input_pins) - 1; ++i) {
            Serial.print(readings[i]);
            Serial.print(", ");
        }
        Serial.print(readings[COUNT_OF(input_pins) - 1]);
        Serial.println();
#endif

        {
            uint32_t frequency = 100;
            uint32_t now = millis();
            if ((now - last_time_pattern) > frequency && now > blink_disable_timer) {
                last_time_pattern = now;

                for (int j = 0; j < 3; ++j) {
                    digitalWrite(led_pins[pin_index][j], LOW);
                }
                pin_index = (pin_index + 1) % COUNT_OF(led_pins);
                for (int j = 0; j < 3; ++j) {
                    int i = pin_index;

                    int transl = led_transl[i][j];
                    int target_state = HIGH;
                    if (readings[transl]) {
                        target_state = LOW;
                    }
                    digitalWrite(led_pins[pin_index][j], target_state);
                }
            }
        }

        if (all_success) {
            state = kStateFinished;
        }
    } else {
        for (int i = 0; i < COUNT_OF(led_pins); ++i) {
            for (int j = 0; j < COUNT_OF(led_pins[i]); ++j) {
                digitalWrite(led_pins[i][j], LOW);
            }
        }
        digitalWrite(led_pin, LOW);
#if BEEPER
        stopBeeper(&beeper);
#endif
    }
}
