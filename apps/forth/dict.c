#include "forth.h"
#include "minio.h"
#include "forth_opcodes.h"
#include <stdlib.h>
#include <string.h>

#ifndef MAX
#define MAX(a,b) ({ __auto_type _a=(a); __auto_type _b=(b); (_a>_b)?_a:_b; })
#endif

/* msb of first byte of the name "smudges" the word, i.e. make it unfindable */
/* from the text parser */
#define SMUDGE_BIT  0x80

uint8_t cstrlen_P(const counted_string * PROGMEM cstr) {
  return pgm_read_byte(cstr) & 31;
}


uint8_t cstrlen(const counted_string * cstr) {
  return cstr->len & 31;
}


int fstr_cstrP_cmp(forth_string s1, const counted_string * PROGMEM s2)
{
  uint8_t len2 = cstrlen_P(s2);
  if (s1.len < len2) {
    return -1;
  } else if (s1.len > len2) {
    return +1;
  } else {
    return strncasecmp_P(s1.addr, (const char *)&(s2->str), s1.len);
  }
}


int fstr_cstr_cmp(forth_string s1, const counted_string * s2)
{
  uint8_t len2 = cstrlen(s2);
  if (s1.len < len2) {
    return -1;
  } else if (s1.len > len2) {
    return +1;
  } else {
    return strncasecmp(s1.addr, (const char *)&(s2->str), s1.len);
  }
}


name_token ramdict_lfa_to_nt(const struct dict_entry *lfa) {
  uint16_t offset = (uint8_t*)(lfa) - (uint8_t*)forth_np0;
  name_token nt;
  nt.offset = offset;
  nt.isrom = 0;
  nt.isvalid = 1;
  return nt;
}

name_token romdict_lfa_to_nt(const struct dict_entry * PROGMEM lfa) {
  uint16_t offset = (uint8_t*)(lfa) - (uint8_t*)romdict;
  name_token nt;
  nt.offset = offset;
  nt.isrom = 1;
  nt.isvalid = 1;
  return nt;
}


static name_token romdict_find(forth_string name)
{
  if (name.len == 0) { return NULL_NT; }
  lfa dent = (lfa)romdict_end;
  do {
    if (fstr_cstrP_cmp(name, lfa_to_nfa(dent)) == 0) {
      return romdict_lfa_to_nt(dent);
    }
  } while ((dent = (lfa)romdict_lfa_link(dent)));
  return NULL_NT;
}


execution_token environment_find(forth_string name)
{
  if (name.len == 0) { return NULL_XT; }
  lfa dent = (lfa)environment_end;
  do {
    if (fstr_cstrP_cmp(name, lfa_to_nfa(dent)) == 0) {
      return romdict_lfa_to_xt((lfa)dent);
    }
  } while ((dent = (lfa)romdict_lfa_link(dent)));
  return NULL_XT;
}


static bool ramdict_is_empty(void) {
  return (forth_np == forth_np0);
}


static name_token ramdict_find(forth_string name)
{
  if (name.len == 0 || ramdict_is_empty()) { return NULL_NT; }
  lfa dent = (lfa)forth_latest;
  do {
    if (fstr_cstr_cmp(name, lfa_to_nfa(dent)) == 0) {
      return ramdict_lfa_to_nt(dent);
    }
  } while ((dent = (lfa)ramdict_lfa_link(dent)));
  return NULL_NT;
}


lfa ramdict_find_extent(forth_string name, uint8_t **def_end)
{
  lfa dent = (lfa)forth_latest;
  *def_end = forth_np;
  do {
    if (fstr_cstr_cmp(name, lfa_to_nfa(dent)) == 0) {
      return dent;
    }
    *def_end = (uint8_t*)dent;
  } while ((dent = (lfa)ramdict_lfa_link(dent)));
  return NULL;
}


