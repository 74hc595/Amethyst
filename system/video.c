#include "video.h"
#include "videomodes.h"
#include "colors.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <stdbool.h>

/* GPIOR1/GPIOR2 hold a pointer to the current video mode routine. */
/* Using I/O registers instead of SRAM saves 2 cycles per line. */
typedef void (*video_fn)(void);
#define video_linehandler (*(video_fn *)(0x4A))

/* Timer3 (frame timer) values. Timer3 uses a 1/8 prescaler. */
static const uint32_t CYCLES_PER_FRAME = (uint32_t)CYCLES_PER_LINE*LINES_PER_FRAME;
static const uint16_t CYCLES_PER_FRAME_SCALED = CYCLES_PER_FRAME / 8;

static const uint32_t VIDEO_START_CYCLE = (uint32_t)CYCLES_PER_LINE*VBLANK_LINES;
static const uint16_t VIDEO_START_COUNT = (VIDEO_START_CYCLE / 8)-1;

static const uint32_t VSYNC_START_CYCLE = (uint32_t)CYCLES_PER_LINE*(LINES_PER_FRAME-VSYNC_LINES);
static const uint16_t VSYNC_START_COUNT = (VSYNC_START_CYCLE / 8)-1;

#define IS_ROM(ptr) (__builtin_avr_flash_segment(ptr)!=255)



static void input_init(void)
{
  /* Port A is all inputs. Pullups on PA0-PA5. */
  DDRA = 0;
  PORTA = 0b00111111;
  /* PA6 and PA7 are analog inputs. Disable input buffers. */
  DIDR0 = _BV(ADC6D) | _BV(ADC7D);
}


static void sync_init(void)
{
  /* Sync pin is an output */
  DDRD |= _BV(4);
  /* Fast PWM mode */
  /* Frequency controlled by ICR1. */
  /* OC1B goes low when count=0, goes high when count=OCR1B. */
  /* OC1A does not control an output pin but generates an interrupt. */
  TCCR1A = _BV(COM1B1) | _BV(COM1B0) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12);
  ICR1 = CYCLES_PER_LINE-1;
  OCR1B = CYCLES_PER_HSYNC-1;
  OCR1A = (CYCLES_PER_HSYNC+CYCLES_PER_BACK_PORCH)-1;
  TIFR1 = 0;
  /* Timer 3 overflow interrupt: start of vblank */
  /* Timer 3 COMPA interrupt: start of active video */
  /* Timer 3 COMPB interrupt: start of vsync */
  TCCR3A = _BV(WGM31);
  /* Timer3 will use a 1/8 prescaler. */
  TCCR3B = _BV(WGM33) | _BV(WGM32);
  ICR3 = CYCLES_PER_FRAME_SCALED-1;
  OCR3A = VIDEO_START_COUNT;
  OCR3B = VSYNC_START_COUNT;
  TIFR3 = 0;
  TIMSK3 = _BV(TOIE3)|_BV(OCIE3B)|_BV(OCIE3A);
}


/* Initialize video data output */
static void pixel_data_init(void)
{
  /* Port C: 8 bit parallel out to shift registers */
  DDRC = 0xFF;
  PORTC = 0;
  /* Timer2 strobes the shift register every colorburst cycle. */
  DDRD |= _BV(7) | _BV(6) | _BV(5);
  /* Fast PWM, TOP=OCR2A, set OC2B on Compare Match, clear at BOTTOM. */
  /* Compare Match happens coincidentally with BOTTOM, resulting in a narrow */
  /* low-going spike when TCNT2=0. */
  /* This strobes the shift registers' /PL line, transferring the contents of */
  /* PORTC into the shift registers. */
  TCCR2A = _BV(COM2B1)|_BV(COM2B0)|_BV(WGM21)|_BV(WGM20);
  TCCR2B = _BV(WGM22)|_BV(CS20);
  OCR2A = CYCLES_PER_COLORBURST-1;
  OCR2B = 0;
  vscroll = 0;
  /* USART0 in SPI Master mode generates a pulse that controls the length of */
  /* the colorburst. Transmitter is only enabled during colorburst. */
  /* Use the fastest baud rate because the baud rate prescaler introduces nondeterminism. */
  UCSR0A = 0;
  UCSR0B = 0;
  UCSR0C = _BV(UMSEL01) | _BV(UMSEL00);
  UBRR0 = 0; /* Baud rate is Fcpu/2 */
  DDRD |= _BV(1); /* TXD0 */
}


