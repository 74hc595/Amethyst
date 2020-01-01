/**
 * Video mode specifics.
 */
#ifndef VIDEOMODES_H_
#define VIDEOMODES_H_

#include "defs.h"

extern void linehandler_160x200x256(void);
extern void linehandler_160x100x256(void);
extern void linehandler_128x100x256(void);
extern void linehandler_80x50x256(void);
extern void linehandler_80x25x256(void);
extern void linehandler_Nx200x16(void);
extern void linehandler_Nx100x16(void);
extern void linehandler_160x200x4(void);
extern void linehandler_160x100x4(void);
extern void linehandler_160x50x4(void);
extern void linehandler_80x100x4(void);
extern void linehandler_160x200_mono(void);
extern void linehandler_160x100_mono(void);
extern void linehandler_80x25_text(void);
extern void linehandler_40x25_text(void);
extern void linehandler_40x25_basictext(void);
extern void linehandler_40x25_color_text_romfont(void);
extern void linehandler_40x25_color_text_ramfont(void);
extern void linehandler_hires_tiled(void);
extern void linehandler_null(void);

/**
 * VIDEO_MODE_x_SRMODE: whether to use shift register mode 1 or 2
 * VIDEO_MODE_x_SRPERIOD: number of colorburst cycles between shift reg. strobes
 * VIDEO_MODE_x_XOFFSET: back porch length adjustment in AVR cycles
 *   (to change horizontal positioning, because different linehandlers have
 *   different numbers of AVR cycles from the interrupt to the first pixel
 *   data output)
 * VIDEO_MODE_x_LINEHANDLER: function (see videomodes.S) that is invoked once
 *   per line to output pixel data.
 */

/* Supports single global color and ROM/RAM font */
#define VIDEO_MODE_40x25_TEXT_SRMODE                      2
#define VIDEO_MODE_40x25_TEXT_SRPERIOD                    2
#define VIDEO_MODE_40x25_TEXT_XOFFSET                     -8
#define VIDEO_MODE_40x25_TEXT_BYTESPERLINE                40
#define VIDEO_MODE_40x25_TEXT_FORMAT                      VFMT_TEXT
#define VIDEO_MODE_40x25_TEXT_LINES                       25
#define VIDEO_MODE_40x25_TEXT_COLORBURST                  0
#define VIDEO_MODE_40x25_TEXT_LINEHANDLER                 linehandler_40x25_text
#define VIDEO_MODE_40x25_TEXT_TILEMAP                     TILEMAP_NO_CHANGE

/* Fixed white-on-black text and fixed ROM tilemap */
#define VIDEO_MODE_40x25_BASICTEXT_SRMODE                 2
#define VIDEO_MODE_40x25_BASICTEXT_SRPERIOD               2
#define VIDEO_MODE_40x25_BASICTEXT_XOFFSET                0
#define VIDEO_MODE_40x25_BASICTEXT_BYTESPERLINE           40
#define VIDEO_MODE_40x25_BASICTEXT_FORMAT                 VFMT_TEXT
#define VIDEO_MODE_40x25_BASICTEXT_LINES                  25
#define VIDEO_MODE_40x25_BASICTEXT_COLORBURST             0
#define VIDEO_MODE_40x25_BASICTEXT_LINEHANDLER            linehandler_40x25_basictext
#define VIDEO_MODE_40x25_BASICTEXT_TILEMAP                TILEMAP_NO_CHANGE

