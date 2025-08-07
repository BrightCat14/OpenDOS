#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdint.h>

// start beep at given frequency in Hz, 0 to stop beep
void speaker_beep(uint32_t freq_hz);

// stop the beep sound
void speaker_off(void);

// play pattern of tones: array of (frequency, duration_ms) pairs
// length is number of pairs in the pattern
void speaker_play_pattern(const uint32_t* pattern, uint32_t length);

// simple blocking delay in milliseconds
void delay_ms(uint32_t ms);

#endif // SPEAKER_H
