#include <avr/io.h>
#include "videodefs.h"
#include "asm_macros.inc"

.global keymatrixrows
.global burst
.global raster_int_scanline
.global raster_int
.comm keymatrixrows, 8
.comm keymatrixhandler_lo, 1
.comm keymatrixcol, 1
.comm raster_int_scanline, 1
.comm raster_int, 2

; In order to support receiving data from the USART without visual corruption,
; we switch between interrupt driven (during vblank) and polling (during active
; video, once per hblank).
; Received bytes are placed into a user-supplied circular queue.
; The implementation is highly optimized for performance. Thus, the receive
; buffer must be 256 bytes in size and must be aligned to a 256 byte boundary.
; This allows the head and tail indexes to require only one byte.
; The implementation distinguishes between "empty" and "full" conditions by
; always leaving one cell in the queue free. Thus, if head == tail, the buffer
; is empty.
; This also allows the implementation to be completely lock-free (i.e. does not
; require any atomic operations that require explicitly disabling/re-enabling
; interrupts).
; As an added bonus, the implementation does not affect the SREG flags.
.global recvbuf_tail
.global recvbuf_head
.global recvbuf_hi
.comm recvbuf_head, 1   ; or low byte of async rx callback
.comm recvbuf_tail, 1   ; or high byte of async rx callback
.comm recvbuf_hi, 1     ; if bit 7 is set, interpret head/tail as a callback function ptr





.section .jumptables
; NOP slide to synchronize the CPU with the shift register phase.
; This code should be reached via an `ijmp` with TCNT2 as the lower byte of the
; jump address. (TCNT2 will be in the range [0,7].)
; Put this table as high up in memory as we can.
.balign 512
colorburst_sync:
        nop
        nop
        nop
        nop
        nop
        nop
        nop
; Emit a pulse for the length of the colorburst
        ldi     ZL, 0
        sts     UDR0, ZL   ; emit low-going pulse for 16 bit times (8 colorburst cycles)
        sts     UDR0, ZL
        movw    Z, zsav                 ; restore regs
        reti

; keymatrixhandler jump destinations.
; All code between __kmh_jumptable_start and __kmh_jumptable_end must live on
; the same 256-word page, since we only store the lower byte of the handler
; pointer. The linker script raises an error if this condition is violated.
; .section .bottomtext
.global __kmh_jumptable_start
__kmh_jumptable_start:
.kmh_init:
.kmh_row0:
        ; read keyboard row inputs and select next column strobe
        in      ZL, IO_(PINA)
        sts     keymatrixrows+0, ZL
        ldi     ZL, 0b11111101
        sts     keymatrixcol, ZL
        ldi     ZL, pm_lo8(.kmh_row1)
        sts     keymatrixhandler_lo, ZL
        ; preload the colorburst pattern into the shift register
        ldi     ZL, COLORBURST_PATTERN
        out     IO_(PORTC), ZL
        movw    Z, zsav
        reti
.kmh_row1:
        in      ZL, IO_(PINA)
        sts     keymatrixrows+1, ZL
        ldi     ZL, 0b11111011
        sts     keymatrixcol, ZL
        ldi     ZL, pm_lo8(.kmh_row2)
        sts     keymatrixhandler_lo, ZL
        ldi     ZL, COLORBURST_PATTERN
        out     IO_(PORTC), ZL
        movw    Z, zsav
        reti
.kmh_row2:
        in      ZL, IO_(PINA)
        sts     keymatrixrows+2, ZL
        ldi     ZL, 0b11110111
        sts     keymatrixcol, ZL
        ldi     ZL, pm_lo8(.kmh_row3)
        sts     keymatrixhandler_lo, ZL
        ldi     ZL, COLORBURST_PATTERN
        out     IO_(PORTC), ZL
        movw    Z, zsav
        reti
