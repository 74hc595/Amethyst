#include "forth.h"
#include "minio.h"
#include "forth_opcodes.h"

#include <avr/pgmspace.h>
#include <alloca.h>
#include <ctype.h>


static cell interpret_literal(cell value)
{
  /* compile a literal instruction into the temporary word buffer */
  forth_rom_word_buf[0] = OP_lit16;
  forth_rom_word_buf[1] = value & 0xFF;
  forth_rom_word_buf[2] = (value >> 8) & 0xFF;
  forth_rom_word_buf[3] = OP_endword;
  /* return execution address to VM */
  return (cell)forth_rom_word_buf;
}


static cell interpret_double_literal(double_cell value)
{
  /* compile two literal instructions into the temporary word buffer */
  forth_rom_word_buf[0] = OP_lit16;
  forth_rom_word_buf[1] = value.low & 0xFF;
  forth_rom_word_buf[2] = (value.low >> 8) & 0xFF;
  forth_rom_word_buf[3] = OP_lit16;
  forth_rom_word_buf[4] = value.high & 0xFF;
  forth_rom_word_buf[5] = (value.high >> 8) & 0xFF;
  forth_rom_word_buf[6] = OP_endword;
  /* return execution address to VM */
  return (cell)forth_rom_word_buf;
}


void *ramdict_lfa_to_body(lfa lfa)
{
  uint8_t flags = ramdict_lfa_to_flags(lfa);
  cell *datafield = ramdict_lfa_to_dfa(lfa);
  switch (flags & FL_MASK) {
    /* Data field *is* the body */
    case FL_CONSTANT:
    case FL_2CONSTANT:
    case FL_COMPILER:
    case FL_COMPILER_CHILD:
    case FL_COMBINED:
      return datafield;
    /* Data field *points to* the body */
    case FL_VARIABLE:
    case FL_COLON:
    case FL_CHILD:
      return (void*)(*datafield);
    default:
      return 0;
  }
}


/* FIXME stupid hack for child words. */
cell ramdict_lfa_to_real_body(lfa lfa)
{
  uint8_t flags = ramdict_lfa_to_flags(lfa);
  uint8_t *datafield = ramdict_lfa_to_dfa(lfa);
  switch (flags & FL_MASK) {
    case FL_CHILD:
      return (cell)(datafield+3);
    case FL_COMPILER_CHILD:
      return *(cell*)(datafield+3);
    default:
      return (cell)ramdict_lfa_to_body(lfa);
  }
}


static bool ramdict_word_has_nondefault_compsem(lfa lfa)
{
  /* FL_COMBINED indicates word has non-default compilation semantics */
  return (ramdict_lfa_to_flags(lfa) == FL_COMBINED);
}


static cell ramdict_word_interpret_or_return_code_address(lfa lfa)
{
  uint8_t flags = ramdict_lfa_to_flags(lfa);
  cell *datafield = ramdict_lfa_to_dfa(lfa);
  switch (flags & FL_MASK) {
    case FL_CONSTANT:
    case FL_VARIABLE:
      /* Interpret the data field as a constant. */
      return interpret_literal(*datafield);
    case FL_2CONSTANT: {
      /* Interpret the data field as a double-cell constant. */
      double_cell value = { .high = datafield[0], .low = datafield[1] };
      return interpret_double_literal(value);
    }
    case FL_COLON:
    case FL_COMPILER:
    case FL_CHILD:
    case FL_COMPILER_CHILD:
      /* Execute the instructions in the body. */
      return (cell)ramdict_lfa_to_body(lfa);
    case FL_COMBINED: {
      /* Read the offset to the interpretation semantics */
      uint8_t isem_offset = *(uint8_t*)datafield;
      if (isem_offset == 0) {
        /* There might not be any */
        forth_throw(FE_INTEPRET_C_ONLY_WORD);
        __builtin_unreachable();
      } else {
        return (cell)(((uint8_t*)datafield)+isem_offset);
      }
    }
    default:
      return 0;
  }
}


extern cell romdict_compile_bytecode();
static bool romdict_word_has_nondefault_compsem(lfa lfa) {
  uint8_t *cfa = romdict_lfa_to_cfa(lfa);
  cell (*csem)() = pgm_read_ptr(cfa+2); /* compilation semantics */
  return (csem != romdict_compile_bytecode);
}


static cell romdict_word_interpret_or_return_code_address(lfa lfa) {
  uint8_t *cfa = romdict_lfa_to_cfa(lfa);
  cell (*fn)() = pgm_read_ptr(cfa);
  return fn(cfa+4);
}


cell execute_xt_or_return_code_address(execution_token xt)
{
  /* check for null xt */
  if (xt.cellvalue == 0) {
    forth_throw(FE_INTEPRET_C_ONLY_WORD);
  }

  if (!xt.islfa) {
    /* if not an LFA, just call function at that address */
    return xt.abs_addr;
  } else {
    /* it's an LFA--ram or rom? */
    if (!xt.isrom) {
      return ramdict_word_interpret_or_return_code_address((lfa)((cell)forth_np0 + xt.rel_offset));
    } else {
      return romdict_word_interpret_or_return_code_address((lfa)((uint8_t*)romdict + xt.rel_offset));
    }
  }
}


