/**
 * Dynamic raster interrupt test.
 *
 * The text should be aqua, except for a sliding stripe of red.
 * Two raster interrupts are used to alternate between text colors.
 * The line numbers are also incremented, which causes the scrolling effect.
 */
#include "app.h"
#include "raster_interrupt.h"

static volatile uint8_t line1 = 0;
static volatile uint8_t line2 = 0;
static volatile uint8_t color1 = 0;
static volatile uint8_t color2 = 0;
static uint8_t screen[1000];

RASTER_INTERRUPT(test_raster_int);
RASTER_INTERRUPT(test_raster_int_2)
{
  RI_SET8(globalcolor, color2);
  RI_SET_NEXT_HANDLER(line2, test_raster_int);
  RI_RET();
}

RASTER_INTERRUPT(test_raster_int)
{
  RI_SET_TEXT_COLOR(color1);
  RI_SET_NEXT_HANDLER(line1, test_raster_int_2);
  RI_RET();
}


APP_MAIN(test_raster_interrupts)
{
  line1 = 128;
  line2 = 64;
  color1 = TEXT_COLOR_AQUA;
  color2 = TEXT_COLOR_RED;
  init_40x25_2color_text_mode(screen, NULL);
  set_raster_interrupt(line1, test_raster_int);

  for (unsigned i = 0; i < sizeof(screen); i++) {
    screen[i] = i&0xFF;
  }

  strcpy_P((char*)screen,    PSTR("q/w: change color 1"));
  strcpy_P((char*)screen+40, PSTR("a/s: change color 2"));
  strcpy_P((char*)screen+80, PSTR("esc: reset colors   "));
  memset(screen+120, ' ', 20);

  struct keyreader kr = {0};
  video_start();
  while (frame_sync()) {
    switch (get_key_ascii(&kr)) {
      case 'q': color1++; break;
      case 'w': color1--; break;
      case 'a': color2++; break;
      case 's': color2--; break;
      case KC_ESC: color1 = TEXT_COLOR_AQUA; color2 = TEXT_COLOR_RED; break;
    }
    line1+=1; if (line1 >= 200) {line1 = 0;}
    line2+=1; if (line2 >= 200) {line2 = 0;}
  }
}
