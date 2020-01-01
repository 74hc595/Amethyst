#include "forth_opcodes.h"
#include "forth_macros.inc"
#include "forth_defs.h"

; The REPL, historically known as "QUIT."
; This is the main function and does not return.
; Hence, it does not need to save call-saved registers.
.global forth_outer_interpreter
.global forth_quit
forth_outer_interpreter:
; make sure constant registers are initialized
        clr     ZEROL
        clr     ZEROH
        movw    TRUE, ZERO
        com     TRUEL
        com     TRUEH
        ldi     CDTH, pm_hi8(vm_dispatch_table)
        lds     ZL, forth_flags
        sbrc    ZL, FF_TRACE_BIT
        ldi     CDTH, pm_hi8(vm_debug_dispatch_table)
; clear the data stack
        ldi     DSPL, lo8(forth_sp0)
        ldi     DSPH, hi8(forth_sp0)
        movw    TOS, ZERO
forth_quit:
; clear the return stack
        ldi     ZL, lo8(forth_rp0)
        ldi     ZH, hi8(forth_rp0)
        z_to_rsp
; clear all exception frames
        sts     forth_exception_frame, TRUEL
        sts     forth_exception_frame+1, TRUEL
; make the terminal (block 0) the input source
        sts     forth_inputsrc, r1
        ldi     ZH, 0x80
        sts     forth_inputsrc+1, ZH
        sts     forth_inputlen, r1
        sts     forth_inputpos, r1
; restore interpretation state
        lds     ZL, forth_flags
        cbr     ZL, (1<<FF_STATE_BIT)
        sts     forth_flags, ZL
; save TOS temporarily because it'll get clobbered by C functions
        movw    TSAV, TOS
; get user input
.interpreter_loop:
        call    forth_refill
        call    mio_bl
.token_loop:
; get a token
        call    forth_parse_name
; if it's empty, we're done
        tstw    r24
        breq    .ok
; perform the behavior associated with the word.
; returns a code address if VM code needs to be executed, or 0 if nothing else
; needs to be done. throws FE_UNDEFINED_WORD if no word with that name was found
; in the dictionary and the word is not a valid number
        call    forth_eval_token
; does the VM need to be run?
        tstw    r24
; if not, just move on to the next token
        breq    .token_loop

execute_word_from_text_interpreter:
; we need a return address! we want the VM to BREAK after executing this word,
; so we push the address of a byte we know will be zero (the BREAK opcode)
; we can use the address of register r1 (0x0001) which is always zero.
        push    r1      ; push 0x00
        ldi     ZL, 1
        push    ZL      ; push 0x01
; set vm's ip to the code address just returned
        movw    IP, r24
; restore TOS
        movw    TOS, TSAV
; make sure constant registers are initialized
        clr     ZEROL
        clr     ZEROH
        movw    TRUE, ZERO
        com     TRUEL
        com     TRUEH
        ldi     CDTH, pm_hi8(vm_dispatch_table)
        lds     ZL, forth_flags
        sbrc    ZL, FF_TRACE_BIT
        ldi     CDTH, pm_hi8(vm_debug_dispatch_table)
        rnext

.ok:
        movw    TOS, TSAV
; check for stack overflow/underflow
        movw    r24, DSP        ; dsp
        rsp_to_tmp
        call    forth_postcommand
        rjmp    .interpreter_loop


.global forth_global_exception_handler
forth_global_exception_handler:
; is there an error? if so, bail out
        brts    .bailout
; if no error, proceed to the next token
        movw    TSAV, TOS
        rjmp    .token_loop
.bailout:
; if so, print error and QUIT
; get top 2 stack items into registers in case it's ABORT" and we need to print
; a custom message
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        jmp     forth_bailout


; string length in r21:r20
; string address in r23:r22
.global forth_evaluate_str
forth_evaluate_str:
; fail if string is more than 255 chars
        cpse    r21, r1
        rjmp    .evaluate_str_too_long
; save current input source
        ldi     ZL, lo8(forth_inputsrc)
        ldi     ZH, hi8(forth_inputsrc)
        ld      r18, Z
        push    r18
        ldd     r18, Z+1
        push    r18
        ldd     r18, Z+2
        push    r18
        ldd     r18, Z+3
        push    r18
; set new input source
        st      Z, TMPL         ; set input string address
        std     Z+1, TMPH
        std     Z+2, r20        ; set input length
        std     Z+3, r1         ; reset input pos to 0
; set exception handler that returns into this eval loop
        push_exception_frame .evaluate_exception_handler
; save TOS temporarily because it'll get clobbered by C functions
        movw    TSAV, TOS
.eval_token_loop:
; get a token
        call    forth_parse_name
; if it's empty, we're done
        tstw    TOS
        breq    .evaluate_done
; perform the behavior associated with the word
        call    forth_eval_token
; does the VM need to be run?
        tstw    r24
