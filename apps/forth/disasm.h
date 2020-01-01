#ifndef DISASM_H_
#define DISASM_H_

#include "forth.h"

/**
 * Prints the disassembly of a single instruction to the output device.
 * Returns the address of the next instruction.
 */
typedef struct {
  cell location:15;
  cell suppress_address:1;
} __attribute__((packed)) disasm_options;

uint8_t *vm_disassemble(disasm_options opts);

uint16_t hexdump(uint16_t addr1, uint16_t addr2, bool as_words);

#endif
