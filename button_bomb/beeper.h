#include <stdint.h>
struct Beeper {
    int pin_number;
    int freq_index;
    uint32_t last_time;
    uint32_t last_freq_update_time;
    bool state;
};

void setupBeeper(int pin_number, Beeper *out);
void updateBeeper(Beeper *beeper);
void stopBeeper(Beeper *beeper);

#ifdef BEEPER_IMPLEMENTATION

#include <Arduino.h>
#include <stdint.h>

#define COUNT_OF(x) (sizeof(x) / sizeof(*x))

uint32_t frequencies[] = {5000, 1000, 500, 200, 0};
uint32_t freq_stay_times[] = {60 * 1000L, 60 * 1000L, 30 * 1000L, 10 * 1000L, 60 * 1000L};

void setupBeeper(int16_t pin_number, Beeper *out) {
    *out = {};
    out->pin_number = pin_number;
    out->freq_index = 0;
    out->last_time = millis();
    out->last_freq_update_time = millis();
    out->state = false;

    pinMode(pin_number, OUTPUT);
}

void updateBeeper(Beeper *beeper) {
    uint32_t now = millis();
    uint32_t freq = frequencies[beeper->freq_index];
    if (beeper->state) { // Always turn beeper off after 100 ms
        freq = 100;
    }

    if (freq != 0) {
        uint32_t elapsed = now - beeper->last_time;
        if (elapsed > freq) {
            beeper->last_time = now;
            beeper->state = !beeper->state;
            digitalWrite(beeper->pin_number, beeper->state ? HIGH : LOW);
        }
    }

    uint32_t elapsed_freq_update = now - beeper->last_freq_update_time;
    uint32_t freq_update_time = freq_stay_times[beeper->freq_index];
    if (elapsed_freq_update > freq_update_time) {
        beeper->last_freq_update_time = now;
        beeper->freq_index = (beeper->freq_index + 1) % COUNT_OF(frequencies);
    }
}
void stopBeeper(Beeper *beeper) {
    beeper->state = false;
    digitalWrite(beeper->pin_number, LOW);
}

#endif
