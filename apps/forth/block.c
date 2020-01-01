#include "forth.h"
#include "forth_opcodes.h"
#include "spi.h"
#include "minio.h"
#include <avr/eeprom.h>

void load_block_range(char *dst, uint8_t blocknum, uint16_t offset, uint16_t nbytes)
{
  if (blocknum <= NUM_EEPROM_BLOCKS) {
    uint16_t src = ((uint16_t)blocknum-1)*BLOCK_SIZE;
    src += offset;
    eeprom_read_block(dst, (char*)src, nbytes);
  } else {
    uint16_t src = ((uint16_t)blocknum-NUM_EEPROM_BLOCKS-1)*BLOCK_SIZE;
    src += offset;
    spi_eeprom_read_block(dst, src, nbytes);
  }
}


void save_block_range(const char *src, uint8_t blocknum, uint16_t offset, uint16_t nbytes)
{
  if (blocknum <= NUM_EEPROM_BLOCKS) {
    uint16_t dst = ((uint16_t)blocknum-1)*BLOCK_SIZE;
    dst += offset;
    eeprom_update_block(src, (void*)dst, nbytes);
  } else {
    uint16_t dst = (((uint16_t)blocknum)-NUM_EEPROM_BLOCKS-1)*BLOCK_SIZE;
    dst += offset;
    spi_eeprom_write_block(src, dst, nbytes);
  }
}


static void fill_block(uint8_t blocknum, char c)
{
  uint16_t maxblk = num_blocks();
  if (blocknum == 0 || blocknum > maxblk) {
    forth_throw(FE_INVALID_BLOCK_NUMBER);
  } else if (blocknum <= NUM_EEPROM_BLOCKS) {
    uint32_t *dst = (uint32_t*)(((uint16_t)blocknum-1)*BLOCK_SIZE);
    union { char byte[4]; uint32_t dword; } val = { .byte = {c,c,c,c}};
    uint8_t n = BLOCK_SIZE/4;
    while (n--) { eeprom_update_dword(dst++, val.dword); }
  } else {
    uint16_t dst = (((uint16_t)blocknum)-NUM_EEPROM_BLOCKS-1)*BLOCK_SIZE;
    spi_eeprom_memset(dst, c, BLOCK_SIZE);
  }
}


static void copy_block(uint8_t dstblk, uint8_t srcblk)
{
  uint16_t maxblk = num_blocks();
  if ((srcblk == 0 || srcblk > maxblk) ||
      (dstblk == 0 || dstblk > maxblk)) {
    forth_throw(FE_INVALID_BLOCK_NUMBER);
  }
  /* use the pictured numeric output buffer as temporary space */
  uint16_t offset = 0;
  uint8_t n = BLOCK_SIZE/32;
  while (n--) {
    load_block_range(forth_hld0, srcblk, offset, 32);
    save_block_range(forth_hld0, dstblk, offset, 32);
    offset += 32;
  }
}


void fill_blocks(uint8_t startblk, uint8_t nblocks, char c) {
  while (nblocks--) { fill_block(startblk++, c); }
}


void copy_blocks(uint8_t dstblk, uint8_t srcblk, uint8_t nblocks) {
  while (nblocks--) { copy_block(dstblk++, srcblk++); }
}


uint8_t num_xmem_blocks(void) {
  return (spi_eeprom_present()) ? NUM_XMEM_BLOCKS : 0;
}


uint8_t num_blocks(void) {
  return NUM_EEPROM_BLOCKS + num_xmem_blocks();
}


void chain_load_next_block(void) {
  /* raise if current input source is not a block */
  if (!forth_inputsrc.isblock || forth_inputsrc.blk == 0) {
    forth_throw(FE_INVALID_BLOCK_NUMBER);
  }
  /* raise if next block isn't a valid block number */
  if (forth_inputsrc.blk+1 == 0 || forth_inputsrc.blk+1 > num_blocks()) {
    forth_throw(FE_INVALID_BLOCK_NUMBER);
  }
  /* discard remainder of current line */
  forth_inputpos = forth_inputlen;
  /* next block */
  forth_inputsrc.blk += 1;
  forth_inputsrc.blkline = 0;
}


/* runtime of --> word */
cell do_chain_load_next_block(void) {
  chain_load_next_block();
  /* no code for the VM to execute */
  forth_rom_word_buf[0] = OP_endword;
  return (cell)forth_rom_word_buf;
}


void forth_dot(cell n, cell width);
void list_block(uint8_t blocknum) {
  forth_scr = blocknum;
  if (blocknum == 0 || blocknum > num_blocks()) {
    forth_throw(FE_INVALID_BLOCK_NUMBER);
  }
  Pnl(); Psl("Block "); Pu8(blocknum); Pnl();
  uint16_t src = 0;
  for (uint8_t r = 0; r < LINES_PER_BLOCK; r++) {
    forth_dot(r, 2); Pbl();
    for (uint8_t c = 0; c < BLOCK_BYTES_PER_LINE; c++) {
      char ch = 0;
      load_block_range(&ch, blocknum, src++, 1);
      Pc(ch);
    }
    Pnl();
  }
}
