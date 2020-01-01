/**
 * Demonstrate all video modes onscreen simultaneously.
 */
#include "app.h"
#include "raster_interrupt.h"
#include <string.h>

static uint8_t bitmap[2048];
static PAGE_ALIGNED uint8_t ramfnt[2048];
static uint8_t text[2000];

RASTER_INTERRUPT(ri1)  { RI_SET_VPTR(text);  RI_SET_TILEMAP(ramfnt);                                                RI_SET_NEXT_HANDLER(184, ri2);  RI_RET(); }
RASTER_INTERRUPT(ri2)  { RI_SET_VPTR(text);  RI_SET_VIDEO_MODE(VIDEO_MODE_40x25_COLOR_TEXT_RAMFONT);                RI_SET_NEXT_HANDLER(176, ri3);  RI_RET(); }
RASTER_INTERRUPT(ri3)  { RI_SET_VPTR(text);  RI_SET_TILEMAP(amscii_font_8x8);RI_SET_VIDEO_MODE(VIDEO_MODE_40x25_COLOR_TEXT_ROMFONT); RI_SET_NEXT_HANDLER(168, ri4);  RI_RET(); }
RASTER_INTERRUPT(ri4)  { RI_SET_VPTR(text);  RI_SET_VIDEO_MODE(VIDEO_MODE_80x25_TEXT);                              RI_SET_NEXT_HANDLER(160, ri5);  RI_RET(); }
RASTER_INTERRUPT(ri5)  { RI_SET_VPTR(text);  RI_SET_TILEMAP(ramfnt);                                                RI_SET_NEXT_HANDLER(152, ri6);  RI_RET(); }
RASTER_INTERRUPT(ri6)  { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_160x200_256COLOR);                        RI_SET_NEXT_HANDLER(144, ri7);  RI_RET(); }
RASTER_INTERRUPT(ri7)  { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_160x100_256COLOR);                        RI_SET_NEXT_HANDLER(136, ri8);  RI_RET(); }
RASTER_INTERRUPT(ri8)  { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_128x100_256COLOR);                        RI_SET_NEXT_HANDLER(128, ri9);  RI_RET(); }
RASTER_INTERRUPT(ri9)  { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_80x100_256COLOR);                         RI_SET_NEXT_HANDLER(120, ri10); RI_RET(); }
RASTER_INTERRUPT(ri10) { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_80x50_256COLOR);                          RI_SET_NEXT_HANDLER(112, ri11); RI_RET(); }
RASTER_INTERRUPT(ri11) { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_80x25_256COLOR);                          RI_SET_NEXT_HANDLER(104, ri12); RI_RET(); }
RASTER_INTERRUPT(ri12) { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_160x200_16COLOR);                         RI_SET_NEXT_HANDLER( 96, ri13); RI_RET(); }
RASTER_INTERRUPT(ri13) { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_160x100_16COLOR);                         RI_SET_NEXT_HANDLER( 88, ri14); RI_RET(); }
RASTER_INTERRUPT(ri14) { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_128x100_16COLOR);                         RI_SET_NEXT_HANDLER( 80, ri15); RI_RET(); }
RASTER_INTERRUPT(ri15) { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_80x100_16COLOR);                          RI_SET_NEXT_HANDLER( 72, ri16); RI_RET(); }
RASTER_INTERRUPT(ri16) { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_80x50_16COLOR);                           RI_SET_NEXT_HANDLER( 64, ri17); RI_RET(); }
RASTER_INTERRUPT(ri17) { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_160x200_4COLOR);                          RI_SET_NEXT_HANDLER( 56, ri18); RI_RET(); }
RASTER_INTERRUPT(ri18) { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_160x100_4COLOR);                          RI_SET_NEXT_HANDLER( 48, ri19); RI_RET(); }
RASTER_INTERRUPT(ri19) { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_80x100_4COLOR);                           RI_SET_NEXT_HANDLER( 40, ri20); RI_RET(); }
RASTER_INTERRUPT(ri20) { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_160x200_MONO);                            RI_SET_NEXT_HANDLER( 32, ri21); RI_RET(); }
RASTER_INTERRUPT(ri21) { RI_SET_VPTR(bitmap);RI_SET_VIDEO_MODE(VIDEO_MODE_160x100_MONO);                            RI_SET_NEXT_HANDLER( 24, ri22); RI_RET(); }
RASTER_INTERRUPT(ri22) { RI_SET_VPTR(text);  RI_SET_TILEMAP(amscii_font_8x8); RI_SET_VIDEO_MODE(VIDEO_MODE_HIRES_TILED);RI_SET_NEXT_HANDLER(16, ri23); RI_RET(); }
RASTER_INTERRUPT(ri23) { RI_SET_VPTR(text);  RI_SET_TILEMAP(ramfnt); RI_SET_VIDEO_MODE(VIDEO_MODE_HIRES_TILED);     RI_SET_NEXT_HANDLER(8, ri24); RI_RET(); }
RASTER_INTERRUPT(ri24) { RI_SET_VPTR(text);  RI_SET_TILEMAP(amscii_font_8x8);RI_SET_VIDEO_MODE(VIDEO_MODE_40x25_TEXT); RI_SET_NEXT_HANDLER(192, ri1);  RI_RET(); }



APP_MAIN(test_mode_alignment)
{
  memset(bitmap, 0xFF, sizeof(bitmap));
  memset(text, 138, 1000);
  memset(text+1000, TEXT_COLOR_WHITE, 1000);
  memcpy_P(ramfnt, amscii_font_8x8, sizeof(ramfnt));

  /* Prepare the first video mode and register the first raster interrupt */
  init_40x25_2color_text_mode(text, 0);
  set_raster_interrupt(192, ri1);
  globalcolor = TEXT_COLOR_WHITE;
  /* disable colorburst to eliminate color fringing */
  COLORBURST_ON();
  struct keyreader kr = {0};
  video_start();
  while (frame_sync()) {
    switch (get_key_ascii(&kr)) {
      case 'q': COLORBURST_ON(); break;
      case 'w': COLORBURST_OFF(); break;
      case 'e': memset(bitmap, 0x81, sizeof(bitmap)); break;
      case 'r': memset(bitmap, 0xFF, sizeof(bitmap)); break;
      case 't': for (unsigned i = 0; i < sizeof(bitmap); i++) { bitmap[i]=i;} break;
    }
  }
}