#define VIDEO_MODE_40x25_COLOR_TEXT_ROMFONT_SRMODE        2
#define VIDEO_MODE_40x25_COLOR_TEXT_ROMFONT_SRPERIOD      2
#define VIDEO_MODE_40x25_COLOR_TEXT_ROMFONT_XOFFSET       -56
#define VIDEO_MODE_40x25_COLOR_TEXT_ROMFONT_BYTESPERLINE  40
#define VIDEO_MODE_40x25_COLOR_TEXT_ROMFONT_FORMAT        VFMT_TEXT
#define VIDEO_MODE_40x25_COLOR_TEXT_ROMFONT_LINES         25
#define VIDEO_MODE_40x25_COLOR_TEXT_ROMFONT_COLORBURST    1
#define VIDEO_MODE_40x25_COLOR_TEXT_ROMFONT_LINEHANDLER   linehandler_40x25_color_text_romfont
#define VIDEO_MODE_40x25_COLOR_TEXT_ROMFONT_TILEMAP       TILEMAP_NO_CHANGE
#define VIDEO_MODE_40x25_COLOR_TEXT_RAMFONT_SRMODE        2
#define VIDEO_MODE_40x25_COLOR_TEXT_RAMFONT_SRPERIOD      2
#define VIDEO_MODE_40x25_COLOR_TEXT_RAMFONT_XOFFSET       -8
#define VIDEO_MODE_40x25_COLOR_TEXT_RAMFONT_BYTESPERLINE  40
#define VIDEO_MODE_40x25_COLOR_TEXT_RAMFONT_FORMAT 0
#define VIDEO_MODE_40x25_COLOR_TEXT_RAMFONT_LINES         25
#define VIDEO_MODE_40x25_COLOR_TEXT_RAMFONT_COLORBURST    1
#define VIDEO_MODE_40x25_COLOR_TEXT_RAMFONT_LINEHANDLER   linehandler_40x25_color_text_ramfont
#define VIDEO_MODE_40x25_COLOR_TEXT_RAMFONT_TILEMAP       TILEMAP_NO_CHANGE

#define VIDEO_MODE_80x25_TEXT_SRMODE                      2
#define VIDEO_MODE_80x25_TEXT_SRPERIOD                    2
#define VIDEO_MODE_80x25_TEXT_XOFFSET                     0
#define VIDEO_MODE_80x25_TEXT_BYTESPERLINE                80
#define VIDEO_MODE_80x25_TEXT_FORMAT                      VFMT_TEXT
#define VIDEO_MODE_80x25_TEXT_LINES                       25
#define VIDEO_MODE_80x25_TEXT_COLORBURST                  0
#define VIDEO_MODE_80x25_TEXT_LINEHANDLER                 linehandler_80x25_text
#define VIDEO_MODE_80x25_TEXT_TILEMAP                     TILEMAP_NO_CHANGE

#define VIDEO_MODE_HIRES_TILED_SRMODE                     2
#define VIDEO_MODE_HIRES_TILED_SRPERIOD                   2
#define VIDEO_MODE_HIRES_TILED_XOFFSET                    -8
#define VIDEO_MODE_HIRES_TILED_BYTESPERLINE               40
#define VIDEO_MODE_HIRES_TILED_FORMAT                     VFMT_TEXT
#define VIDEO_MODE_HIRES_TILED_LINES                      25
#define VIDEO_MODE_HIRES_TILED_COLORBURST                 1
#define VIDEO_MODE_HIRES_TILED_LINEHANDLER                linehandler_hires_tiled
#define VIDEO_MODE_HIRES_TILED_TILEMAP                    TILEMAP_NO_CHANGE

#define VIDEO_MODE_160x200_256COLOR_SRMODE                1
#define VIDEO_MODE_160x200_256COLOR_SRPERIOD              1
#define VIDEO_MODE_160x200_256COLOR_XOFFSET               16
#define VIDEO_MODE_160x200_256COLOR_BYTESPERLINE          160
#define VIDEO_MODE_160x200_256COLOR_FORMAT                VFMT_256COLOR_BITMAP
#define VIDEO_MODE_160x200_256COLOR_LINES                 200
#define VIDEO_MODE_160x200_256COLOR_COLORBURST            1
#define VIDEO_MODE_160x200_256COLOR_LINEHANDLER           linehandler_160x200x256
#define VIDEO_MODE_160x200_256COLOR_TILEMAP               TILEMAP_NO_CHANGE

#define VIDEO_MODE_160x100_256COLOR_SRMODE                1
#define VIDEO_MODE_160x100_256COLOR_SRPERIOD              1
#define VIDEO_MODE_160x100_256COLOR_XOFFSET               16
#define VIDEO_MODE_160x100_256COLOR_BYTESPERLINE          160
#define VIDEO_MODE_160x100_256COLOR_FORMAT                VFMT_256COLOR_BITMAP
#define VIDEO_MODE_160x100_256COLOR_LINES                 100
#define VIDEO_MODE_160x100_256COLOR_COLORBURST            1
#define VIDEO_MODE_160x100_256COLOR_LINEHANDLER           linehandler_160x100x256
#define VIDEO_MODE_160x100_256COLOR_TILEMAP               TILEMAP_NO_CHANGE

