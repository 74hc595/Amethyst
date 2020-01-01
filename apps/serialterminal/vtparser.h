/**
 * VTxxx terminal escape sequence parsing state machine.
 * This aims to be a mostly-complete implementation of Paul Flo Williams'
 * state diagram:
 *   https://www.vt100.net/emu/dec_ansi_parser [1]
 *
 * This parser has the following limitations:
 *  - Only one intermediate character (the "private marker") is stored per
 *    CSI escape sequence.
 *  - Device Control String and Operating System Command sequences are not
 *    supported, and this implementation currently consumes them without taking
 *    any action.
 *
 * See also: "Minimum requirements for VT100 emulation"
 *   http://www.inwap.com/pdp10/ansicode.txt [2]
 */
#ifndef VTPARSER_H_
#define VTPARSER_H_

#include <stdint.h>
struct vt_parser_state;

typedef void (*vt_parser_hook)(uint8_t c);

/* The specs recommend support for a minimum of 16 parameter values in a */
/* CSI sequence. If a smaller memory footprint is desired, this may be */
/* lowered. */
#ifndef VT_PARSER_MAX_PARAMS
#define VT_PARSER_MAX_PARAMS  16
#endif

/* There's no official spec indicating the acceptable range of numeric */
/* parameters. [1] recommends 16383 as a maximum, but [2] recommends 255. */
/* On 8-bit and memory-constrained systems, two bytes per parameter may */
/* be excessive. If so, define VT_PARSER_SMALL_PARAMS to 1, and the parser */
/* will reserve only one byte for each parameter. */
/* Regardless of whether 8-bit or 16-bit parameters are used, there is no */
/* checking for integer overflow/wraparound. */
#ifndef VT_PARSER_SMALL_PARAMS
#define VT_PARSER_SMALL_PARAMS 1
#endif


#if VT_PARSER_SMALL_PARAMS
typedef int8_t vt_param;
#else
typedef int16_t vt_param;
#endif

struct vt_parser {
  const struct vt_parser_state *current;
  vt_param params[VT_PARSER_MAX_PARAMS];
  uint8_t num_params;
  uint8_t intermediate_char; /* also stores the private marker */
  /* User-supplied handlers */
  vt_parser_hook print;
  vt_parser_hook execute_c0_c1;
  vt_parser_hook execute_esc;
  vt_parser_hook execute_csi;
};

void vt_parser_init(struct vt_parser *vtp);

void vt_parser_process(struct vt_parser *vtp, uint8_t c);

vt_param vt_parser_get_param(struct vt_parser *vtp, uint8_t param_index, vt_param default_value);

#endif