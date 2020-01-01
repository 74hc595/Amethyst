#if 0
/**
 * Very small and non-standards-compliant printf() implementation.
 *
 * The following format specifiers are accepted:
 * 
 * %x - 8-bit unsigned integer displayed as two hex digits; A-F are uppercase
 * %X - 16-bit unsigned integer displayed as four hex digits; A-F are uppercase
 * %p - same as %X but prefixed with '0x'
 * %u - 8-bit unsigned integer displayed in decimal without leading zeros
 * %U - 16-bit unsigned integer displayed in decimal without leading zeros
 * %d - 8-bit signed integer displayed in decimal without leading zeros
 * %D - 16-bit signed integer displayed in decimal without leading zeros
 * %c - single ASCII character
 * %s - null-terminated string in RAM
 * %S - null-terminated string in PROGMEM
 * %% - literal % character
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <avr/pgmspace.h>

/* see decimal.S */
extern void s8toa_dec(int8_t n, char *str);
extern void u8toa_dec(uint8_t n, char *str);
extern void s16toa_dec(int16_t n, char *str);
extern void u16toa_dec(uint16_t n, char *str);

/* (adapted from avr-libc's vfprintf.c) */
/* Read the byte at ptr and advance ptr. */
/* If is_progmem=1, the pointer is interpreted as a program space address. */
/* If is_progmem=0, the pointer is interpreted as a data space address. */
#define GETBYTE(is_progmem, ptr) ({ \
  unsigned char __c; \
  asm ( \
    "sbrc %2,0   \n\t" \
    "lpm  %0,Z+  \n\t" \
    "sbrs %2,0   \n\t" \
    "ld   %0,Z+" \
    : "=r" (__c), \
      "+z" (ptr) \
    : "r"  (is_progmem) \
  ); \
  __c; \
})


static void cbputhex8(void *cb_arg, void(*put_cb)(void*,char), uint8_t n) {
  uint8_t lonib = n & 0b00001111;
  uint8_t hinib = (n & 0b11110000) >> 4;
  lonib += (lonib <= 9) ? 0 : 7;
  hinib += (hinib <= 9) ? 0 : 7;
  put_cb(cb_arg, '0'+hinib);
  put_cb(cb_arg, '0'+lonib);
}


static void cbputhex16(void *cb_arg, void(*put_cb)(void*,char), uint16_t n) {
  cbputhex8(cb_arg, put_cb, n>>8);
  cbputhex8(cb_arg, put_cb, n&0xFF);
}


static inline void cbputs(void *cb_arg, void(*put_cb)(void*,char), const char *str) {
  char c; while ((c = *str++)) { put_cb(cb_arg, c); }
}


static inline void cbputs_P(void *cb_arg, void(*put_cb)(void*,char), const char *str) {
  char c; while ((c = pgm_read_byte(str++))) { put_cb(cb_arg, c); }
}


/* A simple printf that invokes a used-defined `put_cb` callback to print a */
/* character. `cb_arg` is an opaque pointer passed as the first argument to */
/* the callback. */
void vcbprintf(void *cb_arg, void(*put_cb)(void*,char), bool is_progmem, const char * restrict format, va_list ap)
{
  char c;
  char buf[7];
  while ((c = GETBYTE(is_progmem, format))) {
    if (c == '%') {
      c = GETBYTE(is_progmem, format);
      switch (c) {
        case 0:   break;
        case '%': put_cb(cb_arg, '%'); break;
        case 'c': put_cb(cb_arg, va_arg(ap, int)); break;
        case 's': cbputs(cb_arg, put_cb, va_arg(ap, const char *)); break;
        case 'S': cbputs_P(cb_arg, put_cb, va_arg(ap, PGM_P)); break;
        case 'x': cbputhex8(cb_arg, put_cb, va_arg(ap, int)); break;
        case 'p': put_cb(cb_arg, '0'); put_cb(cb_arg, 'x'); __attribute__((fallthrough));
        case 'X': cbputhex16(cb_arg, put_cb, va_arg(ap, int)); break;
        case 'u': u8toa_dec(va_arg(ap, int), buf); goto prbuf;
        case 'd': s8toa_dec(va_arg(ap, int), buf); goto prbuf;
        case 'U': u16toa_dec(va_arg(ap, int), buf); goto prbuf; /* fallthrough */
        case 'D': s16toa_dec(va_arg(ap, int), buf);
        prbuf:    cbputs(cb_arg, put_cb, buf); break;
        default:  put_cb(cb_arg, c); break;
      }
    } else {
      put_cb(cb_arg, c);
    }
  }
}


static void _putc_wrapper(void *stream, char c) {
  putc(c, (FILE *)stream);
}


/* `vfprintf()` implemented in terms of `vcbprintf()`. */
int __wrap_vfprintf(FILE * restrict stream, const char * restrict format, va_list ap)
{
  stream->len = 0;
  vcbprintf(stream, _putc_wrapper, (stream->flags & __SPGM), format, ap);
  return stream->len;
}


void fputhex8(uint8_t n, FILE *stream)    { cbputhex8(stream, _putc_wrapper, n); }
void puthex8(uint8_t n)                   { fputhex8(n, stdout); }
void fputhex16(uint16_t n, FILE *stream)  { cbputhex16(stream, _putc_wrapper, n); }
void puthex16(uint16_t n)                 { fputhex16(n, stdout); }
#endif