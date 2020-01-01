/**
 * Slideshow
 * Displays still images received over the serial port. (57600 baud)
 * Images should be raw bitmap data, 160x200 pixels at 8 bits per pixel
 * (16000 bytes total) using the 256-color palette.
 *
 * No synchronization and very little bounds checking.
 */

#include "app.h"

static uint8_t screen[16000];
static volatile uint8_t *pixptr;

/* just store the byte and advance the pointer; */
/* handle wraparound in main loop code. */
/* yes it's possible to get a buffer overflow here */
__attribute__((naked)) static void serial_rx_isr(void) {
  asm volatile(
    "lds ZL, pixptr\n"
    "lds ZH, pixptr+1\n"
    "st Z+, r2\n"
    "sts pixptr, ZL\n"
    "sts pixptr+1, ZH\n"
    "ret\n");
}


APP_MAIN(slideshow)
{
  struct serial_port serialconfig = {
    SERIAL_BAUD_57600,
    SERIAL_PARITY_NONE,
    SERIAL_DATA_BITS_8,
    SERIAL_STOP_BITS_1
  };

  pixptr = screen;
  serial_init(serialconfig);
  serial_async_set_callback(serial_rx_isr);
  memset(screen, 0x55, sizeof(screen));
  init_160x100x256_bitmap_mode(screen);
  video_start();

  while (frame_sync()) {
    /* Handle pixel pointer wraparound. */
    /* Don't need to do anything else, because the serial */
    /* receive ISR puts pixel data directly into the screen buffer .*/
    if (pixptr >= screen+sizeof(screen)) {
      pixptr = screen;
    }
  }
}
