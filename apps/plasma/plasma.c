#include "app.h"
#include <avr/pgmspace.h>

#define XR 80
#define YR 50
static uint8_t screen[XR*YR];

extern const uint8_t hsvtable[256] PROGMEM;
extern const uint8_t cosinetable_lsb[256] PROGMEM;
extern const uint8_t cosinetable_msb[256] PROGMEM;


uint8_t polar_to_xy(uint8_t x, uint8_t y, int16_t a, int16_t b)
{
  y = 0x40 - y;
  int8_t cosx = pgm_read_byte(cosinetable_msb+x);
  int8_t siny = pgm_read_byte(cosinetable_msb+y);
  int16_t ca = (a*cosx);
  int16_t cb = (b*siny);
  uint8_t color = 64|((cb-ca)>>10);
  return pgm_read_byte(hsvtable+color);
}


APP_MAIN(plasma)
{
  init_80x50x256_bitmap_mode(screen);
  video_start();
  struct keyreader kr = {0};

  uint8_t t = 0;
  int16_t a = 0;
  int16_t b = 0;
  while (frame_sync()) {
    uint8_t *p = screen;
    for (uint8_t y = 0; y < YR; y++) {
      for (uint8_t x = 0; x < XR; x++) {
        int8_t cosa = pgm_read_byte(cosinetable_msb+(a&0xFF));
        int8_t cosb = pgm_read_byte(cosinetable_msb+(b&0xFF));
        uint8_t c = polar_to_xy(t+cosb+x, t+cosa+y, a, b);
        *p++ = c;
      }
    }
    t++; a++; b++;
  }
}
