#include <stdint.h>
#include "io.h"

#define PIT_CHANNEL2      0x42
#define PIT_COMMAND_PORT  0x43
#define SPEAKER_PORT      0x61

// simple busy-wait delay for approx milliseconds (not precise)
void delay_ms(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 1000; i++) {
        __asm__ volatile("nop");
    }
}

void speaker_beep(uint32_t freq_hz) {
    if (freq_hz == 0) return;

    uint32_t divisor = 1193182 / freq_hz;
    if (divisor == 0) divisor = 1;
    if (divisor > 0xFFFF) divisor = 0xFFFF;

    outb(PIT_COMMAND_PORT, 0xB6); 
    outb(PIT_CHANNEL2, divisor & 0xFF);
    outb(PIT_CHANNEL2, (divisor >> 8) & 0xFF);

    uint8_t tmp = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, tmp | 3);
}

void speaker_off() {
    uint8_t tmp = inb(SPEAKER_PORT) & ~3;
    outb(SPEAKER_PORT, tmp);
}

// play a pattern of tones with durations (freq in Hz, duration in ms)
// pattern: array of (freq, duration) pairs, length is number of pairs
void speaker_play_pattern(const uint32_t* pattern, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        uint32_t freq = pattern[i * 2];
        uint32_t dur = pattern[i * 2 + 1];
        if (freq == 0) {
            speaker_off();
        } else {
            speaker_beep(freq);
        }
        delay_ms(dur);
    }
    speaker_off();
}
