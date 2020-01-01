/**
 * Macros to simplify definition of raster interrupt routines.
 *
 * Raster interrupt routines must be written in assembler and do not follow the
 * standard calling convention. Specifically:
 * - r2, ZL, and ZH may be clobbered freely
 * - All other registers MUST be saved and restored
 * - SREG must be saved and restored if using instruction that modify flags
 * - Must end with `jmp raster_int_end` instead of `ret`/`reti`
 *
 * These macros aim to make it easier to define simple raster interrupt routines
 * without having to dive into the specifics of assembly language.
 */

#ifndef RASTER_INTERRUPT_H_
#define RASTER_INTERRUPT_H_

#include "videomodes.h"

/* Defines (or declares) a raster interrupt function. */
#define RASTER_INTERRUPT(fn) __attribute__((naked)) void fn(void)

/* Proper exit point for a raster interrupt function. */
#define RI_RET() \
  asm volatile("jmp raster_int_end\n")

/* Loads `val` into the given register. `val` may be a compile-time constant */
/* (in which case, an `ldi` instruction will be used) or a variable (in which */
/* case, an `lds` instruction will be used). */
#define _RI_LDX(reg,val) \
  __builtin_choose_expr(__builtin_constant_p(val), \
    ({asm volatile("ldi " #reg ", %[value]\n" :: [value] "M" ((val))); 0;}), \
    ({asm volatile("lds " #reg ", %[var]\n" :: [var] "m" ((val))); 0;}))

#define _RI_STS(var,reg) \
  asm volatile("sts %[addr], " #reg "\n" :: [addr] "m" ((var)) : "memory")

#define _RI_OUT(var,reg) \
  asm volatile("out %[addr], " #reg "\n" :: [addr] "m" (_SFR_IO_ADDR(var)) : "memory")

/* -------------------- 8-bit loads/stores -------------------- */

/* Sets the 8-bit variable `dst` to the given value. */
#define RI_SET8(dst,val) \
  _RI_LDX(r30, val); \
  _RI_STS(dst, r30)

/* Sets the I/O register `ioreg` to the given value. */
#define RI_IOSET8(ioreg,val) \
  _RI_LDX(r30, val); \
  _RI_OUT(ioreg, r30)



/* -------------------- 16-bit loads/stores -------------------- */

/* Sets the 16-bit variable `dst` to the given value. */
#define RI_SET16(dst,c) \
  _RI_LDX(r30, lo8(val)); \
  _RI_STS(dst, r30); \
  _RI_LDX(r30, hi8(val)); \
  _RI_STS(dst, r30);

/* Sets the 16-bit variable `dstvar` to the given function address `fn`. */
/* `fn` is interpreted as a 16-bit memory address in program space. */
#define RI_SETFPTR(dstvar, fn) \
  asm volatile( \
    "ldi r30, pm_lo8(" #fn ")\n" \
    "sts " #dstvar ", r30\n" \
    "ldi r30, pm_hi8(" #fn ")\n" \
    "sts " #dstvar "+1, r30\n" \
  ::: "r30", "memory") \

/* Sets the 16-bit variable `dstvar` to the given address `addr`. */
/* `addr` is interpreted as a 16-bit memory address in data space. */
#define RI_SETPTR(dstvar, addr) \
  asm volatile( \
    "ldi r30, lo8(" #addr ")\n" \
    "sts " #dstvar ", r30\n" \
    "ldi r30, hi8(" #addr ")\n" \
    "sts " #dstvar "+1, r30\n" \
  ::: "r30", "memory") \



/* -------------------- Video-specific setters -------------------- */

/* Sets the line number and address of the next raster interrupt. */
#define RI_SET_NEXT_HANDLER(line,fn) \
  RI_SET8(raster_int_scanline, line); \
  RI_SETFPTR(raster_int, fn)

/* For text modes: sets the text color to the given value. */
#define RI_SET_TEXT_COLOR(c)    RI_SET8(globalcolor, c)

/* For text/tiled modes: sets the vertical scroll offset. */
#define RI_SET_VSCROLL(n)     RI_SET8(vscroll,n)

/* Sets the current video pointer (the address of the screen contents to be */
/* drawn on the next line). Takes effect at the start of the next line. */
#define RI_SET_VPTR(ptr) \
  asm volatile( \
    "ldi r30, lo8(%[val])\n" \
    "ldi r31, hi8(%[val])\n" \
    "movw r4, r30\n" \
  :: [val] "i" (ptr) : "r4", "r5", "r30", "r31")

/* Sets the base address of the screen contents. (i.e. the framebuffer for */
/* bitmap modes and the nametable for text/tiled modes) */
/* Takes effect at the start of the next frame. */
#define RI_SET_SCREEN_PTR(ptr) RI_SETPTR(screen_ptr, ptr)

/* Sets the address of the pattern table (tilemap) used by text/tiled modes. */
#define RI_SET_TILEMAP(ptr) \
  asm volatile( \
    "ldi r30, hi8(" #ptr ")\n" \
    "sts tilemap_hi, r30\n" \
    "ldi r30, hh8(" #ptr ")\n" \
    "sts tilemap_hh, r30\n" \
  ::: "r30", "memory")

/* Sets the linehandler function. Takes effect at the start of the next line. */
#define RI_SET_LINEHANDLER(fn) \
  asm volatile( \
    "ldi r30, lo8(%[fptr])\n" \
    "out %[loport], r30\n" \
    "ldi r30, hi8(%[fptr])\n" \
    "out %[hiport], r30\n" \
  :: [fptr] "m" (fn), [loport] "I" (_SFR_IO_ADDR(linehandler_lo)), [hiport] "I" (_SFR_IO_ADDR(linehandler_hi)))

/* Sets the video mode mid-screen, for split-screen applications. */
/* The _FAST version saves a few cycles and can be used if all modes onscreen */
/* use the same shift register mode/period. */
#define RI_SET_VIDEO_MODE_FAST(mode) \
  RI_SET_LINEHANDLER(mode##_LINEHANDLER); \
  RI_SET8(OCR1A, OCR1A_VALUE_FOR_XOFFSET(mode##_XOFFSET));

/* Sets the video mode, regardless of previous video mode settings. */
/* (i.e. makes no assumptions about shift register mode and period) */
#define RI_SET_VIDEO_MODE(mode) \
  RI_SET8(OCR2A, OCR2A_VALUE_FOR_SRPERIOD(mode##_SRPERIOD)); \
  RI_SET8(bytesperline, mode##_BYTESPERLINE); \
  if (mode##_FORMAT) { RI_SET8(tilemap_hh, mode##_FORMAT); } \
  SET_SHIFT_REGISTER_MODE(mode##_SRMODE); \
  RI_SET_VIDEO_MODE_FAST(mode);

/* Like ATOMIC_BLOCK(), declares a block that saves SREG on entry and */
/* restores it on exit. Must be used when performing any computations in a */
/* raster interrupt that modify flags. (arithmetic, comparison, etc.) */
/* The previous SREG value is saved in the reserved register r2. */
#define RI_WITH_SAVED_SREG() \
  for (uint8_t __attribute__((__cleanup__(__ri_restore_sreg))) __dummy = __ri_save_sreg(); __dummy; __dummy=0)

/* Saves SREG in r2, which is free to use as a temporary in raster interrupts. */
#define RI_SAVE_SREG()    asm volatile("in r2, %[sreg]" :: [sreg] "I" (_SFR_IO_ADDR(SREG)) : "r2", "memory")

/* Restores the saved SREG value in r2. */
#define RI_RESTORE_SREG() asm volatile("out %[sreg], r2" :: [sreg] "I" (_SFR_IO_ADDR(SREG)) : "memory")

static inline void __ri_restore_sreg() {
  RI_RESTORE_SREG();
}

static inline uint8_t __ri_save_sreg(void) {
  RI_SAVE_SREG(); return 1;
}

#endif
