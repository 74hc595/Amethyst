#include "audio.h"
#include "defs.h"
#include "isr_reserved_registers.h"
#include "note_freqs.h"

void audio_init()
{
  /* Timer0 and OC0B is used to generate audio */
  DDRB |= _BV(4);
  OCR0A = 0;
  OCR0B = 0;
  TCCR0A = _BV(COM0B1) | _BV(WGM00);
  TCCR0B = _BV(WGM02) | _BV(CS02); /* default to 1/256 prescaler */
}


void audio_off()
{
  /* disable audio pin output, but keep the timer configured */
  DDRB &= ~_BV(4);
}


void tone(uint16_t period, uint8_t duration, uint8_t duty_cycle)
{
  /* Calculate prescaler and OCR0A values. */
  uint8_t tccrb, ocra, ocrb;
  /* Frequencies 0-255 use 1/8 prescaler */
  if (period < 256) {
    tccrb = _BV(WGM02) | _BV(CS01);
    ocra = period;
  }
  /* Frequencies 256-479 use 1/64 prescaler */
  else if (period < 480) {
    tccrb = _BV(WGM02) | _BV(CS01) | _BV(CS00);
    ocra = period-224;
  }
  /* Frequencies 480-671 use 1/256 prescaler */
  else if (period < 672) {
    tccrb = _BV(WGM02) | _BV(CS02);
    ocra = period-416;
  }
  /* Frequencies 672-863 use 1/1024 prescaler */
  /* Behavior of frequencies 864 and above is undefined */
  else {
    tccrb = _BV(WGM02) | _BV(CS02) | _BV(CS00);
    ocra = period-608;
  }

  /* Convert duty cycle percentage to OCR0B value */
  ocrb = (ocra*duty_cycle) >> 8;

  /* Execution order is important here. The duration needs to be set first. */
  /* Otherwise, if frequency is set first, and the tone generator is already */
  /* active, and you get a frame interrupt immediately afterward that causes */
  /* tone_duration to be decremented to zero, OCR0A and OCR0B will */
  /* immediately get overwritten with zeros, causing no sound output. */
  tone_duration = duration;
  TCCR0B = tccrb;
  OCR0A = ocra;
  OCR0B = ocrb;
  set_bit(tone_enabled, tone_enabled_bit);
}


void note(enum note_number note_num, uint8_t duration, uint8_t duty_cycle)
{
  note_num &= 0x7f; /* ignore high bit */
  note_def nd = { .n = pgm_read_word(note_freqs+note_num) };
  uint8_t ocrb = (nd.ocra*duty_cycle) >> 8;
  tone_duration = duration;
  TCCR0B = nd.tccrb;
  OCR0A = nd.ocra;
  OCR0B = ocrb;
  set_bit(tone_enabled, tone_enabled_bit);
}