; if not, just move on to the next token
        breq    .eval_token_loop
; otherwise, execute
        rjmp    execute_word_from_text_interpreter
.evaluate_done:
; don't need the exception handler anymore
        pop_exception_frame
; restore TOS
        movw    TOS, TSAV
; restore input source
        pop     ZL
        sts     forth_inputpos, ZL
        pop     ZL
        sts     forth_inputlen, ZL
        pop     ZL
        sts     forth_inputsrc+1, ZL
        pop     ZL
        sts     forth_inputsrc, ZL
; restore old IP and continue
        popr
        rnext
.evaluate_str_too_long:
        throw   FE_PARSED_STR_OVERFLOW

; xt in r23:r22
.global forth_compile_xt
forth_compile_xt:
        callc_0arg_prologue
        movw    TOS, TMP
        call    compile_xt_or_return_code_address
        rjmp    .execute_token_code

; xt in r23:r22
; if r18 != 0, assume the return address is already on the return stack
.global forth_execute_xt
forth_execute_xt:
; perform its interpretation semantics (if they're in native code) or get the
; address of its bytecode
        callc_0arg_prologue
        movw    TOS, TMP
        call    execute_xt_or_return_code_address
.execute_token_code:
        movw    r20, TOS
; r21:r20 now has code address
        callc_0arg_restore
; if nothing to do, just continue on
        cp      r20, ZEROL
        cpc     r21, ZEROH
        breq    1f
; if r18 != 0, don't push a return address
        tst     r18
        brne    2f
; push return address--unless IP is currently in the temporary rom word buffer!
; if that's the case, this word is being invoked from the text interpreter,
; so treat this as a tail-call.
        rcall   .was_called_from_text_interpreter
        breq    2f
        pushr
; set new IP
2:      movw    IP, r20
1:      rnext


; sets Z if is IP within the temporary rom word buffer
.global .was_called_from_text_interpreter
.was_called_from_text_interpreter:
        ldi     ZH, hi8(forth_rom_word_buf)
        cpi     IPL, lo8(forth_rom_word_buf)
        cpc     IPH, ZH
        brlo    1f
        ; save a few cycles by assuming the rom word buf doesn't span a 256-byte boundary
        cpi     IPL, lo8(forth_rom_word_buf+MAX_ROMDICT_ENTRY_SIZE+1)
        cpc     IPH, ZH
        brsh    1f
; in temporary rom word buffer? return with Z flag set
        sez
        ret
; not in temporary rom word buffer? return with Z flag clear
1:      clz
        ret


; block number in r22 (must be valid)
.global forth_load_block
forth_load_block:
; save current input source
        ldi     ZL, lo8(forth_inputsrc)
        ldi     ZH, hi8(forth_inputsrc)
        ld      r18, Z
        push    r18
        ldd     r18, Z+1
        push    r18
        ldd     r18, Z+2
        push    r18
        ldd     r18, Z+3
        push    r18
; set new input source
        ldi     TMPH, 0x80      ; input source is a block, line 0
        st      Z, TMPL         ; set block number
        std     Z+1, TMPH
; set exception handler that returns into this eval loop
        push_exception_frame .load_block_exception_handler
; save TOS temporarily because it'll get clobbered by C functions
        movw    TSAV, TOS
; get line from block, exit if false is returned
.load_line_loop:
        call    forth_refill
        tst     TOSL
        breq    .load_done
.load_token_loop:
; get a token
        call    forth_parse_name
; if it's empty, get next line
        tstw    TOS
        breq    .load_line_loop
; perform the behavior associated with the word
        call    forth_eval_token
; does the VM need to be run?
        tstw    r24
; if not, just move on to the next token
        breq    .load_token_loop
; otherwise, execute
        rjmp    execute_word_from_text_interpreter
.load_done:
; don't need the exception handler anymore
        pop_exception_frame
; restore TOS
        movw    TOS, TSAV
; restore input source
;!!! TODO handle nested loads here
        pop     ZL
        sts     forth_inputpos, ZL
        pop     ZL
        sts     forth_inputlen, ZL
        pop     ZL
        sts     forth_inputsrc+1, ZL
        pop     ZL
        sts     forth_inputsrc, ZL
; restore old IP and continue
        popr
        rnext


; Break/exception handlers for EVALUATE and LOAD
; if no exception, T flag clear
; if exception, T flag set and error code in r21:r20
.evaluate_exception_handler:
; if the word threw a nonzero error code, propagate it to the parent
        brts    .rethrow
; if no error, proceed to the next token
        movw    TSAV, TOS
        rjmp    .eval_token_loop
.rethrow:
        throw_r21r20

.load_block_exception_handler:
; if the word threw a nonzero error code, propagate it to the parent
        brts    .rethrow
; if no error, proceed to the next token
        movw    TSAV, TOS
        rjmp    .load_token_loop