/* used by FIND */
/* returns 1 if xt has non-default compilation semantics, -1 otherwise */
int xt_immediate_flag(execution_token xt)
{
  bool has_ndcs = false;
  if (!xt.islfa) {
    has_ndcs = false;
  } else {
    if (!xt.isrom) {
      has_ndcs = ramdict_word_has_nondefault_compsem((lfa)((cell)forth_np0 + xt.rel_offset));
    } else {
      has_ndcs = romdict_word_has_nondefault_compsem((lfa)((uint8_t*)romdict + xt.rel_offset));
    }
  }
  return (has_ndcs) ? 1 : -1;
}


/* Interpretation semantics of TO. */
/* Returns 1 if a 2VALUE was updated. */
/* Returns 0 if a VALUE was updated. */
/* Throws an exception if the named word is not a VALUE/2VALUE. */
cell interpret_to(cell tos, cell sos)
{
  double_cell valueaddrs = find_value(forth_parse_name());
  *(cell*)(valueaddrs.high) = tos;
  if (valueaddrs.low) {
    *(cell*)(valueaddrs.low) = sos;
    return 1;
  }
  return 0;
}


/* this is DOCOL for words in the ROM dictionary */
cell romdict_interpret_bytecode(const char * PROGMEM arg)
{
  /* read count byte */
  uint8_t len = pgm_read_byte(arg);
  /* copy to RAM, skipping the length byte */
  memcpy_P(forth_rom_word_buf, arg+1, len);
  /* add an EXIT instruction */
  forth_rom_word_buf[len] = OP_endword;
  /* return execution address to VM */
  return (cell)forth_rom_word_buf;
}

cell romdict_interpret_bytecode_linkcall(const char * PROGMEM arg)
{
  /* read count byte */
  uint8_t len = pgm_read_byte(arg);
  /* prepend a LINK opcode to save the return address */
  forth_rom_word_buf[0] = OP_link;
  /* copy to RAM, skipping the length byte */
  memcpy_P(forth_rom_word_buf+1, arg+1, len);
  /* return with an UNLINK instruction that restores the return address */
  forth_rom_word_buf[len+1] = OP_unlink;
  /* return execution address to VM */
  return (cell)forth_rom_word_buf;
}


/* Execute compilation semantics for a word in the ROM dictionary. */
cell romdict_interpret_bytecode_compsem(const char * PROGMEM arg)
{
  /* read count byte, and skip over the interpretation semantics */
  uint8_t impsem_len = pgm_read_byte(arg);
  /* now interpret the rest */
  return romdict_interpret_bytecode(arg+impsem_len+1);
}


static forth_string strip_base_prefix(forth_string token)
{
  if (token.len == 0) { return token; }
  char first = token.addr[0];
  switch (first) {
    case '$': forth_base = 16;  goto deletechar;
    case '#': forth_base = 10;  goto deletechar;
    case '%': forth_base = 2;   goto deletechar;
    deletechar:
      return (forth_string){.addr=token.addr+1, .len=token.len-1};
  }
  return token;
}


static forth_string strip_sign_prefix(forth_string token, bool *hadprefix)
{
  if (token.len > 0 && token.addr[0] == '-') {
    *hadprefix = true;
    return (forth_string){.addr=token.addr+1, .len=token.len-1};
  } else {
    *hadprefix = false;
    return token;
  }
}


static forth_string strip_double_suffix(forth_string token, bool *hadsuffix)
{
  if (token.len > 0 && token.addr[token.len-1] == '.') {
    *hadsuffix = true;
    return (forth_string){.addr=token.addr, .len=token.len-1};
  } else {
    *hadsuffix = false;
    return token;
  }
}


static cell eval_literal(cell value) {
  return (in_interpret_mode()) ? interpret_literal(value) : compile_literal(value);
}


static cell eval_double_literal(double_cell value) {
  return (in_interpret_mode()) ? interpret_double_literal(value) : compile_double_literal(value);
}


/**
 * Valid character literals are:
 * 'X' - 3 characters, first and last characters are 0x27 (')
 * 'X  - 2 characters, first character is 0x27
 */
static bool is_char_literal(forth_string token, uint8_t *value) {
  if (token.len < 2 || token.len > 3) { return false; }
  if (token.len == 3 && token.addr[2] != '\'') { return false; }
  if (token.addr[0] != '\'') { return false; }
  *value = token.addr[1];
  return true;
}


cell eval_numeric(forth_string token)
{
  uint8_t n;
  if (is_char_literal(token, &n)) {
    return eval_literal(n);
  }

  /* save current base value */
  uint8_t prev_base = forth_base;
  bool neg, isdouble;
  /* numbers can't be empty strings! */
  if (token.len == 0) { goto invalid; }
  /* strip off (and handle) base prefix if there is one */
  token = strip_base_prefix(token);
  if (token.len == 0) { goto invalid; }
  /* strip off minus sign if there is one */
  token = strip_sign_prefix(token, &neg);
  if (token.len == 0) { goto invalid; }
  /* strip off double-length suffix if there is one */
  token = strip_double_suffix(token, &isdouble);
  /* now try to get the value. */
  /* we know that the input stream is at least one character */
  double_cell value = {0};
  forth_string result = forth_to_number(token, &value);
  /* were any characters left unconverted? */
  if (result.len == 0) {
    /* negate if necessary */
    if (neg) { value.full = -value.full; }
    /* if none, it's a valid number! */
    cell ret = (!isdouble) ? eval_literal(value.low) : eval_double_literal(value);
    forth_base = prev_base;
    return ret;
  }
  /* otherwise, error */
invalid:
  forth_base = prev_base;
  return 0;
}