.kmh_row3:
        in      ZL, IO_(PINA)
        sts     keymatrixrows+3, ZL
        ldi     ZL, 0b11101111
        sts     keymatrixcol, ZL
        ldi     ZL, pm_lo8(.kmh_row4)
        sts     keymatrixhandler_lo, ZL
        ldi     ZL, COLORBURST_PATTERN
        out     IO_(PORTC), ZL
        movw    Z, zsav
        reti
.kmh_row4:
        in      ZL, IO_(PINA)
        sts     keymatrixrows+4, ZL
        ldi     ZL, 0b11011111
        sts     keymatrixcol, ZL
        ldi     ZL, pm_lo8(.kmh_row5)
        sts     keymatrixhandler_lo, ZL
        ldi     ZL, COLORBURST_PATTERN
        out     IO_(PORTC), ZL
        movw    Z, zsav
        reti
.kmh_row5:
        in      ZL, IO_(PINA)
        sts     keymatrixrows+5, ZL
        ldi     ZL, 0b10111111
        sts     keymatrixcol, ZL
        ldi     ZL, pm_lo8(.kmh_row6)
        sts     keymatrixhandler_lo, ZL
        ldi     ZL, COLORBURST_PATTERN
        out     IO_(PORTC), ZL
        movw    Z, zsav
        reti
.kmh_row6:
        in      ZL, IO_(PINA)
        sts     keymatrixrows+6, ZL
        ldi     ZL, 0b01111111
        sts     keymatrixcol, ZL
        ldi     ZL, pm_lo8(.kmh_row7)
        sts     keymatrixhandler_lo, ZL
        ldi     ZL, COLORBURST_PATTERN
        out     IO_(PORTC), ZL
        movw    Z, zsav
        reti
.kmh_row7:
        in      ZL, IO_(PINA)
        sts     keymatrixrows+7, ZL
        ldi     ZL, COLORBURST_PATTERN
        ; linehandler sets up colorburst
        sts     keymatrixcol, ZL
        out     IO_(PORTC), ZL
        ; disable the keymatrix interrupt (OVF)
        sbic    color_video, color_video_bit
        ldi     ZL, _BV(OCIE1A)|_BV(OCIE1B)
        sbis    color_video, color_video_bit
        ldi     ZL, _BV(OCIE1A)
        sts     TIMSK1, ZL
        out     IO_(TIFR1), ZL
        ; if usart async rx is enabled, switch from interrupt mode to polled
        sbic    usart_async, usart_async_bit
        rjmp    1f
        movw    Z, zsav
        reti
        ; if usart async rx is enabled, switch from interrupt mode to polled
1:      ldi     ZL, _BV(RXEN1)|_BV(TXEN1) ; clear RXCIE1
        sts     UCSR1B, ZL
        movw    Z, zsav
        reti
.global __kmh_jumptable_end
__kmh_jumptable_end:


; NOP slide to synchronize the CPU with the shift register phase.
; This code should be reached via an `ijmp` with TCNT2 as the lower byte of the
; jump address. (TCNT2 will be in the range [0,7].)
; Put this table as high up in memory as we can.
.balign 512
shiftreg_sync_delay:
        nop                             ; [0]
        nop                             ; [1]
        nop                             ; [2]
        nop                             ; [3]
        nop                             ; [4]
        nop                             ; [5]
        nop                             ; [6]
        ldi     ZH, 0                   ; [7]
        out     IO_(PORTC), ZH          ; [0]
        nop                             ; [1]
        sts     UCSR0B, ZH              ; [2][3]
        in      ZL, linehandler_lo      ; [4]
        in      ZH, linehandler_hi      ; [5]
        ijmp                            ; [6][7]



.section .bottomtext
; -------------------- UART interrupts --------------------
.global USART1_RX_vect
USART1_RX_vect:
        movw    zsav, Z
        lds     ZH, recvbuf_hi          ; get upper byte of recvbuf address
        lds     ZL, recvbuf_head        ; get lower byte of recvbuf address
        lds     isr_tmp, UDR1           ; read incoming byte
        ; if high bit of recvbuf_hi is set, interpret recvbuf_head and
        ; recvbuf_tail as a function pointer!
        sbrc    ZH, 7
        rjmp    .usart_invoke_async_callback
        st      Z+, isr_tmp             ; store to buffer and advance head ptr
        ; if head is now equal to tail, buffer was full!
        ; don't store new head pointer, leave a one-cell gap.
        ; the received character is effectively dropped.
        lds     ZH, recvbuf_tail
        cpse    ZL, ZH
        sts     recvbuf_head, ZL        ; save new head ptr
        movw    Z, zsav
        reti

