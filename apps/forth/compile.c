#include "forth.h"
#include "minio.h"
#include "forth_opcodes.h"
#include <string.h>
#include <stdlib.h>

static cell ramdict_word_compile_or_return_code_address(lfa lfa);
static cell romdict_word_compile_or_return_code_address(lfa lfa);

static bool is_in_code_space(cell addr) {
  return (addr < (cell)forth_np0);
}

static bool compiling_into_code_space(void) {
  /* If the dictionary is empty, compile into code space by default. */
  if (forth_latest == forth_np) { return true; }
  /* Otherwise, if in compile mode, return true if the current word's body */
  /* is located in code space. */
  uint8_t flags = ramdict_lfa_to_flags((const struct dict_entry *)forth_latest);
  return !FL_IS_INLINE(flags);
}

forth_result allot(uint16_t nbytes) {
  return (compiling_into_code_space())
    ? codespace_allot(nbytes) : namespace_allot(nbytes);
}

void ccomma(uint8_t num) {
  return (compiling_into_code_space())
    ? codespace_ccomma(num) : namespace_ccomma(num);
}


void comma(uint16_t num) {
  return (compiling_into_code_space())
    ? codespace_comma(num) : namespace_comma(num);
}


void *here(void) {
  return (compiling_into_code_space()) ? forth_cp : forth_np;
}


cell compile_literal(cell num)
{
  /* special cases for 0 and -1 */
  if (num == 0) {
    ccomma(OP_zero);
  } else if (num == 0xFFFF) {
    ccomma(OP_minus1);
  }
  /* one byte? */
  else if ((num>>8) == 0) {
    comma(OP_lit8|(num<<8));
  }
  /* otherwise, two bytes */
  else {
    ccomma(OP_lit16);
    comma(num);
  };
  /* no code for the VM to execute */
  forth_rom_word_buf[0] = OP_endword;
  return (cell)forth_rom_word_buf;
}


cell compile_double_literal(double_cell num) {
  compile_literal(num.low);
  return compile_literal(num.high);
}


static void compile_absolute_call(cell addr) {
  /* a (CALL) instruction is just a byteswapped address */
  uint8_t new_msb = (addr & 0xFF);
  uint8_t new_lsb = (addr >> 8) & 0xFF;
  addr = (new_msb<<8) | new_lsb;
  comma(addr);
}


static void compile_np0_relative_call(cell addr) {
  cell rel_addr = addr - (cell)forth_np0;
  ccomma(OP_np0relcall);
  comma(rel_addr);
}


void compile_string(forth_string str)
{
  if (str.len > 255) {
    forth_throw(FE_PARSED_STR_OVERFLOW);
  }
  uint8_t nbytes = str.len & 0xFF;
  comma(OP_litstr|(nbytes<<8));

  forth_result strspace = allot(nbytes);
  forth_throw_if(strspace.err);
  memcpy(strspace.ptr, str.addr, str.len);
}


void compile_abortq(forth_string str) {
  compile_string(str);
  comma(OP_ext|(OPX_abortq<<8));
}


void compile_cquote(forth_string str) {
  compile_string(str);
  compile_literal((cell)forth_hld0);
  comma(OP_ext|(OPX_place<<8));
}


void compile_dotquote(forth_string str) {
  compile_string(str);
  ccomma(OP_type);
}


void compile_xt_compsem(execution_token xt) {
  compile_literal(xt.cellvalue);
  ccomma(OP_compile);
}


void compile_action_of(execution_token xt) {
  /* compile the xt as a literal, followed by >BODY @ */
  compile_literal(xt.cellvalue);
  comma(OP_ext|(OPX_tobody<<8));
  ccomma(OP_fetch);
}


/* Used by the compilation semantics of a word created with MARKER. */
void compile_marker(cell *bodyaddr)
{
  /* compile each word as a literal */
  compile_literal(bodyaddr[3]); /* latest */
  compile_literal(bodyaddr[2]); /* cp */
  compile_literal(bodyaddr[1]); /* np */
  compile_literal(bodyaddr[0]); /* np0 */
  /* compile a MARKER> instruction */
  comma(OP_ext|(OPX_markerfrom<<8));
}


static cell compile_call(cell dest)
{
  bool callee_space_is_code = is_in_code_space(dest);
  bool caller_space_is_code = compiling_into_code_space();
  /* Calling a call into code space? Compile a (CALL). */
  if (callee_space_is_code) {
    compile_absolute_call(dest);
  }
  /* Compiling a call from name space into name space? Compile an np0-relative call. */
  else if (!caller_space_is_code) {
    compile_np0_relative_call(dest);
  }
  /* Compiling a call from code space into name space? No bueno. */
  else {
    forth_throw(FE_INVALID_POSTPONE);
  }
  /* no code for the VM to execute */
  forth_rom_word_buf[0] = OP_endword;
  return (cell)forth_rom_word_buf;
}