/* Returns the execution token representing this word's interpretation semantics. */
execution_token ramdict_lfa_to_xt(const lfa lfa) {
  uint8_t flags = ramdict_lfa_to_flags(lfa);
  execution_token xt = {0};
  switch (flags & FL_MASK) {
    /* For words with code in code space, the xt is the address of the code start */
    case FL_COLON:
    case FL_CHILD:
      xt.islfa = 0;
      xt.abs_addr = (cell)ramdict_lfa_to_body(lfa);
      break;
    /* For all others (COMPILER_CHILD, COMBINED, CONSTANT, 2CONSTANT, VARIABLE) */
    /* the xt the lfa's offset from the start of name space. At runtime, we */
    /* need to know the word's flags in order to execute the correct behavior. */
    /* Not great but whatever */
    case FL_COMPILER:
    case FL_COMPILER_CHILD:
    case FL_COMBINED:
    default:
      xt.islfa = 1;
      xt.isrom = 0;
      xt.rel_offset = ((cell)lfa) - ((cell)forth_np0);
      break;
  }
  return xt;
}


/* The compilation token of a RAM word is its LFA's offset from the start of */
/* name space. */
compilation_token ramdict_lfa_to_ct(const lfa lfa) {
  uint16_t offset = ((uint8_t*)lfa) - forth_np0;
  compilation_token ct;
  ct.offset = offset;
  ct.isrom = 0;
  ct.isvalid = 1;
  return ct;
}



/* The execution token of a ROM word is its LFA's offset from the start of */
/* the ROM dictionary. */
execution_token romdict_lfa_to_xt(const lfa lfa) {
  execution_token xt;
  xt.islfa = 1;
  xt.isrom = 1;
  xt.rel_offset = ((uint8_t*)lfa) - (uint8_t*)romdict;
  return xt;
}


/* The compilation token of a ROM word is its LFA's offset from the start of */
/* the ROM dictionary. */
compilation_token romdict_lfa_to_ct(const lfa lfa) {
  uint16_t offset = ((uint8_t*)lfa) - (uint8_t*)romdict;
  compilation_token ct;
  ct.offset = offset;
  ct.isrom = 1;
  ct.isvalid = 1;
  return ct;
}


/* The name token of a RAM word is its LFA's offset from the start of name */
/* space. We have to convert it to a valid pointer. */
lfa ramdict_nt_to_lfa(name_token nt) {
  uint8_t *base = (uint8_t *)forth_np0;
  return (lfa)(base+nt.offset);
}


/* The name token of a ROM word is its LFA's offset from the start of the ROM */
/* dictionary. We have to convert it to a valid pointer. */
lfa romdict_nt_to_lfa(name_token nt) {
  uint8_t *base = (uint8_t *)romdict;
  return (lfa)(base+nt.offset);
}


execution_token nt_to_xt(name_token nt)
{
  if (!IS_VALID_NT(nt)) {
    return NULL_XT;
  } else if (nt.isrom) {
    return romdict_lfa_to_xt((romdict_nt_to_lfa(nt)));
  } else {
    return ramdict_lfa_to_xt((ramdict_nt_to_lfa(nt)));
  }
}


execution_token find_xt(forth_string name) {
  name_token nt = find_name(name);
  return nt_to_xt(nt);
}


compilation_token find_ct(forth_string name) {
  name_token nt = find_name(name);
  if (!IS_VALID_NT(nt)) {
    return NULL_CT;
  } else if (nt.isrom) {
    return romdict_lfa_to_ct(romdict_nt_to_lfa(nt));
  } else {
    return ramdict_lfa_to_ct(ramdict_nt_to_lfa(nt));
  }
}


name_token find_name(forth_string name) {
  if (name.len == 0) {
    goto notfound;
  }
  /* is it in the RAM dictionary? */
  name_token nt = ramdict_find(name);
  if (IS_VALID_NT(nt)) {
    return nt;
  }
  /* is it in the ROM dictionary? */
  nt = romdict_find(name);
  if (IS_VALID_NT(nt)) {
    return nt;
  }
  /* otherwise, not found */
notfound:
  return NULL_NT;
}


