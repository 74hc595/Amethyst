#include "app.h"

extern const TILEMAP test_tileset_8x8;

#define SCREEN_SIZE_BYTES 1000
static uint8_t screen[SCREEN_SIZE_BYTES];

APP_MAIN(test_tiled_modes)
{
  for (int i = 0; i < SCREEN_SIZE_BYTES; i++) {
    screen[i] = i;
  }
  init_hires_tiled_mode(screen, test_tileset_8x8);
  video_start();
  struct keyreader kr = {0};

  while (frame_sync()) {
    switch (get_key_ascii(&kr)) {
      case 'q': vscroll++; break;
      case 'w': vscroll--; break;
    }
  }
}
