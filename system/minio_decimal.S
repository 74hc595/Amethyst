; Minimal functions for printing 8- and 16-bit integers as signed or
; unsigned decimal.
.text

.global mio_d8
mio_d8:
        tst     r24
        brpl    mio_u8
        push    r28
        mov     r28, r24
        neg     r28
        ldi     r24, '-'
        lds     ZL, mio_putc
        lds     ZH, mio_putc+1
        icall
        rjmp    .mio_u8

.global mio_u8
mio_u8:
        push    r28
        mov     r28, r24
.mio_u8:
        cpi     r28, 10
        brlo    .ones
        cpi     r28, 100
        brlo    .tens
.hundreds_u8:
        ldi     r24, '/'
1:      inc     r24
        subi    r28, 100
        brsh    1b
        lds     ZL, mio_putc
        lds     ZH, mio_putc+1
        icall
        subi    r28, -100
.tens:
        ldi     r24, '/'
2:      inc     r24
        subi    r28, 10
        brsh    2b
        lds     ZL, mio_putc
        lds     ZH, mio_putc+1
        icall
        subi    r28, -10
.ones:
        mov     r24, r28
        subi    r24, -'0'
        pop     r28
        lds     ZL, mio_putc
        lds     ZH, mio_putc+1
        ijmp

.global mio_d16
mio_d16:
        tst     r25
        breq    mio_u8          ; defer to 8-bit function if <= 255
        brpl    .mio_u16        ; nothing to do if already positive
; otherwise, print minus sign and take two's complement of value
        com     r25
        neg     r24
        sbci    r25, -1
        push    r24
        push    r25
        ldi     r24, '-'        ; print minus sign
        lds     ZL, mio_putc
        lds     ZH, mio_putc+1
        icall
        pop     r25
        pop     r24
        rjmp    mio_d16

.global mio_u16
mio_u16:
; defer to 8-bit function if <= 255
        tst     r25
        breq    mio_u8
.mio_u16:
        push    r28
        push    r29
        movw    r28, r24
        ldi     r25, hi8(1000)
        cpi     r28, lo8(1000)
        cpc     r29, r25
        brlo    .hundreds_u16
        ldi     r25, hi8(10000)
        cpi     r28, lo8(10000)
        cpc     r29, r25
        brlo    .thousands
.tenthousands:
        ldi     r24, '/'
3:      inc     r24
        subi    r28, lo8(10000)
        sbci    r29, hi8(10000)
        brsh    3b
        lds     ZL, mio_putc
        lds     ZH, mio_putc+1
        icall
        subi    r28, lo8(-10000)
        sbci    r29, hi8(-10000)
.thousands:
        ldi     r24, '/'
4:      inc     r24
        subi    r28, lo8(1000)
        sbci    r29, hi8(1000)
        brsh    4b
        lds     ZL, mio_putc
        lds     ZH, mio_putc+1
        icall
        subi    r28, lo8(-1000)
        sbci    r29, hi8(-1000)
.hundreds_u16:
        ldi     r24, '/'
5:      inc     r24
        subi    r28, lo8(100)
        sbci    r29, hi8(100)
        brsh    5b
        lds     ZL, mio_putc
        lds     ZH, mio_putc+1
        icall
        subi    r28, lo8(-100)
        sbci    r29, hi8(-100)
        pop     r29
        rjmp    .tens
