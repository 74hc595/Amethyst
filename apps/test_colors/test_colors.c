/**
 * Displays various color test patterns.
 */
#include "app.h"
#include "raster_interrupt.h"
#include <string.h>
#include <stdio.h>

#define NUM_STATUS_CHAR_COLS  40
#define NUM_STATUS_CHAR_ROWS  1
#define NUM_STATUS_LINES      ((NUM_STATUS_CHAR_ROWS)*8)

#define BITMAP_PIXELS_WIDE    80
#define BITMAP_PIXELS_HIGH    48

#define BITMAP_BYTES          (BITMAP_PIXELS_WIDE*BITMAP_PIXELS_HIGH)
#define STATUS_BYTES          (NUM_STATUS_CHAR_COLS*NUM_STATUS_CHAR_ROWS)

static struct {
  uint8_t bitmap[BITMAP_BYTES];
  char text[STATUS_BYTES];
} splitscreen;


static const uint8_t color_order[256] PROGMEM = {
  0, 80, 160, 240, 5, 10, 85, 90, 165, 170, 245, 250, 15, 95, 175, 255, 192, 148, 12, 76, 44, 60, 92, 172, 197, 108, 204, 236, 124, 252, 158, 207, 72, 104, 200, 232, 28, 140, 202, 156, 213, 188, 220, 77, 109, 205, 237, 143, 208, 88, 120, 133, 138, 248, 218, 13, 45, 141, 93, 173, 125, 221, 253, 223, 128, 8, 40, 24, 136, 56, 168, 152, 184, 216, 201, 233, 29, 61, 157, 189, 144, 9, 41, 73, 137, 149, 154, 105, 89, 169, 153, 121, 185, 217, 249, 159, 16, 176, 1, 129, 17, 193, 145, 209, 177, 25, 57, 139, 75, 203, 155, 219, 81, 161, 21, 26, 241, 181, 186, 11, 27, 91, 171, 235, 187, 31, 251, 191, 65, 33, 97, 49, 225, 113, 131, 19, 147, 211, 179, 43, 59, 107, 123, 63, 48, 3, 35, 67, 99, 51, 53, 58, 83, 163, 195, 115, 227, 243, 151, 183, 2, 112, 34, 18, 130, 50, 146, 82, 114, 178, 210, 37, 42, 7, 23, 55, 32, 66, 98, 162, 242, 117, 122, 39, 135, 87, 167, 119, 215, 247, 47, 127, 96, 194, 6, 226, 38, 22, 54, 86, 150, 101, 106, 182, 71, 103, 199, 231, 36, 52, 116, 70, 134, 102, 166, 198, 118, 230, 214, 246, 46, 110, 62, 111, 4, 68, 20, 100, 228, 180, 244, 14, 78, 94, 174, 238, 126, 254, 79, 239, 64, 224, 132, 84, 164, 196, 212, 69, 74, 229, 234, 142, 30, 206, 222, 190
};


RASTER_INTERRUPT(split_select_text)
{
  RI_SET_VIDEO_MODE(VIDEO_MODE_40x25_TEXT);
  RI_SET_NEXT_HANDLER(0, split_enable_256color);
  RI_RET();
}

RASTER_INTERRUPT(split_enable_256color)
{
  RI_SET_VIDEO_MODE(VIDEO_MODE_80x50_256COLOR);
  RI_SET_NEXT_HANDLER(NUM_STATUS_LINES, split_select_text);
  RI_RET();
}


static void draw_black(void)
{
  memset(splitscreen.bitmap, 0x00, BITMAP_BYTES);
  memset(splitscreen.text, 0, STATUS_BYTES);
  strcpy_P(splitscreen.text, PSTR("Black"));
}


static void draw_25p_gray(void)
{
  memset(splitscreen.bitmap, 0xF0, BITMAP_BYTES);
  memset(splitscreen.text, 0, STATUS_BYTES);
  strcpy_P(splitscreen.text, PSTR("25% gray"));
}


static void draw_50p_gray(void)
{
  memset(splitscreen.bitmap, 0x55, BITMAP_BYTES);
  memset(splitscreen.text, 0, STATUS_BYTES);
  strcpy_P(splitscreen.text, PSTR("50% gray"));
}


static void draw_75p_gray(void)
{
  memset(splitscreen.bitmap, 0x0F, BITMAP_BYTES);
  memset(splitscreen.text, 0, STATUS_BYTES);
  strcpy_P(splitscreen.text, PSTR("75% gray"));
}


static void draw_white(void)
{
  memset(splitscreen.bitmap, 0xFF, BITMAP_BYTES);
  memset(splitscreen.text, 0, STATUS_BYTES);
  strcpy_P(splitscreen.text, PSTR("White"));
}

static void draw_custom_color(uint8_t color)
{
  memset(splitscreen.bitmap, color, BITMAP_BYTES);
  memset(splitscreen.text, 0, STATUS_BYTES);
}

static void replicate_top_bitmap_row(void)
{
  uint8_t *pixptr = splitscreen.bitmap + BITMAP_PIXELS_WIDE;
  for (uint8_t y = 1; y < BITMAP_PIXELS_HIGH; y++) {
    memcpy(pixptr, splitscreen.bitmap, BITMAP_PIXELS_WIDE);
    pixptr += BITMAP_PIXELS_WIDE;
  }
  memset(splitscreen.text, 0, STATUS_BYTES);
}