.usart_invoke_async_callback:
        lds     ZH, recvbuf_tail
        icall
        movw    Z, zsav
        reti



; -------------------- Vertical interrupts --------------------
; Start of vblank: set normal sync pulse polarity and enable colorburst
; *** Does not save SREG! Do not use instructions that modify flags! ***
.global TIMER3_OVF_vect
TIMER3_OVF_vect:
        mov     zsav, r31 ; need to grab a high register
        sbic    color_video, color_video_bit
        ldi     r31, _BV(OCIE1B)
        sbis    color_video, color_video_bit
        ldi     r31, 0
        sts     TIMSK1, r31

        ldi     r31, _BV(COM1B1)|_BV(COM1B0)|_BV(WGM11) ; normal sync waveform
        sts     TCCR1A, r31
        mov     r31, zsav
        reti

; End of vblank: enable horizontal interrupts for keymatrix reading and
; active video
; *** Does not save SREG! Do not use instructions that modify flags! ***
.global TIMER3_COMPA_vect
TIMER3_COMPA_vect:
        mov     zsav, r31 ; need to grab a high register
        ldi     r31, pm_lo8(.kmh_init)
        sts     keymatrixhandler_lo, r31
        ldi     r31, 0b11111110 ; set initial strobe pattern
        sts     keymatrixcol, r31
        sbic    color_video, color_video_bit
        ldi     r31, _BV(TOIE1)|_BV(OCIE1A)|_BV(OCIE1B)
        sbis    color_video, color_video_bit
        ldi     r31, _BV(TOIE1)|_BV(OCIE1A)
        out     IO_(TIFR1), r31 ; ensure interrupt flags are clear
        sts     TIMSK1, r31 ; enable horizontal interrupt
        mov     r31, zsav
        reti

; Start of vertical sync: invert sync pulse
; Horizontal interrupts are assumed to be disabled
; *** Does not save SREG! Do not use instructions that modify flags! ***
.global TIMER3_COMPB_vect
TIMER3_COMPB_vect:
        movw    zsav, Z ; need to grab a high register pair
        ldi     r31, _BV(COM1B1)|_BV(WGM11) ; invert sync waveform
        sts     TCCR1A, r31
        ; get address of frame handler (one word before line handler)
        in      ZL, linehandler_lo
        in      ZH, linehandler_hi
        ; perform a dummy read to decrement Z without affecting flags
        ld      isr_tmp, -Z
        ; invoke frame handler
        ijmp




; -------------------- Horizontal interrupts --------------------

; Sync pulse falling edge: read key matrix row and emit colorburst
.global TIMER1_OVF_vect
TIMER1_OVF_vect:
        movw    zsav, Z
        ldi     ZH, pm_hi8(__kmh_jumptable_start)       ; 1 cycle
        lds     ZL, keymatrixhandler_lo                 ; 2 cycles
        ijmp                                            ; 2 cycles

; Sync pulse rising edge: emit colorburst (color modes only)
; *** Does not save SREG! Do not use instructions that modify flags! ***
.global TIMER1_COMPB_vect
TIMER1_COMPB_vect:
        movw    zsav, Z
        ; Synchronize to shift register strobe to compensate for interrupt jitter.
        ldi     ZH, pm_hi8(colorburst_sync)
        lds     ZL, TCNT2
        ijmp

; End of back porch: active video
; *** Does not save SREG! Do not use instructions that modify flags! ***
.global TIMER1_COMPA_vect
TIMER1_COMPA_vect:
        movw    zsav, Z
        ; Synchronize to shift register strobe and invoke linehandler
        lds     ZL, TCNT2
        ldi     ZH, pm_hi8(shiftreg_sync_delay)
        ijmp