static cell romdict_word_compile_or_return_code_address(lfa lfa) {
  uint8_t *cfa = romdict_lfa_to_cfa(lfa);
  cell (*fn)() = pgm_read_ptr(cfa+2);
  return fn(cfa+4);
}


cell compile_recursive_call(void) {
  return ramdict_word_compile_or_return_code_address((lfa)forth_latest);
}


cell compile_ct_or_return_code_address(compilation_token ct)
{
  if (!ct.isvalid) {
    return 0;
  }
  cell offset = ct.offset;
  if (ct.isrom) {
    return romdict_word_compile_or_return_code_address((lfa)((cell)romdict + offset));
  } else {
    return ramdict_word_compile_or_return_code_address((lfa)((cell)forth_np0 + offset));
  }
}


/* Converts an execution token to its compilation token. */
/* Returns NULL_CT if the word represented by xt has default compilation semantics. */
static compilation_token xt_to_ct(execution_token xt) {

  if (xt.islfa) {
    /* Combined ramdict words have special compilation semantics. */
    if (!xt.isrom) {
      lfa word = (lfa)(xt.rel_offset + (cell)forth_np0);
      uint8_t flags = ramdict_lfa_to_flags(word);
      if (flags == FL_COMBINED) {
        return ramdict_lfa_to_ct(word);
      } else {
        return NULL_CT;
      }
    } else {
      /* Romdict words always have special compilation semantics. */
      lfa word = (lfa)(xt.rel_offset + (cell)romdict);
      return romdict_lfa_to_ct(word);
    }
  } else {
    /* Regular colon words always have default compilation semantics. */
    return NULL_CT;
  }
}



cell compile_xt_or_return_code_address(execution_token xt)
{
  if (!xt.islfa) {
    return compile_call(xt.abs_addr);
  } else {
    return compile_ct_or_return_code_address(xt_to_ct(xt));
  }
  // if (!ct.isvalid) {
  //   return 0;
  // }
  // cell offset = ct.offset;
  // if (ct.isrom) {
  //   return romdict_word_compile_or_return_code_address((lfa)((cell)romdict + offset));
  // } else {
  //   return ramdict_word_compile_or_return_code_address((lfa)((cell)forth_np0 + offset));
  // }
}


static cell compile_xt_wrapper(execution_token xt, cell bodyaddr)
{
  forth_rom_word_buf[0] = OP_lit16;
  forth_rom_word_buf[1] = bodyaddr & 0xFF;
  forth_rom_word_buf[2] = (bodyaddr>>8) & 0xFF;
  forth_rom_word_buf[3] = OP_lit16;
  forth_rom_word_buf[4] = xt.cellvalue & 0xFF;
  forth_rom_word_buf[5] = (xt.cellvalue>>8) & 0xFF;
  forth_rom_word_buf[6] = OP_compile;
  forth_rom_word_buf[7] = OP_endword;
  return (cell)forth_rom_word_buf;
}


static cell ramdict_word_compile_or_return_code_address(lfa lfa)
{
  uint8_t flags = ramdict_lfa_to_flags(lfa);
  cell *datafield = ramdict_lfa_to_dfa(lfa);
  switch (flags & FL_MASK) {
    case FL_CONSTANT:
    case FL_VARIABLE:
      return compile_literal(*datafield);
    case FL_2CONSTANT: {
      /* Interpret the data field as a double-cell constant, and compile it. */
      double_cell value = { .high = datafield[0], .low = datafield[1] };
      return compile_double_literal(value);
    }
    case FL_COMPILER_CHILD: {
      /* Does the parent have non-default compilation semantics? */
      uint8_t *dfa = ramdict_lfa_to_dfa(lfa);
      execution_token parent_xt = *(execution_token *)(dfa+1);
      /* Return a snippet of bytecode that will push the child's body address */
      /* and invoke the parent's compilation semantics. */
      return compile_xt_wrapper(parent_xt, (cell)(dfa+3));
      // compilation_token nondefault_ct = xt_to_ct(parent_xt);
      // /* If parent compilation semantics are non-default, perform them */
      // if (IS_VALID_CT(nondefault_ct)) {
      //   /* Return a snippet of bytecode that will push the child's body address */
      //   /* and invoke the parent's compilation token. */
      //   return ct_wrapper(nondefault_ct, (cell)(dfa+3));
      // }
      // /* Otherwise, perform default compilation semantics */
      // __attribute__((fallthrough));
    }
    case FL_COMPILER:
      /* Compiler words are "pseudo-immediate": */
      /* When in compile mode and the current definition is a compiler word, */
      /* perform the word's compilation semantics. Otherwise, execute it as */
      /* if it were immediate. */
      if (compiling_into_code_space()) {
        goto execute_body;
      }
      __attribute__((fallthrough));
    case FL_COLON:
    case FL_CHILD:
      /* Compile the data field's value into a CALL instruction. */
      return compile_call((cell)ramdict_lfa_to_body(lfa));
    execute_body:
      return (cell)ramdict_lfa_to_body(lfa);
    case FL_COMBINED:
      /* Compilation semantics begin one byte after the data field. */
      return ((cell)datafield+1);
  }
  return 0;
}