static void draw_gray_stairstep(void)
{
  for (uint8_t x = 0; x < BITMAP_PIXELS_WIDE; x+=4) {
    splitscreen.bitmap[x+0] = 0x00;
    splitscreen.bitmap[x+1] = 0xF0;
    splitscreen.bitmap[x+2] = 0x0F;
    splitscreen.bitmap[x+3] = 0xFF;
  }
  replicate_top_bitmap_row();
  memset(splitscreen.text, 0, STATUS_BYTES);
  strcpy_P(splitscreen.text, PSTR("Gray stairstep: black/25/75/white"));
}


static void draw_5shade_graydient(void)
{
  memset(splitscreen.bitmap,    0x00, 16); /* 0.000 hi_black + lo_black */
  memset(splitscreen.bitmap+16, 0xF0, 16); /* 0.250 hi_black + lo_white */
  memset(splitscreen.bitmap+32, 0x55, 16); /* 0.500 hi_gray + lo_gray */
  memset(splitscreen.bitmap+48, 0x0F, 16); /* 0.705 hi_white + lo_black */
  memset(splitscreen.bitmap+64, 0xFF, 16); /* 1.000 hi_white + lo_white */
  replicate_top_bitmap_row();
  memset(splitscreen.text, 0, STATUS_BYTES);
  strcpy_P(splitscreen.text, PSTR("5 gray levels"));
}


static void draw_9shade_graydient(void)
{
  /* msb=lo, lsb=hi */
  memset(splitscreen.bitmap,    0x00, 9); /* 0.000 hi_black + lo_black */
  memset(splitscreen.bitmap+9,  0x50, 9); /* 0.125 hi_black + lo_gray */
  memset(splitscreen.bitmap+18, 0xF0, 9); /* 0.250 hi_black + lo_white */
  memset(splitscreen.bitmap+27, 0x05, 9); /* 0.375 hi_gray + lo_black */
  memset(splitscreen.bitmap+36, 0x55, 9); /* 0.500 hi_gray + lo_gray */
  memset(splitscreen.bitmap+45, 0xF5, 9); /* 0.625 hi_gray + lo_white */
  memset(splitscreen.bitmap+54, 0x0F, 9); /* 0.750 hi_white + lo_black */
  memset(splitscreen.bitmap+63, 0x5F, 9); /* 0.825 hi_white + lo_gray */
  memset(splitscreen.bitmap+72, 0xFF, 8); /* 1.000 hi_white + lo_white */
  replicate_top_bitmap_row();
  memset(splitscreen.text, 0, STATUS_BYTES);
  strcpy_P(splitscreen.text, PSTR("9 gray levels"));
}


static void draw_4stripes(uint8_t *pixptr, uint8_t firstcolor) {
  for (uint8_t x = 0; x < 4; x++) {
    memset(pixptr, firstcolor, BITMAP_PIXELS_WIDE/4);
    pixptr += BITMAP_PIXELS_WIDE/4;
    firstcolor += 0x11;
  }
}


static void draw_16stripes(uint8_t *pixptr, uint8_t firstcolor) {
  for (uint8_t x = 0; x < 16; x++) {
    memset(pixptr, pgm_read_byte(color_order+firstcolor), BITMAP_PIXELS_WIDE/16);
    pixptr += BITMAP_PIXELS_WIDE/16;
    firstcolor += 0x01;
  }
}



static uint8_t *replicate_bitmap_row(uint8_t *dstrow, uint8_t *srcrow, uint8_t nrows) {
  for (uint8_t y = 0; y < nrows; y++) {
    memcpy(dstrow, srcrow, BITMAP_PIXELS_WIDE);
    dstrow += BITMAP_PIXELS_WIDE;
  }
  return dstrow;
}


static void draw_16color_grid(void)
{
  uint8_t *pixptr = splitscreen.bitmap;
  uint8_t color = 0x00;
  for (uint8_t i = 0; i < 4; i++) {
    draw_4stripes(pixptr, color);
    pixptr = replicate_bitmap_row(pixptr+BITMAP_PIXELS_WIDE, pixptr, (BITMAP_PIXELS_HIGH/4)-1);
    color += 0x44;
  }
  memset(splitscreen.text, 0, STATUS_BYTES);
  strcpy_P(splitscreen.text, PSTR("16 color grid"));
}


static void draw_256color_grid(void)
{
  uint8_t *pixptr = splitscreen.bitmap;
  uint8_t color = 0x00;
  for (uint8_t i = 0; i < 16; i++) {
    draw_16stripes(pixptr, color);
    pixptr = replicate_bitmap_row(pixptr+BITMAP_PIXELS_WIDE, pixptr, (BITMAP_PIXELS_HIGH/16)-1);
    color += 0x10;
  }
  strcpy_P(splitscreen.text, PSTR("256 color grid"));
}


APP_MAIN(test_colors)
{
  init_80x50x256_bitmap_mode(splitscreen.bitmap);
  set_raster_interrupt(NUM_STATUS_LINES, split_select_text);
  draw_25p_gray();
  video_start();
  struct keyreader kr = {0};

  uint8_t color = 0;

  while (frame_sync()) {
    switch (get_key_ascii(&kr)) {
      case 'q': draw_black(); break;
      case 'w': draw_25p_gray(); break;
      case 'e': draw_50p_gray(); break;
      case 'r': draw_75p_gray(); break;
      case 't': draw_white(); break;
      case 'y': draw_gray_stairstep(); break;
      case 'u': draw_5shade_graydient(); break;
      case 'i': draw_9shade_graydient(); break;
      case 'o': draw_16color_grid(); break;
      case 'p': draw_256color_grid(); break;
      case KC_LEFT: color--; draw_custom_color(color); break;
      case KC_RIGHT: color++; draw_custom_color(color); break;
      case KC_UP: color+=0x10; draw_custom_color(color); break;
      case KC_DOWN: color-=0x10; draw_custom_color(color); break;
    }
  }
}