void ramdict_forget_lfa(const lfa lfa) {
  uint8_t flags = ramdict_lfa_to_flags(lfa);
  switch (flags & FL_MASK) {
    /* these don't have any data in code space */
    case FL_CONSTANT:
    case FL_2CONSTANT:
    case FL_COMPILER:
    case FL_COMPILER_CHILD:
    case FL_COMBINED:
      break;
    /* these have data in code space AND data in name space */
    case FL_VARIABLE:
    case FL_COLON:
    case FL_CHILD:
      /* the data field contains the location of the word's body in code space */
      forth_cp = *(uint8_t**)ramdict_lfa_to_dfa(lfa);
      break;
  }
  /* roll back name space */
  forth_np = (uint8_t*)lfa;
  forth_latest = (uint8_t*)ramdict_lfa_link(lfa);
  /* TODO not a great hack */
  if (!forth_latest) { forth_latest = forth_np; }
}


void forget_name(name_token nt) {
  /* can't forget ROM words */
  if (!nt.isvalid || nt.isrom) {
    forth_throw(FE_INVALID_FORGET);
  }
  ramdict_forget_lfa(ramdict_nt_to_lfa(nt));
}


cell dict_print_words(void)
{
  lfa word;
  /* RAM words first */
  if (!ramdict_is_empty()) {
    word = (const lfa)forth_latest;
    do {
      forth_string str = ramdict_lfa_to_name(word);
      print_fstr(str);
      mio_nl();
    } while ((word = (lfa)ramdict_lfa_link(word)));
  }
  /* then ROM words */
  word = (lfa)romdict_end;
  do {
    forth_string str = romdict_lfa_to_name(word);
    print_fstr(str);
    mio_nl();
  } while ((word = (lfa)romdict_lfa_link(word)));
  return 0;
}


cell err_compile_only(const char * PROGMEM arg) {
  (void)arg;
  forth_throw(FE_INTEPRET_C_ONLY_WORD);
}


cell err_interpret_only(const char * PROGMEM arg)
{
  (void)arg;
  forth_throw(FE_COMPILE_I_ONLY_WORD);
}


void codespace_ccomma(uint8_t num) {
  forth_result result = codespace_allot(1);
  forth_throw_if(result.err);
  *(result.byteptr) = num;
}


void codespace_comma(uint16_t num) {
  forth_result result = codespace_allot(2);
  forth_throw_if(result.err);
  *(result.cellptr) = num;
}


void namespace_ccomma(uint8_t num) {
  forth_result result = namespace_allot(1);
  forth_throw_if(result.err);
  *(result.byteptr) = num;
}


void namespace_comma(uint16_t num) {
  forth_result result = namespace_allot(2);
  forth_throw_if(result.err);
  *(result.cellptr) = num;
}


static bool namespace_move(uint8_t *new_np0) {
  uint16_t dsize = forth_np-forth_np0;
  uint16_t new_nmax;
  if (__builtin_uadd_overflow((uint16_t)new_np0, dsize, &new_nmax) || new_nmax > (uint16_t)forth_npmax) {
    return false;
  }
  //!!! FIXME WITH YOUR RHYTHM STICK - this could be Bad News if this function
  // is invoked while executing inline from name space. this MAY be ok as long
  // as the memory previously occupied by the name space is left untouched. but
  // a fully correct implementation would adjust ip and also need to handle
  // relative returns (because moving the name space would invalidate any
  // return addresses in name space on the return stack)
  forth_latest = new_np0 + (forth_latest-forth_np0);
  memmove(new_np0, forth_np0, dsize);
  forth_np = new_np0 + dsize;
  forth_np0 = new_np0;
  return true;
}


static bool moar_bytecode(uint16_t nbytes) {
  /* see if the name space will be nice and make room for us */
  uint8_t *new_np0 = forth_cp+nbytes;
  return namespace_move(new_np0);
}


