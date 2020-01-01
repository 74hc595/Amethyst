#ifndef FORTH_H_
#define FORTH_H_

#include <stdint.h>
#include <stdbool.h>
#include <avr/pgmspace.h>

#include "forth_defs.h"
#include "console.h"

typedef uint16_t cell;

typedef uint32_t dcell;
typedef union {
  struct {
    cell low;
    cell high;
  };
  dcell full;
} double_cell;

/* Entire memory region used by Forth. */
extern uint8_t forth_mem[];
extern uint8_t forth_mem_end[];

/* Code space and name space. */
extern uint8_t forth_cp0[];
extern uint8_t forth_pmax[];

/* Data stack. */
extern uint8_t forth_sp0[];
extern uint8_t forth_spmax[];

/* Return stack. */
extern uint8_t forth_rp0[];
extern uint8_t forth_rpmax[];

extern uint8_t forth_tiny_rp0[];
extern uint8_t forth_tiny_sp0[];
extern char forth_tiny_mem_end[];
extern char *screen_ptr;
#define FORTH_MEM_MODEL_IS_TINY() ((ptr_to_int(screen_ptr) & 0x8000) != 0)

extern uint8_t *forth_np;     /* next free location in name space */
extern uint8_t *forth_np0;    /* name space start and code space limit */
extern uint8_t *forth_npmax;  /* name space limit */
extern uint8_t *forth_latest; /* start of the most recently defined word in name space */
extern uint8_t *forth_cp;     /* next free location in code space */

extern char *forth_hld0;      /* location of the start of the pictured numeric output buffer */
extern uint8_t forth_hld;     /* current offset in the pictured numeric output buffer */

/* Saved values for error recovery */
extern uint8_t *forth_saved_dsp;
extern uint8_t *forth_saved_rsp;
extern uint8_t *forth_saved_np;
extern uint8_t *forth_saved_np0;
extern uint8_t *forth_saved_latest;
extern uint8_t *forth_saved_cp;

typedef union {
  struct {
    cell addr:15;
    /* if 0, input source is a string starting at addr */
    /* if 1, input source is the terminal or block */
    cell isblock:1;
  };
  struct {
    cell blk:8; /* block 0 is the terminal */
    cell blkline:5;
    cell :3;
  };
  cell cellvalue;
} __attribute__((packed)) input_source;

extern input_source forth_inputsrc;
extern uint8_t forth_inputlen;
extern uint8_t forth_inputpos;
extern uint8_t forth_base;
extern uint8_t forth_last_word_pos;
char *forth_inputbuf(void);

extern struct console fcon;

extern char forth_tib[80];
extern char forth_blockbuf[BLOCK_BYTES_PER_LINE];

extern uint8_t forth_rom_word_buf[];

typedef struct {
  struct {
    unsigned len:5;
    unsigned :3;
  };
  char str[];
} __attribute__((packed)) counted_string;


struct dict_entry {
  uint8_t link;
  counted_string name;
} __attribute__((packed));
typedef struct dict_entry * lfa;

/**
 * An execution token is a value that uniquely represents the interpretation
 * semantics of a word.
 */
typedef union {
  struct {
    cell rel_offset:14;
    cell isrom:1;
    cell islfa:1;
  };
  struct {
    cell abs_addr:15;
    cell :1;
  };
  cell cellvalue;
} __attribute__((packed)) execution_token;
#define IS_VALID_XT(xt)   ((xt).cellvalue != 0)
#define NULL_XT           ((execution_token){0})
#define NOP_XT            ((execution_token){.rel_offset=0, .isrom=1, .islfa=1})

/**
 * A compilation token is a value that uniquely represents the compilation
 * semantics of a word.
 */
typedef union {
  struct {
    cell offset:14;
    cell isrom:1;
    cell isvalid:1;
  };
  cell cellvalue;
} __attribute__((packed)) compilation_token;
#define IS_VALID_CT(ct)   ((ct).isvalid)
#define NULL_CT           ((compilation_token){0})

/**
 * A name token is a value that uniquely represents a dictionary entry.
 */
