#include <stdint.h>

#define BEEPER_FREQ_INFINITE 0xffff

struct BeeperFreq {
    uint16_t off_time; // how long beeper should stay off. BEEPER_FREQ_INFINITE -> don't turn on
    uint16_t on_time;  // how long beeper should stay on. BEEPER_FREQ_INFINITE -> don't turn off
    uint32_t duration; // how long until next frequency. 0 -> don't cycle to next frequency
};

struct Beeper {
    int pin_number;
    int freq_index;
    uint32_t last_time;
    uint32_t last_freq_update_time;

    BeeperFreq *frequencies;
    int freq_count;

    bool state;
};

void setupBeeper(int pin_number, int freq_count, BeeperFreq *frequencies, Beeper *out);
void changeFrequencies(Beeper *beeper, int freq_count, BeeperFreq *frequencies);
void updateBeeper(Beeper *beeper);
void stopBeeper(Beeper *beeper);

#ifdef BEEPER_IMPLEMENTATION

#include <Arduino.h>
#include <stdint.h>

#define COUNT_OF(x) (sizeof(x) / sizeof(*x))
void setupBeeper(int16_t pin_number, int freq_count, BeeperFreq *frequencies, Beeper *out) {
    uint32_t now = millis();

    *out = {};
    out->pin_number = pin_number;
    out->freq_index = 0;
    out->last_time = now;
    out->last_freq_update_time = now;

    out->frequencies = frequencies;
    out->freq_count = freq_count;

    out->state = false;

    pinMode(pin_number, OUTPUT);
}

void changeFrequencies(Beeper *beeper, int freq_count, BeeperFreq *frequencies) {
    beeper->freq_index = 0;
    beeper->freq_count = freq_count;
    beeper->frequencies = frequencies;

    uint32_t now = millis();
    beeper->last_time = now;
    beeper->last_freq_update_time = now;

    beeper->state = false;
    digitalWrite(beeper->pin_number, LOW);
}

void updateBeeper(Beeper *beeper) {
    uint32_t now = millis();
    BeeperFreq freq = beeper->frequencies[beeper->freq_index];

    uint16_t beeper_state_freq;
    if (beeper->state) { // Always turn beeper off after 100 ms
        beeper_state_freq = freq.on_time;
    } else {
        beeper_state_freq = freq.off_time;
    }

    if (beeper_state_freq != BEEPER_FREQ_INFINITE) {
        uint32_t elapsed = now - beeper->last_time;
        if (elapsed >= (uint32_t)beeper_state_freq) {
            beeper->last_time = now;

            beeper->state = !beeper->state;
            digitalWrite(beeper->pin_number, beeper->state ? HIGH : LOW);
        }
    }

    uint32_t freq_update_time = freq.duration;
    if (freq_update_time != 0) {
        uint32_t elapsed_freq_update = now - beeper->last_freq_update_time;
        if (elapsed_freq_update >= freq_update_time) {
            beeper->last_freq_update_time = now;
            beeper->freq_index = (beeper->freq_index + 1) % beeper->freq_count;
        }
    }
}
void stopBeeper(Beeper *beeper) {
    beeper->state = false;
    digitalWrite(beeper->pin_number, LOW);
}

#endif
