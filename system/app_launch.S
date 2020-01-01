; This function does not return, so it doesn't care about clobbering the
; caller-saved registers.
; Much of the implementation is adapted from libgcc's __do_clear_bss and
; __do_copy_data.
#include <avr/io.h>
.text
.global app_launch
app_launch:
        cli             ; interrupts must be reenabled by the app
        clr     r1      ; make sure the zero register is clear
        ; get entry point
        movw    Z, r24
        lpm     r8, Z+  ; entry point low byte
        lpm     r9, Z+  ; entry point high byte
        ; get data load start
        lpm     r10, Z+ ; data load start address low byte
        lpm     r11, Z+ ; data load start address middle byte
        lpm     r0, Z+  ; data load start address high byte
        adiw    Z, 1    ; skip padding byte
        ; get data end
        lpm     r12, Z+ ; data end address low byte
        lpm     r13, Z+ ; data end address high byte
        ; get bss end
        lpm     r14, Z+ ; bss end address low byte
        lpm     r15, Z+ ; bss end address high byte
        ; copy data from flash to RAM
        ldi     XL, lo8(__app_data_start)
        ldi     XH, hi8(__app_data_start)
        out     _SFR_IO_ADDR(RAMPZ), r0
        movw    Z, r10
        rjmp    .copy_data_start
.copy_data_loop:
        elpm    r0, Z+
        st      X+, r0
.copy_data_start:
        cp      XL, r12 ; compare current address with end address
        cpc     XH, r13
        brne    .copy_data_loop
        ; clear bss
        rjmp    .clear_bss_start
.clear_bss_loop:
        st      X+, r1
.clear_bss_start:
        cp      XL, r14
        cpc     XH, r15
        brne    .clear_bss_loop
        ; clear SREG and reset stack/frame pointers
        out     _SFR_IO_ADDR(SREG), r1
        out     _SFR_IO_ADDR(RAMPZ), r1
        ldi     YL, lo8(__stack)
        ldi     YH, hi8(__stack)
        out     _SFR_IO_ADDR(SPL), YL
        out     _SFR_IO_ADDR(SPH), YH
        ; off we go!
        movw    Z, r8
        ijmp