typedef union {
  struct {
    cell offset:14;
    cell isrom:1;
    cell isvalid:1;
  };
  cell cellvalue;
} __attribute__((packed)) name_token;
#define IS_VALID_NT(nt)   ((nt).isvalid)
#define NULL_NT           ((name_token){0})

/**
 * When passed as the first argument to, or returned from, a C function:
 * - addr in r23:r22
 * -  len in r25:r24
 */
typedef struct {
  char *addr;
  struct {
    uint16_t len:15;
    uint16_t inrom:1;
  };
} __attribute__((packed)) forth_string;


extern const struct dict_entry romdict[] PROGMEM;
extern const struct dict_entry romdict_end[] PROGMEM;
extern const struct dict_entry environment[] PROGMEM;
extern const struct dict_entry environment_end[] PROGMEM;

PGM_P forth_error_str(forth_err_code err);

/**
 * Returns length of the counted string cstr.
 */
uint8_t cstrlen_P(const counted_string * PROGMEM cstr);
uint8_t cstrlen(const counted_string *cstr);

/**
 * Compares a Forth string (length/address pair) to a counted string in PROGMEM,
 * case-insensitive.
 * Return value is the same as strcmp()/memcmp().
 */
int fstr_cstrP_cmp(forth_string s1, const counted_string * PROGMEM s2);
int fstr_cstr_cmp(forth_string s1, const counted_string *s2);

/**
 * Given a link field address (i.e. the start of a dictionary header)
 * return the address of the entry's name.
 * The name is a counted string; lower 5 bits of first byte indicate string
 * length.
 */
const counted_string *lfa_to_nfa(const struct dict_entry *lfa);

/**
 * Given a link field address (i.e. the start of a dictionary header)
 * returns this word's flags.
 */
uint8_t ramdict_lfa_to_flags(const struct dict_entry *lfa);

/**
 * Given a link field address, set that word's flags.
 */
void ramdict_word_set_flags(const struct dict_entry *lfa, uint8_t newflags);

/**
 * Given a link field address (i.e. the start of a dictionary header)
 * returns this word's name as a Forth string. (address/length pair)
 */
forth_string romdict_lfa_to_name(const struct dict_entry *lfa);
forth_string ramdict_lfa_to_name(const struct dict_entry *lfa);

name_token ramdict_lfa_to_nt(const struct dict_entry *lfa);
name_token romdict_lfa_to_nt(const struct dict_entry * PROGMEM lfa);

/**
 * Write a Forth string (address/length pair) to the output device.
 */
void print_fstr(forth_string str);

/**
 * Given a link field address (i.e. the start of a dictionary header in ROM)
 * return the (ROM) address of the word's code field.
 * Reading the word at cfa and executing it will perform this word's
 * execution semantics.
 * Reading the word at (cfa+2) and executing it will perform this
 * word's compilation semantics.
 */
void *romdict_lfa_to_cfa(const struct dict_entry *lfa);

/**
 * Given a link field address (i.e. the start of a dictionary header)
 * return the address in name space of the word's data field.
 */
void *ramdict_lfa_to_dfa(const struct dict_entry *lfa);

/**
 * Given a link field address (i.e. the start of a dictionary header)
 * return the address where the word's body starts. (either in name space or
 * in code space)
 */
void *ramdict_lfa_to_body(lfa lfa);

/**
 * Given a link field address (i.e. the start of a dictionary header) for a word
 * return its execution token.
 */
execution_token ramdict_lfa_to_xt(const lfa lfa);
execution_token romdict_lfa_to_xt(const lfa lfa);

/**
 * Given a link field address (i.e. the start of a dictionary header) for a word
 * return its compilation token.
 */
compilation_token ramdict_lfa_to_ct(const lfa lfa);
compilation_token romdict_lfa_to_ct(const lfa lfa);

/**
 * Given a link field address (i.e. the start of a dictionary header)
 * follow its link and return the address of the next header in name space.
 * Returns NULL if this was the last entry.
 */
