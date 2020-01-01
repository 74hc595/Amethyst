/**
 * Bitmap mode test patterns
 *
 * Key | Pattern                | Mode
 * ----+------------------------+--------------
 * esc | 1px vertical stripes   | 16K (160x200)
 * q   | 1px horizontal stripes | 16K (160x200)
 * w   | 1px checkerboard       | 16K (160x200)
 * e   | random rectangles      | 16K (160x200)
 * r   | 1px vertical stripes   | 8K  (160x100)
 * t   | 1px horizontal stripes | 8K  (160x100)
 * y   | 1px checkerboard       | 8K  (160x100)
 * u   | random rectangles      | 8K  (160x100)
 * i   | 1px vertical stripes   | 4K  (80x100)
 * o   | 1px horizontal stripes | 4K  (80x100)
 * p   | 1px checkerboard       | 4K  (80x100)
 * del | random rectangles      | 4K  (80x100)
 * a   | 1px vertical stripes   | 2K  (80x50)
 * s   | 1px horizontal stripes | 2K  (80x50)
 * d   | 1px checkerboard       | 2K  (80x50)
 * f   | random rectangles      | 2K  (80x50)
 */
#include "app.h"
#include <string.h>
#include <avr/interrupt.h>

static uint8_t screen[16000];

static void draw_1px_horiz_stripes(uint8_t bytes_per_line, uint8_t num_lines)
{
  uint8_t color = 0x11; /* don't use black */
  uint8_t *pixptr = screen;
  for (uint8_t y = 0; y < num_lines; y++) {
    memset(pixptr, color, bytes_per_line);
    pixptr += bytes_per_line;
    color = (color == 0xFF) ? 0x11 : color+0x11;
  }
}


static void draw_1px_vert_stripes(uint8_t bytes_per_line, uint8_t num_lines)
{
  uint8_t color = 0x21; /* don't use black */
  uint8_t *pixptr = screen;
  /* draw the first line */
  for (uint8_t x = 0; x < bytes_per_line; x++) {
    uint8_t px = color;
    *pixptr++ = px;
    color = (color == 0xEF) ? 0x12 : color+0x11;
  }
  /* then replicate it down the screen */
  for (uint8_t y = 1; y < num_lines; y++) {
    memcpy(pixptr, screen, bytes_per_line);
    pixptr += bytes_per_line;
  }
}


static void draw_1px_checkerboard(uint8_t bytes_per_line, uint8_t num_lines)
{
  uint8_t color = 0x21; /* don't use black */
  uint8_t *pixptr = screen;
  while (pixptr != screen+(bytes_per_line*num_lines)) {
    uint8_t px = color;
    *pixptr++ = px;
    color = (color == 0xEF) ? 0x21 : color+0x11;
  }
}


static void draw_1px_horiz_stripes_4color(uint8_t bytes_per_line, uint8_t num_lines)
{
  uint8_t *pixptr = screen;
  for (uint8_t y = 0; y < num_lines/4; y++) {
    memset(pixptr, 0b11111111, bytes_per_line); /* white */
    pixptr += bytes_per_line;
    memset(pixptr, 0b00000000, bytes_per_line); /* black */
    pixptr += bytes_per_line;
    memset(pixptr, 0b01010101, bytes_per_line); /* blue */
    pixptr += bytes_per_line;
    memset(pixptr, 0b10101010, bytes_per_line); /* red */
    pixptr += bytes_per_line;
  }
}


static void draw_1px_vert_stripes_4color(uint8_t bytes_per_line, uint8_t num_lines)
{
  memset(screen, 0b10100101, bytes_per_line*num_lines);
}


static void draw_1px_checkerboard_4color(uint8_t bytes_per_line, uint8_t num_lines)
{
  uint8_t *pixptr = screen;
  for (uint8_t y = 0; y < num_lines/4; y++) {
    memset(pixptr, 0b10010011, bytes_per_line);
    pixptr += bytes_per_line;
    memset(pixptr, 0b01001110, bytes_per_line);
    pixptr += bytes_per_line;
    memset(pixptr, 0b00111001, bytes_per_line); /* red */
    pixptr += bytes_per_line;
    memset(pixptr, 0b11100100, bytes_per_line); /* blue */
    pixptr += bytes_per_line;
  }
}


