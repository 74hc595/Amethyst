/**
 * Displays keyboard state and key-down events.
 */
#include "app.h"
#include "keys.h"
#include <string.h>

#define SCREEN_SIZE_BYTES 2000
static char screen[SCREEN_SIZE_BYTES];
#define CELLADDR(x,y) (screen+((y)*40)+(x))

static void draw_key_row_state(char *pos, uint8_t bits, const char * PROGMEM legend)
{
  bits |= 0b11000000;
  uint8_t bitmask = 0x80;
  strcpy_P(pos, legend);
  uint32_t *p = (uint32_t *)(pos+2);
  while (bitmask) {
    if ((bits & bitmask) == 0) { *p |= 0x80808080; }
    p++;
    bitmask >>= 1;
  }
}


APP_MAIN(test_keys)
{
  init_40x25_mono_text_mode(screen, NULL);
  video_start();
  struct keyreader kr = {0};

  strcpy_P(CELLADDR(3,0), PSTR("7   6   5   4   3   2   1   0"));
  *CELLADDR(1,1) = 21;
  memset(CELLADDR(2,1), 19, 32);
  *CELLADDR(1,3)  = 28;
  *CELLADDR(1,5)  = 28;
  *CELLADDR(1,7)  = 28;
  *CELLADDR(1,9)  = 28;
  *CELLADDR(1,11) = 28;
  *CELLADDR(1,13) = 28;
  *CELLADDR(1,15) = 28;
  strcpy_P(CELLADDR(0, 18), PSTR("Last key down:"));
  strcpy_P(CELLADDR(0, 20), PSTR("ASCII value:"));
  while (frame_sync()) {
#if KEYBOARD_LAYOUT_1_0
    draw_key_row_state(CELLADDR(0,2),  keymatrixrows[0], PSTR("0\x1c             O   U   T   E   Q  "));
    draw_key_row_state(CELLADDR(0,4),  keymatrixrows[1], PSTR("1\x1c             Rt  Lf  ,   =  Sym "));
    draw_key_row_state(CELLADDR(0,6),  keymatrixrows[2], PSTR("2\x1c             Up  M   B   C   Z  "));
    draw_key_row_state(CELLADDR(0,8),  keymatrixrows[3], PSTR("3\x1c             L   J   G   D   A  "));
    draw_key_row_state(CELLADDR(0,10), keymatrixrows[4], PSTR("4\x1c        Del  '   N   V   X  Shf "));
    draw_key_row_state(CELLADDR(0,12), keymatrixrows[5], PSTR("5\x1c         P   I   Y   R   W  Esc "));
    draw_key_row_state(CELLADDR(0,14), keymatrixrows[6], PSTR("6\x1c        Ent  K   H   F   S  Ctrl"));
    draw_key_row_state(CELLADDR(0,16), keymatrixrows[7], PSTR("7\x1c             /   Dn  .  Spc  -  "));
#else
    draw_key_row_state(CELLADDR(0,2),  keymatrixrows[0], PSTR("0\x1c         0   8   6   4   2  Esc "));
    draw_key_row_state(CELLADDR(0,4),  keymatrixrows[1], PSTR("1\x1c         Rt  Lf  N   V   X      "));
    draw_key_row_state(CELLADDR(0,6),  keymatrixrows[2], PSTR("2\x1c         Dn  M   B   C   Z  Sym "));
    draw_key_row_state(CELLADDR(0,8),  keymatrixrows[3], PSTR("3\x1c         Up  K   H   F   S  Shf "));
    draw_key_row_state(CELLADDR(0,10), keymatrixrows[4], PSTR("4\x1c        Spc  O   U   T   E   Q  "));
    draw_key_row_state(CELLADDR(0,12), keymatrixrows[5], PSTR("5\x1c        Del  9   7   5   3   1  "));
    draw_key_row_state(CELLADDR(0,14), keymatrixrows[6], PSTR("6\x1c         P   I   Y   R   W  Ctrl"));
    draw_key_row_state(CELLADDR(0,16), keymatrixrows[7], PSTR("7\x1c        Ent  L   J   G   D   A  "));
#endif
    key_code code = get_key_down(&kr);
    if (code.isdown) {
      *CELLADDR(15,18) = '0' + code.row + 128;
      *CELLADDR(16,18) = '0' + code.col + 128;
    } else {
      *CELLADDR(15,18) &= 0x7F;
      *CELLADDR(16,18) &= 0x7F;
    }
    char asc = key_code_to_ascii(code);
    if (asc) {
      *CELLADDR(15,20) = key_code_to_ascii(code);
    }
  }
}
