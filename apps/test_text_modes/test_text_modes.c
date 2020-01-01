/**
 * Text mode test patterns
 *
 * Key | Pattern       | Mode        | Font location
 * ----+---------------+-------------+--------------
 * esc | grid          | 40x25 mono  | ROM
 * q   | grid          | 40x25 mono  | RAM
 * w   | grid          | 80x25 mono  | ROM
 * e   | grid          | 80x25 mono  | RAM
 * r   | blocks        | 40x25 mono  | ROM
 * t   | blocks        | 40x25 mono  | RAM
 * y   | blocks        | 80x25 mono  | ROM
 * u   | blocks        | 80x25 mono  | RAM
 * i   | charset       | 40x25 mono  | ROM
 * o   | charset       | 40x25 mono  | RAM
 * p   | charset       | 80x25 mono  | ROM
 * del | charset       | 80x25 mono  | RAM
 * a   | charset       | 40x25 color | RAM
 * s   | charset+color | 40x25 color | RAM
 * Other keys:
 * -   | disable RAM font cycling
 * =   | enable RAM font cycling
 *
 * "RAM font cycling":
 * In RAM font modes, each character pattern in the set is rotated left one bit
 * per frame. This is to allow you to visually distinguish between ROM and RAM
 * font modes (because only patterns in RAM can be modified at runtime).
 * When RAM font cycling is off, ROM and RAM tests should look identical.
 *
 * Things to look for:
 * - Horizontal image position should be consistent for all test patterns
 * - Rightmost column of image should not be cropped or corrupt
 * - ROM and RAM font patterns should look identical when RAM font cycling is
 *   off.
 */
#include "app.h"
#include <string.h>

#define SCREEN_SIZE_BYTES 2000
static uint8_t screen[SCREEN_SIZE_BYTES];
#define h 25

#define DBOX_UL 201
#define DBOX_UR 187
#define DBOX_LL 200
#define DBOX_LR 188
#define DBOX_TT 203
#define DBOX_TB 202
#define DBOX_TL 204
#define DBOX_TR 185
#define DBOX_CR 206

#define SBOX_UL 21
#define SBOX_UR 22
#define SBOX_LL 25
#define SBOX_LR 26
#define SBOX_TT 23
#define SBOX_TB 27
#define SBOX_TL 29
#define SBOX_TR 30
#define SBOX_CR 31

#define BLOCK_F 160

#define cell(x,y) screen[((y)*w)+x]
static void draw_grid(uint8_t w) {
  cell(0,   0)   = SBOX_UL;
  cell(w-1, 0)   = SBOX_UR;
  cell(0,   h-1) = SBOX_LL;
  cell(w-1, h-1) = SBOX_LR;
  for (uint8_t x = 1; x < w-1; x++) {
    cell(x, 0)   = SBOX_TT;
    cell(x, h-1) = SBOX_TB;
  }
  for (uint8_t y = 1; y < h-1; y++) {
    cell(0,   y) = SBOX_TL;
    cell(w-1, y) = SBOX_TR;
    memset(&cell(1,y), SBOX_CR, w-2);
  }
}


static void draw_filled(uint8_t w) {
  for (uint8_t x = 0; x < w; x++) {
    cell(x, 0)   = BLOCK_F;
    cell(x, h-1) = BLOCK_F;
  }
  for (uint8_t y = 1; y < h-1; y++) {
    cell(0,   y) = BLOCK_F;
    cell(w-1, y) = BLOCK_F;
    memset(&cell(1,y), 6, w-2);
  }
}


static void draw_chars(uint8_t w) {
  for (uint16_t n = 0; n < w*h; n++) {
    screen[n] = (n&0xFF);
  }
}


static void draw_color_text(uint8_t w) {
  draw_chars(w);
  memset(screen+(w*h), TEXT_COLOR_WHITE, w*h);
}


static void draw_color(uint8_t w) {
  draw_chars(w);
  for (uint8_t y = 0; y < h; y++) {
    memset(&cell(0,25+y), TEXT_COLOR(y)+1, w);
  }
}


static void printhelp(void) {
  memset(screen, 0, 40*h);
  char *tptr = (char *)screen;
  strcpy_P(tptr, PSTR("Text mode test patterns")); tptr += 40;
  strcpy_P(tptr, PSTR("(See source code for key assignments.)"));
  init_40x25_mono_text_mode(screen, NULL);
}

static PAGE_ALIGNED uint8_t ram_font[2048];


#include <avr/interrupt.h>

APP_MAIN(test_text_modes)
{
  bool ram_font_cycling = true;
  memcpy_P(ram_font, amscii_font_8x8, sizeof(ram_font));

  printhelp();
  video_start();

  struct keyreader kr = {0};
  while (frame_sync()) {
    switch (get_key_ascii(&kr)) {
      case '\e':  draw_grid(40);   init_40x25_mono_text_mode(screen, NULL);     break;
      case 'q':   draw_grid(40);   init_40x25_mono_text_mode(screen, ram_font); break;
      case 'w':   draw_grid(80);   init_80x25_mono_text_mode(screen, NULL);     break;
      case 'e':   draw_grid(80);   init_80x25_mono_text_mode(screen, ram_font); break;

      case 'r':   draw_filled(40); init_40x25_mono_text_mode(screen, NULL);     break;
      case 't':   draw_filled(40); init_40x25_mono_text_mode(screen, ram_font); break;
      case 'y':   draw_filled(80); init_80x25_mono_text_mode(screen, NULL);     break;
      case 'u':   draw_filled(80); init_80x25_mono_text_mode(screen, ram_font); break;

      case 'i':   draw_chars(40);  init_40x25_mono_text_mode(screen, NULL);     break;
      case 'o':   draw_chars(40);  init_40x25_mono_text_mode(screen, ram_font); break;
      case 'p':   draw_chars(80);  init_80x25_mono_text_mode(screen, NULL);     break;
      case '\b':  draw_chars(80);  init_80x25_mono_text_mode(screen, ram_font); break;

      case 'a':   draw_color_text(40);  init_40x25_color_text_mode(screen, NULL); break;
      case 's':   draw_color_text(40);  init_40x25_color_text_mode(screen, ram_font); break;
      case 'd':   draw_color(40);  init_40x25_color_text_mode(screen, NULL); break;
      case 'f':   draw_color(40);  init_40x25_color_text_mode(screen, ram_font); break;

      case '-':   ram_font_cycling = false; memcpy_P(ram_font, amscii_font_8x8, sizeof(ram_font)); break;
      case '=':   ram_font_cycling = true;  break;
      case '.':   vscroll++; break;
      case ',':   vscroll--; break;
    };
    if (ram_font_cycling) {
      for (uint16_t n = 0; n < 2048; n++) {
        ram_font[n] = (!!(ram_font[n]&0x80))|(ram_font[n]<<1);
      }
    }
  }
}
