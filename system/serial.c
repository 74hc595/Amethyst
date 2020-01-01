#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

#include "serial.h"

/* see tinyprintf.c */
extern void vcbprintf(void *cb_arg, void(*put_cb)(void*,char), bool is_progmem, const char * restrict format, va_list ap);

extern uint8_t recvbuf_tail;
extern uint8_t recvbuf_head;
extern uint8_t recvbuf_hi;


struct serial_port serial_port_with_options(
  enum serial_baud_rate baudrate,
  enum serial_parity    parity,
  enum serial_data_bits databits,
  enum serial_stop_bits stopbits)
{
  struct serial_port out = {
    .baudrate = baudrate,
    .parity = parity,
    .databits = databits,
    .stopbits = stopbits
  };
  return out;
}


#define BAUD_TOL 1

static void serial_setbaud_300(void) {
  #undef BAUD
  #define BAUD 300
  #include <util/setbaud.h>
  UBRR1H = UBRRH_VALUE;
  UBRR1L = UBRRL_VALUE;
  UCSR1A = (USE_2X) ? _BV(U2X1) : 0;
}


static void serial_setbaud_1200(void) {
  #undef BAUD
  #define BAUD 1200
  #include <util/setbaud.h>
  UBRR1H = UBRRH_VALUE;
  UBRR1L = UBRRL_VALUE;
  UCSR1A = (USE_2X) ? _BV(U2X1) : 0;
}


static void serial_setbaud_2400(void) {
  #undef BAUD
  #define BAUD 2400
  #include <util/setbaud.h>
  UBRR1H = UBRRH_VALUE;
  UBRR1L = UBRRL_VALUE;
  UCSR1A = (USE_2X) ? _BV(U2X1) : 0;
}


static void serial_setbaud_9600(void) {
  #undef BAUD
  #define BAUD 9600
  #include <util/setbaud.h>
  UBRR1H = UBRRH_VALUE;
  UBRR1L = UBRRL_VALUE;
  UCSR1A = (USE_2X) ? _BV(U2X1) : 0;
}


static void serial_setbaud_19200(void) {
  #undef BAUD
  #define BAUD 19200
  #include <util/setbaud.h>
  UBRR1H = UBRRH_VALUE;
  UBRR1L = UBRRL_VALUE;
  UCSR1A = (USE_2X) ? _BV(U2X1) : 0;
}


static void serial_setbaud_38400(void) {
  #undef BAUD
  #define BAUD 38400
  #include <util/setbaud.h>
  UBRR1H = UBRRH_VALUE;
  UBRR1L = UBRRL_VALUE;
  UCSR1A = (USE_2X) ? _BV(U2X1) : 0;
}


static void serial_setbaud_57600(void) {
  #undef BAUD
  #define BAUD 57600
  #include <util/setbaud.h>
  UBRR1H = UBRRH_VALUE;
  UBRR1L = UBRRL_VALUE;
  UCSR1A = (USE_2X) ? _BV(U2X1) : 0;
}


void serial_init(struct serial_port port) {
  switch (port.baudrate) {
    case SERIAL_BAUD_300:   serial_setbaud_300(); break;
    case SERIAL_BAUD_1200:  serial_setbaud_1200(); break;
    case SERIAL_BAUD_2400:  serial_setbaud_2400(); break;
    case SERIAL_BAUD_9600:  serial_setbaud_9600(); break;
    case SERIAL_BAUD_19200: serial_setbaud_19200(); break;
    case SERIAL_BAUD_38400: serial_setbaud_38400(); break;
    case SERIAL_BAUD_57600: serial_setbaud_57600(); break;
  }
  uint8_t paritybits;
  switch (port.parity) {
    case SERIAL_PARITY_EVEN: paritybits = _BV(UPM11); break;
    case SERIAL_PARITY_ODD:  paritybits = _BV(UPM11) | _BV(UPM10); break;
    case SERIAL_PARITY_NONE:
    default:                 paritybits = 0; break;
  }
  uint8_t databits = _BV(UCSZ11) | ((port.databits == SERIAL_DATA_BITS_8) ? _BV(UCSZ10) : 0);
  uint8_t stopbits = ((port.stopbits == SERIAL_STOP_BITS_2) ? _BV(USBS0) : 0);
  UCSR1B = _BV(RXEN1) | _BV(TXEN1);
  UCSR1C = paritybits | databits | stopbits;

  /* TX goes low when USART is disabled. */
  /* Can be used for sending break conditions. */
  DDRD |= _BV(3);
  PORTD &= ~_BV(3);
}


void serial_async(page_aligned_ptr rxbuf)
{
  recvbuf_tail = 0;
  recvbuf_head = 0;
  recvbuf_hi = hi8(rxbuf);
  usart_async |= _BV(usart_async_bit);
}


void serial_async_set_callback(void (*cb)(void)) {
  recvbuf_hi = 0x80; /* high bit indicates head/tail is a function pointer */
  recvbuf_head = lo8(cb);
  recvbuf_tail = hi8(cb);
  usart_async |= _BV(usart_async_bit);
}



bool serial_bytes_available(void) {
  return recvbuf_head != recvbuf_tail;
}


void serial_send_char(char c) {
  loop_until_bit_is_set(UCSR1A, UDRE1);
  UDR1 = c;
}


void serial_send_str_P(const char *data) {
  char c;
  while ((c = pgm_read_byte(data))) {
    serial_send_char(c);
    data++;
  }
}

static void _serial_send_char_cb(void *x, char c) { (void)x; serial_send_char(c); }
void serial_send_fmt_P(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vcbprintf(NULL, _serial_send_char_cb, true, fmt, ap);
  va_end(ap);
}


void serial_transmit_block(const uint8_t *data, uint16_t len)
{
  while (len--) {
    loop_until_bit_is_set(UCSR1A, UDRE1);
    UDR1 = *data++;
  }
}
 

char serial_receive_char(void) {
  loop_until_bit_is_set(UCSR1A, RXC1);
  return UDR1;
}


uint16_t serial_receive_block(uint8_t *data, uint16_t maxlen)
{
  uint16_t bytes_received = 0;
  while (maxlen) {
    uint8_t serial_status;
    while (((serial_status = UCSR1A) & (_BV(RXC1)|_BV(FE1))) == 0) {}
    if (serial_status & _BV(FE1)) {
      break;
    } else if (serial_status & _BV(RXC1)) {
      *data++ = UDR1;
      bytes_received++;
      maxlen--;
    }
  }
  return bytes_received;
}


void serial_send_break(void)
{
  /* disable transmitter */
  UCSR1B = _BV(RXEN1);
  _delay_ms(10); //!!! TODO
  UCSR1B = _BV(RXEN1) | _BV(TXEN1);
}


int serial_putchar(char c, FILE *stream) {
  loop_until_bit_is_set(UCSR1A, UDRE1);
  UDR1 = c;
  return 0;
}


int serial_getchar(FILE *stream) {
  loop_until_bit_is_set(UCSR1A, RXC1);
  return UDR1;
}


void serial_mio_putc(char c) {
  loop_until_bit_is_set(UCSR1A, UDRE1);
  UDR1 = c;
}


int serial_mio_getc(char chk) {
  if (chk) {
    return bit_is_set(UCSR1A, RXC1);
  } else {
    loop_until_bit_is_set(UCSR1A, RXC1);
    return UDR1;
  }
}
