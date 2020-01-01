#include "forth.h"
#include "minio.h"
#include "forth_opcodes.h"
#include "disasm.h"

#include <alloca.h>

void forth_dot(cell n, cell width);
void forth_udot(cell n, cell width);
void forth_hdot(cell n, cell width);
void forth_ddot(int32_t n, cell width);

/* called by (.S) opcode. must return tos unmodified */
cell forth_dump_stack(cell tos, uint8_t *dsp, int8_t fmt) {
  void (*printfn)(cell,cell);
  if (fmt < 0) {
    printfn = forth_dot;
  } else if (fmt > 0) {
    printfn = forth_udot;
  } else {
    printfn = forth_hdot;
  }

  uint16_t depth = (forth_sp0 - dsp) / sizeof(cell);
  Pc('<'); Pu16(depth); Psl("> ");
  if (depth == 0) { return tos; }
  /* print from bottom to second item on stack */
  cell *sp0 = (cell *)(forth_sp0-4);
  for (cell *c = sp0; c >= (cell*)dsp; c--) {
    printfn(*c, 0);
  }
  /* top-of-stack is special-cased */
  printfn(tos, 0);
  return tos;
}


uint16_t forth_available_memory(void) {
  return (forth_npmax-forth_np)+(forth_np0-forth_cp);
}


void forth_dump_regs(void) {
  Psl("DP-"); Pp(forth_cp); Psl(" NP-"); Pp(forth_np0); Pc('>'); Pp(forth_np); Pc('<'); Pp(forth_npmax);
  Psl(" FREE-"); Pu16(forth_available_memory()); Pbl(); Pc((in_compile_mode()) ? 'C' : 'I'); Pnl();
}


void hexdump_range(forth_string range) {
  if (range.len == 0) { return; }
  mio_nl();
  hexdump((cell)(range.addr), (cell)(range.addr+range.len-1), 0);
}


static const char * PROGMEM flags_to_str(uint8_t flags) {
  switch (flags & FL_MASK) {
    case FL_VARIABLE:       return PSTR("variable");
    case FL_COLON:          return PSTR("colon");
    case FL_CHILD:          return PSTR("child");
    case FL_CONSTANT:       return PSTR("constant");
    case FL_2CONSTANT:      return PSTR("2constant");
    case FL_COMPILER:       return PSTR("compiler");
    case FL_COMPILER_CHILD: return PSTR("compiler-child");
    case FL_COMBINED:       return PSTR("combined");
    default:                return PSTR("");
  }
}


static void ramdict_describe_word(lfa word) {
  uint8_t flags = ramdict_lfa_to_flags(word);
  cell *datafield = ramdict_lfa_to_dfa(word);
  Psl("\n Name: ");
  print_fstr(ramdict_lfa_to_name(word));
  Psl("\n LFA:  "); Pp(word);
  Psl("\n Type: "); PSp(flags_to_str(flags));
  Psl(  " DFA:  "); Pp(datafield);
  Psl("\n Body: "); Pp(ramdict_lfa_to_body(word));
  Psl("\n NT:   "); Pp(ramdict_lfa_to_nt(word).cellvalue);
  Psl("\n XT:   "); Pp(ramdict_lfa_to_xt(word).cellvalue);
  Psl("\n CT:   "); Pp(ramdict_lfa_to_ct(word).cellvalue);
  Pnl();
}


/* functions to help classify romdict words */
extern cell romdict_interpret_bytecode();
extern cell romdict_interpret_bytecode_linkcall();
extern cell romdict_interpret_bytecode_compsem();
extern cell romdict_compile_bytecode();
extern cell err_compile_only();
extern cell err_interpret_only();

static bool interprets_bytecode(cell (*semfn)()) {
  return (semfn == romdict_interpret_bytecode) ||
         (semfn == romdict_interpret_bytecode_linkcall) ||
         (semfn == romdict_interpret_bytecode_compsem);
}

static bool interprets_separate_compsem_bytecode(cell (*semfn)()) {
  return (semfn == romdict_interpret_bytecode_compsem);
}

static bool compiles_bytecode(cell (*semfn)()) {
  return (semfn == romdict_compile_bytecode);
}


