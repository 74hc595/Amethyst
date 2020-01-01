#include "forth.h"
#include "disasm.h"
#include "forth_opcodes.h"
#include "minio.h"

#include <avr/pgmspace.h>

#undef X
#define X(opnum,str,props,asmid,enumid,v) static __attribute__((unused)) const char opcode_name_ ## enumid[] PROGMEM = str;
OPCODES

#undef X
#define X(opnum,str,props,asmid,enumid,v) static __attribute__((unused)) const char ext_opcode_name_ ## enumid[] PROGMEM = str;
EXT_OPCODES

#undef X
#define X(opnum,str,props,asmid,enumid,v) opcode_name_ ## asmid,
static const char * const opcode_names[] PROGMEM = {
  OPCODES
};

#undef X
#define X(opnum,str,props,asmid,enumid,v) ext_opcode_name_ ## asmid,
static const char * const ext_opcode_names[] PROGMEM = {
  EXT_OPCODES
};

#undef X
#define X(opnum,str,props,asmid,enumid,v) OP_TYPE_TO_FLAGS(OP_##props),
static const uint8_t opcode_properties[] PROGMEM = {
  OPCODES
};


static uint8_t *vm_disassemble_helper(uint8_t *location, const char *opname, uint8_t opflags)
{
  uint8_t opbyte1 = location[0];
  uint8_t opbyte2 = location[1];
  uint8_t opbyte3 = location[2];
  switch (OP_FLAGS_TYPE(opflags)) {
    case OP_2U:
      Pbl(); Px8(opbyte2); Psl("     "); Psp(opname); Pbl(); Pu8(opbyte2); Pnl();
      return location+2;
    case OP_2X:
      Pbl(); Px8(opbyte2); Psl("     "); Psp(opname); Pbl(); Pu8(opbyte2); Pnl();
      return location+2;
    case OP_2I:
      Pbl(); Px8(opbyte2); Psl("     "); Psp(pgm_read_ptr(ext_opcode_names+opbyte2)); Pnl();
      return location+2;
    case OP_2F:
      Pbl(); Px8(opbyte2); Psl("     "); Psp(opname); Pbl(); Pu8(opbyte2); Psl(" [");
      Px16(((cell)location)+2+opbyte2); Psl("]\n");
      return location+2;
    case OP_2B:
      Pbl(); Px8(opbyte2); Psl("     "); Psp(opname); Pbl(); Pu8(opbyte2); Psl(" [");
      Px16(((cell)location)-opbyte2); Psl("]\n");
      return location+2;
    case OP_2C:
      Pbl(); Px8(opbyte2); Psl("     "); Psp(opname); Pbl(); Px8(opbyte1); Px8(opbyte2); Pnl();
      return location+2;
    case OP_3S:
      Pbl(); Px8(opbyte2); Pbl(); Px8(opbyte3); Psl("  "); Psp(opname); Pbl(); Pd16(((opbyte3<<8)|opbyte2)); Pnl();
      return location+3;
    case OP_3X:
      Pbl(); Px8(opbyte2); Pbl(); Px8(opbyte3); Psl("  "); Psp(opname); Pbl(); Px16(((opbyte3<<8)|opbyte2)); Pnl();
      return location+3;
    case OP_STR:
      Pbl(); Px8(opbyte2); Psl("     "); Psp(opname); Psl(" [");
      mio_putns((char *)(location+2), opbyte2);
      Psl("]\n");
      return location+opbyte2+2;
    default:
      Psl("        "); Psp(opname); Pnl();
      return location+1;
  }
}


uint8_t *vm_disassemble(disasm_options opts)
{
  uint8_t *location = (uint8_t*)((cell)opts.location);
  uint8_t opc = location[0];
  uint8_t opflags = pgm_read_byte(opcode_properties+opc);
  const char *opname = pgm_read_ptr(opcode_names+opc);
  if (!opts.suppress_address) {
    Pp(location); Psl(":  ");
  } else {
    Psl("       ");
  }
  Px8(opc);
  return vm_disassemble_helper(location, opname, opflags);
}


static inline uint8_t mem_read(uint16_t addr) {
  return *((uint8_t *)addr);
}

static inline uint16_t mem_read_word(uint16_t addr) {
  return *((uint16_t *)addr);
}

bool using_console(void);
static uint8_t termwidth(void) {
  return (using_console()) ? console_cols(&fcon) : 80;
}

/* how many bytes should hexdump display on one line? */
static uint8_t hexdump_nbytes(void) {
  return (termwidth() >= 80) ? 16 : 8;
}

/* hexdump bytes between addr1 and addr2 inclusive */
uint16_t hexdump(uint16_t addr1, uint16_t addr2, bool as_words)
{
  uint8_t cols = hexdump_nbytes();
  if (as_words) { cols /= 2; }
  while (1) {
    Pbl(); Px16(addr1); Pc('-');
    for (uint8_t i = 0; i < cols; i++) {
      Pbl();
      uint16_t prevaddr = addr1;
      if (as_words) {
        Px16(mem_read_word(addr1)); addr1 += 2;
      } else {
        Px8(mem_read(addr1)); addr1++;
      }
      /* stop after printing value at last address, */
      /* or if we wrap around FFFF->0000 */
      if (addr1 <= prevaddr || addr1 > addr2) {
        Pnl();
        goto done;
      }
    }
    Pnl();
  }
done:
  return addr1;
}
