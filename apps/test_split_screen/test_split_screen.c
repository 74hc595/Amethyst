/**
 * Split-screen implementation by changing video modes using raster interrupts.
 */
#include "app.h"
#include "raster_interrupt.h"
#include <string.h>

static uint8_t screen[16000];
static volatile uint8_t splitline1;
static volatile uint8_t splitline2;
static volatile uint8_t splitline3;

RASTER_INTERRUPT(split_enable_256color_mode);
RASTER_INTERRUPT(split_enable_16color_mode);
RASTER_INTERRUPT(split_enable_40col_text_mode);
RASTER_INTERRUPT(split_enable_80col_text_mode);


RASTER_INTERRUPT(split_enable_16color_mode)
{
  RI_SET_VIDEO_MODE(VIDEO_MODE_160x200_16COLOR);
  RI_SET_VPTR(screen);
  RI_SET_NEXT_HANDLER(splitline2, split_enable_40col_text_mode);
  RI_RET();
}


RASTER_INTERRUPT(split_enable_40col_text_mode)
{
  RI_SET_VIDEO_MODE_FAST(VIDEO_MODE_40x25_TEXT);
  RI_SET_VPTR(screen);
  RI_SET_NEXT_HANDLER(splitline3, split_enable_80col_text_mode);
  RI_WITH_SAVED_SREG() {
    asm volatile(
      "mov r30, r3\n"
      "neg r30\n"
      "andi r30, 7\n"
      "sts vscroll, r30\n"
    );
  }
  RI_RET();
}


RASTER_INTERRUPT(split_enable_80col_text_mode)
{
  RI_SET_VIDEO_MODE_FAST(VIDEO_MODE_80x25_TEXT);
  RI_SET_VPTR(screen);
  RI_SET_NEXT_HANDLER(0, split_enable_256color_mode);
  RI_WITH_SAVED_SREG() {
    asm volatile(
      "mov r30, r3\n"
      "neg r30\n"
      "andi r30, 7\n"
      "sts vscroll, r30\n"
    );
  }
  RI_RET();
}


RASTER_INTERRUPT(split_enable_256color_mode)
{
  RI_SET_VIDEO_MODE(VIDEO_MODE_80x50_256COLOR);
  RI_SET_VPTR(screen);
  RI_SET_NEXT_HANDLER(splitline1, split_enable_16color_mode);
  RI_RET();
}



APP_MAIN(test_split_screen)
{
  struct keyreader kr = {0};
  splitline1 = 144;
  splitline2 = 96;
  splitline3 = 48;
 
  set_tilemap(cp437_font_8x8);
  globalcolor = TEXT_COLOR_WHITE;
  for (unsigned i = 0; i < sizeof(screen); i++) {
    screen[i] = i;
  }
  strcpy_P((char*)screen,     PSTR("q/w: move split 1 up/down one line     "));
  strcpy_P((char*)screen+40,  PSTR("a/s: move split 2 up/down one line     "));
  strcpy_P((char*)screen+80,  PSTR("z/x: move split 3 up/down one line     "));
  strcpy_P((char*)screen+120, PSTR("esc: reset split positions             "));

  /* Prepare the first video mode and register the first raster interrupt */
  init_80x50x256_bitmap_mode(screen);
  set_raster_interrupt(splitline1, split_enable_16color_mode);

  video_start();

  while (frame_sync()) {
    switch (get_key_ascii(&kr)) {
      case 'q': splitline1++; break;
      case 'w': splitline1--; break;
      case 'a': splitline2++; break;
      case 's': splitline2--; break;
      case 'z': splitline3++; break;
      case 'x': splitline3--; break;
      case KC_ESC: splitline1 = 144; splitline2 = 96; splitline3 = 48; break;
    }
  }
}
