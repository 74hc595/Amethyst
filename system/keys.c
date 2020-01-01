#include "keys.h"
#include "video.h"
#include "amscii.h"
#include <string.h>
#include <avr/pgmspace.h>

#define KEYNUM(k) (((k##_ROW)<<3)|(k##_COL))
#define MOD_SHIFT 64
#define MOD_SYM   128

#define NO_KEYS_DOWN_IN_ROW 0b00111111
#define KEY_REPEAT_DELAY    32
#define KEY_REPEAT_RATE     2

#define KEY_LAYOUT_SHIFT_PLUS_SYM 0
static const char key_to_ascii_table[256] PROGMEM = {
#if KEYBOARD_LAYOUT_1_0
  [KEYNUM(K_ESC)]   = KC_ESC,   [MOD_SHIFT|KEYNUM(K_ESC)]   = KC_ESC,   [MOD_SYM|KEYNUM(K_ESC)]   = KC_TAB,
  [KEYNUM(K_Q)]     = 'q',      [MOD_SHIFT|KEYNUM(K_Q)]     = 'Q',      [MOD_SYM|KEYNUM(K_Q)]     = '1',
  [KEYNUM(K_W)]     = 'w',      [MOD_SHIFT|KEYNUM(K_W)]     = 'W',      [MOD_SYM|KEYNUM(K_W)]     = '2',
  [KEYNUM(K_E)]     = 'e',      [MOD_SHIFT|KEYNUM(K_E)]     = 'E',      [MOD_SYM|KEYNUM(K_E)]     = '3',
  [KEYNUM(K_R)]     = 'r',      [MOD_SHIFT|KEYNUM(K_R)]     = 'R',      [MOD_SYM|KEYNUM(K_R)]     = '4',
  [KEYNUM(K_T)]     = 't',      [MOD_SHIFT|KEYNUM(K_T)]     = 'T',      [MOD_SYM|KEYNUM(K_T)]     = '5',
  [KEYNUM(K_Y)]     = 'y',      [MOD_SHIFT|KEYNUM(K_Y)]     = 'Y',      [MOD_SYM|KEYNUM(K_Y)]     = '6',
  [KEYNUM(K_U)]     = 'u',      [MOD_SHIFT|KEYNUM(K_U)]     = 'U',      [MOD_SYM|KEYNUM(K_U)]     = '7',
  [KEYNUM(K_I)]     = 'i',      [MOD_SHIFT|KEYNUM(K_I)]     = 'I',      [MOD_SYM|KEYNUM(K_I)]     = '8',
  [KEYNUM(K_O)]     = 'o',      [MOD_SHIFT|KEYNUM(K_O)]     = 'O',      [MOD_SYM|KEYNUM(K_O)]     = '9',
  [KEYNUM(K_P)]     = 'p',      [MOD_SHIFT|KEYNUM(K_P)]     = 'P',      [MOD_SYM|KEYNUM(K_P)]     = '0',
  [KEYNUM(K_DEL)]   = KC_DEL,   [MOD_SHIFT|KEYNUM(K_DEL)]   = KC_DEL,   [MOD_SYM|KEYNUM(K_DEL)]   = KC_DEL,
  [KEYNUM(K_CTRL)]  = 0,        [MOD_SHIFT|KEYNUM(K_CTRL)]  = 0,        [MOD_SYM|KEYNUM(K_CTRL)]  = 0,
  [KEYNUM(K_A)]     = 'a',      [MOD_SHIFT|KEYNUM(K_A)]     = 'A',      [MOD_SYM|KEYNUM(K_A)]     = '@',
  [KEYNUM(K_S)]     = 's',      [MOD_SHIFT|KEYNUM(K_S)]     = 'S',      [MOD_SYM|KEYNUM(K_S)]     = '#',
  [KEYNUM(K_D)]     = 'd',      [MOD_SHIFT|KEYNUM(K_D)]     = 'D',      [MOD_SYM|KEYNUM(K_D)]     = '$',
  [KEYNUM(K_F)]     = 'f',      [MOD_SHIFT|KEYNUM(K_F)]     = 'F',      [MOD_SYM|KEYNUM(K_F)]     = '%',
  [KEYNUM(K_G)]     = 'g',      [MOD_SHIFT|KEYNUM(K_G)]     = 'G',      [MOD_SYM|KEYNUM(K_G)]     = '^',
  [KEYNUM(K_H)]     = 'h',      [MOD_SHIFT|KEYNUM(K_H)]     = 'H',      [MOD_SYM|KEYNUM(K_H)]     = '&',
  [KEYNUM(K_J)]     = 'j',      [MOD_SHIFT|KEYNUM(K_J)]     = 'J',      [MOD_SYM|KEYNUM(K_J)]     = '*',
  [KEYNUM(K_K)]     = 'k',      [MOD_SHIFT|KEYNUM(K_K)]     = 'K',      [MOD_SYM|KEYNUM(K_K)]     = '(',
  [KEYNUM(K_L)]     = 'l',      [MOD_SHIFT|KEYNUM(K_L)]     = 'L',      [MOD_SYM|KEYNUM(K_L)]     = ')',
  [KEYNUM(K_ENTER)] = KC_ENTER, [MOD_SHIFT|KEYNUM(K_ENTER)] = KC_ENTER, [MOD_SYM|KEYNUM(K_ENTER)] = KC_ENTER,
  [KEYNUM(K_SHIFT)] = 0,        [MOD_SHIFT|KEYNUM(K_SHIFT)] = 0,        [MOD_SYM|KEYNUM(K_SHIFT)] = 0,
  [KEYNUM(K_Z)]     = 'z',      [MOD_SHIFT|KEYNUM(K_Z)]     = 'Z',      [MOD_SYM|KEYNUM(K_Z)]     = '!',
  [KEYNUM(K_X)]     = 'x',      [MOD_SHIFT|KEYNUM(K_X)]     = 'X',      [MOD_SYM|KEYNUM(K_X)]     = '\\',
  [KEYNUM(K_C)]     = 'c',      [MOD_SHIFT|KEYNUM(K_C)]     = 'C',      [MOD_SYM|KEYNUM(K_C)]     = '|',
  [KEYNUM(K_V)]     = 'v',      [MOD_SHIFT|KEYNUM(K_V)]     = 'V',      [MOD_SYM|KEYNUM(K_V)]     = '~',
  [KEYNUM(K_B)]     = 'b',      [MOD_SHIFT|KEYNUM(K_B)]     = 'B',      [MOD_SYM|KEYNUM(K_B)]     = '{',
  [KEYNUM(K_N)]     = 'n',      [MOD_SHIFT|KEYNUM(K_N)]     = 'N',      [MOD_SYM|KEYNUM(K_N)]     = '}',
  [KEYNUM(K_M)]     = 'm',      [MOD_SHIFT|KEYNUM(K_M)]     = 'M',      [MOD_SYM|KEYNUM(K_M)]     = '[',
  [KEYNUM(K_QUOTE)] = '\'',     [MOD_SHIFT|KEYNUM(K_QUOTE)] = '"',      [MOD_SYM|KEYNUM(K_QUOTE)] = ']',
  [KEYNUM(K_UP)]    = KC_UP,    [MOD_SHIFT|KEYNUM(K_UP)]    = KC_UP,    [MOD_SYM|KEYNUM(K_UP)]    = KC_PGUP,
  [KEYNUM(K_SLASH)] = '/',      [MOD_SHIFT|KEYNUM(K_SLASH)] = '?',      [MOD_SYM|KEYNUM(K_SLASH)] = '`',
  [KEYNUM(K_SYM)]   = 0,        [MOD_SHIFT|KEYNUM(K_SYM)]   = 0,        [MOD_SYM|KEYNUM(K_SYM)]   = 0,
  [KEYNUM(K_MINUS)] = '-',      [MOD_SHIFT|KEYNUM(K_MINUS)] = '_',      [MOD_SYM|KEYNUM(K_MINUS)] = KC_F1,
  [KEYNUM(K_EQUAL)] = '=',      [MOD_SHIFT|KEYNUM(K_EQUAL)] = '+',      [MOD_SYM|KEYNUM(K_EQUAL)] = KC_F2,
  [KEYNUM(K_SPACE)] = ' ',      [MOD_SHIFT|KEYNUM(K_SPACE)] = ' ',      [MOD_SYM|KEYNUM(K_SPACE)] = ' ',
  [KEYNUM(K_COMMA)] = ',',      [MOD_SHIFT|KEYNUM(K_COMMA)] = '<',      [MOD_SYM|KEYNUM(K_COMMA)] = ';',
  [KEYNUM(K_DOT)]   = '.',      [MOD_SHIFT|KEYNUM(K_DOT)]   = '>',      [MOD_SYM|KEYNUM(K_DOT)]   = ':',
  [KEYNUM(K_LEFT)]  = KC_LEFT,  [MOD_SHIFT|KEYNUM(K_LEFT)]  = KC_LEFT,  [MOD_SYM|KEYNUM(K_LEFT)]  = KC_HOME,
  [KEYNUM(K_DOWN)]  = KC_DOWN,  [MOD_SHIFT|KEYNUM(K_DOWN)]  = KC_DOWN,  [MOD_SYM|KEYNUM(K_DOWN)]  = KC_PGDN,
  [KEYNUM(K_RIGHT)] = KC_RIGHT, [MOD_SHIFT|KEYNUM(K_RIGHT)] = KC_RIGHT, [MOD_SYM|KEYNUM(K_RIGHT)] = KC_END,
#else
/* Shift+Sym can be used to type graphical/box-drawing characters */
/* (provided the application does not interpret them as control characters) */
  [KEYNUM(K_ESC)]   = KC_ESC,   [MOD_SHIFT|KEYNUM(K_ESC)]   = KC_ESC,   [MOD_SYM|KEYNUM(K_ESC)]   = KC_CLEAR, [MOD_SHIFT|MOD_SYM|KEYNUM(K_ESC)]   = acBLKFM,
  [KEYNUM(K_1)]     = '1',      [MOD_SHIFT|KEYNUM(K_1)]     = '!',      [MOD_SYM|KEYNUM(K_1)]     = '!',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_1)]     = acBOXH,
  [KEYNUM(K_2)]     = '2',      [MOD_SHIFT|KEYNUM(K_2)]     = '@',      [MOD_SYM|KEYNUM(K_2)]     = '@',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_2)]     = acBOXUL,
  [KEYNUM(K_3)]     = '3',      [MOD_SHIFT|KEYNUM(K_3)]     = '#',      [MOD_SYM|KEYNUM(K_3)]     = '#',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_3)]     = acBOXUR,
  [KEYNUM(K_4)]     = '4',      [MOD_SHIFT|KEYNUM(K_4)]     = '$',      [MOD_SYM|KEYNUM(K_4)]     = '$',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_4)]     = acBOXTD,
  [KEYNUM(K_5)]     = '5',      [MOD_SHIFT|KEYNUM(K_5)]     = '%',      [MOD_SYM|KEYNUM(K_5)]     = '%',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_5)]     = acBOXLL,
  [KEYNUM(K_6)]     = '6',      [MOD_SHIFT|KEYNUM(K_6)]     = '^',      [MOD_SYM|KEYNUM(K_6)]     = '^',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_6)]     = acBOXLR,
  [KEYNUM(K_7)]     = '7',      [MOD_SHIFT|KEYNUM(K_7)]     = '&',      [MOD_SYM|KEYNUM(K_7)]     = '&',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_7)]     = acBOXTU,
  [KEYNUM(K_8)]     = '8',      [MOD_SHIFT|KEYNUM(K_8)]     = '*',      [MOD_SYM|KEYNUM(K_8)]     = '*',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_8)]     = acBOXV,
  [KEYNUM(K_9)]     = '9',      [MOD_SHIFT|KEYNUM(K_9)]     = '(',      [MOD_SYM|KEYNUM(K_9)]     = '(',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_9)]     = acBOXTR,
  [KEYNUM(K_0)]     = '0',      [MOD_SHIFT|KEYNUM(K_0)]     = ')',      [MOD_SYM|KEYNUM(K_0)]     = ')',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_0)]     = acBOXTL,
  [KEYNUM(K_DEL)]   = KC_BKSP,  [MOD_SHIFT|KEYNUM(K_DEL)]   = KC_BKSP,  [MOD_SYM|KEYNUM(K_DEL)]   = KC_DEL,   [MOD_SHIFT|MOD_SYM|KEYNUM(K_DEL)]   = acBOXHV,
  [KEYNUM(K_CTRL)]  = 0,        [MOD_SHIFT|KEYNUM(K_CTRL)]  = 0,        [MOD_SYM|KEYNUM(K_CTRL)]  = 0,        [MOD_SHIFT|MOD_SYM|KEYNUM(K_CTRL)]  = 0,
  [KEYNUM(K_Q)]     = 'q',      [MOD_SHIFT|KEYNUM(K_Q)]     = 'Q',      [MOD_SYM|KEYNUM(K_Q)]     = KC_F1,    [MOD_SHIFT|MOD_SYM|KEYNUM(K_Q)]     = KC_F1,
  [KEYNUM(K_W)]     = 'w',      [MOD_SHIFT|KEYNUM(K_W)]     = 'W',      [MOD_SYM|KEYNUM(K_W)]     = '|',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_W)]     = '|',
  [KEYNUM(K_E)]     = 'e',      [MOD_SHIFT|KEYNUM(K_E)]     = 'E',      [MOD_SYM|KEYNUM(K_E)]     = '+',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_E)]     = '+',
  [KEYNUM(K_R)]     = 'r',      [MOD_SHIFT|KEYNUM(K_R)]     = 'R',      [MOD_SYM|KEYNUM(K_R)]     = '<',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_R)]     = '<',
  [KEYNUM(K_T)]     = 't',      [MOD_SHIFT|KEYNUM(K_T)]     = 'T',      [MOD_SYM|KEYNUM(K_T)]     = '?',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_T)]     = '?',
  [KEYNUM(K_Y)]     = 'y',      [MOD_SHIFT|KEYNUM(K_Y)]     = 'Y',      [MOD_SYM|KEYNUM(K_Y)]     = ':',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_Y)]     = ':',
  [KEYNUM(K_U)]     = 'u',      [MOD_SHIFT|KEYNUM(K_U)]     = 'U',      [MOD_SYM|KEYNUM(K_U)]     = '>',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_U)]     = '>',
  [KEYNUM(K_I)]     = 'i',      [MOD_SHIFT|KEYNUM(K_I)]     = 'I',      [MOD_SYM|KEYNUM(K_I)]     = '_',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_I)]     = '_',
  [KEYNUM(K_O)]     = 'o',      [MOD_SHIFT|KEYNUM(K_O)]     = 'O',      [MOD_SYM|KEYNUM(K_O)]     = '"',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_O)]     = '"',
  [KEYNUM(K_P)]     = 'p',      [MOD_SHIFT|KEYNUM(K_P)]     = 'P',      [MOD_SYM|KEYNUM(K_P)]     = KC_F2,    [MOD_SHIFT|MOD_SYM|KEYNUM(K_P)]     = KC_F2,
  [KEYNUM(K_ENTER)] = KC_ENTER, [MOD_SHIFT|KEYNUM(K_ENTER)] = KC_ENTER, [MOD_SYM|KEYNUM(K_ENTER)] = KC_RET,   [MOD_SHIFT|MOD_SYM|KEYNUM(K_ENTER)] = KC_VT,
  [KEYNUM(K_SHIFT)] = 0,        [MOD_SHIFT|KEYNUM(K_SHIFT)] = 0,        [MOD_SYM|KEYNUM(K_SHIFT)] = 0,        [MOD_SHIFT|MOD_SYM|KEYNUM(K_SHIFT)] = 0,
  [KEYNUM(K_A)]     = 'a',      [MOD_SHIFT|KEYNUM(K_A)]     = 'A',      [MOD_SYM|KEYNUM(K_A)]     = KC_TAB,   [MOD_SHIFT|MOD_SYM|KEYNUM(K_A)]     = KC_TAB,
  [KEYNUM(K_S)]     = 's',      [MOD_SHIFT|KEYNUM(K_S)]     = 'S',      [MOD_SYM|KEYNUM(K_S)]     = '\\',     [MOD_SHIFT|MOD_SYM|KEYNUM(K_S)]     = '\\',
  [KEYNUM(K_D)]     = 'd',      [MOD_SHIFT|KEYNUM(K_D)]     = 'D',      [MOD_SYM|KEYNUM(K_D)]     = '=',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_D)]     = '=',
  [KEYNUM(K_F)]     = 'f',      [MOD_SHIFT|KEYNUM(K_F)]     = 'F',      [MOD_SYM|KEYNUM(K_F)]     = ',',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_F)]     = ',',
  [KEYNUM(K_G)]     = 'g',      [MOD_SHIFT|KEYNUM(K_G)]     = 'G',      [MOD_SYM|KEYNUM(K_G)]     = '/',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_G)]     = '/',
  [KEYNUM(K_H)]     = 'h',      [MOD_SHIFT|KEYNUM(K_H)]     = 'H',      [MOD_SYM|KEYNUM(K_H)]     = ';',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_H)]     = ';',
  [KEYNUM(K_J)]     = 'j',      [MOD_SHIFT|KEYNUM(K_J)]     = 'J',      [MOD_SYM|KEYNUM(K_J)]     = '.',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_J)]     = '.',
  [KEYNUM(K_K)]     = 'k',      [MOD_SHIFT|KEYNUM(K_K)]     = 'K',      [MOD_SYM|KEYNUM(K_K)]     = '-',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_K)]     = '-',
  [KEYNUM(K_L)]     = 'l',      [MOD_SHIFT|KEYNUM(K_L)]     = 'L',      [MOD_SYM|KEYNUM(K_L)]     = '\'',     [MOD_SHIFT|MOD_SYM|KEYNUM(K_L)]     = '\'',
  [KEYNUM(K_UP)]    = KC_UP,    [MOD_SHIFT|KEYNUM(K_UP)]    = KC_UP,    [MOD_SYM|KEYNUM(K_UP)]    = KC_PGUP,  [MOD_SHIFT|MOD_SYM|KEYNUM(K_UP)]    = KC_PGUP,
  [KEYNUM(K_SPACE)] = ' ',      [MOD_SHIFT|KEYNUM(K_SPACE)] = ' ',      [MOD_SYM|KEYNUM(K_SPACE)] = ' ',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_SPACE)] = ' ',
  [KEYNUM(K_SYM)]   = 0,        [MOD_SHIFT|KEYNUM(K_SYM)]   = 0,        [MOD_SYM|KEYNUM(K_SYM)]   = 0,        [MOD_SHIFT|MOD_SYM|KEYNUM(K_SYM)]   = 0,
  [KEYNUM(K_Z)]     = 'z',      [MOD_SHIFT|KEYNUM(K_Z)]     = 'Z',      [MOD_SYM|KEYNUM(K_Z)]     = '[',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_Z)]     = acBLKUL,
  [KEYNUM(K_X)]     = 'x',      [MOD_SHIFT|KEYNUM(K_X)]     = 'X',      [MOD_SYM|KEYNUM(K_X)]     = ']',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_X)]     = acBLKUR,
  [KEYNUM(K_C)]     = 'c',      [MOD_SHIFT|KEYNUM(K_C)]     = 'C',      [MOD_SYM|KEYNUM(K_C)]     = '{',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_C)]     = acBLKULUR,
  [KEYNUM(K_V)]     = 'v',      [MOD_SHIFT|KEYNUM(K_V)]     = 'V',      [MOD_SYM|KEYNUM(K_V)]     = '}',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_V)]     = acBLKLL,
  [KEYNUM(K_B)]     = 'b',      [MOD_SHIFT|KEYNUM(K_B)]     = 'B',      [MOD_SYM|KEYNUM(K_B)]     = '~',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_B)]     = acBLKULLL,
  [KEYNUM(K_N)]     = 'n',      [MOD_SHIFT|KEYNUM(K_N)]     = 'N',      [MOD_SYM|KEYNUM(K_N)]     = '`',      [MOD_SHIFT|MOD_SYM|KEYNUM(K_N)]     = acBLKURLL,
  [KEYNUM(K_M)]     = 'm',      [MOD_SHIFT|KEYNUM(K_M)]     = 'M',      [MOD_SYM|KEYNUM(K_M)]     = KC_F3,    [MOD_SHIFT|MOD_SYM|KEYNUM(K_M)]     = acBLKULURLL,
  [KEYNUM(K_LEFT)]  = KC_LEFT,  [MOD_SHIFT|KEYNUM(K_LEFT)]  = KC_LEFT,  [MOD_SYM|KEYNUM(K_LEFT)]  = KC_HOME,  [MOD_SHIFT|MOD_SYM|KEYNUM(K_LEFT)]  = KC_HOME,
  [KEYNUM(K_DOWN)]  = KC_DOWN,  [MOD_SHIFT|KEYNUM(K_DOWN)]  = KC_DOWN,  [MOD_SYM|KEYNUM(K_DOWN)]  = KC_PGDN,  [MOD_SHIFT|MOD_SYM|KEYNUM(K_DOWN)]  = KC_PGDN,
  [KEYNUM(K_RIGHT)] = KC_RIGHT, [MOD_SHIFT|KEYNUM(K_RIGHT)] = KC_RIGHT, [MOD_SYM|KEYNUM(K_RIGHT)] = KC_END,   [MOD_SHIFT|MOD_SYM|KEYNUM(K_RIGHT)] = KC_END,