#define VIDEO_MODE_128x100_256COLOR_SRMODE                1
#define VIDEO_MODE_128x100_256COLOR_SRPERIOD              1
#define VIDEO_MODE_128x100_256COLOR_XOFFSET               72
#define VIDEO_MODE_128x100_256COLOR_BYTESPERLINE          128
#define VIDEO_MODE_128x100_256COLOR_FORMAT                VFMT_256COLOR_BITMAP
#define VIDEO_MODE_128x100_256COLOR_LINES                 100
#define VIDEO_MODE_128x100_256COLOR_COLORBURST            1
#define VIDEO_MODE_128x100_256COLOR_LINEHANDLER           linehandler_128x100x256
#define VIDEO_MODE_128x100_256COLOR_TILEMAP               TILEMAP_NO_CHANGE

#define VIDEO_MODE_80x100_256COLOR_SRMODE                 1
#define VIDEO_MODE_80x100_256COLOR_SRPERIOD               2
#define VIDEO_MODE_80x100_256COLOR_XOFFSET                16
#define VIDEO_MODE_80x100_256COLOR_BYTESPERLINE           80
#define VIDEO_MODE_80x100_256COLOR_FORMAT                 VFMT_256COLOR_BITMAP
#define VIDEO_MODE_80x100_256COLOR_LINES                  100
#define VIDEO_MODE_80x100_256COLOR_COLORBURST             1
#define VIDEO_MODE_80x100_256COLOR_LINEHANDLER            linehandler_Nx100x16
#define VIDEO_MODE_80x100_256COLOR_TILEMAP                TILEMAP_NO_CHANGE

#define VIDEO_MODE_80x50_256COLOR_SRMODE                  1
#define VIDEO_MODE_80x50_256COLOR_SRPERIOD                2
#define VIDEO_MODE_80x50_256COLOR_XOFFSET                 16
#define VIDEO_MODE_80x50_256COLOR_BYTESPERLINE            80
#define VIDEO_MODE_80x50_256COLOR_FORMAT                  VFMT_256COLOR_BITMAP
#define VIDEO_MODE_80x50_256COLOR_LINES                   50
#define VIDEO_MODE_80x50_256COLOR_COLORBURST              1
#define VIDEO_MODE_80x50_256COLOR_LINEHANDLER             linehandler_80x50x256
#define VIDEO_MODE_80x50_256COLOR_TILEMAP                 TILEMAP_NO_CHANGE

#define VIDEO_MODE_80x25_256COLOR_SRMODE                  1
#define VIDEO_MODE_80x25_256COLOR_SRPERIOD                2
#define VIDEO_MODE_80x25_256COLOR_XOFFSET                 16
#define VIDEO_MODE_80x25_256COLOR_BYTESPERLINE            80
#define VIDEO_MODE_80x25_256COLOR_FORMAT                  VFMT_256COLOR_BITMAP
#define VIDEO_MODE_80x25_256COLOR_LINES                   25
#define VIDEO_MODE_80x25_256COLOR_COLORBURST              1
#define VIDEO_MODE_80x25_256COLOR_LINEHANDLER             linehandler_80x25x256
#define VIDEO_MODE_80x25_256COLOR_TILEMAP                 TILEMAP_NO_CHANGE

#define VIDEO_MODE_160x200_16COLOR_SRMODE                 2
#define VIDEO_MODE_160x200_16COLOR_SRPERIOD               2
#define VIDEO_MODE_160x200_16COLOR_XOFFSET                16
#define VIDEO_MODE_160x200_16COLOR_BYTESPERLINE           80
#define VIDEO_MODE_160x200_16COLOR_FORMAT                 VFMT_16COLOR_BITMAP
#define VIDEO_MODE_160x200_16COLOR_LINES                  200
#define VIDEO_MODE_160x200_16COLOR_COLORBURST             1
#define VIDEO_MODE_160x200_16COLOR_LINEHANDLER            linehandler_Nx200x16
#define VIDEO_MODE_160x200_16COLOR_TILEMAP                TILEMAP_NO_CHANGE

