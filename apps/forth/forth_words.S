#include "forth_opcodes.h"
#include "forth_macros.inc"
#include "forth_defs.h"

.macro dictheader, name
.byte . - lastentry
.set lastentry, .-1
.byte (9f-8f)
8:
.ascii "\name"
9:
.endm

; "Normal" word.
; Interpret: perform execution semantics
; Compile:   compile execution semantics
.macro dictentry_colon, name
dictheader "\name"
.word pm(romdict_interpret_bytecode)
.word pm(romdict_compile_bytecode)
.byte (7f-6f)
6:
.endm

; "Compile-only" word.
; Interpret: error
; Compile:   perform execution semantics
.macro dictentry_compileonly, name
dictheader "\name"
.word pm(err_compile_only)
.word pm(romdict_interpret_bytecode)
.byte (7f-6f)
6:
.endm

; A "normal" word that uses the return stack, and thus needs to save the return
; address in a temporary link register.
; Interpret: perform execution semantics
; Compile:   compile execution semantics
.macro dictentry_colon_rstack, name
dictheader "\name"
.word pm(romdict_interpret_bytecode_linkcall)
.word pm(romdict_compile_bytecode)
.byte (7f-6f)
6:
.endm

; A "compile-only" word that uses the return stack, and thus needs to save the
; return address in a temporary link register.
; Interpret: error
; Compile:   perform execution semantics
.macro dictentry_compileonly_rstack, name
dictheader "\name"
.word pm(err_compile_only)
.word pm(romdict_interpret_bytecode_linkcall)
.byte (7f-6f)
6:
.endm

; "Immediate" word.
; Interpret: perform execution semantics
; Compile:   perform execution semantics
.macro dictentry_immediate, name
dictheader "\name"
.word pm(romdict_interpret_bytecode)
.word pm(romdict_interpret_bytecode)
.byte (7f-6f)
6:
.endm

; "Interpret-only" word.
; Interpret: perform execution semantics
; Compile:   error
.macro dictentry_interpretonly, name
dictheader "\name"
.word pm(romdict_interpret_bytecode)
.word pm(err_interpret_only)
.byte (7f-6f)
6:
.endm

.macro dictentryend
7:
.if (7b-6b) > MAX_ROMDICT_ENTRY_SIZE
.error "ROM dictionary entry is too long"
.endif
.endm

.macro dictentry_interpretation_semantics
.byte (7f-6f)
6:
.endm

.macro dictentry_compilation_semantics
7:
.if (7b-6b) > MAX_ROMDICT_ENTRY_SIZE
.error "ROM dictionary entry is too long"
.endif
.byte (7f-6f)
6:
.endm

.macro cfunc_semantics, isem, csem
.word pm(\isem)
.word pm(\csem)
6:
.endm

.macro cfunc_immediate, fn
cfunc_semantics \fn, \fn
.endm

.macro cfunc_compile_only fn
cfunc_semantics err_compile_only, \fn
.endm

.macro cfunc_interpret_only fn
cfunc_semantics \fn, err_interpret_only
.endm

; macros for defining words in ROM
.macro op, o
.byte OP_\o
.endm

.macro opx, o
.byte OP_ext
.byte OPX_\o
.endm

.macro l8, n
.byte OP_lit8, \n
.endm

.macro l16, lsb, msb
.byte OP_lit16, \lsb, \msb
.endm

.macro laddr, addr
.byte OP_lit16, lo8(\addr), hi8(\addr)
.endm

.macro ccall, fn
op ccall
.word pm(\fn)
.endm

.macro enter_compile_mode
op minus1
op cstorebitshi
.byte FORTH_STATE_MASK
.byte HIGHADDR(forth_flags)
.endm

.macro exit_compile_mode
op zero
op cstorebitshi
.byte FORTH_STATE_MASK
.byte HIGHADDR(forth_flags)
.endm

; export the name token of a ROM word
.macro export_xt, name
.global xt_of_\name
.equ xt_of_\name, 0xC000+.-romdict
.endm

; convert LFA to NFA
.global lfa_to_nfa
lfa_to_nfa:
        adiw    r24, 1
        ret

; get flags
.global ramdict_lfa_to_flags
ramdict_lfa_to_flags:
        adiw    r24, 1  ; get NFA
        movw    Z, r24
        ld      r24, Z  ; get flags
        andi    r24, 0b11100000 ; mask off name length
        ret