forth_result codespace_allot(uint16_t nbytes)
{
  uint8_t *new_np0;
  if (__builtin_uadd_overflow((uint16_t)forth_cp, nbytes, (uint16_t*)(&new_np0)) || new_np0 > forth_np0) {
    /* Request more space. Try to grab large chunks if possible, to cut down */
    /* on allocation overhead. */
    if (!moar_bytecode(MAX(nbytes, 256U))) {
      /* If that failed, ask again for just the amount we want */
      if (!moar_bytecode(nbytes)) {
        /* And if that failed, there's just no more free memory */
        return RESULT_ERR(FE_CODESPACE_OVERFLOW);
      }
    }
  }
  uint8_t *prev_cp = forth_cp;
  forth_cp += nbytes;
  return RESULT_OK(prev_cp);
}


forth_result namespace_allot(uint16_t nbytes)
{
  if (forth_available_memory() < nbytes) {
    return RESULT_ERR(FE_NAMESPACE_OVERFLOW);
  }

  /* If there's enough memory, but it's not contiguous, close the gap between */
  /* code space and name space. */
  if (forth_np+nbytes > forth_npmax) {
    namespace_move(forth_cp);
  }

  /* Dictionary uses 8-bit links. So we can't let dp get more than 255 */
  /* bytes away from latest, or the linked list will get broken. */
  uint8_t *new_np;
  if (__builtin_uadd_overflow((uint16_t)forth_np, nbytes, (uint16_t*)(&new_np)) ||
      (new_np-forth_latest > 255)) {
    return RESULT_ERR(FE_DICT_ENTRY_TOO_LONG);
  }
  uint8_t *prev_np = forth_np;
  forth_np += nbytes;
  return RESULT_OK(prev_np);
}


void dict_save(void) {
  forth_saved_np     = forth_np;
  forth_saved_np0    = forth_np0;
  forth_saved_latest = forth_latest;
  forth_saved_cp     = forth_cp;
}


void dict_restore(void) {
  forth_np      = forth_saved_np;
  forth_np0     = forth_saved_np0;
  forth_latest  = forth_saved_latest;
  forth_cp      = forth_saved_cp;
}


void namespace_create(forth_string name, uint8_t flags)
{
  if (name.len >= 32) {
    forth_throw(FE_NAME_TOO_LONG);
  } else if (name.len == 0) {
    forth_throw(FE_ZERO_LENGTH_NAME);
  }
  namespace_create_nonamecheck(name, flags);
}


void namespace_create_nonamecheck(forth_string name, uint8_t flags)
{
  /* Save dictionary state, so we can roll it back and scratch this word */
  /* if an error occurs in its definition. */
  dict_save();

  /* Ensure there's enough space */
  forth_result result = namespace_allot(name.len+2); /* need extra bytes for the link and the name length */
  forth_throw_if(result.err);
  uint8_t *p = result.byteptr;

  /* Add the link */
  forth_latest = p;
  *p++ = forth_latest-forth_saved_latest;
  /* Add the name length and flags */
  *p++ = name.len | (flags & FL_MASK);
  /* Add the name */
  memcpy(p, name.addr, name.len);
  /* Set the smudge bit if directed to do so */
  if ((flags & FL_CREATE_SMUDGED) && name.len > 0) {
    *p |= SMUDGE_BIT;
  }

  /* Initialize the data field */
  switch (flags & FL_MASK) {
    /* Data field is the current value of the code space pointer */
    case FL_VARIABLE:
    case FL_COLON:
      namespace_comma((cell)forth_cp);
      break;
    /* Data field is also value of the code space pointer, but also */
    /* pre-populate the word with default child behavior. */
    case FL_CHILD:
      namespace_comma((cell)forth_cp);
      ccomma(OP_dodoes);
      comma(NOP_XT.cellvalue);
      break;
    case FL_COMPILER_CHILD:
      ccomma(OP_dodoes);
      comma(NOP_XT.cellvalue);
      break;
    /* These start with an empty data field */
    case FL_CONSTANT:
    case FL_2CONSTANT:
    case FL_COMPILER:
    case FL_COMBINED:
    default:
      break;
  }
}