static void sync_start(void)
{
  /* Clear timer counts and enable */
  TCNT1 = 0;
  TCNT3 = 0;
  TCCR3B |= _BV(CS31); /* 1/8 prescaler */
  TCCR1B |= _BV(CS10); /* No prescaler */
}


void video_init(void)
{
  disable_raster_interrupt();
  input_init();
  sync_init();
  pixel_data_init();
}


void video_start(void)
{
  if ((TCCR1B & 0b111) == 0) {
    sync_start();
    PORTD |= _BV(5)|_BV(0);
    linenum = 0xFE;  //!!! prevents spurious raster interrupt that sometimes happens on startup. TODO explain why
    TCNT2 = 0;
    sei();
  }
}


void video_stop(void)
{
  cli();
  clear_bit(PORTD, 5); /* blank shift registers */
  clear_bits(TCCR1B, _BV(CS12)|_BV(CS11)|_BV(CS10)); /* stop timers */
  clear_bits(TCCR3B, _BV(CS32)|_BV(CS31)|_BV(CS30));
  TIFR1 = 0;
  TIFR3 = 0;
}


void set_tilemap(TILEMAP_PTR tileptr)
{
  if (!tileptr) { tileptr = amscii_font_8x8; }
  tilemap_hi = xhi8(tileptr);
  tilemap_hh = xhh8(tileptr);
}


void set_linehandler(video_fn handler)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    video_linehandler = handler;
  }
}


#define SET_VIDEO_MODE(mode) \
  set_linehandler(mode##_LINEHANDLER); \
  bytesperline = mode##_BYTESPERLINE; \
  linesperscreen = mode##_LINES; \
  SET_SHIFT_REGISTER_MODE(mode##_SRMODE); \
  OCR2A = OCR2A_VALUE_FOR_SRPERIOD(mode##_SRPERIOD); \
  OCR1A = OCR1A_VALUE_FOR_XOFFSET(mode##_XOFFSET);


void init_40x25_2color_text_mode(void *chars, TILEMAP_PTR font)
{
  SET_VIDEO_MODE(VIDEO_MODE_40x25_TEXT);
  set_tilemap(font);
  screen_ptr = chars;
  COLORBURST_ON();
}


void init_40x25_mono_text_mode(void *chars, TILEMAP_PTR font)
{
  SET_VIDEO_MODE(VIDEO_MODE_40x25_TEXT);
  set_tilemap(font);
  screen_ptr = chars;
  globalcolor = TEXT_COLOR_WHITE;
  COLORBURST_OFF();
}


void init_40x25_color_text_mode(void *chars_and_attrs, TILEMAP_PTR font)
{
  if (IS_ROM(font)) {
    SET_VIDEO_MODE(VIDEO_MODE_40x25_COLOR_TEXT_ROMFONT);
  } else {
    SET_VIDEO_MODE(VIDEO_MODE_40x25_COLOR_TEXT_RAMFONT);
  }
  screen_ptr = chars_and_attrs;
  set_tilemap(font);
  COLORBURST_ON();
}


void init_80x25_mono_text_mode(void *chars, TILEMAP_PTR font)
{
  SET_VIDEO_MODE(VIDEO_MODE_80x25_TEXT);
  screen_ptr = chars;
  set_tilemap(font);
  COLORBURST_OFF();
}


void init_hires_tiled_mode(void *tiles, TILEMAP_PTR patterns)
{
  SET_VIDEO_MODE(VIDEO_MODE_HIRES_TILED);
  screen_ptr = tiles;
  set_tilemap(patterns);
  COLORBURST_ON();
}


void init_160x200x256_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_160x200_256COLOR);
  screen_ptr = bitmap;
  COLORBURST_ON();
}


void init_160x100x256_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_160x100_256COLOR);
  screen_ptr = bitmap;
  COLORBURST_ON();
}