const struct dict_entry * PROGMEM romdict_lfa_link(const struct dict_entry *lfa); 
const struct dict_entry * ramdict_lfa_link(const struct dict_entry *lfa);

/**
 * Get the execution token of the word with the given name.
 */
execution_token find_xt(forth_string name);

/**
 * Get the compilation token of the word with the given name.
 */
compilation_token find_ct(forth_string name);

/**
 * Get the name token of the first word in the search order with the given name.
 * Returns a null nt if no entry could be found or name is a zero-length string.
 */
name_token find_name(forth_string name);

/**
 * Attempts to find a VALUE or 2VALUE with the given name. Used by TO.
 * Throws an exception if the named word is not a VALUE or 2VALUE.
 * Returns a pair of addresses; high is the address of the VALUE/2VALUE's upper
 * word. lower is the address of the 2VALUE's lower word, or 0 if the word is
 * a VALUE.
 */
double_cell find_value(forth_string name);

/**
 * Attempts to find a DEFERred word with the given name. Used by IS.
 */
void *find_defer(forth_string name);

lfa ramdict_nt_to_lfa(name_token nt);
lfa romdict_nt_to_lfa(name_token nt);

execution_token   nt_to_xt(name_token nt);
compilation_token nt_to_ct(name_token nt);

typedef void (*ram_word_void_fn)(const lfa);
typedef void (*rom_word_void_fn)(const lfa);
void do_for_name(name_token nt, ram_word_void_fn ramfn, ram_word_void_fn romfn);

cell execute_xt_or_return_code_address(execution_token xt);
cell compile_ct_or_return_code_address(compilation_token ct);

/**
 * Extracts a name from the input buffer, delimited by spaces.
 */
forth_string forth_parse_name(void);

/**
 * Attempts to parse a number from token in the current base.
 * This is the behavior of the Forth word >NUMBER.
 */
forth_string forth_to_number(forth_string token, double_cell *value);

cell eval_numeric(forth_string token);

#define COMPILE_OK            0x0001

void ramdict_word_compile(lfa lfa);

uint16_t forth_available_memory(void);

void  forth_outer_interpreter(void);
_Noreturn void forth_throw(forth_err_code err);
void forth_throw_if(forth_err_code err);

void forth_dump_regs(void);

cell compile_literal(cell value);
cell compile_double_literal(double_cell value);

typedef struct {
  union {
    cell cellvalue;
    void *ptr;
    uint8_t *byteptr;
    cell *cellptr;
  };
  int16_t err;
} __attribute__((packed)) forth_result;
#define RESULT_ERR(e) (forth_result){.err=(e)}
#define RESULT_OK(v)  (forth_result){.ptr=(v)}

forth_result allot(uint16_t nbytes);
void ccomma(uint8_t num);
void comma(uint16_t num);
void *here(void);

forth_result codespace_allot(uint16_t nbytes);
void codespace_ccomma(uint8_t num);
void codespace_comma(uint16_t num);

forth_result namespace_allot(uint16_t nbytes);
void namespace_ccomma(uint8_t num);
void namespace_comma(uint16_t num);
void namespace_create(forth_string name, uint8_t flags);
void namespace_create_nonamecheck(forth_string name, uint8_t flags);
void unsmudge(void);

/* Reserves the top nbytes of user-available memory for the screen buffer. */
/* Throws an exception if there is not enough free space. */
char *alloc_screen_buffer(uint16_t nbytes);

void dict_init(void);
void dict_save(void);
void dict_restore(void);

struct forth_flags {
  uint8_t :4;
  uint8_t editing:1;    /* is the editor active? */
  uint8_t singlestep:1;
  uint8_t trace:1;
  uint8_t state:1;      /* 1=compiling, 0=interpreting */
} __attribute__((packed));
extern struct forth_flags forth_flags;

extern uint8_t forth_scr;


void load_block_range(char *dst, uint8_t blocknum, uint16_t offset, uint16_t nbytes);
void save_block_range(const char *src, uint8_t blocknum, uint16_t offset, uint16_t nbytes);
uint8_t num_xmem_blocks(void);
uint8_t num_blocks(void);

#endif
