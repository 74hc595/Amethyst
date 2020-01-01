/**
 * Maps MIDI note numbers to timer parameters.
 */
#ifndef NOTE_FREQS_H_
#define NOTE_FREQS_H_

#include <avr/io.h>
#include <avr/pgmspace.h>

typedef union {
  struct {
    uint8_t tccrb;
    uint8_t ocra;
  };
  uint16_t n;
} note_def;

extern const note_def note_freqs[128] PROGMEM;

#endif
