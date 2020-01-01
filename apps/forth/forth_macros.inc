#include "forth_regs.h"

#define SIMAVR_DEBUG_PORT       EIFR
.macro trace, n
        ; ldi     IR, \n
        ; out     _SFR_IO_ADDR(SIMAVR_DEBUG_PORT), r1
.endm

; test 16-bit register pair
.macro tstw, regpair
        sbiw  \regpair, 0
.endm

; fetch next instruction
.macro next_fetch
        ld      IR, IP+
.endm

; decode and execute opcode in IR
.macro next_execute
        ijmp
.endm

; fetch and execute next instruction
.macro next
        next_fetch
        next_execute
.endm

; reload dispatch table high byte, then fetch and execute next instruction
.macro rnext
        mov     DTH, CDTH
        next
.endm

; push value onto data stack
; DSP := DSP - 2
; [DSP] = TOS
.macro pushd
        st      -DSP, TOSH
        st      -DSP, TOSL
.endm

; pop value off data stack
; TOS = [DSP]
; DSP := DSP + 2
.macro popd
        ld      TOSL, DSP+
        ld      TOSH, DSP+
.endm

; drop 2nd value from stack and copy to TMP
; TMP = [DSP]
; DSP := DSP + 2
.macro nip
        ld      TMPL, DSP+
        ld      TMPH, DSP+
.endm

; drop 2 words from stack
.macro drop2
        ldd     TOSL, DSP+2
        ldd     TOSH, DSP+3
        adiw    DSP, 4
.endm

; drop 3 words from stack
.macro drop3
        ldd     TOSL, DSP+4
        ldd     TOSH, DSP+5
        adiw    DSP, 6
.endm

; push IP onto return stack
; RSP := RSP - 2
; [RSP] = IP
.macro pushr
        push    IPH
        push    IPL
.endm

; pop IP from return stack
; IP = [RSP]
; RSP := RSP + 2
.macro popr
        pop     IPL
        pop     IPH
.endm

; Which registers are caller-saved according to AVR-GCC's calling convention?
.irp reg,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,r16,r17,r28,r29
.equ caller_saved_\reg, 1
.endr

; Which registers need to be cleared before returning to C code?
.equ caller_clear_on_exit_r1, 1


; Saves any caller-saved registers in the argument list by pushing them onto
; the stack in order.
.macro callsave, reg1, regs:vararg
.ifdef caller_saved_\reg1
push \reg1
.endif
.ifnes "\regs",""
callsave \regs
.endif
.endm

; Restores any caller-saved registers in the argument list by popping them from
; the stack in *reverse* order.
.macro callrestore, reg1, regs:vararg
.ifnes "\regs",""
callrestore \regs
.endif
.ifdef caller_saved_\reg1
pop \reg1
.endif
.ifdef caller_clear_on_exit_\reg1
clr \reg1
.endif
.endm


.macro dispatch_table_entry prefix, num
rjmp \prefix\num
.endm

.macro dispatch_table_label prefix, num
\prefix\num\():
.endm


.macro create_dispatch_table prefix, numentries
.altmacro
.set i,0
.rept \numentries
dispatch_table_entry \prefix,%i
.set i, i+1
.endr
.endm


.macro dispatch_stubs prefix, numentries
.altmacro
.set i,0
\prefix\().unimplemented:
.rept \numentries
dispatch_table_label \prefix,%i
.set i, i+1
.endr
.endm


.macro callc_prologue
        movw    XSAV, IP
.endm

.macro callc_restore
; clear r0 without affecting flags
; (gcc assures that r1 will always be zero after a C function returns)
        mov     ZEROL, r1
        movw    IP, XSAV
.endm

.macro callc_epilogue
        callc_restore
        rnext
.endm

; for C functions that take no arguments; need to save TOS
.macro callc_0arg_prologue
        callc_prologue
        movw    TSAV, TOS
.endm

.macro callc_0arg_restore
        callc_restore
        movw    TOS, TSAV
.endm

.macro callc_0arg_epilogue
        callc_0arg_restore
        callc_epilogue
.endm


; atomic stack pointer operations

.macro atomic_start
        in      r16, _SFR_IO_ADDR(SREG)
        cli
.endm

.macro atomic_end
        out     _SFR_IO_ADDR(SREG), r16
.endm

;//!!! TODO this makes the video flickery!
.macro z_to_rsp
        ; atomic_start
        out     _SFR_IO_ADDR(SPL), ZL
        out     _SFR_IO_ADDR(SPH), ZH
        ; atomic_end
.endm

; don't need extra protection for reads. if an interrupt happens between the two
; `in` instructions, when the ISR returns, the stack pointer will still have the
; same value.
.macro rsp_to_z
        in      ZL, _SFR_IO_ADDR(SPL)
        in      ZH, _SFR_IO_ADDR(SPH)
.endm

.macro rsp_to_tmp
        in      TMPL, _SFR_IO_ADDR(SPL)
        in      TMPH, _SFR_IO_ADDR(SPH)
.endm

.macro rsp_to_r21r20
        in      r20, _SFR_IO_ADDR(SPL)
        in      r21, _SFR_IO_ADDR(SPH)
.endm

.macro rsp_to_r19r18
        in      r18, _SFR_IO_ADDR(SPL)
        in      r19, _SFR_IO_ADDR(SPH)
.endm


.macro push_exception_frame handler
; push handler address
        ldi     ZL, pm_lo8(\handler)
        ldi     ZH, pm_hi8(\handler)
        push    ZH
        push    ZL
; push address of previous frame
        lds     ZL, forth_exception_frame
        lds     ZH, forth_exception_frame+1
        push    ZH
        push    ZL
; set current exception frame
        rsp_to_z
        sts     forth_exception_frame, ZL
        sts     forth_exception_frame+1, ZH
.endm


.macro pop_exception_frame
; restore previous frame
        pop     ZL
        pop     ZH
        sts     forth_exception_frame, ZL
        sts     forth_exception_frame+1, ZH
; discard exception handler
        pop     ZL
        pop     ZH
.endm


.macro throw_r21r20
        jmp     do_throw
.endm

.macro rthrow_r21r20
        rjmp    do_throw
.endm

.macro throw value
        ldi     r20, \value
        ldi     r21, 0xFF
        throw_r21r20
.endm

.macro rthrow value
        ldi     r20, \value
        ldi     r21, 0xFF
        rthrow_r21r20
.endm