#define VIDEO_MODE_160x100_16COLOR_SRMODE                 2
#define VIDEO_MODE_160x100_16COLOR_SRPERIOD               2
#define VIDEO_MODE_160x100_16COLOR_XOFFSET                16
#define VIDEO_MODE_160x100_16COLOR_BYTESPERLINE           80
#define VIDEO_MODE_160x100_16COLOR_FORMAT                 VFMT_16COLOR_BITMAP
#define VIDEO_MODE_160x100_16COLOR_LINES                  100
#define VIDEO_MODE_160x100_16COLOR_COLORBURST             1
#define VIDEO_MODE_160x100_16COLOR_LINEHANDLER            linehandler_Nx100x16
#define VIDEO_MODE_160x100_16COLOR_TILEMAP                TILEMAP_NO_CHANGE

#define VIDEO_MODE_128x100_16COLOR_SRMODE                 2
#define VIDEO_MODE_128x100_16COLOR_SRPERIOD               2
#define VIDEO_MODE_128x100_16COLOR_XOFFSET                72
#define VIDEO_MODE_128x100_16COLOR_BYTESPERLINE           64
#define VIDEO_MODE_128x100_16COLOR_FORMAT                 VFMT_16COLOR_BITMAP
#define VIDEO_MODE_128x100_16COLOR_LINES                  100
#define VIDEO_MODE_128x100_16COLOR_COLORBURST             1
#define VIDEO_MODE_128x100_16COLOR_LINEHANDLER            linehandler_Nx100x16
#define VIDEO_MODE_128x100_16COLOR_TILEMAP                TILEMAP_NO_CHANGE

#define VIDEO_MODE_80x100_16COLOR_SRMODE                  2
#define VIDEO_MODE_80x100_16COLOR_SRPERIOD                2
#define VIDEO_MODE_80x100_16COLOR_XOFFSET                 0
#define VIDEO_MODE_80x100_16COLOR_BYTESPERLINE            40
#define VIDEO_MODE_80x100_16COLOR_FORMAT                  VFMT_16COLOR_BITMAP
#define VIDEO_MODE_80x100_16COLOR_LINES                   100
#define VIDEO_MODE_80x100_16COLOR_COLORBURST              1
#define VIDEO_MODE_80x100_16COLOR_LINEHANDLER             linehandler_160x100x4
#define VIDEO_MODE_80x100_16COLOR_TILEMAP                 TILEMAP_HORIZ_RES_HALF

#define VIDEO_MODE_160x50_4COLOR_SRMODE                   2
#define VIDEO_MODE_160x50_4COLOR_SRPERIOD                 2
#define VIDEO_MODE_160x50_4COLOR_XOFFSET                  0
#define VIDEO_MODE_160x50_4COLOR_BYTESPERLINE             40
#define VIDEO_MODE_160x50_4COLOR_FORMAT                   VFMT_4COLOR_BITMAP
#define VIDEO_MODE_160x50_4COLOR_LINES                    50
#define VIDEO_MODE_160x50_4COLOR_COLORBURST               1
#define VIDEO_MODE_160x50_4COLOR_LINEHANDLER              linehandler_160x50x4
#define VIDEO_MODE_160x50_4COLOR_TILEMAP                  TILEMAP_BITSPREAD

#define VIDEO_MODE_80x50_16COLOR_SRMODE                   2
#define VIDEO_MODE_80x50_16COLOR_SRPERIOD                 2
#define VIDEO_MODE_80x50_16COLOR_XOFFSET                  0
#define VIDEO_MODE_80x50_16COLOR_BYTESPERLINE             40
#define VIDEO_MODE_80x50_16COLOR_FORMAT                   VFMT_16COLOR_BITMAP
#define VIDEO_MODE_80x50_16COLOR_LINES                    50
#define VIDEO_MODE_80x50_16COLOR_COLORBURST               1
#define VIDEO_MODE_80x50_16COLOR_LINEHANDLER              linehandler_160x50x4
#define VIDEO_MODE_80x50_16COLOR_TILEMAP                  TILEMAP_HORIZ_RES_HALF

#define VIDEO_MODE_160x200_4COLOR_SRMODE                  2
#define VIDEO_MODE_160x200_4COLOR_SRPERIOD                2
#define VIDEO_MODE_160x200_4COLOR_XOFFSET                 0
#define VIDEO_MODE_160x200_4COLOR_BYTESPERLINE            40
#define VIDEO_MODE_160x200_4COLOR_FORMAT                  VFMT_4COLOR_BITMAP
#define VIDEO_MODE_160x200_4COLOR_LINES                   200
#define VIDEO_MODE_160x200_4COLOR_COLORBURST              1
#define VIDEO_MODE_160x200_4COLOR_LINEHANDLER             linehandler_160x200x4
#define VIDEO_MODE_160x200_4COLOR_TILEMAP                 TILEMAP_BITSPREAD

