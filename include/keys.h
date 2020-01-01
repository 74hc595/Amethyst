#ifndef KEYS_H_
#define KEYS_H_

#include <stdbool.h>
#include <stdint.h>
#include "keycodes.h"

/* Keyboard matrix state. Only the lower 6 bits of each byte are significant. */
/* 0 bits indicate pressed keys. */
/* Updated during the first 10 scanlines of active video. */
extern uint8_t keymatrixrows[KEYMATRIX_ROWS];
#define key_is_pressed(k) (!(keymatrixrows[k##_ROW] & (1<<(k##_COL))))


struct keyreader {
  uint8_t lastkeys[KEYMATRIX_ROWS];
  uint8_t repeat_count;
};

/* col and row are 1-indexed. if 0, no key is down */
typedef union {
  struct {
    unsigned col:3;
    unsigned row:3;
    unsigned :1;
    unsigned isdown:1;
  };
  struct {
    unsigned index:6;
    unsigned :2;
  };
  uint8_t value;
} __attribute__((packed)) key_code;

bool key_down_pending(struct keyreader *kr);

key_code get_key_down(struct keyreader *kr);

/**
 * Returns the ASCII value of the key most recently pressed, taking the state
 * of the Shift and Symbol modifier keys into account.
 * If Ctrl is held down, the most significant bit of the return value is set.
 * This allows you to, for example, distinguish between Backspace and Ctrl-H,
 * using the CTRL() macro in keycodes.h.
 * If no non-modifier keys are pressed, returns 0.
 */
char get_key_ascii(struct keyreader *kr);
#define CTRL(n) ((n) | 0x80)

key_code wait_for_key(struct keyreader *kr);
char wait_for_key_ascii(struct keyreader *kr);

char key_code_to_ascii(key_code kc);

#endif
