#ifndef SPI_H_
#define SPI_H_

#define SPI0_CS_PORT  PORTD
#define SPI0_CS_DDR   DDRD
#define SPI0_CS_PIN   0

#define SPI1_CS_PORT  PORTB
#define SPI1_CS_DDR   DDRB
#define SPI1_CS_PIN   0

#define SPI2_CS_PORT  PORTB
#define SPI2_CS_DDR   DDRB
#define SPI2_CS_PIN   2

#define SPI3_CS_PORT  PORTB
#define SPI3_CS_DDR   DDRB
#define SPI3_CS_PIN   3

/* SPI EEPROM (AT25xxx compatible devices) status register bits */
#define SPI_EEPROM_WPEN     7
#define SPI_EEPROM_IPL      6
#define SPI_EEPROM_nPRESENT 5
#define SPI_EEPROM_LIP      4
#define SPI_EEPROM_BP1      3
#define SPI_EEPROM_BP0      2
#define SPI_EEPROM_WEL      1
#define SPI_EEPROM_nRDY     0

/* SPI EEPROM (AT25xxx compatible devices) commands */
#define SPI_EEPROM_WREN     0b00000110  /* Enable Write Operations */
#define SPI_EEPROM_WRDI     0b00000100  /* Disable Write Operations */
#define SPI_EEPROM_RDSR     0b00000101  /* Read Status Register */
#define SPI_EEPROM_WRSR     0b00000001  /* Write Status Register */
#define SPI_EEPROM_READ     0b00000011  /* Read Data from Memory */
#define SPI_EEPROM_WRITE    0b00000010  /* Write Data to Memory */

#define SPI_EEPROM_PAGE_SIZE  128       /* max number of bytes that can be written at a time */
#define SPI_EEPROM_PAGE_MASK  ((SPI_EEPROM_PAGE_SIZE)-1)

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <stdbool.h>

void spi_init(void);

/**
 * SPI EEPROM (AT25xxx compatible devices) support on port 0
 */

uint8_t spi_eeprom_status(void);

bool spi_eeprom_present(void);

void spi_eeprom_set_status(uint8_t value);

uint8_t spi_eeprom_read_byte(uint16_t addr);

void spi_eeprom_write_byte(uint16_t addr, uint8_t value);

void spi_eeprom_read_block(void *dst, uint16_t src, uint16_t n);

void spi_eeprom_write_block(const void *src, uint16_t dst, uint16_t n);

void spi_eeprom_memset(uint16_t dst, char c, uint16_t n);


#endif /* !__ASSEMBLER__ */
#endif
