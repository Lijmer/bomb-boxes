#define COUNT_OF(x) (sizeof(x) / sizeof(*x))
#define DEBUGGING 0
#define LED_PATTERN 1
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

int reset_pin = 9;

int led_pin = 12;
int beeper_pin = 10;
int input_pins[] = {A0, A1, A2, A3, A4};

byte histories[COUNT_OF(input_pins)][32];
int history_index = 0;
void setup() {
#if DEBUGGING
    Serial.begin(9600);
#endif

    pinMode(reset_pin, INPUT_PULLUP);
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);

    for (int i = 0; i < COUNT_OF(input_pins); ++i) {
        for (int j = 0; j < COUNT_OF(histories[i]); ++j) {
            histories[i][j] = 0xff;
        }
    }

    setupBeeper(beeper_pin, COUNT_OF(beeper_frequencies), beeper_frequencies, &beeper);
}

int state = kStateInitial;
uint32_t last_time = 0;
bool led_state = 0;
uint32_t blink_disable_timer = 0;

void loop() {
    if (digitalRead(reset_pin) == LOW) {
        state = kStateInitial;
        history_index = 0;
        memset(histories, 0xff, sizeof(histories));

        blink_disable_timer = millis() + 2000;
        led_state = false;
        digitalWrite(led_pin, LOW);

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

        if (all_success) {
            state = kStateFinished;
        }
    } else {
        digitalWrite(led_pin, LOW);
        stopBeeper(&beeper);
    }
}
