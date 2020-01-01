#include "forth.h"
#include "minio.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

extern void forth_finalize_numeric_output(void);

void forth_spaces(uint16_t n) {
  while (n--) { mio_bl(); }
}


/* this implements the Forth word HOLD */
void forth_hold(char c) {
  forth_hld0[forth_hld++] = c;
}


/* this implements the Forth word HOLDS */
void forth_holds(forth_string str) {
  while (str.len) {
    forth_hld0[forth_hld++] = str.addr[--str.len];
  }
}


/* this implements the Forth word # */
uint32_t forth_extract_digit(uint32_t n) {
  /* gcc knows to combine these two into one __udivmodsi4 call */
  uint32_t nn = n / forth_base;
  uint8_t digit = n % forth_base;
  forth_hold((digit < 10) ? '0'+digit : 'A'+(digit-10));
  return nn;
}


/* this implements the Forth word #S */
uint32_t forth_extract_digits(uint32_t n) {
  do {
    n = forth_extract_digit(n);
  } while (n);
  return n;
}


static void forth_right_align(cell width) {
  /* don't need to do anything if output is larger than field width */
  if (forth_hld >= width) {
    return;
  }
  /* otherwise, output padding */
  forth_spaces(width-forth_hld);
}


void forth_uddot(int32_t n, cell width) {
  forth_hld = 0;
  forth_extract_digits(n);
  forth_right_align(width);
  forth_finalize_numeric_output();
  mio_putns(forth_hld0, forth_hld); Pbl();
}


/* print byte pair */
void forth_lhdot(uint16_t bytepair) {
  forth_hld = 0;
  forth_extract_digits((bytepair>>8)&0xFF);
  forth_hold('^');
  forth_extract_digits(bytepair&0xFF);
  forth_finalize_numeric_output();
  mio_putns(forth_hld0, forth_hld); Pbl();
}


void forth_ddot(int32_t n, cell width) {
  if (n >= 0) {
    forth_uddot(n, width);
  } else {
    forth_hld = 0;
    n = -n;
    forth_extract_digits(n);
    forth_hold('-');
    forth_right_align(width);
    forth_finalize_numeric_output();
    mio_putns(forth_hld0, forth_hld); Pbl();
  }
}


void forth_dot(cell n, cell width) {
  int16_t s = (int16_t)n;
  forth_ddot((int32_t)s, width);
}


void forth_udot(cell n, cell width) {
  forth_uddot((uint32_t)n, width);
}


void forth_hdot(cell n, cell width) {
  (void)width; /* unused, always 4 digits */
  Px16(n); Pbl();
}


static bool forth_char_is_digit(char c, uint8_t base)
{
  /* handle both upper and lower cases */
  c = toupper(c);
  if (c < '0') { return false; }
  if (base <= 10) {
    return (c < '0'+base);
  } else {
    return (c <= '9') || (c >= 'A' && c < ('A'+(base-10)));
  }
}


/* assumes c is a valid digit in this base */
static uint8_t forth_char_to_digit(char c)
{
  c = toupper(c);
  if (c <= '9') {
    return c-'0';
  } else {
    return c-'A'+10;
  }
}


forth_string forth_to_number(forth_string token, double_cell *value)
{
  bool isbytepair = false; uint8_t lowbyte = 0;
  while (token.len) {
    char c = *token.addr;
    /* caret '^' indicates that this is now a byte pair: */
    /* value currently parsed will now become the low byte */
    /* only the next 8 bits will be recognized */
    /* a second caret will abort parsing */
    if (c == '^') {
      if (isbytepair) { break; }
      token.addr++;
      token.len--;
      lowbyte = (value->full & 0xFF);
      isbytepair = true;
      value->full = 0;
    }
    else if (!forth_char_is_digit(c, forth_base)) {
      break;
    } else {
      value->full = ((value->full)*forth_base) + forth_char_to_digit(c);
      token.addr++;
      token.len--;
    }
  }
  if (isbytepair) {
    value->high = 0;
    value->low <<= 8;
    value->low |= lowbyte;
  }
  return token;
}


/**
 * Write a Forth string (address/length pair) to the output device.
 */
void print_fstr(forth_string str)
{
  __auto_type fn = (str.inrom) ? mio_putns_P : mio_putns;
  fn(str.addr, str.len);
}


static char ascii_to_xdigit(char c) {
  if      (c <= '9') { return c-'0'; }
  else if (c <= 'F') { return 10+(c-'A'); }
  return 10+(c-'a');
}


/* Parse a string containing escape sequences from the input buffer. */
/* Modifies the input buffer in-place. */
forth_string parse_escaped_string(void)
{
  char *inbuf = forth_inputbuf();
  forth_string str = { .addr=inbuf+forth_inputpos, .len=0 };
  char *p = str.addr;
  /* Gather characters until we find an unescaped quote or run out of chars. */
  while (forth_inputpos < forth_inputlen) {
    char c = inbuf[forth_inputpos++];
    if (c == '\"') {
      break;
    } else if (c == '\\') {
      if (forth_inputpos >= forth_inputlen) { break; }
      c = inbuf[forth_inputpos++];
      switch (c) {
        case 'a':  c = 7;  break;
        case 'b':  c = 8;  break;
        case 'e':  c = 27; break;
        case 'f':  c = 12; break;
        case 'm': /* CR and LF */
          *p++ = 13; str.len++;
          __attribute__((fallthrough));
        case 'l': case 'n':  c = 10; break;
        case 'q': case '\"': c = 34; break;
        case 'r':            c = 13; break;
        case 't':            c = 9;  break;
        case 'v':            c = 11; break;
        case 'z':            c = 0;  break;
        case '\\':           c = 92; break;
        case 'x': {
          if (forth_inputpos >= forth_inputlen) { continue; }
          char c1 = inbuf[forth_inputpos++];
          if ((!isxdigit(c1)) || forth_inputpos >= forth_inputlen) { continue; }
          char c2 = inbuf[forth_inputpos++];
          if (!isxdigit(c2)) { continue; }
          c = (ascii_to_xdigit(c1)<<4) | ascii_to_xdigit(c2);
          break;
        }
        default:
          break;
      }
      *p++ = c; str.len++;
    } else {
      *p++ = c; str.len++;
    }
  }
  return str;
}
