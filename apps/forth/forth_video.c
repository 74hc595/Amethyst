#include "forth.h"
#include "console.h"
#include "video.h"
#include "videomodes.h"
#include "raster_interrupt.h"
#include <stdlib.h>
#include <string.h>

void forth_40x25_console(void) {
  disable_raster_interrupt();
  char *buf = alloc_screen_buffer(1000);
  fcon.split = 0;
  fcon.eightycols = 0;
  fcon.color = 0;
  fcon.scrollok = 1;
  fcon.cursorphase = 1;
  console_init(&fcon, buf, DEFAULT_FONT);
}

void forth_80x25_console(void) {
  disable_raster_interrupt();
  char *buf = alloc_screen_buffer(2000);
  fcon.split = 0;
  fcon.eightycols = 1;
  fcon.color = 0;
  fcon.scrollok = 1;
  fcon.cursorphase = 1;
  console_init(&fcon, buf, DEFAULT_FONT);
}

void forth_40x25_color_console(void) {
  disable_raster_interrupt();
  char *buf = alloc_screen_buffer(2000);
  fcon.split = 0;
  fcon.eightycols = 0;
  fcon.color = 1;
  fcon.scrollok = 1;
  fcon.cursorphase = 1;
  console_init(&fcon, buf, DEFAULT_FONT);
}

void forth_set_bitmap_mode(enum bitmap_mode_num mode) {
  uint16_t bufsize = screen_buf_size_for_bitmap_mode(mode, 0);
  if (!bufsize) { forth_throw(FE_UNSUPPORTED_OPERATION); }
  disable_raster_interrupt();
  screen_ptr = alloc_screen_buffer(bufsize);
  memset(screen_ptr, 0, bufsize);
  console_setshowcursor(&fcon, 0); /* don't try to blink the cursor! */
  // for (uint16_t i = 0; i < bufsize; i++) { screen_ptr[i] = i; } /* test pattern */
  set_bitmap_mode(mode, 0);
}


#define SPLIT_SCREEN_TEXT_COLS    40
#define SPLIT_SCREEN_TEXT_ROWS    5
#define SPLIT_SCREEN_TEXT_BYTES   (SPLIT_SCREEN_TEXT_COLS*SPLIT_SCREEN_TEXT_ROWS)
#define SPLIT_SCREEN_TEXT_LINES   (SPLIT_SCREEN_TEXT_ROWS*8)
#define SPLIT_SCREEN_BITMAP_LINES  ACTIVE_VIDEO_LINES-SPLIT_SCREEN_TEXT_LINES

#undef X
#define X(m) \
static RASTER_INTERRUPT(ri_splitstart_##m) { \
  RI_SET8(OCR2A, OCR2A_VALUE_FOR_SRPERIOD(VIDEO_MODE_40x25_BASICTEXT##_SRPERIOD)); \
  SET_SHIFT_REGISTER_MODE(VIDEO_MODE_40x25_BASICTEXT##_SRMODE); \
  RI_SET_VIDEO_MODE_FAST(VIDEO_MODE_40x25_BASICTEXT); /* don't change bytesperline! */ \
  RI_SET_NEXT_HANDLER(0, ri_splitend_##m); \
  RI_SAVE_SREG(); \
  asm volatile( \
    "lds r30, %[ptr]\n" \
    "lds r31, %[ptr]+1\n" \
    "subi r30, lo8(%[offset])\n" \
    "sbci r31, hi8(%[offset])\n" \
    "movw r4, r30\n" \
  :: [ptr] "m" (screen_ptr), [offset] "M" (SPLIT_SCREEN_TEXT_BYTES)); \
  RI_RESTORE_SREG(); \
  RI_RET(); \
} \
static __attribute__((used)) RASTER_INTERRUPT(ri_splitend_##m) { \
  RI_SET_VIDEO_MODE(m); \
  RI_SET_NEXT_HANDLER(40, ri_splitstart_##m); \
  RI_RET(); \
}

BITMAP_MODES

#undef X
#define X(m) ri_splitstart_##m,

typedef void (*ri_fn)(void);
static const ri_fn split_screen_raster_interrupts[NUM_BITMAP_MODES] PROGMEM = {
  BITMAP_MODES
};

void forth_set_split_screen_mode(enum bitmap_mode_num mode) {
  uint16_t bufsize = screen_buf_size_for_bitmap_mode(mode, SPLIT_SCREEN_BITMAP_LINES);
  if (!bufsize) { forth_throw(FE_UNSUPPORTED_OPERATION); }
  /* First 200 bytes are used for the text pane. Remainder used for the bitmap. */
  bufsize += SPLIT_SCREEN_TEXT_BYTES;
  char *bufstart = alloc_screen_buffer(bufsize);
  screen_ptr = bufstart + SPLIT_SCREEN_TEXT_BYTES;
  memset(bufstart, 0, bufsize);
  // for (uint16_t i = 0; i < bufsize; i++) { bufstart[i] = i; } /* test pattern */
  ri_fn fn = pgm_read_ptr(&split_screen_raster_interrupts[mode]);
  set_bitmap_mode(mode, SPLIT_SCREEN_BITMAP_LINES);
  set_raster_interrupt(40, fn);
  fcon.split = 1;
  console_setshowcursor(&fcon, 1); /* blink the cursor */
  console_clrhome(&fcon);
}