/* Compilation semantics of TO. */
void compile_to(void)
{
  double_cell valueaddrs = find_value(forth_parse_name());
  if (valueaddrs.low) {
    /* Compile the value's address and a 2! instruction. */
    compile_literal(valueaddrs.high);
    ccomma(OP_twostore);
  } else {
    /* VALUE. Compile a store-to-literal instruction. */
    ccomma(OP_storelit);
    comma(valueaddrs.high);
  }
}


/* this is DOCOL, for words in the ROM dictionary */
cell romdict_compile_bytecode(const char * PROGMEM arg)
{
  /* read count byte */
  uint8_t len = pgm_read_byte(arg);
  /* request space in dictionary and advance cp */
  forth_result dst = allot(len);
  forth_throw_if(dst.err); /* abort if insufficient space */
  /* copy contents into newly allotted space */
  memcpy_P(dst.ptr, arg+1, len);
  /* no code for the VM to execute */
  forth_rom_word_buf[0] = OP_endword;
  return (cell)forth_rom_word_buf;
}


void do_semicolon(void)
{
  unsmudge();
  ccomma(OP_endword); /* compile the EXIT instruction */
  /* TODO do tail call optimization here? */
  /* exit compile mode */
  forth_flags.state = FORTH_STATE_INTERPRETING;
}


static void compile_does(uint8_t parentflags)
{
  /* Allocate space for the code that ends this definition. */
  /* We need 4 bytes: */
  /* - 3 for the (DOES>) instruction including the parent's nt */
  /* - 1 for (EXIT) */
  forth_result result = allot(4);
  forth_throw_if(result.err);
  uint8_t *epilogue = result.byteptr;
  epilogue[0] = OP_does;
  epilogue[3] = OP_endword;
  unsmudge(); /* make the current definition visible */
  /* now create a new anonymous definition for the parent semantics */
  namespace_create_nonamecheck((forth_string){0}, parentflags);
  /* get the nt for the word we just created */
  name_token parent_nt = ramdict_lfa_to_nt((lfa)forth_latest);
  epilogue[1] = lo8(parent_nt.cellvalue);
  epilogue[2] = hi8(parent_nt.cellvalue);
}
cell do_compile_does(const char * PROGMEM arg) {
  uint8_t parentflags = pgm_read_byte(arg);
  compile_does(parentflags);
  /* no code for the VM to execute */
  forth_rom_word_buf[0] = OP_endword;
  return (cell)forth_rom_word_buf;
}


void interpret_does(name_token nt) {
  /* get the word's xt */
  execution_token xt = nt_to_xt(nt);
  uint8_t *body = ramdict_lfa_to_body((lfa)forth_latest)+1;
  execution_token *old_xt = (execution_token *)body;
  /* Update the old (DODOES) instruction with the new xt. */
  *old_xt = xt;
}


static void compiles(uint8_t isemlen) {
  /* empty the word's body (TODO fail if not empty?) */
  forth_np = ramdict_lfa_to_dfa((lfa)forth_latest);
  ramdict_word_set_flags((lfa)forth_latest, FL_COMBINED);
  /* dummy byte that will indicate location of runtime semantics */
  ccomma(isemlen);
}
cell do_compiles(cell arg) {
  uint8_t isemlen = pgm_read_byte(arg);
  compiles(isemlen);
  /* no code for the VM to execute */
  forth_rom_word_buf[0] = OP_endword;
  return (cell)forth_rom_word_buf;
}


static void runs(void) {
  /* fail if latest word is not combined */
  uint8_t latest_flags = ramdict_lfa_to_flags((lfa)forth_latest);
  if (latest_flags != FL_COMBINED) {
    forth_throw(FE_INVALID_TOBODY);
  }
  /* end the compilation semantics */
  ccomma(OP_endword);
  /* update the data field with the length of the compilation semantics */
  /* fail if offset is too large */
  uint8_t *datafield = ramdict_lfa_to_dfa((lfa)forth_latest);
  uint16_t offset = forth_np - datafield;
  if (offset > 255) {
    forth_throw(FE_DICT_ENTRY_TOO_LONG);
  }
  *datafield = (uint8_t)offset;
}
cell do_runs(void) {
  runs();
  /* no code for the VM to execute */
  forth_rom_word_buf[0] = OP_endword;
  return (cell)forth_rom_word_buf;
}