#define VIDEO_MODE_160x100_4COLOR_SRMODE                  2
#define VIDEO_MODE_160x100_4COLOR_SRPERIOD                2
#define VIDEO_MODE_160x100_4COLOR_XOFFSET                 0
#define VIDEO_MODE_160x100_4COLOR_BYTESPERLINE            40
#define VIDEO_MODE_160x100_4COLOR_FORMAT                  VFMT_4COLOR_BITMAP
#define VIDEO_MODE_160x100_4COLOR_LINES                   100
#define VIDEO_MODE_160x100_4COLOR_COLORBURST              1
#define VIDEO_MODE_160x100_4COLOR_LINEHANDLER             linehandler_160x100x4
#define VIDEO_MODE_160x100_4COLOR_TILEMAP                 TILEMAP_BITSPREAD

#define VIDEO_MODE_80x100_4COLOR_SRMODE                   2
#define VIDEO_MODE_80x100_4COLOR_SRPERIOD                 2
#define VIDEO_MODE_80x100_4COLOR_XOFFSET                  -8
#define VIDEO_MODE_80x100_4COLOR_BYTESPERLINE             20
#define VIDEO_MODE_80x100_4COLOR_FORMAT                   VFMT_4COLOR_BITMAP
#define VIDEO_MODE_80x100_4COLOR_LINES                    100
#define VIDEO_MODE_80x100_4COLOR_COLORBURST               1
#define VIDEO_MODE_80x100_4COLOR_LINEHANDLER              linehandler_80x100x4
#define VIDEO_MODE_80x100_4COLOR_TILEMAP                  TILEMAP_BITSPREAD

#define VIDEO_MODE_640x200_MONO_SRMODE                    2
#define VIDEO_MODE_640x200_MONO_SRPERIOD                  2
#define VIDEO_MODE_640x200_MONO_XOFFSET                   16
#define VIDEO_MODE_640x200_MONO_BYTESPERLINE              80
#define VIDEO_MODE_640x200_MONO_FORMAT                    VFMT_MONO_BITMAP
#define VIDEO_MODE_640x200_MONO_LINES                     200
#define VIDEO_MODE_640x200_MONO_COLORBURST                0
#define VIDEO_MODE_640x200_MONO_LINEHANDLER               linehandler_Nx200x16
#define VIDEO_MODE_640x200_MONO_TILEMAP                   TILEMAP_NO_CHANGE

#define VIDEO_MODE_320x200_MONO_SRMODE                    2
#define VIDEO_MODE_320x200_MONO_SRPERIOD                  2
#define VIDEO_MODE_320x200_MONO_XOFFSET                   0
#define VIDEO_MODE_320x200_MONO_BYTESPERLINE              40
#define VIDEO_MODE_320x200_MONO_FORMAT                    VFMT_MONO_BITMAP
#define VIDEO_MODE_320x200_MONO_LINES                     200
#define VIDEO_MODE_320x200_MONO_COLORBURST                0
#define VIDEO_MODE_320x200_MONO_LINEHANDLER               linehandler_160x200x4
#define VIDEO_MODE_320x200_MONO_TILEMAP                   TILEMAP_BITSPREAD

#define VIDEO_MODE_256x200_MONO_SRMODE                    2
#define VIDEO_MODE_256x200_MONO_SRPERIOD                  2
#define VIDEO_MODE_256x200_MONO_XOFFSET                   56
#define VIDEO_MODE_256x200_MONO_BYTESPERLINE              32
#define VIDEO_MODE_256x200_MONO_FORMAT                    VFMT_MONO_BITMAP
#define VIDEO_MODE_256x200_MONO_LINES                     200
#define VIDEO_MODE_256x200_MONO_COLORBURST                0
#define VIDEO_MODE_256x200_MONO_LINEHANDLER               linehandler_160x200x4
#define VIDEO_MODE_256x200_MONO_TILEMAP                   TILEMAP_BITSPREAD