.section .linehandlers
.global framehandler_end
framehandler_end:
        movw    Z, zsav
; Update simple tone generator if bit 4 in PORTB is set
        sbis    tone_enabled, tone_enabled_bit
        reti
        in      isr_tmp, IO_(SREG)
; Decrement tone duration
        lds     zsav_lo, tone_duration
        dec     zsav_lo
        sts     tone_duration, zsav_lo
        breq    .tone_off
        out     IO_(SREG), isr_tmp
        reti
.tone_off:
        out     IO_(SREG), isr_tmp
        clrnf   isr_tmp
        out     IO_(OCR0A), isr_tmp
        out     IO_(OCR0B), isr_tmp
        cbi     tone_enabled, tone_enabled_bit
        reti

.global linehandler_end
linehandler_end:
        ; gate the shift register clock by enabling the USART
        ldi     ZL, _BV(TXEN0)
        sts     UCSR0B, ZL
        ; PORTC no longer being used for video, set up key matrix column strobes
        ; (row inputs will be sampled before outputting the colorburst of the
        ; next line; hblank gives the row inputs time to stabilize)
        lds     isr_tmp, keymatrixcol
        out     IO_(PORTC), isr_tmp
        ; have we registered for a custom handler on this scanline?
        lds     ZL, raster_int_scanline
        cpse    linenum, ZL
        ; if not, just finish up this line normally
        rjmp    raster_int_end
        ; otherwise, load the handler pointer and jump to it
        lds     ZL, raster_int
        lds     ZH, raster_int+1
        ijmp
.global raster_int_end
raster_int_end:
        ; if usart async rx is enabled, check for incoming serial data
        sbis    usart_async, usart_async_bit
        rjmp    .line_end
        lds     isr_tmp, UCSR1A         ; have we received a character?
        sbrs    isr_tmp, RXC1
        rjmp    .line_end
        lds     ZH, recvbuf_hi          ; get upper byte of recvbuf address
        lds     ZL, recvbuf_head        ; get lower byte of recvbuf address
        lds     isr_tmp, UDR1           ; read incoming byte
        ; if high bit of recvbuf_hi is set, interpret recvbuf_head and
        ; recvbuf_tail as a function pointer!
        sbrc    ZH, 7
        rjmp    .usart_invoke_async_callback2
        st      Z+, isr_tmp             ; store to buffer and advance head ptr
        ; if head is now equal to tail, buffer was full!
        ; don't store new head pointer, leave a one-cell gap.
        ; the received character is effectively dropped.
        lds     ZH, recvbuf_tail
        cpse    ZL, ZH
        sts     recvbuf_head, ZL        ; save new head ptr
.line_end:
        ; restore Z
        movw    Z, zsav
        ; check line count; begin vblank if zero
        clrnf   isr_tmp
        cpse    linenum, isr_tmp
        reti
.last_line_end:
        ; if this was the last line, disable horizontal interrupts
        sts     TIMSK1, isr_tmp
        ; set "new frame" flag
        sbi     new_frame, new_frame_bit
        ; if usart async rx is enabled, switch from polled to interrupt read
        sbis    usart_async, usart_async_bit
        reti
.usart1_enable_interrupts:
        mov     zsav, r31        ; need to grab a high register, ick...
        ldi     r31, _BV(RXCIE1)|_BV(RXEN1)|_BV(TXEN1)
        sts     UCSR1B, r31
        mov     r31, zsav
        reti
.usart_invoke_async_callback2:
        lds     ZH, recvbuf_tail
        icall
        rjmp    .line_end



.text
; callable from C (r24 holds the return value)
.global serial_get_byte
serial_get_byte:
        lds     ZH, recvbuf_hi
        lds     ZL, recvbuf_tail
        ld      r24, Z+                 ; get byte and advance tail pointer
        sts     recvbuf_tail, ZL
        ret
