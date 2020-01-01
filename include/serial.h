#ifndef SERIAL_H_
#define SERIAL_H_

#include "defs.h"
#include "minio.h"
#include <stdbool.h>
#include <stdio.h>

enum serial_baud_rate {
  SERIAL_BAUD_57600,  /* default */
  SERIAL_BAUD_300,
  SERIAL_BAUD_1200,
  SERIAL_BAUD_2400,
  SERIAL_BAUD_9600,
  SERIAL_BAUD_19200,
  SERIAL_BAUD_38400,
};
#define SERIAL_BAUD_NUM_VALUES 7

enum serial_parity {
  SERIAL_PARITY_NONE,
  SERIAL_PARITY_EVEN,
  SERIAL_PARITY_ODD,
};
#define SERIAL_PARITY_NUM_VALUES 3

enum serial_data_bits {
  SERIAL_DATA_BITS_8, /* default */
  SERIAL_DATA_BITS_7,
};
#define SERIAL_DATA_BITS_NUM_VALUES 2

enum serial_stop_bits {
  SERIAL_STOP_BITS_1,
  SERIAL_STOP_BITS_2,
};
#define SERIAL_STOP_BITS_NUM_VALUES 2

struct serial_port {
  enum serial_baud_rate baudrate:3;
  enum serial_parity    parity:2;
  enum serial_data_bits databits:1;
  enum serial_stop_bits stopbits:1;
};

struct serial_port serial_port_with_options(
  enum serial_baud_rate baudrate,
  enum serial_parity    parity,
  enum serial_data_bits databits,
  enum serial_stop_bits stopbits);

void serial_init(struct serial_port port);

/**
 * Enables asynchronous serial mode.
 * This causes bytes received over the USART to be written to the given buffer
 * without disturbing the video image. Characters can then be dequeued and
 * processed during vblank.
 * rxbuf must be a 256-byte-aligned pointer to a 256-byte array to use as the
 * receive buffer.
 */
void serial_async(page_aligned_ptr rxbuf);

/**
 * Enables asynchronous serial mode.
 * When a byte is received over the USART, the given callback is invoked.
 * The callback does NOT use the standard calling convention and therefore
 * must be declared with __attribute__((naked)).
 * - The function is free to clobber ZL, ZH and GPIOR0
 * - The received byte is in r2 and the function is free to clobber it
 * - Any other registers (including SREG) must be saved
 */
void serial_async_set_callback(void (*cb)(void));

/* Not usable in async-callback mode. */
bool serial_bytes_available(void);

/* Not usable in async-callback mode. */
uint8_t serial_get_byte(void);

void serial_send_char(char c);

void serial_send_str_P(const char *data);

void serial_send_fmt_P(const char *fmt, ...);

void serial_transmit_block(const uint8_t *data, uint16_t len);

char serial_receive_char(void);

uint16_t serial_receive_block(uint8_t *data, uint16_t maxlen);

void serial_send_break(void);

/* stdio-compatible putchar implementation. */
int serial_putchar(char c, FILE *stream);
/* minio-compatible putc implementation. */
void serial_mio_putc(char c);

/* stdio-compatible getchar implementation. Blocks until a character is */
/* received. Returns EOF if a break condition is received. */
int serial_getchar(FILE *stream);
/* minio-compatible getc implementation. If chk==0, block until a character */
/* is received, then return that character. Otherwise, return a flag */
/* indicating if a character is ready. */
int serial_mio_getc(char chk);

/* Equivalent of FDEV_SETUP_STREAM() that creates a stream attached to the
 * serial port. */
#define SERIAL_SETUP_STREAM_(p,g,f) { .put=p, .get=g, .flags=f }
#define SERIAL_SETUP_STREAM_RDWR()   SERIAL_SETUP_STREAM_(serial_putchar,serial_getchar,_FDEV_SETUP_RW)
#define SERIAL_SETUP_STREAM_WRONLY() SERIAL_SETUP_STREAM_(serial_putchar,NULL,_FDEV_SETUP_WRITE)
#define SERIAL_SETUP_STREAM_RDONLY() SERIAL_SETUP_STREAM_(NULL,serial_getchar,_FDEV_SETUP_READ)

/* minio equivalent of FDEV_SETUP_STREAM() that connects the input and output */
/* vectors to the serial port. */
#define SERIAL_MIO_SETUP()  ({ mio_putc=serial_mio_putc; mio_getc=serial_mio_getc; })
#define SERIAL_MIO_SETUP_WRONLY()  ({ mio_putc=serial_mio_putc; mio_getc=NULL; })
#define SERIAL_MIO_SETUP_RDONLY()  ({ mio_putc=NULL; mio_getc=serial_mio_getc; })

#endif