static void romdict_describe_word(lfa word) {
  const uint8_t *cfa = romdict_lfa_to_cfa(word);
  cell (*isem)() = pgm_read_ptr(cfa);   /* interpretation semantics */
  cell (*csem)() = pgm_read_ptr(cfa+2); /* compilation semantics */
  Psl("\n Name: ");
  print_fstr(romdict_lfa_to_name(word));
  Psl("\n LFA:  "); Pp(word);
  Psl("\n Type: rom-builtin ");
  if (interprets_bytecode(isem) && compiles_bytecode(csem)) {
    PSl("colon");
  } else if (interprets_bytecode(isem) && interprets_separate_compsem_bytecode(csem)) {
    PSl("colon-interpret colon-compile");
  } else if (interprets_bytecode(isem) && interprets_bytecode(csem)) {
    PSl("immediate");
  } else if (isem==err_compile_only) {
    PSl("compile-only");
  } else if (csem==err_interpret_only) {
    PSl("interpret-only");
  } else {
    Psl("interpret="); Pp(isem); Psl(" compile="); Pp(csem); Pnl();
  }
  Psl(" CFA:  "); Pp(cfa);
  Psl("\n NT:   "); Pp(romdict_lfa_to_nt(word).cellvalue);
  Psl("\n XT:   "); Pp(romdict_lfa_to_xt(word).cellvalue); Psl(" ("); Pp(isem); Pc(')');
  Psl("\n CT:   "); Pp(romdict_lfa_to_ct(word).cellvalue); Psl(" ("); Pp(csem); Pc(')');
  Pnl();
}


cell do_describe(void) {
  do_for_name(find_name(forth_parse_name()),
    ramdict_describe_word,
    romdict_describe_word);
  return 0;
}

static uint8_t *decompile_one(uint8_t *addr) {
  return vm_disassemble((disasm_options){.location=(cell)addr});
}


static void decompile(uint8_t *addr) {
  uint8_t last_opcode = 0;
  do {
    uint8_t *new_addr = decompile_one(addr);
    last_opcode = *addr;
    addr = new_addr;
  } while (last_opcode != OP_endword);
}

static __attribute__((noinline)) void decompile_rom_bytecode(const uint8_t * PROGMEM addr, uint8_t nbytes) {
  uint8_t *ip = alloca(nbytes);
  memcpy_P(ip, addr, nbytes);
  uint8_t *endaddr = ip+nbytes;
  while (ip < endaddr) {
    ip = vm_disassemble((disasm_options){.location=(cell)ip, .suppress_address=1});
  }
}


static void romdict_see_word(lfa word) {
  const uint8_t *cfa = romdict_lfa_to_cfa(word);
  cell (*isem)() = pgm_read_ptr(cfa);   /* interpretation semantics */
  cell (*csem)() = pgm_read_ptr(cfa+2); /* compilation semantics */
  forth_string name = romdict_lfa_to_name(word);

  Psl("\nROM-BUILTIN "); print_fstr(name); Pnl();
  if (interprets_bytecode(isem)) {
    PSl("--- interpret:");
    decompile_rom_bytecode(cfa+5, pgm_read_byte(cfa+4));
  } else if (isem != err_compile_only) {
    Psl("--- interpret: (CCALL) "); Pp(isem); Pnl();
  }

  if (interprets_bytecode(csem)) {
    PSl("--- compile:");
    if (interprets_separate_compsem_bytecode(csem)) {
      uint8_t impsem_len = pgm_read_byte(cfa+4);
      const uint8_t *comp_code = cfa+5+impsem_len;
      decompile_rom_bytecode(comp_code+1, pgm_read_byte(comp_code));
    } else {
      decompile_rom_bytecode(cfa+5, pgm_read_byte(cfa+4));
    }
  } else if (compiles_bytecode(csem)) {
    PSl("--- compile:   default");
  } else if (csem != err_interpret_only) {
    Psl("--- compile:   (CCALL) "); Pp(csem); Pnl();
  }
}


