#include "minio.h"

void mio_nl(void) {
  mio_putc('\n');
}


void mio_bl(void) {
  mio_putc(' ');
}


void mio_puts(const char *s) {
  char c;
  while ((c = *s++)) { mio_putc(c); }
}


void mio_puts_P(const char * PROGMEM s) {
  char c;
  while ((c = pgm_read_byte(s++))) { mio_putc(c); }
}


void mio_putsnl(const char *s) {
  mio_puts(s);
  mio_nl();
}


void mio_putsnl_P(const char * PROGMEM s) {
  mio_puts_P(s);
  mio_nl();
}


void mio_putns(const char *s, uint16_t nbytes) {
  while (nbytes--) { mio_putc(*s++); }
}

void mio_putns_P(const char *s, uint16_t nbytes) {
  while (nbytes--) { mio_putc(pgm_read_byte(s++)); }
}


void mio_x8(uint8_t n) {
  uint8_t lonib = n & 0b00001111;
  uint8_t hinib = (n & 0b11110000) >> 4;
  lonib += (lonib <= 9) ? 0 : 7;
  hinib += (hinib <= 9) ? 0 : 7;
  mio_putc('0'+hinib);
  mio_putc('0'+lonib);
}


void mio_x16(uint16_t n) {
  mio_x8(n>>8);
  mio_x8(n&0xFF);
}
