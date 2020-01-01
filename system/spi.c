#include "spi.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#define CS_INIT(p)     ({ p##_PORT |= _BV(p##_PIN); p##_DDR |= _BV(p##_PIN); })
#define CS_ASSERT(p)   ({ p##_PORT &= ~(_BV(p##_PIN)); })
#define CS_DEASSERT(p) ({ p##_PORT |=   _BV(p##_PIN); })

#define SPI_TRANSMIT(b) ({ SPDR = b; while (!(SPSR & _BV(SPIF))){}; uint8_t r = SPDR; r; })
#define SPI_RECEIVE()   SPI_TRANSMIT(0)



void spi_init(void)
{
  /* initialize MOSI/SCK */
  DDRB |= _BV(5)|_BV(7);
  /* initialize MISO, enable pullup resistor */
  DDRB &= ~_BV(6);
  PORTB |= _BV(6);
  /* initialize chip select pins */
  CS_INIT(SPI0_CS);
  CS_INIT(SPI1_CS);
  CS_INIT(SPI2_CS);
  CS_INIT(SPI3_CS);
  /* initialize SPI master at full speed (Fosc/2) */
  SPCR = _BV(SPE) | _BV(MSTR);
  SPSR = _BV(SPI2X);
}



#define SPI_EEPROM_CS_ASSERT()    CS_ASSERT(SPI0_CS)
#define SPI_EEPROM_CS_DEASSERT()  CS_DEASSERT(SPI0_CS)

static uint8_t _spi_eeprom_status(void)
{
  SPI_EEPROM_CS_ASSERT();
  SPI_TRANSMIT(SPI_EEPROM_RDSR); /* send "Read Status Register" command */
  uint8_t status = SPI_RECEIVE();
  SPI_EEPROM_CS_DEASSERT();
  return status;
}


static void _spi_eeprom_write_enable(void)
{
  SPI_EEPROM_CS_ASSERT();
  SPI_TRANSMIT(SPI_EEPROM_WREN);
  SPI_EEPROM_CS_DEASSERT();
}


uint8_t spi_eeprom_status(void)
{
  // cli();
  uint8_t status = _spi_eeprom_status();
  // sei();
  return status;
}


bool spi_eeprom_present(void)
{
  // cli();
  uint8_t status = _spi_eeprom_status();
  // sei();
  return bit_is_clear(status, SPI_EEPROM_nPRESENT);
}


void spi_eeprom_set_status(uint8_t value)
{
  // cli();
  /* need to enable write access to modify status register */
  _spi_eeprom_write_enable();
  SPI_EEPROM_CS_ASSERT();
  SPI_TRANSMIT(SPI_EEPROM_WRSR);
  SPI_TRANSMIT(value);
  SPI_EEPROM_CS_DEASSERT();
  // sei();
}


uint8_t spi_eeprom_read_byte(uint16_t addr)
{
  uint8_t value = 0;
  // cli();
  /* wait until the device is ready */
  uint8_t status = _spi_eeprom_status(); /* read status register */
  if (bit_is_set(status, SPI_EEPROM_nPRESENT)) { goto done; } /* if bit 5 of status register is not 0, there's no device present */
  while (bit_is_set(status, SPI_EEPROM_nRDY)) {
    status = _spi_eeprom_status();
  }

  SPI_EEPROM_CS_ASSERT();
  SPI_TRANSMIT(SPI_EEPROM_READ); /* send command */
  SPI_TRANSMIT((addr >> 8) & 0xFF); /* send MSB of address */
  SPI_TRANSMIT(addr & 0xFF); /* send LSB of address */
  value = SPI_RECEIVE();
  SPI_EEPROM_CS_DEASSERT();

done:
  // sei();
  return value;
}


void spi_eeprom_write_byte(uint16_t addr, uint8_t value)
{
  // cli();
  /* wait until the device is ready */
  uint8_t status = _spi_eeprom_status(); /* read status register */
  if (bit_is_set(status, SPI_EEPROM_nPRESENT)) { goto done; } /* if bit 5 of status register is not 0, there's no device present */
  while (bit_is_set(status, SPI_EEPROM_nRDY)) {
    status = _spi_eeprom_status();
  }
  /* enable write operations if necessary */
  if (bit_is_clear(status, SPI_EEPROM_WEL)) {
    _spi_eeprom_write_enable();
  }
  /* send write command */
  SPI_EEPROM_CS_ASSERT();
  SPI_TRANSMIT(SPI_EEPROM_WRITE); /* send "Write Data to Memory" command */
  SPI_TRANSMIT((addr >> 8) & 0xFF); /* send MSB of address */
  SPI_TRANSMIT(addr & 0xFF); /* send LSB of address */
  SPI_TRANSMIT(value); /* send byte value */
  SPI_EEPROM_CS_DEASSERT();
done:
  // sei();
  return;
}