; set flags
.global ramdict_word_set_flags
ramdict_word_set_flags:
        adiw    r24, 1  ; get NFA
        movw    Z, r24
        ld      r24, Z  ; get flags/namelength
        andi    r24, 0b00011111 ; clear old flags
        andi    r22, 0b11100000
        or      r24, r22        ; store new flags
        st      Z, r24
        ret


; get name from LFA as a forth string (addr/len pair)
.global romdict_lfa_to_name
romdict_lfa_to_name:
        adiw    r24, 1          ; get NFA
        movw    Z, r24
        lpm     r24, Z+         ; get name length/flags
        andi    r24, 0b00011111 ; mask off flags
        ldi     r25, 0x80       ; set ROM bit
        movw    r22, Z          ; address of first character in name
        ret
.global ramdict_lfa_to_name
ramdict_lfa_to_name:
        adiw    r24, 1          ; get NFA
        movw    Z, r24
        ld      r24, Z+         ; get name length/flags
        andi    r24, 0b00011111 ; mask off flags
        clr     r25             ; clear ROM bit
        movw    r22, Z          ; address of first character in name
        ret

; convert LFA to code field address
; (ROM words only; RAM words don't have a code field)
.global romdict_lfa_to_cfa
romdict_lfa_to_cfa:
        adiw    r24, 1  ; get NFA
        movw    Z, r24
        lpm     ZL, Z   ; get name length/flags
        andi    ZL, 0b00011111  ; mask off flags
        inc     ZL ; add 1 to length so we skip the length/flags byte
        add     r24, ZL ; advance past name
        adc     r25, r1
        ret

; convert LFA to DFA (RAM words only)
.global ramdict_lfa_to_dfa
ramdict_lfa_to_dfa:
        adiw    r24, 1  ; get NFA
        movw    Z, r24
        ld      ZL, Z   ; get name length/flags
        andi    ZL, 0b00011111  ; mask off flags
        inc     ZL ; add 1 to length so we skip the length/flags byte
        add     r24, ZL ; advance past name
        adc     r25, r1
        ret

; convert LFA to LFA of next entry (returning NULL if last entry)
.global romdict_lfa_link
romdict_lfa_link:
        movw    Z, r24
        lpm     ZL, Z
1:      tst     ZL
        brne    2f
        clr     r24
        clr     r25
        ret
2:      sub     r24, ZL
        sbc     r25, r1
        ret

; convert LFA to LFA of next entry in RAM (returning NULL if last entry)
.global ramdict_lfa_link
ramdict_lfa_link:
        movw    Z, r24
        ld      ZL, Z
        rjmp    1b

.section .progmem.forth
.set lastentry, .
.global romdict
romdict:

; Default behavior for uninitialized child words.
dictentry_colon " "
dictentryend

; Addresses of AVR IO registers
; Requires about 1.5K of flash space, so this is optional.
; #include "hwregs.words"

; generate dictionary entries for standard opcodes
#undef X
#define X(opnum,str,props,asmid,enumid,visibility) \
.if ((visibility) & OP_VISIBLE) $ \
  .if ((visibility) & OP_COMPILEONLY) $ \
    .if ((visibility) & OP_USES_RSTACK) $ \
      dictentry_compileonly_rstack str $ .byte (opnum) $ dictentryend $ \
    .else $ \
      dictentry_compileonly str $ .byte (opnum) $ dictentryend $ \
    .endif $ \
  .else $ \
    .if ((visibility) & OP_USES_RSTACK) $ \
      dictentry_colon_rstack str $ .byte (opnum) $ dictentryend $ \
    .else $ \
      dictentry_colon str $ .byte (opnum) $ dictentryend $ \
    .endif $ \
  .endif $ \
.endif $
OPCODES

; generate dictionary entries for extended opcodes
#undef X
#define X(opnum,str,props,asmid,enumid,visibility) \
.if ((visibility) & OP_VISIBLE) $ \
.if ((visibility) & OP_COMPILEONLY) $ \
dictentry_compileonly str $ .byte OP_ext, (opnum) $ dictentryend $ \
.else $ \
dictentry_colon str $ .byte OP_ext, (opnum) $ dictentryend $ \
.endif $ \
.endif $
EXT_OPCODES



;-------------------------------------------------------------------------------
; Immediate words for interacting with the interpretaer
;-------------------------------------------------------------------------------

dictheader "COLD"
cfunc_immediate forth_cold_start
dictentryend

dictheader "WARM"
cfunc_immediate forth_warm_start
dictentryend



;-------------------------------------------------------------------------------
; Forth aliases for VM opcodes
;-------------------------------------------------------------------------------
dictentry_colon "FALSE"
op zero
dictentryend

dictentry_colon "TRUE"
op minus1
dictentryend

; this implementation can always emit, always return true
dictentry_colon "EMIT?"
op minus1
dictentryend

dictentry_colon "CELLS"
op twostar
dictentryend

dictentry_colon_rstack "I"
op rfetch
dictentryend

dictentry_colon "CELL+"
op twoplus
dictentryend

dictentry_colon "CHAR+"
op oneplus
dictentryend

dictentry_colon_rstack "UNLOOP"
op twordrop
dictentryend

dictentry_colon "NOT"
op eq0
dictentryend

dictentry_colon "D>S"
op drop
dictentryend

dictentry_colon "LFA>NFA"
op oneplus
dictentryend

dictentry_colon "NFA>LFA"
op oneminus
dictentryend

; S>DU ( n -- d ) unsigned expand single cell into double cell
; i.e. just push a zero
dictentry_colon "S>DU"
op zero
dictentryend

; COUNT ( addr1 -- addr2 u ) convert counted string to forth string
dictentry_colon "COUNT"
op cfetchplus
dictentryend

; alias for :NONAME
dictentry_colon ":["
opx noname
dictentryend

; does nothing (multiplies TOS by 1)
dictentry_colon "CHARS"
dictentryend

; does nothing (this implementation does not have any alignment restrictions)
dictentry_colon "ALIGN"
dictentryend

; does nothing (this implementation does not have any alignment restrictions)
dictentry_colon "ALIGNED"
dictentryend



;-------------------------------------------------------------------------------
; Inlined words; aliases for short VM opcode sequences (typically 2 or 3 bytes)
;-------------------------------------------------------------------------------
dictentry_colon "0<>"
op eq0
op invert
dictentryend

dictentry_colon "0<="
op gt0
op invert
dictentryend

dictentry_colon "0>="
op lt0
op invert
dictentryend

dictentry_colon "<>"
op eq
op invert
dictentryend

dictentry_colon "<="
op gt
op invert
dictentryend

dictentry_colon ">="
op lt
op invert
dictentryend

dictentry_colon "U<="
op ugt
op invert
dictentryend

dictentry_colon "U>="
op ult
op invert
dictentryend

dictentry_colon "M-"
op  negate
opx mplus
dictentryend

; I' ( -- n ) limit of current DO loop
dictentry_colon_rstack "I'"
op rfetchlitoffset
.byte 3
dictentryend

; J ( -- n ) index of outer DO loop
dictentry_colon_rstack "J"
op rfetchlitoffset
.byte 5
dictentryend

; J' ( -- n ) limit of outer DO loop
dictentry_colon_rstack "J'"
op rfetchlitoffset
.byte 7
dictentryend

dictentry_colon_rstack "K"
op rfetchlitoffset
.byte 9
dictentryend

dictentry_colon_rstack "K'"
op rfetchlitoffset
.byte 11
dictentryend

dictentry_colon "BL"
l8  ' '
dictentryend

dictentry_colon "SPACE"
op emitlit
.byte ' '
dictentryend

dictentry_colon "CR"
op emitlit
.byte '\n'
dictentryend

dictentry_colon "PAGE"
op emitlit
.byte '\f'
dictentryend

dictentry_colon "?"
op fetch
opx dot
dictentryend

dictentry_colon "U?"
op fetch
opx udot
dictentryend

dictentry_colon "C?"
op cfetch
opx dot
dictentryend

; .. ( n1 n2 -- ) print n1 followed by n2 using .
dictentry_colon ".."
op swap
opx dot
opx dot
dictentryend

; U.. ( u1 u2 -- ) print u1 followed by u2 using U.
dictentry_colon "U.."
op swap
opx udot
opx udot
dictentryend

; DH. ( ud1 -- ) print double-cell number as 8 hex digits (with leading zeros)
dictentry_colon "DH."
op hdot
op hdot
dictentryend

dictentry_colon "BLANK"
l8 ' '
op fill
dictentryend

dictentry_colon "ERASE"
op zero
op fill
dictentryend

dictentry_colon "<#"
op zero
.byte OP_cstorelithi, HIGHADDR(forth_hld)
dictentryend

; [ - exit compile mode. This is an immediate word.
dictentry_compileonly "["
exit_compile_mode
dictentryend

; ] - enter compile mode. This is (currently) an immediate word.
dictentry_immediate "]"
enter_compile_mode
dictentryend

; STATE ( -- flag ) leave true flag if in compile mode
dictentry_colon "STATE@"
op cfetchbitshi
.byte FORTH_STATE_MASK
.byte HIGHADDR(forth_flags)
dictentryend

dictentry_colon "BINARY"
l8 2
.byte OP_cstorelithi, HIGHADDR(forth_base)
dictentryend

dictentry_colon "DECIMAL"
l8 10
.byte OP_cstorelithi, HIGHADDR(forth_base)
dictentryend

dictentry_colon "HEX"
l8 16
.byte OP_cstorelithi, HIGHADDR(forth_base)
dictentryend

dictentry_colon "QUIT"
laddr FE_QUIT
op throw
dictentryend

dictentry_colon "ABORT"
op minus1
op throw
dictentryend

dictentry_colon "DEFER@"
opx tobody
op fetch
dictentryend

dictentry_colon "DEFER!"
opx tobody
op store
dictentryend

;-------------------------------------------------------------------------------
; These words provide access to what Forth traditionally calls "user variables"
;-------------------------------------------------------------------------------

; NOTE: this word returns a *byte* address, must use C@
dictentry_colon "BASE"
laddr forth_base
dictentryend

; NOTE: this word returns a *byte* address, must use C@
dictentry_colon ">IN"
laddr forth_inputpos
dictentryend

; NOTE: this word returns a *byte* address, must use C@
; value is a nonnegative offset from start of pictured numeric output buffer
dictentry_colon "HLD"
laddr forth_hld
dictentryend

dictentry_colon "LATEST"
laddr forth_latest
dictentryend

dictentry_colon "CP"
laddr forth_cp
dictentryend

; "DP" is a more standard Forth word, here it's a synonym for CP.
dictentry_colon "DP"
laddr forth_cp
dictentryend

dictentry_colon "CP0"
laddr forth_cp0
dictentryend

; "DP0" is a more standard Forth word, here it's a synonym for CP0
dictentry_colon "DP0"
laddr forth_cp0
dictentryend

dictentry_colon "NP0"
.byte OP_fetchlithi, HIGHADDR(forth_np0)
dictentryend

dictentry_colon "NP"
laddr forth_np
dictentryend

; Maximum RAM address available for name space. Memory past this point is
; reserved for the screen buffer, stacks, Forth runtime, and system.
dictentry_colon "NPMAX"
.byte OP_fetchlithi, HIGHADDR(forth_npmax)
dictentryend

dictentry_colon "RP0"
laddr forth_rp0
dictentryend

dictentry_colon_rstack "RP@"
.byte OP_fetchlit, lo8(RSP), hi8(RSP)
dictentryend

dictentry_colon_rstack "RP!"
.byte OP_storelit, lo8(RSP), hi8(RSP)
dictentryend

dictentry_colon "SP0"
laddr forth_sp0
dictentryend

; PAD returns a location just past the pictured numeric output buffer.
dictentry_colon "PAD"
.byte OP_fetchlithi, HIGHADDR(forth_hld0)
l8 FORTH_HOLD_BUFFER_SIZE
op plus
dictentryend

; Address where RAM begins. AVR RAM starts at 0x0100.
dictentry_colon "MINMEM"
laddr 0x0100
dictentryend

; Address where available RAM ends. Memory past this point is reserved for
; the stacks, the Forth runtime, and the system.
dictentry_colon "MAXMEM"
laddr forth_pmax
dictentryend

; /EE ( -- u ) size of EEPROM
dictentry_colon "/EE"
laddr E2END+1
dictentryend

#include "controlstructures.words"
#include "defining.words"
#include "compiling.words"
#include "parsing.words"
#include "block.words"
#include "bcd.words"
#include "bytepair.words"
#include "iovec.words"
#include "random.words"
#include "video.words"
#include "console.words"
#include "keyboard.words"
#include "debug.words"
#include "special.words"

.global romdict_end
.equ romdict_end, lastentry



;-------------------------------------------------------------------------------
; Environment queries
;-------------------------------------------------------------------------------
.set lastentry, .
.global environment
environment:

.macro enventry, name
dictentry_colon \name
.endm

.macro enventryend
op minus1
dictentryend
.endm

#include "environment.words"

.global environment_end
.equ environment_end, lastentry
