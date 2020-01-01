/**
 * Displays various color test patterns.
 */
#include "app.h"
#include <string.h>

static uint8_t screen[16016];

static const uint8_t nice_color_order[16] PROGMEM = {
  0, 10, 4, 8, 1, 2, 6, 14, 12, 13, 9, 11, 7, 3, 5, 15
};

static void test_pattern_16_color(void)
{
  init_160x200x16_bitmap_mode(screen);
  /* 16 vertical stripes, one for each color, each 10 pixels wide. */
  /* Do the first line and then replicate it. */
  uint8_t *pixptr = screen;
  for (uint8_t c = 0; c < 16; c++) {
    uint8_t color = pgm_read_byte(nice_color_order+c);
    memset(pixptr, color|(color<<4), 4);
    pixptr[4] = 0;
    pixptr += 5;
  }

  /* leave last 8 rows black, so it goes nicer with the 256 color test */
  for (uint8_t y = 0; y < 191; y++) {
    memcpy(pixptr, screen, 80);
    pixptr += 80;
  }
}


static void test_pattern_256_color(void)
{
  init_160x100x256_bitmap_mode(screen);
  uint8_t *pixptr = screen;
  uint8_t color = 0;
  for (uint8_t gridrow = 0; gridrow < 16; gridrow++) {
    uint8_t *gridrowstart = pixptr;
    /* Do the first pixel row of each grid row */
    for (uint8_t gridcol = 0; gridcol < 16; gridcol++) {
      uint8_t lo = color & 0xF;
      uint8_t hi = (color >> 4) & 0xF;
      uint8_t nice_color = pgm_read_byte(nice_color_order+lo) | (pgm_read_byte(nice_color_order+hi)<<4);
      memset(pixptr, nice_color, 9);
      pixptr[9] = 0;
      pixptr += 10;
      color++;
    }
    /* Then replicate this row for the remaining 11 pixel rows in this grid row */
    for (uint8_t subrow = 0; subrow < 5; subrow++) {
      if (subrow != 4) {
        memcpy(pixptr, gridrowstart, 160);
      } else {
        memset(pixptr, 0, 160);
      }
      pixptr += 160;
    }
  }
  /* Blank the remaining four rows */
  memset(pixptr, 0, 640);
}


static void test_pattern_256_color_grays_only(void)
{
  init_160x100x256_bitmap_mode(screen);
}


APP_MAIN(test_patterns)
{
  test_pattern_256_color();
  video_start();
  struct keyreader kr = {0};
  uint8_t sound = 31;
  while (frame_sync()) {
    switch (get_key_ascii(&kr)) {
      case 'q': test_pattern_16_color(); break;
      case 'w': test_pattern_256_color(); break;
      case 'e': test_pattern_256_color_grays_only(); break;
      case 'r': sound++; break;
      case 't': sound--; break;
    }
    /* weird sound */
    OCR0A+=sound;
    OCR0B = OCR0A>>2;
  }
}
