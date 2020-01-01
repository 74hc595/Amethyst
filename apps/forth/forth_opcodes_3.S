#include "forth_opcodes.h"
#include "forth_macros.inc"
#include "forth_defs.h"

; Debug dispatch table aligned to a 256-word (512-byte) boundary.
; (All opcodes point to a single debugger entry point)
.align 9
.global vm_debug_dispatch_table
vm_debug_dispatch_table:
        create_dispatch_table   .debug_opcode_, 255
.align 0

dispatch_stubs .debug_opcode_, 256
; back up IP
        ld      IR, -IP
; breakpoint() takes these args:
; r25:r24 - TOS
; r23:r22 - DSP
; r21:r20 - IP
; r19:r18 - RSP
        movw    TMP, DSP
        movw    r20, IP
        rsp_to_r19r18
        callc_0arg_prologue
        call    breakpoint
        tst     r24
        breq    1f
; reload the dispatch table in case the debug flags changed
        ldi     CDTH, pm_hi8(vm_dispatch_table)
        lds     ZL, forth_flags
        sbrc    ZL, FF_TRACE_BIT
        ldi     CDTH, pm_hi8(vm_debug_dispatch_table)
1:      callc_0arg_restore
; single-step
        ldi     ZH, pm_hi8(vm_debug_dispatch_table)
; execute the real instruction.
; we can't use ijmp, because we'd have to clobber ZH and then we couldn't
; single-step. so use the old push-n-return trick to do an indirect jump.
        ld      IR, IP+
        ldi     r20, pm_hi8(vm_dispatch_table)
        push    IR
        push    r20
        ret