static void draw_4px_checkerboard_4color(uint8_t bytes_per_line, uint8_t num_lines)
{
  uint8_t *pixptr = screen;
  uint8_t color = 0b01010101;
  for (uint8_t y = 0; y < num_lines; y+=4) {
    uint8_t *rowstart = pixptr;
    for (uint8_t x = 0; x < bytes_per_line; x++) {
      *pixptr++ = color;
      color = (color == 0b11111111) ? 0b01010101 : color+0b01010101;
    }
    memcpy(pixptr, rowstart, bytes_per_line);
    pixptr += bytes_per_line;
    memcpy(pixptr, rowstart, bytes_per_line);
    pixptr += bytes_per_line;
    memcpy(pixptr, rowstart, bytes_per_line);
    pixptr += bytes_per_line;
  }
}


static void draw_1px_checkerboard_256color(uint8_t bytes_per_line, uint8_t num_lines)
{
  uint8_t *pixptr = screen;
  uint8_t color = 1;
  while (pixptr != screen+(bytes_per_line*num_lines)) {
    *pixptr++ = color++;
    if (color == 0) { color = 1; }
  }
}


static void draw_1px_diagonals_mono(uint8_t bytes_per_line, uint8_t num_lines)
{
  uint8_t *pixptr = screen;
  uint8_t pattern = 0x80;
  for (uint8_t y = 0; y < num_lines; y++) {
    memset(pixptr, pattern, bytes_per_line);
    pixptr += bytes_per_line;
    pattern >>= 1;
    if (pattern == 0) { pattern = 0x80; }
  }
}


static void draw_rect(uint8_t screenwidth, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color)
{
  uint8_t *upperleft = screen+(screenwidth*y)+x;
  memset(upperleft, color, w);
  uint8_t *lowerleft = screen+(screenwidth*(y-1+h))+x;
  memset(lowerleft, color, w);
  while (upperleft != lowerleft) {
    upperleft += screenwidth;
    *upperleft = color;
    *(upperleft+(w-1)) = color;
  }
}