void init_128x100x256_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_128x100_256COLOR);
  screen_ptr = bitmap;
  COLORBURST_ON();
}


void init_80x100x256_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_80x100_256COLOR);
  screen_ptr = bitmap;
  COLORBURST_ON();
}


void init_80x50x256_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_80x50_256COLOR);
  screen_ptr = bitmap;
  COLORBURST_ON();
}


static void init_80byteperline_bitmap_mode(void *bitmap, bool color)
{
  SET_VIDEO_MODE(VIDEO_MODE_160x200_16COLOR);
  screen_ptr = bitmap;
  if (color) { COLORBURST_ON(); } else { COLORBURST_OFF(); }
}


void init_640x200_mono_bitmap_mode(void *bitmap) {
  init_80byteperline_bitmap_mode(bitmap, 0);
}


void init_160x200x16_bitmap_mode(void *bitmap) {
  init_80byteperline_bitmap_mode(bitmap, COLORBURST_PATTERN);
}


void init_160x100x16_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_160x100_16COLOR);
  screen_ptr = bitmap;
  COLORBURST_ON();
}


void init_128x100x16_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_128x100_16COLOR);
  screen_ptr = bitmap;
  COLORBURST_ON();
}


void init_80x100x16_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_80x100_16COLOR);
  screen_ptr = bitmap;
  tilemap_hi = HORIZ_RES_HALF;
  COLORBURST_ON();
}


static void init_40byteperline_bitmap_mode(void *bitmap, bool color)
{
  SET_VIDEO_MODE(VIDEO_MODE_160x200_4COLOR);
  screen_ptr = bitmap;
  tilemap_hi = TEXT_COLOR_WHITE;
  if (color) { COLORBURST_ON(); } else { COLORBURST_OFF(); }
}


void init_320x200_mono_bitmap_mode(void *bitmap) {
  init_40byteperline_bitmap_mode(bitmap, false);
}


void init_160x200x4_bitmap_mode(void *bitmap) {
  init_40byteperline_bitmap_mode(bitmap, true);
}


void init_160x100x4_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_160x100_4COLOR);
  screen_ptr = bitmap;
  tilemap_hi = TEXT_COLOR_WHITE;
  COLORBURST_ON();
}


void init_80x100x4_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_80x100_4COLOR);
  screen_ptr = bitmap;
  tilemap_hi = TEXT_COLOR_WHITE;
  COLORBURST_ON();
}


void init_80x50x16_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_80x50_16COLOR);
  screen_ptr = bitmap;
  tilemap_hi = TEXT_COLOR_WHITE;
  COLORBURST_ON();
}


void init_160x200_mono_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_160x200_MONO);
  screen_ptr = bitmap;
  COLORBURST_OFF();
}


void init_160x100_mono_bitmap_mode(void *bitmap)
{
  SET_VIDEO_MODE(VIDEO_MODE_160x100_MONO);
  screen_ptr = bitmap;
  COLORBURST_OFF();
}


void set_raster_interrupt(uint8_t linenum, video_fn code)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    raster_int_scanline = linenum;
    raster_int = code;
  };
}


void disable_raster_interrupt(void) {
  raster_int_scanline = 255;
}


void load_char_pattern(uint8_t *ramchar, const uint8_t *romchar, uint8_t xormask)
{
  for (uint8_t row = 0; row < 8; row++) {
    *ramchar = pgm_read_byte(romchar) ^ xormask;
    ramchar += 256;
    romchar += 256;
  }
}


