#ifndef VIDEO_H_
#define VIDEO_H_

#include <avr/io.h>
#include <stdint.h>
#include "videodefs.h"

typedef void (*video_fn)(void);

/* Initializes the video system. */
void video_init(void);

/* Enables interrupts and starts outputting the video signal. */
void video_start(void);

/* Disables interrupts and stops outputting the video signal. */
void video_stop(void);

/**
 * Text mode
 * 40 columns by 25 rows
 * White on black
 * Characters in RAM (1000 bytes)
 * Font in RAM or ROM (2048 bytes, defaults to built-in 8x8 font if NULL)
 */
void init_40x25_mono_text_mode(void *chars, TILEMAP_PTR font);

/**
 * Text mode
 * 40 columns by 25 rows
 * Global foreground/background color
 * Characters in RAM (1000 bytes)
 * Font in RAM or ROM (2048 bytes, defaults to built-in 8x8 font if NULL)
 */
void init_40x25_2color_text_mode(void *chars, TILEMAP_PTR font);

/**
 * Text mode
 * 40 columns by 25 rows
 * Color settable per character
 * Characters and colors in RAM (2000 bytes)
 * Font in RAM or ROM (2048 bytes, defaults to built-in 8x8 font if NULL)
 */
void init_40x25_color_text_mode(void *chars_and_attrs, TILEMAP_PTR font);

/**
 * Text mode
 * 80 columns by 25 rows
 * White on black
 * Characters in RAM (2000 bytes)
 * Font in RAM or ROM (2048 bytes, defaults to built-in 8x8 font if NULL)
 */
void init_80x25_mono_text_mode(void *chars, TILEMAP_PTR font);

/**
 * High-resolution tiled mode
 * 40 columns by 25 rows
 * Tiles are 16 bits wide and 8 lines high (effective 4x8 16-color resolution,
 * or 16x8 mono resolution)
 * Name table in RAM (1000 bytes)
 * Pattern table in RAM or ROM (4096 bytes)
 * Can also be used as a 40x25 text mode with high resolution 16x8 characters.
 */
void init_hires_tiled_mode(void *tiles, TILEMAP_PTR patterns);

/**
 * Bitmap mode
 * 160 by 200 pixels
 * 8 bits per pixel (256 colors)
 * RAM requirement: 32000 bytes (more than the total RAM of the device)
 * It's not possible to have a fullscreen 256-color framebuffer, so you'll have
 * to get creative with raster interrupts
 */
extern void init_160x200x256_bitmap_mode(void *bitmap);

/**
 * Bitmap mode
 * 160 by 100 pixels
 * 8 bits per pixel (256 colors)
 * RAM requirement: 16000 bytes
 */
extern void init_160x100x256_bitmap_mode(void *bitmap);

/**
 * "Square" bitmap mode
 * 128 by 100 pixels
 * 8 bits per pixel (256 colors)
 * RAM requirement: 12800 bytes
 */
extern void init_128x100x256_bitmap_mode(void *bitmap);

/**
 * Low-resolution bitmap mode
 * 80 by 100 pixels
 * 8 bits per pixel (256 colors)
 * RAM requirement: 8000 bytes
 */
extern void init_80x100x256_bitmap_mode(void *bitmap);

/**
 * Double low-resolution bitmap mode
 * 80 by 50 pixels
 * 8 bits per pixel (256 colors)
 * RAM requirement: 4000 bytes
 */
extern void init_80x50x256_bitmap_mode(void *bitmap);

/**
 * Bitmap mode
 * 80 bytes per line by 200 lines
 * 160x200 at 16 colors (or 640x200 mono)
 * RAM requirement: 16000 bytes
 */
extern void init_160x200x16_bitmap_mode(void *bitmap);
extern void init_640x200_mono_bitmap_mode(void *bitmap);

/**
 * Bitmap mode
 * 80 bytes per line by 100 lines (vertical resolution halved)
 * 160x100 at 16 colors (or 640x100 mono)
 * RAM requirement: 8000 bytes
 */
extern void init_160x100x16_bitmap_mode(void *bitmap);

/**
 * "Square" bitmap mode
 * 64 bytes per line by 100 lines (vertical resolution halved)
 * 128x100 at 16 colors (or 512x100 mono)
 * RAM requirement: 6400 bytes
 */
extern void init_160x100x16_bitmap_mode(void *bitmap);

/**
 * Low-resolution bitmap mode
 * 40 bytes per line by 100 lines (one-quarter resolution)
 * 80x100 at 16 colors
 * RAM requirement: 4000 bytes
 */