void do_for_name(name_token nt, ram_word_void_fn ramfn, rom_word_void_fn romfn)
{
  if (!IS_VALID_NT(nt)) {
    forth_throw(FE_UNDEFINED_WORD);
  } else if (nt.isrom) {
    romfn(romdict_nt_to_lfa(nt));
  } else {
    ramfn(ramdict_nt_to_lfa(nt));
  }
}


/* Attempts to find a VALUE or 2VALUE with the given name. */
/* Throws an exception if the word isn't found or is not a VALUE or 2VALUE. */
extern char xt_of_doval[];
extern char xt_of_do2val[];
double_cell find_value(forth_string name)
{
  double_cell valueaddrs;
  name_token nt = find_name(name);
  /* error if word not found, or it's a ROM word */
  if (!IS_VALID_NT(nt)) { forth_throw(FE_UNDEFINED_WORD); }
  if (nt.isrom)         { goto invalid_to; }
  /* all VALUEs/2VALUEs are compiler-child words. error if word isn't a child */
  lfa word = ramdict_nt_to_lfa(nt);
  uint8_t flags = ramdict_lfa_to_flags(word);
  if (flags != FL_COMPILER_CHILD) {
    goto invalid_to;
  }
  /* what's the parent word? */
  uint8_t *body = ramdict_lfa_to_body(word);
  execution_token parent_xt = *(execution_token*)(body+1);
  cell *valueaddr = *(cell**)(body+3);
  /* is it a child of DOVAL? */
  if (parent_xt.cellvalue == (cell)xt_of_doval) {
    valueaddrs.high = (cell)valueaddr;
    valueaddrs.low = 0;
    return valueaddrs;
  }
  /* is it a child of DO2VAL? */
  else if (parent_xt.cellvalue == (cell)xt_of_do2val) {
    /* For double-cell values, the high word is stored in the lower address. */
    valueaddrs.high = (cell)valueaddr;
    valueaddrs.low = (cell)(valueaddr+1);
    return valueaddrs;
  }
invalid_to:
  forth_throw(FE_INVALID_NAME_ARG);
}


extern char xt_of_dodefer[];
void *find_defer(forth_string name)
{
  name_token nt = find_name(name);
  /* error if word not found, or it's a ROM word */
  if (!IS_VALID_NT(nt)) { forth_throw(FE_UNDEFINED_WORD); }
  if (nt.isrom)         { goto invalid_is; }
  /* all DEFERs are compiler-child words. error if word isn't a child */
  lfa word = ramdict_nt_to_lfa(nt);
  uint8_t flags = ramdict_lfa_to_flags(word);
  if (flags != FL_COMPILER_CHILD) {
    goto invalid_is;
  }
  /* what's the parent word? */
  uint8_t *body = ramdict_lfa_to_body(word);
  execution_token parent_xt = *(execution_token*)(body+1);
  cell *valueaddr = *(cell**)(body+3);
  /* is it a child of DOVAL? */
  if (parent_xt.cellvalue == (cell)xt_of_dodefer) {
    return valueaddr;
  }
invalid_is:
  forth_throw(FE_INVALID_NAME_ARG);
}


void dict_init(void) {
  forth_cp = forth_cp0;
  forth_np0 = forth_cp0 + 1024;
  forth_npmax = forth_pmax;
  forth_np = forth_np0;
  forth_latest = forth_np;
}


void unsmudge(void) {
  lfa latest = (lfa)forth_latest;
  if (latest->name.len > 0) {
    latest->name.str[0] &= (~SMUDGE_BIT);
  }
}


char *alloc_screen_buffer(uint16_t nbytes) {
  if (!FORTH_MEM_MODEL_IS_TINY()) {
    uint8_t *new_npmax = forth_pmax-nbytes;
    if (new_npmax < forth_np) {
      /* try closing gap between name space and code space */
      namespace_move(forth_cp);
      if (new_npmax < forth_np) {
        forth_throw(FE_NAMESPACE_OVERFLOW);
      }
    }
    forth_npmax = forth_pmax-nbytes;
    return (char *)forth_npmax;
  } else {
    return 0x8000+forth_tiny_mem_end-nbytes;
  }
}