static void ramdict_see_word(lfa lfa) {
  uint8_t flags = ramdict_lfa_to_flags(lfa);
  const cell *bodyaddr = ramdict_lfa_to_body(lfa);
  forth_string name = ramdict_lfa_to_name(lfa);
  Pnl();
  switch (flags & FL_MASK) {
    case FL_VARIABLE:
      forth_dot((int16_t)bodyaddr, 0);
      Psl("VARIABLE ");
      print_fstr(name); Pnl();
      break;
    case FL_CONSTANT:
      forth_dot(*bodyaddr, 0);
      Psl("CONSTANT ");
      print_fstr(name); Pnl();
      break;
    case FL_2CONSTANT: {
      double_cell value = { .high = bodyaddr[0], .low = bodyaddr[1] };
      forth_ddot(value.full, 0);
      Psl("2CONSTANT ");
      print_fstr(name); Pnl();
      break;
    }
    case FL_COMPILER:
      Pc(':');
      goto colon;
    case FL_COLON:
    colon:
      Psl(": ");
      print_fstr(name); Pnl();
      decompile((uint8_t*)bodyaddr);
      break;
    case FL_COMPILER_CHILD:
      Psl("CREATE:: ");
      goto child;
    case FL_CHILD:
      Psl("CREATE ");
    child:
      print_fstr(name); Pnl(); Pp(bodyaddr+1); Psl("\nDOES>\n");
      decompile_one((uint8_t*)bodyaddr);
      break;
    case FL_COMBINED: {
      Psl(":: ");
      print_fstr(name);
      Psl(" <COMPILES\n");
      /* Compilation semantics start one byte after the data field */
      uint8_t *dfa = ramdict_lfa_to_dfa(lfa);
      decompile(dfa+1);
      uint8_t isem_offset = *dfa;
      if (isem_offset != 0) {
        Psl("RUNS>\n");
        decompile(dfa+isem_offset);
      }
      break;
    }
  }
}


cell do_see(void) {
  do_for_name(find_name(forth_parse_name()),
    ramdict_see_word,
    romdict_see_word);
  return 0;
}



cell debug_enter(void) {
  forth_flags.singlestep = 1;
  forth_flags.trace = 1;
  return 0;
}

cell debug_exit(void) {
  forth_flags.singlestep = 0;
  forth_flags.trace = 0;
  return 0;
}

cell debug_trace(void) {
  forth_flags.singlestep = 0;
  forth_flags.trace = 1;
  return 0;
}

#define MAX_STACK_TRACE_DEPTH 5
void breakpoint(cell tos, uint8_t *dsp, uint8_t *ip, uint8_t *rsp)
{
  Psl("\nIP-");
  vm_disassemble((disasm_options){.location=(cell)ip});
  Psl("RS-"); Pp(rsp); Psl(" [ ");
  uint8_t d = MAX_STACK_TRACE_DEPTH;
  for (; (rsp < forth_rp0) && (--d); rsp+=2) {
    Px8(rsp[2]); Px8(rsp[1]); Pbl();
  }
  Psp((d) ? PSTR("]\nDS-") : PSTR("...\nDS-"));
  Pp(dsp); Psl(" [ ");
  d = MAX_STACK_TRACE_DEPTH;
  if (dsp < forth_sp0) {
    Px16(tos); Pbl();
    for (; dsp+2 < forth_sp0 && (--d); dsp+=2) {
      Px16(*(cell*)dsp); Pbl();
    }
  }
  Psp((d) ? PSTR("]\n") : PSTR("...\n"));
    if (forth_flags.singlestep) {
    /* prompt for command */
    Psl("abort, continue, trace, step? ");
    char c = mio_waitc();
    switch (c) {
      case 'a':  case 'A':
      case 'q':  case 'Q':    /* q is a synonym, "quit" */
      case '\e': /* Esc should abort too */
        /* Abort: abandon current execution */
        Pc('a');
        forth_throw(FE_QUIT);
        __builtin_unreachable();
      case 'c': case 'C':
        /* Continue: turn debugging off and continue execution */
        Pc('c');
        forth_flags.singlestep = 0;
        forth_flags.trace = 0;
        break;
      case 't': case 'T':
        /* Trace: print disassembly of each instruction as executed */
        Pc('t');
        debug_trace();
        break;
      case 's': case 'S': default:
        /* Step: single-step one instruction */
        Pc('s');
        debug_enter();
        break;
    };
    Pnl();
  }
}
