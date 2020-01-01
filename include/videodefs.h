#ifndef VIDEODEFS_H_
#define VIDEODEFS_H_

#include "defs.h"

/* Actual NTSC spec value is 227.5, but many computers and game consoles */
/* rounded up to 228 */
#define COLORBURST_PERIODS_PER_LINE       228
/* approx. 3.91 usec. this is out of spec (NTSC wants 4.7 us) but I've tried */
/* numerous TVs (CRT and LCD) and capture/HD-converter devices and they are */
/* OK with it. Some displays are fine with hsync pulses as short as 3 us. */
#define COLORBURST_PERIODS_PER_HSYNC      14
/* back porch duration; controls horizontal position of image */
#define COLORBURST_PERIODS_PER_BACK_PORCH 22
/* "fake progressive scan" uses 262 lines/frame */
#define LINES_PER_FRAME                   262
#define ACTIVE_VIDEO_LINES                200
/* change this number to adjust vertical positioning */
#define VBLANK_LINES                      34
/* change this number to adjust length of vsync */
/* some TVs are fine with values as high as 12, */
/* but some cheap NTSC-to-digital converters don't like that */
#define VSYNC_LINES                       2

#if F_CPU==14318182
#define CYCLES_PER_COLORBURST         4
#define COLOR_BITS                    8
#else
#error Clock frequency must be 4x NTSC colorburst (14.3181818 MHz)
#endif

/* 912 AVR cycles per line */
#define CYCLES_PER_LINE       ((COLORBURST_PERIODS_PER_LINE)*(CYCLES_PER_COLORBURST))
#define CYCLES_PER_HSYNC      ((COLORBURST_PERIODS_PER_HSYNC)*(CYCLES_PER_COLORBURST))
#define CYCLES_PER_BACK_PORCH ((COLORBURST_PERIODS_PER_BACK_PORCH)*(CYCLES_PER_COLORBURST))

#define ACTIVE_VIDEO_BYTES_PER_LINE   (20*COLOR_BITS)

#define COLORBURST_PATTERN 0x88

#ifndef __ASSEMBLER__
/* Tilemaps can be in ROM or RAM, so they need to be referenced */
/* by 24-bit pointers. */
typedef __memx PAGE_ALIGNED uint8_t TILEMAP[];
typedef const __memx PAGE_ALIGNED uint8_t *TILEMAP_PTR;
#endif

/* Bit number in tilemap_hh that indicates if the tilemap is in RAM or ROM. */
/* avr-gcc sets the high bit of a __memx pointer if it points to RAM. */
#define TILEMAP_IS_RAM_BIT  7

#define VFMT_TEXT             (0<<5)
#define VFMT_256COLOR_BITMAP  (1<<5)
#define VFMT_16COLOR_BITMAP   (2<<5)
#define VFMT_4COLOR_BITMAP    (3<<5)
#define VFMT_MONO_BITMAP      (5<<5)

#endif
