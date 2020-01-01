/**
 * Simpler routines for numeric input/output for times when printf()/scanf()
 * are overkill.
 */
#ifndef NUMERIC_IO_H_
#define NUMERIC_IO_H_

#include <stdio.h>

/**
 * Write an 8-bit value as a 2-character hexadecimal number (with leading zero)
 * to the given stream. Nibble values of 10-15 are rendered with the uppercase
 * characters A-F.
 */
void fputhex8(uint8_t n, FILE *stream);

/**
 * Write an 8-bit value as a 2-character hexadecimal number (with leading zero)
 * to stdout. Nibble values of 10-15 are rendered with the uppercase characters
 * A-F.
 */
void puthex8(uint8_t n);

/**
 * Write a 16-bit value as a 4-character hexadecimal number (with leading zeros)
 * to the given stream. Nibble values of 10-15 are rendered with the uppercase
 * characters A-F.
 */
void fputhex16(uint16_t n, FILE *stream);

/**
 * Write a 16-bit value as a 4-character hexadecimal number (with leading zeros)
 * to stdout. Nibble values of 10-15 are rendered with the uppercase characters
 * A-F.
 */
void puthex16(uint16_t n);

#endif
