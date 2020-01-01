; Most Forths use a single dictionary that mixes headers (names, link pointers,
; flags, etc.) with contents (code, buffers, etc.)
;
; This implementation separates them. The "code space" is for compiled code,
; buffers, variables, etc. It contains no metadata. The "name space" contains
; word information like names, flags, and code to be executed in compile mode
; only. Basically, this allows the entire name space to be moved or erased
; without affecting the execution of the code in the code space.
;
; Code space begins at the bottom of SRAM. Name space begins after the bytecode
; area. The initial size of the code space is 1 KB. When filled, the entire
; name space is moved ahead to the next 256-byte block. Since name space entries
; use relative offsets for their links, the contents of name space are
; position-independent. The contents of the name space can be moved to any
; location in memory without having to modify them.
;
; "The dictionary" refers to the combined contents of name space and code space.
; The traditional Forth dictionary words (ALLOT HERE , C, DP etc.) operate
; within code space, since this is where the majority of a program's contents
; (in fact, the *entirety* of a program's contents; the name space contains
; only metadata and definitions relating to the compilation process; it can be
; discarded without affecting the runtime behavior of the code.

#include "forth_defs.h"
#include "console.h"

.section .bss

; Memory for general Forth use.
; (bytecode, variables, buffers, dictionary)
.global forth_mem
forth_mem:
.global forth_cp0
.global forth_pmax
forth_cp0:
.space 16078
forth_pmax:

; Data stack.
.global forth_spmax
.global forth_sp0
forth_spmax:
.space 64
forth_sp0:


; Forth system variables.

; Pointer to the next free byte in name space.
; (Also a pointer to the start of the pictured numeric output buffer.)
.global forth_np
.global forth_hld0
forth_np:
forth_hld0:
.space 2

; Boundary between code space and name space.
.global forth_np0
forth_np0:
.space 2

; Upper boundary of name space.
.global forth_npmax
forth_npmax:
.space 2

; Pointer to the start of the most recently defined word in name space.
.global forth_latest
forth_latest:
.space 2

; Pointer to the next free byte in code space.
.global forth_cp
forth_cp:
.space 2

; State saved when entering compile mode. Used to check for control structure
; imbalance and delete partial dictionary entries.
.global forth_saved_dsp
.global forth_saved_rsp
.global forth_saved_np
.global forth_saved_np0
.global forth_saved_latest
.global forth_saved_cp
forth_saved_dsp:    .space 2
forth_saved_rsp:    .space 2
forth_saved_np:     .space 2
forth_saved_np0:    .space 2
forth_saved_latest: .space 2
forth_saved_cp:     .space 2

; Input source.
; Order of these fields is important.
.global forth_inputsrc
.global forth_inputlen
.global forth_inputpos
forth_inputsrc: ; input string address, or block number
.space 2
forth_inputlen: ; Total size of the current input buffer
.space 1
forth_inputpos: ; Current parsing position in the current input buffer
.space 1

.global forth_last_word_pos
forth_last_word_pos:
.space 1

; Index of the next free byte in the pictured numeric output buffer.
.global forth_hld
forth_hld:
.space 1

; Radix for number<->string conversions.
.global forth_base
forth_base:
.space 1

; Character console.
.global fcon
fcon:
.space sizeof_console
#define DEFINE_FCON_FIELD(field) .global fcon_##field $ .equ fcon_##field, fcon+offsetof_console_##field
#define DEFINE_FCON_FIELD_BITMASK(field) .global fcon_bitmask_##field $ .equ fcon_bitmask_##field, bitmask_console_##field
DEFINE_FCON_FIELD(cx)
DEFINE_FCON_FIELD_BITMASK(cx)
DEFINE_FCON_FIELD(cy)
DEFINE_FCON_FIELD_BITMASK(cy)
DEFINE_FCON_FIELD(mleft)
DEFINE_FCON_FIELD_BITMASK(mleft)
DEFINE_FCON_FIELD(mtop)
DEFINE_FCON_FIELD_BITMASK(mtop)
DEFINE_FCON_FIELD(mright)
DEFINE_FCON_FIELD_BITMASK(mright)
DEFINE_FCON_FIELD(mbottom)
DEFINE_FCON_FIELD_BITMASK(mbottom)
DEFINE_FCON_FIELD(slow)
DEFINE_FCON_FIELD_BITMASK(slow)
DEFINE_FCON_FIELD(rvs)
DEFINE_FCON_FIELD_BITMASK(rvs)
DEFINE_FCON_FIELD(color)
DEFINE_FCON_FIELD_BITMASK(color)
DEFINE_FCON_FIELD(cursorphase)
DEFINE_FCON_FIELD_BITMASK(cursorphase)


; ROM word buffer.
.global forth_rom_word_buf
forth_rom_word_buf:
.space MAX_ROMDICT_ENTRY_SIZE+1

; Core runtime variables.

; Location of the current exception frame on the return stack
; If bit 15 is 1, there is no exception frame, and the global exception handler
; will be invoked.
; The layout of an exception frame is:
; .  .  .  .  .  .  .  .  .  .
; |  optional context data   |
; |      (0+ words)          |
; +--------------------------+
; |   addr of handler code   |
; +--------------------------+
; |  previous exn frame ptr  |
; +--------------------------+ <-- RSP

.global forth_exception_frame
forth_exception_frame:
.space 2

; Flags, including STATE, whether the editor is running, and debug options.
.global forth_flags
forth_flags:
.space 1

; Number of last block LISTed or EDITed
.global forth_scr
forth_scr:
.space 1

; Terminal input buffer.
.global forth_tib
forth_tib:
.space 80
; Upper 32 bytes of the TIB are used by LOAD.
.global forth_blockbuf
.equ forth_blockbuf, .-BLOCK_BYTES_PER_LINE

; -------------- A cold start zeros all RAM below this address -------------- ;
.global forth_mem_end
forth_mem_end:

; Return stack.
; The Forth VM uses the AVR hardware stack pointer as RSP, and the AVR stack is
; empty-descending, i.e. it points to the first free location on the stack.
; A push is a store and a post-decrement.
; Hence we define rp0 to point to the last byte in this block of space.
.global forth_rpmax
.global forth_rp0
forth_rpmax:
.space 72
.equ forth_rp0, .-1

.global forth_tiny_rp0
.global forth_tiny_sp0
.global forth_tiny_mem_end
.equ forth_tiny_rp0, forth_rp0
.equ forth_tiny_sp0, forth_tiny_rp0+1-16
.equ forth_tiny_mem_end, forth_tiny_sp0-16

.global forth_rstack_cells
.equ forth_rstack_cells, (forth_rp0-forth_rpmax+1)>>1
.global forth_dstack_cells
.equ forth_dstack_cells, (forth_sp0-forth_spmax)>>1
