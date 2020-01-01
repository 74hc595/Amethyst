/* included by both C and assembly */
#ifndef FORTH_DEFS_H_
#define FORTH_DEFS_H_

#include "error_codes.h"

/* Bits in the forth_flags byte. */
#define FF_SINGLESTEP_BIT 5
#define FF_TRACE_BIT      6
#define FF_STATE_BIT      7
#define FORTH_STATE_MASK  (1<<FF_STATE_BIT)
/* STATE value. */
#define FORTH_STATE_INTERPRETING  false
#define FORTH_STATE_COMPILING     true
#define in_compile_mode()         (forth_flags.state != FORTH_STATE_INTERPRETING)
#define in_interpret_mode()       (!in_compile_mode())

#define FL_MASK           0b11100000
#define FL_SHIFT          5
#define FL_VARIABLE       (0<<FL_SHIFT)
#define FL_COLON          (1<<FL_SHIFT)
#define FL_CHILD          (2<<FL_SHIFT)
#define FL_CONSTANT       (3<<FL_SHIFT)
#define FL_2CONSTANT      (4<<FL_SHIFT)
#define FL_COMPILER       (5<<FL_SHIFT)
#define FL_COMPILER_CHILD (6<<FL_SHIFT)
#define FL_COMBINED       (7<<FL_SHIFT) /* aka "non-default compilation semantics" */

#define FL_IS_INLINE(f)   ((f)>=FL_CONSTANT)

/* if the flags argument to namespace_create() has this bit set, the new word */
/* will be created with the smudge bit set, making it invisible */
#define FL_CREATE_SMUDGED   1

/* if the flags argument to namespace_create() has this bit set, snapshot the */
/* stack pointers (to be checked upon exiting compilation mode */
#define FL_SAVE_STACK_PTRS  2

#define HIGHADDR(addr)      lo8((addr)-(((RAMEND+1)-0x100)))

#define FORTH_EVALUATE_MAX_LEN  255
#define FORTH_HOLD_BUFFER_SIZE  34  /* capacity of pictured numeric output buffer; minimum required by standard */
#define FORTH_PAD_SIZE          84  /* to comply with ANS */
#define MAX_ROMDICT_ENTRY_SIZE  8

/* Most Forth systems use 1024-byte blocks. */
/* We use 512-byte blocks (arranged as 32 characters by 16 rows), so that an */
/* entire block can fit on a 40-column screen, and because this system doesn't */
/* have much memory in the first place. (It also matches the sector size of */
/* most mass storage devices, like SD cards.) */
#define BLOCK_BYTES_PER_LINE    32
#define LINES_PER_BLOCK         16
#define BLOCK_SIZE              ((BLOCK_BYTES_PER_LINE)*(LINES_PER_BLOCK))
#define NUM_EEPROM_BLOCKS       ((E2END+1)/BLOCK_SIZE)

/* How many blocks are available on external media? */
#define NUM_XMEM_BLOCKS         (65536/BLOCK_SIZE)
#define NUM_BLOCKS              (NUM_EEPROM_BLOCKS+NUM_XMEM_BLOCKS)

/* for simavr */
#define TRACE(c) \
asm volatile( \
"ldi r30, %[value]\n" \
"out 0x1c, r1\n" :: [value] "M" ((c)) : "r30")

#endif