void spi_eeprom_read_block(void *dstp, uint16_t src, uint16_t n)
{
  // cli();
  char *dst = (char*)dstp;
  /* wait until the device is ready */
  uint8_t status = _spi_eeprom_status(); /* read status register */
  if (bit_is_set(status, SPI_EEPROM_nPRESENT)) { goto done; } /* if bit 5 of status register is not 0, there's no device present */
  while (bit_is_set(status, SPI_EEPROM_nRDY)) {
    status = _spi_eeprom_status();
  }

  SPI_EEPROM_CS_ASSERT();
  SPI_TRANSMIT(SPI_EEPROM_READ); /* send command */
  SPI_TRANSMIT((src >> 8) & 0xFF); /* send MSB of address */
  SPI_TRANSMIT(src & 0xFF); /* send LSB of address */
  while (n--) {
    *dst++ = SPI_RECEIVE();
  }
  SPI_EEPROM_CS_DEASSERT();
done:
  // sei();
  return;
}


static uint16_t _spi_eeprom_write_page(const uint8_t **srcp, uint16_t *dstp, uint16_t n)
{
  uint16_t dst = *dstp;
  const uint8_t *src = *srcp;
  /* need to enable write operations before each write */
  _spi_eeprom_write_enable();
  SPI_EEPROM_CS_ASSERT();
  SPI_TRANSMIT(SPI_EEPROM_WRITE); /* send command */
  SPI_TRANSMIT((dst >> 8) & 0xFF); /* send MSB of address */
  SPI_TRANSMIT(dst & 0xFF); /* send LSB of address */
  /* write until we reach the end of this page */
  do {
    char c = *src;
    src++;
    SPI_TRANSMIT(c); /* send byte and advance src */
    dst++; /* advance dest */
    n--;
  } while ((n) && (dst & SPI_EEPROM_PAGE_MASK)); /* terminate after page rollover */
  SPI_EEPROM_CS_DEASSERT();
  /* write back new src/dst positions */
  *srcp = src;
  *dstp = dst;
  return n;
}


static uint16_t _spi_eeprom_memset_page(uint16_t *dstp, char c, uint16_t n)
{
  uint16_t dst = *dstp;
  /* need to enable write operations before each write */
  _spi_eeprom_write_enable();
  SPI_EEPROM_CS_ASSERT();
  SPI_TRANSMIT(SPI_EEPROM_WRITE); /* send command */
  SPI_TRANSMIT((dst >> 8) & 0xFF); /* send MSB of address */
  SPI_TRANSMIT(dst & 0xFF); /* send LSB of address */
  /* write until we reach the end of this page */
  do {
    SPI_TRANSMIT(c); /* send byte and advance src */
    dst++; /* advance dest */
    n--;
  } while ((n) && (dst & SPI_EEPROM_PAGE_MASK)); /* terminate after page rollover */
  SPI_EEPROM_CS_DEASSERT();
  /* write back new src/dst positions */
  *dstp = dst;
  return n;
}


void spi_eeprom_write_block(const void *srcp, uint16_t dst, uint16_t n)
{
  // cli();
  const uint8_t *src = srcp;
  while (n) {
    uint8_t status = _spi_eeprom_status(); /* read status register */
    if (bit_is_set(status, SPI_EEPROM_nPRESENT)) { goto done; } /* if bit 5 of status register is not 0, there's no device present */
    while (bit_is_set(status, SPI_EEPROM_nRDY)) {
      status = _spi_eeprom_status();
    }
    n = _spi_eeprom_write_page(&src, &dst, n);
  }
done:
  // sei();
  return;
}


void spi_eeprom_memset(uint16_t dst, char c, uint16_t n)
{
  // cli();
  while (n) {
    uint8_t status = _spi_eeprom_status(); /* read status register */
    if (bit_is_set(status, SPI_EEPROM_nPRESENT)) { goto done; } /* if bit 5 of status register is not 0, there's no device present */
    while (bit_is_set(status, SPI_EEPROM_nRDY)) {
      status = _spi_eeprom_status();
    }
    n = _spi_eeprom_memset_page(&dst, c, n);
  }
done:
  // sei();
  return;
}
