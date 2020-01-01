/**
 * defs.h
 * Global defines.
 */
#ifndef DEFS_H_
#define DEFS_H_

/* yes, this works with -std=gnu11 */
#define auto __auto_type

/* Just an abbreviation to keep asm code narrow. */
#define IO_ _SFR_IO_ADDR

#ifndef __ASSEMBLER__

/* thank u chrome */
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#include <stdint.h>
#define PAGE_ALIGNED __attribute__((aligned(256)))
typedef PAGE_ALIGNED uint8_t *page_aligned_ptr;
typedef const PAGE_ALIGNED uint8_t *const_page_aligned_ptr;

/* those golden oldies */
#define XPASTE_(x,y)  x##y
#define PASTE_(x,y)   XPASTE_(x,y)
#define STR_(x)  #x
#define XSTR_(x) STR_(x)

/* assembler provides these as relocation modifiers, */
/* but they're handy from C as well */
#define ptr_to_int(n) ((uint16_t)(n))
#define lo8(n)        (ptr_to_int(n)&0xFF)
#define hi8(n)        lo8(ptr_to_int(n)>>8)

/* as above, but for 24-bit __memx pointers */
#define xptr_to_int(n) ((__uint24)(n))
#define xlo8(n)        (xptr_to_int(n)&0xFF)
#define xhi8(n)        lo8(xptr_to_int(n)>>8)
#define xhh8(n)        lo8(xptr_to_int(n)>>16)

/* a matter of style */
#define set_bits(n,b)   ((n) |= (b))
#define clear_bits(n,b) ((n) &= ~(b))
#define set_bit(n,b)    set_bits(n,_BV(b))
#define clear_bit(n,b)  clear_bits(n,_BV(b))

#endif  /* !__ASSEMBLER__ */


/* Hardware versions. */

/* First prototype; 43 keys, no number keys */
#define AMETHYST_HW_1_0       0x0100
/* Second prototype: 47 keys, number keys added */
#define AMETHYST_HW_1_1       0x0101

#define AMETHYST_HW_LATEST    AMETHYST_HW_1_1

#ifndef HW_VERSION
#define HW_VERSION            AMETHYST_HW_LATEST
#endif

#endif  /* DEFS_H_ */