#endif
};


/* Flags indicating whether a key is a modifier (i.e. does not affect key repeat). */
static const char modifier_keys[KEYMATRIX_ROWS] PROGMEM = {
  [K_SHIFT_ROW] = _BV(K_SHIFT_COL),
  [K_CTRL_ROW]  = _BV(K_CTRL_COL),
  [K_SYM_ROW]   = _BV(K_SYM_COL)
};


bool key_down_pending(struct keyreader *kr)
{
  for (uint8_t row = 0; row < KEYMATRIX_ROWS; row++) {
    uint8_t keys = keymatrixrows[row] | (~NO_KEYS_DOWN_IN_ROW) | pgm_read_byte(modifier_keys+row); /* ignore modifiers and upper bits */
    if ((~keys) & kr->lastkeys[row]) { return true; }
  }
  return false;
}


key_code get_key_down(struct keyreader *kr)
{
  uint8_t all_non_modifier_keys = NO_KEYS_DOWN_IN_ROW;
  key_code lastkeydown = {0};
  for (uint8_t row = 0; row < KEYMATRIX_ROWS; row++) {
    uint8_t keys = keymatrixrows[row] | (~NO_KEYS_DOWN_IN_ROW); /* ignore upper bits */
    all_non_modifier_keys &= keys | pgm_read_byte(modifier_keys+row); /* ignore modifiers */
    uint8_t new_keydowns = (~keys) & kr->lastkeys[row];
    kr->lastkeys[row] = keys;
    if (new_keydowns) {
      lastkeydown.isdown = 1;
      lastkeydown.row = row;
      lastkeydown.col = 0;
      /* find first bit set in new_keydowns */
      while ((new_keydowns & 1) == 0) {
        new_keydowns >>= 1;
        lastkeydown.col++;
      }
    }
  }

  /* Disable the key repeat timer when all (non-modifier) keys are released. */
  if (all_non_modifier_keys == NO_KEYS_DOWN_IN_ROW) {
    kr->repeat_count = 0;
  }
  /* If a key is down and the repeat timer is not running, start it. */
  else if (kr->repeat_count == 0) {
    kr->repeat_count = KEY_REPEAT_DELAY;
  }

  /* Decrement key repeat timer if it's running. */
  if (kr->repeat_count) {
    kr->repeat_count--;
    /* If the repeat counter hits 0, clear lastkeys so that a new key-down event */
    /* is generated. */
    if (kr->repeat_count == 0) {
      memset(kr->lastkeys, 0xFF, sizeof(kr->lastkeys));
      /* Re-generate key-down events until release. */
      kr->repeat_count = KEY_REPEAT_RATE;
    }
  }

  return lastkeydown;
}


char get_key_ascii(struct keyreader *kr) {
  return key_code_to_ascii(get_key_down(kr));
}


key_code wait_for_key(struct keyreader *kr)
{
  while (frame_sync()) {
    key_code k = get_key_down(kr);
    if (k.isdown) { return k; }
  }
}


char wait_for_key_ascii(struct keyreader *kr) {
  return key_code_to_ascii(wait_for_key(kr));
}


char key_code_to_ascii(key_code kc) {
  if (!kc.isdown) { return 0; }
  /* get table index from row/column */
  uint8_t idx = kc.index;
  /* OR in modifier keys */
  if (key_is_pressed(K_SHIFT)) { idx |= MOD_SHIFT; }
  if (key_is_pressed(K_SYM))   { idx |= MOD_SYM;   }
  char asciival = pgm_read_byte(key_to_ascii_table+idx);
  /* If CTRL is pressed, set high bit */
  if (asciival && key_is_pressed(K_CTRL))  { asciival = CTRL(asciival); }
  return asciival;
}