extern void init_80x100x16_bitmap_mode(void *bitmap);

/**
 * Double-low resolution bitmap mode
 * 40 bytes per line by 50 lines
 * 80x50 at 16 colors
 * RAM requirement: 2000 bytes
 */
extern void init_80x50x16_bitmap_mode(void *bitmap);

/**
 * Bitmap mode
 * 40 bytes per line by 200 lines
 * 160x200 at 4 colors (or 320x200 mono)
 * RAM requirement: 8000 bytes
 */
extern void init_160x200x4_bitmap_mode(void *bitmap);
extern void init_320x200_mono_bitmap_mode(void *bitmap);

/**
 * Bitmap mode
 * 40 bytes per line by 100 lines
 * 160x100 at 4 colors (or 320x100 mono)
 * RAM requirement: 4000 bytes
 */
extern void init_160x100x4_bitmap_mode(void *bitmap);

/**
 * Bitmap mode
 * 20 bytes per line by 100 lines
 * 80x100 at 4 colors
 * RAM requirement: 2000 bytes
 */
extern void init_80x100x4_bitmap_mode(void *bitmap);

/**
 * Bitmap mode
 * 20 bytes per line by 200 lines
 * 160x200 mono
 * RAM requirement: 4000 bytes
 */
extern void init_160x200_mono_bitmap_mode(void *bitmap);

/**
 * Bitmap mode
 * 20 bytes per line by 100 lines
 * 160x100 mono
 * RAM requirement: 2000 bytes
 */
extern void init_260x200_mono_bitmap_mode(void *bitmap);

/* Macro used to pause execution until the start of a new frame. */
#define _frame_sync() loop_until_bit_is_set(DDRB,1); DDRB &= ~_BV(1)
#define frame_sync()  (({ _frame_sync(); 1; }) || 1)

/**
 * Default font using the "AMSCII" 7-bit character set.
 * Code points 0-127 are normal white-on-black video.
 * Code points 128-255 are inverse black-on-white video.
 */
extern const TILEMAP amscii_font_8x8;

/**
 * Font using the IBM/DOS "code page 437" 8-bit character set.
 */
extern const TILEMAP cp437_font_8x8;


/**
 * Specifies a piece of code to be executed after the given scanline has been
 * drawn.
 *
 * This should be called during vblank to avoid potential visual glitches.
 * To disable the raster interrupt, set linenum to a value greater than or
 * equal to the number of screen rows, e.g. 255.
 *
 * The code block should be written in assembler. It has special register
 * restrictions and must end with a jump instruction (NOT a ret.)
 */
void set_raster_interrupt(uint8_t linenum, video_fn code);
void disable_raster_interrupt(void);

void set_tilemap(TILEMAP_PTR tileptr);
void set_linehandler(video_fn handler);

void load_char_pattern(uint8_t *ramchar, const uint8_t *romchar, uint8_t xormask);
void load_char_pattern_packed(uint8_t *ramchar, const uint8_t *romchar, uint8_t xormask);

#define COLORBURST_OFF()    clear_bit(color_video, color_video_bit)
#define COLORBURST_ON()     set_bit(color_video, color_video_bit)



/* Pointer to the start of screen contents. */
/* (In bitmap modes, this is the address of the screen bitmap. */
/* In text/tiled modes, this is the address of the name table. */
extern char *screen_ptr;

/* Upper 2 bytes byte of the tilemap address for text/tiled modes. */
/* (Tilemaps must be 256-byte aligned, the low byte is always 0.) */
extern uint8_t tilemap_hi;
/* Only bits 0 and 7 of tilemap_hh are used. */
/* If bit 7 is 0, tilemap is in ROM. If 1, tilemap is in RAM. */
/* If tilemap is in ROM, bit 0 indicates page 0 (lower 64KB) or 1 (upper 64KB). */
/* The other bits are free to be used for other purposes. */
extern uint8_t tilemap_hh;

/* Global text color for 2-color 40x25 text mode. */
extern uint8_t globalcolor;

/* Vertical scroll offset for text/tiled modes. Lower 3 bits are significant. */
extern uint8_t vscroll;

/* Number of bytes per line in the current video mode. */
/* Changing this value does not alter the display, but its value can be used */
/* to convert an XY coordinate pair to a byte address in the screen buffer. */
extern uint8_t bytesperline;

/* Number of lines (vertical resolution) of the current video mode. */
/* Changing this value does not alter the display, but its value can be */
/* multiplied by bytesperline to get the size of the screen buffer. */
extern uint8_t linesperscreen;

extern uint8_t raster_int_scanline;
extern video_fn raster_int;


#endif