/* straight from Rosetta Code, not optimized */
static void draw_line(uint8_t screenwidth, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color) {
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = (dx>dy ? dx : -dy)/2, e2;
  for(;;){
    screen[(y0*screenwidth)+x0] = color;
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}


static void draw_rects_256color(uint8_t bytes_per_line, uint8_t num_lines)
{
  memset(screen, 0, bytes_per_line*num_lines);
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t w = bytes_per_line;
  uint8_t h = num_lines;
  uint8_t color = 0xFF;
  while (w >= 4 && h >= 4) {
    draw_rect(bytes_per_line, x, y, w, h, color);
    x += 2;
    y += 2;
    w -= 4;
    h -= 4;
    color-=3;
  }
}


static void draw_lines_256color(uint8_t bytes_per_line, uint8_t num_lines)
{
  memset(screen, 0, bytes_per_line*num_lines);
  uint8_t color = 0xFF;
  for (uint8_t x = 0; x < bytes_per_line; x+=2) {
    draw_line(bytes_per_line, 0, 0, x, num_lines-1, color);
    color--;
    draw_line(bytes_per_line, bytes_per_line-1, 0, x, num_lines-1, color);
    color--;
  }
}


/* for alignment comparisons with text modes */
static void draw_chars(uint8_t w) {
  for (uint16_t n = 0; n < w*25; n++) {
    screen[n] = (n&0xFF);
  }
}


static void randxy(uint8_t *px, uint8_t *py) {
  uint8_t x = 255, y = 255;
  while (x > 160 || y > 100) {
    uint16_t r = fastrand();
    x = (r >> 8) & 0xFF;
    y = (r & 0x7F);
  }
  *px = x;
  *py = y;
}


static void draw_random_lines(void)
{
  memset(screen, 0, 160*100);
  uint8_t color = 0xFF;
  bool f = 0;

  while (frame_sync()) {
  uint8_t x0, y0;
  uint8_t x1, y1;
  randxy(&x0, &y0);
  randxy(&x1, &y1);
    draw_line(160, x0, y0, x1, y1, (f) ? color : 0);
    color--;
    f = !f;
  }
}



APP_MAIN(test_bitmap_modes)
{
  draw_1px_horiz_stripes(80,200);
  init_160x200x16_bitmap_mode(screen);
  video_start();
  struct keyreader kr = {0};

  while (frame_sync()) {
    switch (get_key_ascii(&kr)) {
      case KC_ESC:     draw_chars(40);  init_40x25_mono_text_mode(screen, NULL);     break;
      case 'q': draw_1px_horiz_stripes(80,200); init_160x200x16_bitmap_mode(screen); break;
      case 'w': draw_1px_vert_stripes(80,200);  init_160x200x16_bitmap_mode(screen); break;
      case 'e': draw_1px_checkerboard(80,200);  init_160x200x16_bitmap_mode(screen); break;

      case 't': draw_1px_horiz_stripes(80,100); init_160x100x16_bitmap_mode(screen); break;
      case 'y': draw_1px_vert_stripes(80,100);  init_160x100x16_bitmap_mode(screen); break;
      case 'u': draw_1px_checkerboard(80,100);  init_160x100x16_bitmap_mode(screen); break;
      case 'i': draw_1px_checkerboard(40,100);  init_80x100x16_bitmap_mode(screen); break;
      case 'o': draw_1px_checkerboard(40,50);   init_80x50x16_bitmap_mode(screen); break;

      case 'p': draw_1px_diagonals_mono(80,200); init_640x200_mono_bitmap_mode(screen); break;
      case '\b': draw_1px_diagonals_mono(40,200); init_320x200_mono_bitmap_mode(screen); break;

      case 'a': draw_1px_horiz_stripes_4color(40,200); init_160x200x4_bitmap_mode(screen); break;
      case 's': draw_1px_vert_stripes_4color(40,200);  init_160x200x4_bitmap_mode(screen); break;
      case 'd': draw_1px_checkerboard_4color(40,200);  init_160x200x4_bitmap_mode(screen); break;
      case 'f': draw_4px_checkerboard_4color(40,200);  init_160x200x4_bitmap_mode(screen); break;

      case 'g': draw_1px_horiz_stripes_4color(40,100); init_160x100x4_bitmap_mode(screen); break;
      case 'h': draw_1px_vert_stripes_4color(40,100);  init_160x100x4_bitmap_mode(screen); break;
      case 'j': draw_1px_checkerboard_4color(40,100);  init_160x100x4_bitmap_mode(screen); break;
      case 'k': draw_4px_checkerboard_4color(40,100);  init_160x100x4_bitmap_mode(screen); break;

      case 'z': draw_1px_checkerboard_256color(160,100); init_160x100x256_bitmap_mode(screen); break;
      case 'x': draw_rects_256color(160,100);            init_160x100x256_bitmap_mode(screen); break;
      case 'c': draw_lines_256color(160,100);            init_160x100x256_bitmap_mode(screen); break;
      case 'v': draw_1px_checkerboard_256color(80,100);  init_80x100x256_bitmap_mode(screen); break;
      case 'b': draw_rects_256color(80,100);             init_80x100x256_bitmap_mode(screen); break;
      case 'n': draw_lines_256color(80,100);             init_80x100x256_bitmap_mode(screen); break;
      case 'm': draw_1px_checkerboard_256color(80,50);  init_80x50x256_bitmap_mode(screen); break;
      case '\'': draw_rects_256color(80,50);             init_80x50x256_bitmap_mode(screen); break;
      case KC_UP: draw_lines_256color(80,50);             init_80x50x256_bitmap_mode(screen); break;
      case '/': init_160x200x256_bitmap_mode(screen); break;
      case ',': init_160x200x256_bitmap_mode(screen); draw_random_lines(); break;
      case '.': init_160x100x256_bitmap_mode(screen); draw_random_lines(); break;

      case '=': tilemap_hi++; break;
      case '-': tilemap_hi--; break;
      case '+': COLORBURST_ON(); break;
      case '_': COLORBURST_OFF(); break;
    }
  }
}