#define VIDEO_MODE_160x200_MONO_SRMODE                    2
#define VIDEO_MODE_160x200_MONO_SRPERIOD                  2
#define VIDEO_MODE_160x200_MONO_XOFFSET                   8
#define VIDEO_MODE_160x200_MONO_BYTESPERLINE              20
#define VIDEO_MODE_160x200_MONO_FORMAT                    VFMT_MONO_BITMAP
#define VIDEO_MODE_160x200_MONO_LINES                     200
#define VIDEO_MODE_160x200_MONO_COLORBURST                0
#define VIDEO_MODE_160x200_MONO_LINEHANDLER               linehandler_160x200_mono
#define VIDEO_MODE_160x200_MONO_TILEMAP                   TILEMAP_NO_CHANGE

#define VIDEO_MODE_160x100_MONO_SRMODE                    2
#define VIDEO_MODE_160x100_MONO_SRPERIOD                  2
#define VIDEO_MODE_160x100_MONO_XOFFSET                   8
#define VIDEO_MODE_160x100_MONO_BYTESPERLINE              20
#define VIDEO_MODE_160x100_MONO_FORMAT                    VFMT_MONO_BITMAP
#define VIDEO_MODE_160x100_MONO_LINES                     100
#define VIDEO_MODE_160x100_MONO_COLORBURST                0
#define VIDEO_MODE_160x100_MONO_LINEHANDLER               linehandler_160x100_mono
#define VIDEO_MODE_160x100_MONO_TILEMAP                   TILEMAP_NO_CHANGE

/**
 * Constants for the 16 bitmap graphics modes.
 */
#define BITMAP_MODES \
  X(VIDEO_MODE_160x100_256COLOR) \
  X(VIDEO_MODE_128x100_256COLOR) \
  X(VIDEO_MODE_80x100_256COLOR)  \
  X(VIDEO_MODE_80x50_256COLOR)   \
  X(VIDEO_MODE_80x25_256COLOR)   \
  X(VIDEO_MODE_160x200_16COLOR)  \
  X(VIDEO_MODE_160x100_16COLOR)  \
  X(VIDEO_MODE_128x100_16COLOR)  \
  X(VIDEO_MODE_80x100_16COLOR)   \
  X(VIDEO_MODE_80x50_16COLOR)    \
  X(VIDEO_MODE_160x200_4COLOR)   \
  X(VIDEO_MODE_160x100_4COLOR)   \
  X(VIDEO_MODE_80x100_4COLOR)    \
  X(VIDEO_MODE_640x200_MONO)     \
  X(VIDEO_MODE_320x200_MONO)     \
  X(VIDEO_MODE_256x200_MONO)     \
  X(VIDEO_MODE_160x200_MONO)     \
  X(VIDEO_MODE_160x100_MONO)     \


#define NUM_BITMAP_MODES 18

#undef X
#define X(m) m,
enum bitmap_mode_num {
  BITMAP_MODES
};


/**
 * Returns the number of bytes required for a framebuffer in the given video mode.
 * If height_rows == 0, return the number of bytes required for a full screen
 * framebuffer.
 */
uint16_t screen_buf_size_for_bitmap_mode(enum bitmap_mode_num mode, uint8_t height);

/**
 * Activates the given numbered bitmap mode.
 */
void set_bitmap_mode(enum bitmap_mode_num mode, uint8_t height);


/**
 * Shift register mode 1 (multiplexer input = 0)
 * PORTC is interpreted as four 2-bit pixels
 * Shift register is strobed every 4 cycles.
 * Can be strobed every 8 cycles but horizontal resolution will be halved.
 */
#define SET_SHIFT_REGISTER_MODE_1()               asm volatile("cbi %[port], 7" :: [port] "I" (_SFR_IO_ADDR(PORTD)));

/**
 * Shift register mode 2 (multiplexer input = 1)
 * PORTC is interpreted as eight 1-bit pixels
 * Shift register is strobed every 8 cycles
 */
#define SET_SHIFT_REGISTER_MODE_2()               asm volatile("sbi %[port], 7" :: [port] "I" (_SFR_IO_ADDR(PORTD)));

#define SET_SHIFT_REGISTER_MODE(mode)             PASTE_(SET_SHIFT_REGISTER_MODE_,mode)()
#define OCR2A_VALUE_FOR_SRPERIOD(p)               (((p)*(CYCLES_PER_COLORBURST))-1)
#define OCR1A_VALUE_FOR_XOFFSET(x)                (((CYCLES_PER_HSYNC+CYCLES_PER_BACK_PORCH)-1)+(x))

#endif
