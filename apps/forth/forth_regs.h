#ifndef FORTH_REGS_H_
#define FORTH_REGS_H_

#ifdef __AVR__
#include <avr/io.h>
#endif

/* Used as a 16-bit zero register. */
#define ZEROL r0
#define ZEROH r1
#define ZERO  ZEROL

/* Used for temporary values. */
/* Also used as the second argument when calling C functions. */
#define TMPL  r22
#define TMPH  r23
#define TMP   TMPL

/* A common optimization: use a register to hold the top-of-stack value. */
/* Also used as the return value and first argument when calling C functions. */
#define TOSL  r24
#define TOSH  r25
#define TOS   TOSL

/* The virtual machine's instruction pointer. */
#define IPL   r26
#define IPH   r27
#define IP    X

/* Data stack pointer. */
#define DSPL  r28
#define DSPH  r29
#define DSP   Y

/* Dispatch table pointer/instruction register. */
#define IR    r30 /* holds the most recently fetched instruction byte */
#define DTH   r31 /* high byte of dispatch table address */
#define CDTH  r17 /* cached value of current dispatch table high byte */

/* Return stack pointer. (Use the hardware stack pointer.) */
#define RSPL  _SFR_IO_ADDR(SPL)
#define RSPH  _SFR_IO_ADDR(SPH)
#define RSP   SPL

/* 16 bit "true" constant. (0xFFFF) */
#define TRUEL r8
#define TRUEH r9
#define TRUE  TRUEL

/* Used to save TOS when calling C functions. */
#define TSAVL r10
#define TSAVH r11
#define TSAV  TSAVL

/* Used to save IP when calling C functions. */
#define XSAVL r12
#define XSAVH r13
#define XSAV  XSAVL

/* Temporary register. Its value may be clobbered by an exception. */
#define LINKL r14
#define LINKH r15

#define HIGH_PAGE_BASE_ADDR 0x4000

#endif
