#ifndef AUDIO_H_
#define AUDIO_H_

#include <stdint.h>
#include "note_numbers.h"

void audio_init(void);
void audio_off(void);

/* Emit a pulse wave with the given duty cycle. (128 = 50% square wave) */
/* Higher period values produce lower-pitched sounds. */
enum {
  PULSE_WAVE_NARROW = 32, /* 12.5% */
  PULSE_WAVE_WIDE   = 64, /* 25% duty cycle */
  SQUARE_WAVE       = 128,
};
void tone(uint16_t period, uint8_t duration, uint8_t duty_cycle);

/* Valid note numbers are 0-127 inclusive. */
void note(enum note_number note_num, uint8_t duration, uint8_t duty_cycle);

#endif
