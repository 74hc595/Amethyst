/**
 * Much simpler (less-full-featured, non-POSIX-compliant) replacement for
 * avr-libc's stdio, which can be quite heavyweight with respect to RAM and
 * stack usage.
 */
#ifndef MINIO_H_
#define MINIO_H_

#include <avr/pgmspace.h>

/* put-character function */
typedef void (*mio_putc_fn)(char c);
/* get-character function. */
/* if chk == 0, return a flag indicating if a character is ready. */
/* otherwise, block until a character is ready. */
typedef int  (*mio_getc_fn)(char chk);

extern mio_putc_fn mio_putc;
extern mio_getc_fn mio_getc;


#define mio_hasc()  mio_getc(1)
#define mio_waitc() mio_getc(0)

void mio_puts(const char *s);
void mio_puts_P(const char * PROGMEM s);
void mio_putns(const char *s, uint16_t nbytes);
void mio_putns_P(const char * PROGMEM s, uint16_t nbytes);
void mio_putsnl(const char *s);
void mio_putsnl_P(const char * PROGMEM s);
void mio_nl(void);
void mio_bl(void);
void mio_x8(uint8_t n);
void mio_u8(uint8_t n);
void mio_d8(int8_t n);
void mio_x16(uint16_t n);
void mio_u16(uint16_t n);
void mio_d16(int16_t n);

#define Pc          mio_putc
#define Ps          mio_puts
#define Psl(litstr) mio_puts_P(PSTR(litstr))
#define Psp         mio_puts_P
#define PS          mio_putsnl
#define PSl(litstr) mio_putsnl_P(PSTR(litstr))
#define PSp         mio_putsnl_P
#define Pnl         mio_nl
#define Pbl         mio_bl
#define Px8         mio_x8
#define Pu8         mio_u8
#define Pd8         mio_d8
#define Px16        mio_x16
#define Pu16        mio_u16
#define Pd16        mio_d16
#define Pp(p)       mio_x16((uint16_t)(p))
#endif