void load_char_pattern_packed(uint8_t *ramchar, const uint8_t *romchar, uint8_t xormask)
{
  for (uint8_t row = 0; row < 8; row++) {
    *ramchar = pgm_read_byte(romchar) ^ xormask;
    ramchar += 256;
    romchar += 1;
  }
}


#undef X
#define X(m)  m##_BYTESPERLINE * m##_LINES,
static const uint16_t bitmap_mode_screen_buf_sizes[NUM_BITMAP_MODES] PROGMEM = {
  BITMAP_MODES
};

struct bitmap_mode_params {
  uint8_t bytesperline;
  uint8_t format;
  uint8_t linesperscreen;
  uint8_t linesperrow;
  uint8_t ocr2a_val;
  uint8_t ocr1a_val;
  struct {
    uint8_t tilemap:2;
    uint8_t srmode2:1;
    uint8_t colorburst:1;
  };
  video_fn linehandler;
};

#define TILEMAP_NO_CHANGE       0
#define TILEMAP_BITSPREAD       1
#define TILEMAP_HORIZ_RES_HALF  2

#undef X
#define X(m) { \
  .bytesperline = m##_BYTESPERLINE, \
  .format = m##_FORMAT, \
  .linesperscreen = m##_LINES, \
  .linesperrow = (m##_LINES)/(ACTIVE_VIDEO_LINES/8), \
  .ocr2a_val = OCR2A_VALUE_FOR_SRPERIOD(m##_SRPERIOD), \
  .ocr1a_val = OCR1A_VALUE_FOR_XOFFSET(m##_XOFFSET), \
  .colorburst = m##_COLORBURST, \
  .tilemap = m##_TILEMAP, \
  .srmode2 = (m##_SRMODE == 2), \
  .linehandler = m##_LINEHANDLER \
}, \


static const struct bitmap_mode_params bitmap_modes[NUM_BITMAP_MODES] PROGMEM = {
  BITMAP_MODES
};


void set_bitmap_mode(enum bitmap_mode_num mode, uint8_t height)
{
  if (mode >= COUNT_OF(bitmap_modes)) { return; }
  const struct bitmap_mode_params *paramptr = &bitmap_modes[mode];
  struct bitmap_mode_params params;
  memcpy_P(&params, paramptr, sizeof(params));
  globalcolor = 0xFF; /* restore current color to white */
  bytesperline = params.bytesperline;
  if (height == 0) {
    linesperscreen = params.linesperscreen;
  } else {
    uint8_t nrows = height >> 3; /* a "row" is 8 pixels tall */
    linesperscreen = nrows*params.linesperrow;
  }
  if (params.tilemap == TILEMAP_BITSPREAD) {
    tilemap_hi = TEXT_COLOR_WHITE;
  } else if (params.tilemap == TILEMAP_HORIZ_RES_HALF) {
    tilemap_hi = HORIZ_RES_HALF;
  }
  tilemap_hh = params.format;
  if (params.srmode2) {
    SET_SHIFT_REGISTER_MODE_2();
  } else {
    SET_SHIFT_REGISTER_MODE_1();
  }
  OCR2A = params.ocr2a_val;
  OCR1A = params.ocr1a_val;
  if (params.colorburst) { COLORBURST_ON(); } else { COLORBURST_OFF(); }
  set_linehandler(params.linehandler);
  video_start();
}


uint16_t screen_buf_size_for_bitmap_mode(enum bitmap_mode_num mode, uint8_t height)
{
  if (mode < COUNT_OF(bitmap_mode_screen_buf_sizes)) {
    if (height == 0) {
      return pgm_read_word(&bitmap_mode_screen_buf_sizes[mode]);
    } else {
      uint8_t nrows = height >> 3; /* a "row" is 8 pixels tall */
      uint8_t bytesperline = pgm_read_byte(&(bitmap_modes[mode].bytesperline));
      uint8_t linesperrow = pgm_read_byte(&(bitmap_modes[mode].linesperrow));
      return bytesperline * linesperrow * nrows;
    }
  } else {
    return 0;
  }
}
