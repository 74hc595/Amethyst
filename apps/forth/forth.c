#include "app.h"
#include "console.h"
#include "forth.h"
#include "minio.h"
#include "readline.h"
#include "serial.h"
#include "spi.h"
#include "forth_version.h"
#include <string.h>

char *forth_inputbuf(void) {
  if (forth_inputsrc.isblock) {
    return (forth_inputsrc.blk) ? forth_blockbuf : forth_tib;
   } else {
     return (char*)((cell)forth_inputsrc.addr);
   }
}


static void read_block_line(char *dst, uint8_t blk, uint8_t line) {
  load_block_range(dst, blk, ((uint16_t)line)*BLOCK_BYTES_PER_LINE, BLOCK_BYTES_PER_LINE);
}


/**
 * Fills the input buffer from the current input source.
 * (runtime of the Forth word REFILL)
 */
bool forth_refill(void) {
  if (!forth_inputsrc.isblock) {
    return false;
  }
  char *inbuf = forth_inputbuf();
  /* block 0? get line from terminal */
  if (forth_inputsrc.blk == 0) {
    forth_inputlen = readline(inbuf, 80);
    if ((int8_t)forth_inputlen < 0) {
      inbuf[0] = '\0';
      forth_inputlen = 0;
    }
  } else {
    if (forth_inputsrc.blkline >= LINES_PER_BLOCK) {
      return false;
    }
    /* otherwise, read from block and increment line number */
    read_block_line(inbuf, forth_inputsrc.blk, forth_inputsrc.blkline);
    forth_inputlen = BLOCK_BYTES_PER_LINE;
    forth_inputsrc.blkline++;
  }

  forth_inputpos = 0;
  return true;
}


/**
 * Advances >IN past any occurrences of the given character in the TIB.
 * Before:
 *   [space] [space] [space] [A] [B] [C]
 *   #TIB=6, >IN=0
 * After forth_consume_delim(' '), >IN=3.
 */
void forth_consume_delim(char delim) {
  while (forth_inputpos < forth_inputlen) {
    char c = forth_inputbuf()[forth_inputpos];
    if (c != delim) { break; }
    forth_inputpos++;
  }
}


/* Does NOT consume leading delimiters. */
forth_string forth_parse(char delim) {
  char *inbuf = forth_inputbuf();
  forth_string str = { .addr=inbuf+forth_inputpos, .len=0 };
  /* Gather characters until we find another delimiter or run out of chars. */
  while (forth_inputpos < forth_inputlen) {
    char c = inbuf[forth_inputpos++];
    if (c == delim) { break; }
    str.len++;
  }
  return str;
}


forth_string forth_parse_name(void) {
  forth_consume_delim(' ');
  forth_string str = forth_parse(' ');
  forth_last_word_pos = str.addr-forth_inputbuf();
  return str;
}


cell forth_eval_token(forth_string token) {
  cell ret;
  if (in_interpret_mode()) {
    execution_token xt = find_xt(token);
    if (!IS_VALID_XT(xt)) { goto number; }
    return execute_xt_or_return_code_address(xt);
  } else {
    compilation_token ct = find_ct(token);
    if (!IS_VALID_CT(ct)) { goto number; }
    return compile_ct_or_return_code_address(ct);
  }

number:
  ret = eval_numeric(token);
  if (!ret) {
    forth_throw(FE_UNDEFINED_WORD);
  }
  return ret;
}


bool using_console(void);
void forth_postcommand(const uint8_t *dsp, const uint8_t *rsp)
{
  if (dsp > forth_sp0) {
    forth_throw(FE_DSTACK_UNDERFLOW);
  } else if (dsp < forth_spmax) {
    forth_throw(FE_DSTACK_OVERFLOW);
  } else if (rsp > forth_rp0) {
    forth_throw(FE_RSTACK_UNDERFLOW);
  } else if (rsp <= forth_rpmax) {
    forth_throw(FE_RSTACK_OVERFLOW);
  }
  if (using_console()) { fcon.slow = 0; }
  PSp((in_interpret_mode()) ? PSTR(" ok") : PSTR(" \\"));
}


/* print error message and QUIT */
void forth_bailout(forth_string abortq_errmsg, forth_err_code err) {
  /* QUIT is silent */
  if (err != FE_QUIT) {
    console_beep();
    if (forth_inputsrc.isblock && forth_inputsrc.blk != 0) {
      /* if loading from a block, print block number and line */
      Psl("block "); Pu8(forth_inputsrc.blk); Psl(" line ");
      /* line number has already been advanced, subtract 1 */
      Pu8(forth_inputsrc.blkline-1); Psl(": ");
    }
    mio_putc('`');
    forth_inputpos = forth_last_word_pos;
    print_fstr(forth_parse_name());
    mio_putc('`');
    mio_bl();
    if (err == FE_ABORTQ) {
      print_fstr(abortq_errmsg); Pnl();
    } else {
      PGM_P errmsg = forth_error_str(err);
      if (errmsg) { PSp(errmsg); } else { Psl("error "); Pd16(err); Pnl(); }
    }
  }
  /* erase partially-compiled word if necessary */
  if (in_compile_mode()) {
    dict_restore();
    forth_flags.state = FORTH_STATE_INTERPRETING;
  }
  /* clear data and return stacks, then QUIT */
  if (using_console()) { fcon.slow = 0; }
  forth_outer_interpreter();
}


void forth_outer_interpreter(void);

MAKE_CONSOLE_MIO_HANDLERS_SC(fcon, extern);
void forth_40x25_console(void);

bool using_console(void) {
  return (mio_putc == fcon_mio_putc) && (mio_getc == fcon_mio_getc);
}

/* Reset interpreter state, but do not clear name and code spaces. */
void forth_warm_start(void) {
  forth_base = 10;
  forth_flags.editing = 0;
  forth_flags.singlestep = 0;
  forth_flags.trace = 0;
  forth_flags.state = FORTH_STATE_INTERPRETING;
  forth_outer_interpreter();
  __builtin_unreachable();
}


/* Clear RAM, clear code space and name space, and reset interpreter state. */
void forth_cold_start(void) {
  spi_init();
  memset(forth_mem, 0, forth_mem_end-forth_mem);
  dict_init();
  if (using_console()) {
    forth_40x25_console();
  }
  PSl(PRODUCT_NAME_STR " " VERSION_STR " " COPYRIGHT_STR);
  Psl(XSTR_(TOTAL_MEM_KB) "K system, ");
  Pu16(forth_available_memory()); PSl(" bytes free");
  forth_warm_start();
  __builtin_unreachable();
}


APP_MAIN(forth)
{
  CONSOLE_MIO_ACTIVATE(fcon);
  forth_cold_start();
  while (1) {}
}


APP_ALIAS(forth,forth_serial)
{
  serial_init((struct serial_port){
    SERIAL_BAUD_57600,
    SERIAL_PARITY_NONE,
    SERIAL_DATA_BITS_8,
    SERIAL_STOP_BITS_1
  });
  SERIAL_MIO_SETUP();
  forth_cold_start();
  while (1) {}
}
