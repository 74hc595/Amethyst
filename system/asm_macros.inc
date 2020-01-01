#define ALWAYS_ZERO_IOREG _SFR_IO_ADDR(DDRA)

; Clear a register without affecting SREG flags.
; Relies on keeping one I/O register always set to zero.
.macro clrnf reg
        in      \reg, ALWAYS_ZERO_IOREG
.endm

; "Returns" from a video mode handler.
.macro fhret
        rjmp    framehandler_end
.endm

.macro lhret
        rjmp    linehandler_end
.endm

; A 2-cycle nop that only takes up one word of flash.
.macro short_nop2
        rjmp    .
.endm

; A dummy lpm can be used as a 3-cycle nop that only takes up one word of flash.
; Clobbers ZL. Does not affect flags
.macro short_nop3
        lpm     ZL, Z
.endm

; A 3-cycle nop that takes up two words of flash.
.macro nop3
        jmp     .
.endm

; A 4-cycle nop that takes up two words of flash.
.macro short_nop4
        short_nop2
        short_nop2
.endm

; A 5-cycle nop that takes up two words of flash.
.macro short_nop5
        short_nop3
        short_nop2
.endm
