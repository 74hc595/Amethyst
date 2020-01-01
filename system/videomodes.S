#include <avr/io.h>
#include "videodefs.h"
#include "asm_macros.inc"

; -------------------- Video linehandler calling convention --------------------
; r2 (isr_tmp) can be used freely but must be cleared before returning.
;
; r31:r30 (Z) and GPIOR0 can be used freely and does not have to be restored.
;
; GPIOR1 and GPIOR2 can be used freely (e.g. for saving registers without using
; the stack) *BUT* they must be restored to the low and high byte of the word
; address of the linehandler with `ldi`+`out` instructions.
;
; SREG must be explicitly saved and restored.
;
; All other registers must be explicitly saved and restored.
;
; Line handlers should end with a jump to `linehandler_end` and not a `ret`.
; Use the `lhret` macro.
;
; The instruction just before the start of a linehandler should be a relative
; jump to the frame handler, which is executed at the start of each frame.
; The builtin linehandlers all use the default framehandlers, but a custom
; linehandler is free to use its own.
;
; Frame handlers should end with a jump to `framehandler_end` and not a `ret`.
; Use the `fhret` macro.
;
; The stack is evil, avoid using it whenever possible!
; In an application with a full 16KB framebuffer, there are only a few hundred
; bytes of work RAM available, so every byte is sacred!
; Instead, consider saving registers/flags in isr_tmp (good for saving SREG),
; vptr (good for saving X), and GPIOR0/GPIOR1/GPIOR2.
;
; When the first instruction of a linehandler executed, TCNT2 will be 0.
; To keep horizontal alignment consistent with other modes, and to minimize
; jitter, all writes to PORTC should be done when TCNT2=1.
.global screen_ptr
.comm screen_ptr, 2
.comm tilemap_hi, 1
.comm tilemap_hh, 1
.comm globalcolor, 1
.comm vscroll, 1
.comm bytesperline, 1
.comm linesperscreen, 1

.section .linehandlers


; Cycles until first PORTC write: 33
.global linehandler_hires_tiled
        rjmp    framehandler
linehandler_hires_tiled:
9:      ; initialize pointers, save registers
        movw    Z, vptr                 ; [0]
        movw    vptr, X                 ; [1] use vptr to save previous X value
        movw    X, Z                    ; [2] so we can use X to point into the name table
        in      isr_tmp, IO_(SREG)      ; [3]
        out     IO_(GPIOR0), isr_tmp    ; [4] save flags in GPIOR0
        dec     linenum                 ; [5]
        ; compute tilemap pointer high byte for this row
        lds     ZL, vscroll             ; [6][7]
        add     ZL, linenum             ; [0]
        andi    ZL, 7                   ; [1]
        lds     ZH, tilemap_hi          ; [2][3]
        add     ZH, ZL                  ; [4]
        ; we can use GPIOR1 and GPIOR2 as temporaries here,
        ; because they have known values and be restored with load-immediates
        out     IO_(GPIOR1), ZH         ; [5] save high byte of left-half table
        subi    ZH, -8                  ; [6] right-half table is 2048 bytes ahead
        out     IO_(GPIOR2), ZH         ; [7] save high byte of right-half table
        subi    ZH, 8                   ; [0] bring back address of left-half table
        push    r16                     ; [1][2]
        ldi     r16, 40                 ; [3] loop count
        ; is the tilemap in ROM or RAM?
        lds     ZL, tilemap_hh          ; [4][5] tilemap pointer page byte
        sbrc    ZL, TILEMAP_IS_RAM_BIT  ; [6][7] (if skipped)
        rjmp    .linehandler_hires_tiled_rampatterns ; [5][6] (if skipped)
.linehandler_hires_tiled_rompatterns:
        ; save previous RAMPZ0 bit in the carry flag, and set new RAMPZ0
        ; (only lowest bit of RAMPZ is significant on 128K flash parts)
        in      isr_tmp, IO_(RAMPZ)     ; [0]
        lsr     isr_tmp                 ; [1]
        out     IO_(RAMPZ), ZL          ; [2] set new RAMPZ0
        nop                             ; [3]
        ; loop (each iteration takes 16 cycles and outputs 16 bits)
1:      ld      ZL, X+                  ; [4][5] get tile index (2 cycles)
        elpm    isr_tmp, Z              ; [6][7][0] get left pixels (3 cycles)
        out     IO_(PORTC), isr_tmp     ; [1]* output left half of tile (1 cycle)
        in      ZH, IO_(GPIOR2)         ; [2] switch to right-half table (1 cycle)
        nop                             ; [3] spare (1 cycle)
        elpm    isr_tmp, Z              ; [4][5][6] get right pixels (3 cycles)
        in      ZH, IO_(GPIOR1)         ; [7] Z back to left-half tiles (1 cycle)
        dec     r16                     ; [0] decrement loop count (1 cycle)
        out     IO_(PORTC), isr_tmp     ; [1]* output right half of tile (1 cycle)
        brne    1b                      ; [2][3] loop if columns left (2 cycles, 1 on last iteration)
        ; restore RAMPZ0
        rol     r16                     ; [4] bring carry flag (old RAMPZ0) back into the low bit of a register
        out     IO_(RAMPZ), r16         ; [5] restore old RAMPZ0
        nop                             ; [6]
.linehandler_hires_tiled_common_end:
        ; restore registers, blank video
        clrnf   isr_tmp                 ; [7]
        ldi     ZL, pm_lo8(9b)          ; [0]
        out     IO_(PORTC), isr_tmp     ; [1]* blank video
        pop     r16
        out     IO_(GPIOR1), ZL
        ldi     ZL, pm_hi8(9b)
        out     IO_(GPIOR2), ZL
        ; unless this was the 8th row, rewind X back to the start of the line
        lds     ZL, tilemap_hi
        cpse    ZH, ZL
        sbiw    X, 40
        ; restore remaining registers
2:      movw    Z, X
        movw    X, vptr                 ; restore X register
        movw    vptr, Z                 ; save new vptr
        in      isr_tmp, IO_(GPIOR0)    ; restore flags
        out     IO_(SREG), isr_tmp      ;
        lhret
.linehandler_hires_tiled_rampatterns:
        short_nop5                      ; for consistent alignment w/romfont
1:      ld      ZL, X+                  ; [5][6] get tile index (2 cycles)
        ld      isr_tmp, Z              ; [6][7] get left pixels (2 cycles)
        in      ZH, IO_(GPIOR2)         ; [0] switch to right-half table (1 cycle)
        out     IO_(PORTC), isr_tmp     ; [1]* output left half of tile (1 cycle)
        short_nop2                      ; [2][3] spare (2 cycles)
        ld      isr_tmp, Z              ; [4][5] get right pixels (2 cycles)
        in      ZH, IO_(GPIOR1)         ; [6] Z back to left-half tiles (1 cycle)
        dec     r16                     ; [7] decrement loop count (1 cycle)
        nop                             ; [0] spare (1 cycle)
        out     IO_(PORTC), isr_tmp     ; [1]* output right half of tile (1 cycle)
        brne    1b                      ; [3][4] loop if columns left (2 cycles, 1 on last iteration)
        rjmp    .linehandler_hires_tiled_common_end ; [5][6]


; Decrement line number, saving/restoring SREG. Takes 3 cycles and clobbers isr_tmp.
.macro dec_line_num
        in      isr_tmp, IO_(SREG)
        dec     linenum
        out     IO_(SREG), isr_tmp
.endm

; Cycles until first PORTC write: 9
.global linehandler_160x100x256
        rjmp    framehandler
linehandler_160x100x256:
        ; use bit 0 of GPIOR0 to determine if vertical resolution should be halved
        sbi     IO_(GPIOR0), 0  ; [0][1]
        short_nop2              ; [2][3] for alignment with 160x200 mode
.linehandler_160_bytes_per_line:
        nop                     ; [0]
        movw    Z, vptr         ; [1]
        .rept 32
                nop                             ; [2] 1 cycle
                ld    isr_tmp, Z+               ; [3][0] 2 cycles
                out   IO_(PORTC), isr_tmp       ; [1]* 1 cycle
        .endr
.linehandler_128_bytes_per_line:
        .rept 128
                nop                             ; [2] 1 cycle
                ld    isr_tmp, Z+               ; [3][0] 2 cycles
                out   IO_(PORTC), isr_tmp       ; [1]* 1 cycle
        .endr
        ; done; blank video, save new pixel pointer, restore Z
        short_nop2                              ; [2][3]
        clrnf   isr_tmp                         ; [0] clear isr_tmp without affecting flags
        out     IO_(PORTC), isr_tmp             ; [1]*
        dec_line_num
        sbic    IO_(GPIOR0), 0
        sbrs    linenum, 0
        movw    vptr, Z
        nop     ; mystery nop--without it, horizontal alignment is off!
        lhret

.global linehandler_128x100x256
        rjmp    framehandler
linehandler_128x100x256:
        ; use bit 0 of GPIOR0 to determine if vertical resolution should be halved
        sbi     IO_(GPIOR0), 0  ; [0][1]
        movw    Z, vptr         ; [2]
        short_nop2              ; [3][4]
        rjmp    .linehandler_128_bytes_per_line ; [0][1]

; There's not enough RAM for a full 160x200x256 framebuffer,
; so you'll need to get creative with raster interrupts and beam-racing
; if you use this mode
; Cycles until first PORTC write: 9
.global linehandler_160x200x256
        rjmp    framehandler
linehandler_160x200x256:
        cbi     IO_(GPIOR0), 0                  ; [0][1]
        rjmp    .linehandler_160_bytes_per_line ; [2][3]


; Cycles until first PORTC write: 9
.global linehandler_80x50x256
        rjmp    framehandler
linehandler_80x50x256:
9:      in      isr_tmp, IO_(SREG)      ; [0] save SREG in GPIOR0
        out     IO_(GPIOR0), isr_tmp    ; [1]
        movw    Z, vptr                 ; [2]
        out     IO_(GPIOR1), r16        ; [3]
        ldi     r16, 80                 ; [0]
        dec     linenum                 ; [1]
        ; 8 cycles per pixel (2 shift register periods)
1:      ld      isr_tmp, Z+             ; [2][3]
        dec     r16                     ; [0]
        out     IO_(PORTC), isr_tmp     ; [1]*
        short_nop2                      ; [2][3]
        brne    1b                      ; [0][1]
        ; set Z flag if we need to advance to the next row
        ldi     r16, 3                  ; [1]
        and     r16, linenum            ; [2]
        ; blank video
        clrnf   isr_tmp                 ; [3]
        nop                             ; [0]
        out     IO_(PORTC), isr_tmp     ; [1]*
        ; if Z=0, advance vptr
        brne    2f
        movw    vptr, Z
        ; restore registers
2:      in      isr_tmp, IO_(GPIOR0)
        out     IO_(SREG), isr_tmp
        in      r16, IO_(GPIOR1)
        ldi     ZL, pm_lo8(9b)
        out     IO_(GPIOR1), ZL
        lhret


; Cycles until first PORTC write: 9
; (This is just a copy/paste of linehandler_80x50x256, but with a different
; bitmask applied to the line number)
.global linehandler_80x25x256
        rjmp    framehandler
linehandler_80x25x256:
9:      in      isr_tmp, IO_(SREG)      ; [0] save SREG in GPIOR0
        out     IO_(GPIOR0), isr_tmp    ; [1]
        movw    Z, vptr                 ; [2]
        out     IO_(GPIOR1), r16        ; [3]
        ldi     r16, 80                 ; [0]
        dec     linenum                 ; [1]
        ; 8 cycles per pixel (2 shift register periods)
1:      ld      isr_tmp, Z+             ; [2][3]
        dec     r16                     ; [0]
        out     IO_(PORTC), isr_tmp     ; [1]*
        short_nop2                      ; [2][3]
        brne    1b                      ; [0][1]
        ; set Z flag if we need to advance to the next row
        ldi     r16, 7                  ; [1]
        and     r16, linenum            ; [2]
        ; blank video
        clrnf   isr_tmp                 ; [3]
        nop                             ; [0]
        out     IO_(PORTC), isr_tmp     ; [1]*
        ; if Z=0, advance vptr
        brne    2f
        movw    vptr, Z
        ; restore registers
2:      in      isr_tmp, IO_(GPIOR0)
        out     IO_(SREG), isr_tmp
        in      r16, IO_(GPIOR1)
        ldi     ZL, pm_lo8(9b)
        out     IO_(GPIOR1), ZL
        lhret


; Cycles until first PORTC write: 9
.global linehandler_Nx200x16
        rjmp    framehandler
linehandler_Nx200x16:
        in      isr_tmp, IO_(SREG)      ; [0] save SREG
        set                             ; [1] T flag indicates if vertical resolution should be halved
        short_nop2                      ; [2][3] for alignment with 160x100
.linehandler_4_bits_per_pixel:
        movw    Z, vptr                 ; [4]
        ; we can use vptr_lo and vptr_hi as spares
        lds     vptr_lo, bytesperline   ; [5][6] use vptr_lo as loop count
1:      ld      vptr_hi, Z+             ; [7][0] 2 cycles
        out     IO_(PORTC), vptr_hi     ; [1]* 1 cycle
        dec     vptr_lo                 ; [2] decrement loop count (1 cycle)
        short_nop2                      ; [3][4]
        brne    1b                      ; [5][6] 2 cycles
        ; restore registers and blank video
        short_nop2                      ; [6][7]
        clrnf   vptr_hi                 ; [0]
        out     IO_(PORTC), vptr_hi     ; [1]*
        dec     linenum
        brts    2f                      ; if T is clear, rewind the bitmap ptr to the start of the line
        lds     vptr_lo, bytesperline
        sub     ZL, vptr_lo             ; move Z back by # of bytes per line
        sbc     ZH, vptr_hi             ; will be zero
2:      out     IO_(SREG), isr_tmp      ; restore SREG
        movw    vptr, Z                 ; save updated bitmap ptr
        lhret


; Cycles until first PORTC write: 9
.global linehandler_Nx100x16
        rjmp    framehandler
linehandler_Nx100x16:
        in      isr_tmp, IO_(SREG)      ; [0] save SREG
        bst     linenum, 0              ; [1] every other line, rewind bitmap ptr
        rjmp    .linehandler_4_bits_per_pixel ; [2][3]


; Black screen
.global linehandler_null
        rjmp    framehandler
linehandler_null:
        dec_line_num
        lhret


; Cycles until first PORTC write: 25
.global linehandler_160x200x4
        rjmp    framehandler
linehandler_160x200x4:
        in      isr_tmp, IO_(SREG)      ; [0] save SREG
        dec     linenum                 ; [1]
        clt                             ; [2] T flag indicates if vertical resolution should be halved
        short_nop2                      ; [3][4]
.linehandler_40_bytes_per_line:
        short_nop2                      ; [5][6]
.linehandler_40_bytes_per_line_no_nop2:
        movw    Z, vptr                 ; [7]
        movw    vptr, X                 ; [0] use vptr to save previous X value
        movw    X, Z                    ; [1] so we can use X to point into the bitmap
        lds     ZL, tilemap_hi          ; [2][3] specifies the bitspread table, i.e. the palette
        out     IO_(GPIOR0), ZL         ; [4]
        push    r16                     ; [5][6]
        lds     r16, bytesperline       ; [7][0]
        in      ZH, IO_(GPIOR0)         ; [1] Z points into bitspread table
        short_nop2                      ; [2][3]

1:      ld      ZL, X+                  ; [4][5] get 4 pixels (2 cycles)
        lpm     ZH, Z                   ; [6][7][0] get 8 bits for left 2 pixels (3 cycles)
        out     IO_(PORTC), ZH          ; [1]* output 8 bits for left 2 pixels (1 cycle)
        swap    ZL                      ; [2] select upper nibble (1 cycle)
        dec     r16                     ; [3] decrement loop count (1 cycle)
        in      ZH, IO_(GPIOR0)         ; [4] Z points into bitspread table again (1 cycle)
        lpm     ZL, Z                   ; [5][6][7] get 8 bits for right 2 pixels (3 cycles)
        nop                             ; [0]
        out     IO_(PORTC), ZL          ; [1]* output 8 bits for right 2 pixels (1 cycle)
        brne    1b                      ; [2][3] loop if bytes left (2 cycles, 1 on last iteration)
        ; restore registers and blank video
        pop     r16                     ; [3][4]
        lds     ZL, bytesperline        ; [5][6]
        nop                             ; [7]
        clrnf   ZH                      ; [0]
        out     IO_(PORTC), ZH          ; [1]*
        brtc    2f              ; if T is set, rewind the bitmap ptr to the start of the line
        sub     XL, ZL
        sbc     XH, ZH
        ; restore SREG and X, save new bitmap ptr
2:      out     IO_(SREG), isr_tmp
        movw    Z, X
        movw    X, vptr                 ; restore previous X value
        movw    vptr, Z                 ; save updated bitmap ptr
        lhret

; Cycles until first PORTC write: 25
.global linehandler_160x100x4
        rjmp    framehandler
linehandler_160x100x4:
        in      isr_tmp, IO_(SREG)      ; [0] save SREG
        dec     linenum                 ; [1]
        bst     linenum, 0              ; [2] every other line, rewind bitmap ptr
        rjmp    .linehandler_40_bytes_per_line  ; [3][4]

; Cycles until first PORTC write: 25
.global linehandler_160x50x4
        rjmp    framehandler
linehandler_160x50x4:
        in      isr_tmp, IO_(SREG)      ; [0] save SREG
        dec     linenum                 ; [1]
        bst     linenum, 0              ; [2] every fourth line, rewind bitmap ptr
        sbrc    linenum, 1              ; [3]
        set                             ; [4]
        rjmp    .linehandler_40_bytes_per_line_no_nop2 ; [5][6]

; Cycles until first PORTC write: 25
.global linehandler_80x25_text
        rjmp    framehandler
linehandler_80x25_text:
        ; initialize pointers, save registers
        in      isr_tmp, IO_(SREG)      ; [0]
        dec     linenum                 ; [1]
        movw    Z, vptr                 ; [2]
        movw    vptr, X                 ; [3] use vptr to save previous X value
        movw    X, Z                    ; [4] so we can use X to point into the name table
        ; compute tilemap pointer high byte for this row
        lds     ZL, vscroll             ; [5][6]
        add     ZL, linenum             ; [7]
        andi    ZL, 7                   ; [0]
        lds     ZH, tilemap_hi          ; [1][2]
        add     ZH, ZL                  ; [3]
        out     IO_(GPIOR0), r16        ; [4] need a temp register, save in GPIOR0 instead of the stack
        ; is the tilemap in ROM or RAM?
        lds     ZL, tilemap_hh          ; [5][6] tilemap pointer page byte
        sbrc    ZL, TILEMAP_IS_RAM_BIT  ; [7][0] (if skipped)
        rjmp    .linehandler_80x25_text_ramfont ; [0][1] (if not skipped)
.linehandler_80x25_text_romfont:
        ; save previous RAMPZ0 bit in the carry flag, and set new RAMPZ0
        ; (only lowest bit of RAMPZ is significant on 128K flash parts)
        in      r16, IO_(RAMPZ)         ; [1]
        lsr     r16                     ; [2]
        out     IO_(RAMPZ), ZL          ; [3] set new RAMPZ0
        ; do the first 2 characters
        ld      ZL, X+                  ; [4][5] get character table offset (2 cycles)
        elpm    ZL, Z                   ; [6][7][0] get 8 pixels (3 cycles)
        out     IO_(PORTC), ZL          ; [1] output (1 cycle)
        ldi     r16, 39                 ; [2] initialize loop count (1 cycle)
        nop                             ; [3] spare (1 cycle)
        ld      ZL, X+                  ; [4][5] get character table offset (2 cycles)
        elpm    ZL, Z                   ; [6][7][0] get 8 pixels (3 cycles)
        out     IO_(PORTC), ZL          ; [1] output (1 cycle)
        short_nop2                      ; [2][3] spare (2 cycles)
        ; loop (each iteration takes 16 cycles with 2 characters per iteration)
1:      ld      ZL, X+                  ; [4][5] get character table offset (2 cycles)
        elpm    ZL, Z                   ; [6][7][0] get 8 pixels (3 cycles)
        out     IO_(PORTC), ZL          ; [1] output (1 cycle)
        dec     r16                     ; [2] decrement loop count (1 cycle)
        nop                             ; [3] spare (1 cycle)
        ld      ZL, X+                  ; [4][5] get character table offset (2 cycles)
        elpm    ZL, Z                   ; [6][7][0] get 8 pixels (3 cycles)
        out     IO_(PORTC), ZL          ; [1] output (1 cycle)
        brne    1b                      ; [2][3] loop if characters left (2 cycles, 1 on last iteration)
        ; blank video, restore registers
        rol     r16                     ; [3] bring carry flag (old RAMPZ0) back into the low bit of a register
        out     IO_(RAMPZ), r16         ; [4] restore old RAMPZ0
.linehandler_80x25_text_common_end:
        in      r16, IO_(GPIOR0)        ; [5] restore r16
        clrnf   ZL                      ; [6]
        short_nop2                      ; [7][0]
        out     IO_(PORTC), ZL          ; [1] blank video
        ; unless this was the 8th row, rewind X back to the start of the line
        lds     ZL, tilemap_hi
        cp      ZH, ZL
        breq    2f
        subi    XL, lo8(80)
        sbci    XH, hi8(80)
2:      movw    Z, X
        movw    X, vptr                 ; restore X register
        movw    vptr, Z                 ; save new vptr
        out     IO_(SREG), isr_tmp      ; restore flags
        lhret

.linehandler_80x25_text_ramfont:
        ; delay to match the horizontal alignment of rom font
        nop                             ; [2]
        ; loop (each iteration takes 8 cycles)
        ldi     r16, 80                 ; [3]
3:      ld      ZL, X+                  ; [4][5] get character table offset (2 cycles)
        ld      ZL, Z                   ; [6][7] get 8 pixels (2 cycles)
        dec     r16                     ; [0] decrement loop count (1 cycle)
        out     IO_(PORTC), ZL          ; [1] output (1 cycle)
        brne    3b                      ; [2][3] loop if characters left (2 cycles, 1 on last iteration)
        rjmp    .linehandler_80x25_text_common_end ; [3][4]


; number in [brackets] indicates expected value of TCNT2 at start of this instruction
; TCNT2 should always be 0 when entering a linehandler
; Cycles until first PORTC write: 33
.global linehandler_40x25_text
        rjmp    framehandler
linehandler_40x25_text:
9:      movw    Z, vptr                 ; [0]
        movw    vptr, X                 ; [1] use vptr to save previous X value
        movw    X, Z                    ; [2] so we can use X to point into the name table
        in      isr_tmp, IO_(SREG)      ; [3]
        dec     linenum                 ; [4]
        ; compute tilemap pointer high byte for this row
        lds     ZL, vscroll             ; [5][6]
        add     ZL, linenum             ; [7]
        andi    ZL, 7                   ; [0]
        lds     ZH, tilemap_hi          ; [1][2]
        add     ZH, ZL                  ; [3]
        ; cache it in GPIOR0
        out     IO_(GPIOR0), ZH         ; [4]
        ; is the tilemap in ROM or RAM?
        lds     ZL, tilemap_hh          ; [5][6] tilemap pointer page byte
        sbrc    ZL, TILEMAP_IS_RAM_BIT  ; [7][0] (if skipped)
        rjmp    .linehandler_40x25_text_ramfont ; [0][1] (if not skipped)
.linehandler_40x25_text_romfont:
        ; we need 2 more registers, use GPIOR1/2 so we don't need the stack
        out     IO_(GPIOR1), r16        ; [1]
        lds     r16, globalcolor        ; [2][3]
        out     IO_(GPIOR2), r17        ; [4]
        ; save previous RAMPZ in the T flag and set new RAMPZ0
        in      r17, IO_(RAMPZ)         ; [5]
        bst     r17, 0                  ; [6]
        out     IO_(RAMPZ), ZL          ; [7]
        ; prep for the first character
        ld      ZL, X+                  ; [0][1] get first character (2 cycles)
        elpm    ZL, Z                   ; [2][3][4] get 8 pixels for first character (3 cycles)
        mov     ZH, r16                 ; [5] Z now points into bitspread table (1 cycle)
        ; loop (16 cycles per character)
        .rept 39
                lpm     r17, Z          ; [6][7][0] get right 8 pixels from bitspread table (3 cycles)
                out     IO_(PORTC), r17 ; [1]* output left 8 pixels (1 cycle)
                swap    ZL              ; [2] select upper nibble (1 cycle)
                lpm     r17, Z          ; [3][4][5] get left 8 pixels from bitspread table (3 cycles)
                ld      ZL, X+          ; [6][7] get next character (2 cycles)
                in      ZH, IO_(GPIOR0) ; [0] Z points into tilemap (1 cycle)
                out     IO_(PORTC), r17 ; [1]* output right 8 pixels (1 cycle)
                elpm    ZL, Z           ; [2][3][4] get 8 pixels for character (3 cycles)
                mov     ZH, r16         ; [5] Z now points into bitspread table (1 cycle)
        .endr
        ; tidy up the last character
        lpm     r17, Z                  ; [6][7][0] get left 8 pixels from bitspread table (3 cycles)
        out     IO_(PORTC), r17         ; [1]* output left 8 pixels (1 cycle)
        swap    ZL                      ; [2] select upper nibble (1 cycle)
        lpm     ZH, Z                   ; [3][4][5] get right 8 pixels from bitspread table (3 cycles)
        ; use the spare cycles until the next shift register strobe to pop registers and restore RAMPZ
        bld     r17, 0                  ; [6]
        out     IO_(RAMPZ), r17         ; [7]
        in      r16, IO_(GPIOR1)        ; [0]
        out     IO_(PORTC), ZH          ; [1]* output right 8 pixels (1 cycle)
        in      r17, IO_(GPIOR2)        ; [2]
        ; restore proper values to GPIOR1/GPIOR2 and blank video
        ldi     ZL, pm_lo8(9b)          ; [3]
        out     IO_(GPIOR1), ZL         ; [4]
        ldi     ZL, pm_hi8(9b)          ; [5]
        out     IO_(GPIOR2), ZL         ; [6]
        clrnf   ZL                      ; [7]
        nop                             ; [0]
        out     IO_(PORTC), ZL          ; [1]* blank video
.linehandler_40x25_text_common_end:
        ; unless this was the 8th row, rewind X back to the start of the line
        in      ZH, IO_(GPIOR0)
        lds     ZL, tilemap_hi
        cpse    ZH, ZL
        sbiw    X, 40
        ; restore registers, save new vptr
.linehandler_40x25_text_common_end2:
        movw    Z, X
        movw    X, vptr                 ; restore X register
        movw    vptr, Z                 ; save new vptr
        out     IO_(SREG), isr_tmp      ; restore flags
        lhret
.linehandler_40x25_text_ramfont:
        ; only need one extra register here
        out     IO_(GPIOR2), r17        ; [2]
        ; spare cycles to match length of rom font branch
        short_nop5                      ; [3][4][5][6][7]
        ; prep for the first character
        ld      ZL, X+                  ; [0][1] get first character (2 cycles)
        ld      ZL, Z                   ; [2][3] get 8 pixels for first character (2 cycles)
        lds     ZH, globalcolor         ; [4][5]
        ; loop (16 cycles per character)
        .rept 39
                lpm     r17, Z          ; [6][7][0] get left 8 pixels from bitspread table (3 cycles)
                out     IO_(PORTC), r17 ; [1]* output left 8 pixels (1 cycle)
                swap    ZL              ; [2] select upper nibble (1 cycle)
                lpm     r17, Z          ; [3][4][5] get right 8 pixels from bitspread table (3 cycles)
                ld      ZL, X+          ; [6][7] get next character (2 cycles)
                in      ZH, IO_(GPIOR0) ; [0] Z points into tilemap (1 cycle)
                out     IO_(PORTC), r17 ; [1]* output right 8 pixels (1 cycle)
                ld      ZL, Z           ; [2][3] get 8 pixels for character (2 cycles)
                lds     ZH, globalcolor ; [4][5] Z now points into bitspread table (2 cycles)
        .endr
        ; tidy up the last character
        lpm     r17, Z                  ; [6][7][0] get left 8 pixels from bitspread table (3 cycles)
        out     IO_(PORTC), r17         ; [1]* output left 8 pixels (1 cycle)
        swap    ZL                      ; [2] select upper nibble (1 cycle)
        lpm     ZH, Z                   ; [3][4][5] get right 8 pixels from bitspread table (3 cycles)
        ; use the spare cycles until the next shift register strobe to pop registers
        in      r17, IO_(GPIOR2)        ; [6]
        ldi     ZL, pm_hi8(9b)          ; [7] restore GPIOR2
        out     IO_(GPIOR2), ZL         ; [0]
        out     IO_(PORTC), ZH          ; [1]* output right 8 pixels
        ; blank video
        short_nop3                      ; [2][3][4]
        short_nop3                      ; [5][6][7]
        clrnf   ZL                      ; [0]
        out     IO_(PORTC), ZL          ; [1]*
        rjmp    .linehandler_40x25_text_common_end



; Default framehandler for all modes.
framehandler:
        ; reset pixel pointer to start of buffer
        lds     ZL, screen_ptr
        lds     ZH, screen_ptr+1
        movw    vptr, Z
        ; initialize line count
        ; linehandler is responsible for decrementing it!
        ldi     r31, ACTIVE_VIDEO_LINES
        mov     linenum, r31
        fhret



.global linehandler_40x25_color_text_romfont
        rjmp    framehandler
linehandler_40x25_color_text_romfont:
9:      ; initialize pointers, save registers
        in      isr_tmp, IO_(SREG)      ; [0]
        dec     linenum                 ; [1]
        movw    Z, vptr                 ; [2]
        movw    vptr, X                 ; [3] use vptr to save previous X value
        movw    X, Z                    ; [4] so we can use X to point into the name table
        out     IO_(GPIOR1), YL         ; [5] save Y in GPIOR1/2 instead of the stack
        out     IO_(GPIOR2), YH         ; [6]
        ; Y points to the color table (1000 bytes past the name table)
        movw    Y, X                    ; [7]
        subi    YL, lo8(-1000)          ; [0]
        sbci    YH, hi8(-1000)          ; [1]
        ; compute tilemap address high byte for this line
        lds     ZL, vscroll             ; [2][3]
        add     ZL, linenum             ; [4]
        andi    ZL, 7                   ; [5]
        lds     ZH, tilemap_hi          ; [6][7]
        add     ZH, ZL                  ; [0]
        ; cache it in GPIOR0
        out     IO_(GPIOR0), ZH         ; [1]
        ; save previous RAMPZ0 bit in the carry flag, and set new RAMPZ0
        ; (only lowest bit of RAMPZ is significant on 128K flash parts)
        push    r16                     ; [2][3]
        lds     ZL, tilemap_hh          ; [4][5] tilemap pointer page byte
        in      r16, IO_(RAMPZ)         ; [6]
        lsr     r16                     ; [7]
        out     IO_(RAMPZ), ZL          ; [0] set new RAMPZ0
        ; this is where things get crazy... this mode requires 17 cycles to get
        ; the pixels for each tile, but tiles are output every 16 cycles.
        ; so we preload the first few pixels in registers, which requires quite
        ; a bit of time (hence the large X offset correction).
        ; the code to read and output the pixel data is fully unrolled, and
        ; autogenerated using the script `codegen_40x25_color_text_romfont.py`.
        push    r17                     ; [1][2]
        push    r18                     ; [3][4]
        push    r19                     ; [5][6]
        push    r20                     ; [7][0]
        #include "linehandler_40x25_color_text_romfont.inc"
        pop     r20
        pop     r19
        pop     r18
        clrnf   ZL
        out     IO_(PORTC), ZL
        pop     r17
        rol     r16             ; bring carry flag (old RAMPZ0) back into the low bit of a register
        out     IO_(RAMPZ), r16 ; restore old RAMPZ0
        pop     r16
        ; unless this was the 8th row, rewind X back to the start of the line
        lds     ZL, tilemap_hi
        in      ZH, IO_(GPIOR0)
        cpse    ZH, ZL
        sbiw    X, 40
        in      YL, IO_(GPIOR1)
        in      YH, IO_(GPIOR2)
        ldi     ZL, pm_lo8(9b)
        out     IO_(GPIOR1), ZL
        ldi     ZL, pm_hi8(9b)
        out     IO_(GPIOR2), ZL
        rjmp    .linehandler_40x25_text_common_end2

; Cycles until first PORTC write: 33
.global linehandler_40x25_color_text_ramfont
        rjmp    framehandler
linehandler_40x25_color_text_ramfont:
9:      ; initialize pointers, save registers
        in      isr_tmp, IO_(SREG)      ; [0]
        dec     linenum                 ; [1]
        movw    Z, vptr                 ; [2]
        movw    vptr, X                 ; [3] use vptr to save previous X value
        movw    X, Z                    ; [4] so we can use X to point into the name table
        out     IO_(GPIOR1), YL         ; [5] save Y in GPIOR1/2 instead of the stack
        out     IO_(GPIOR2), YH         ; [6]
        ; Y points to the color table (1000 bytes past the name table)
        movw    Y, X                    ; [7]
        subi    YL, lo8(-1000)          ; [0]
        sbci    YH, hi8(-1000)          ; [1]
        ; compute tilemap address high byte for this line
        lds     ZL, vscroll             ; [2][3]
        add     ZL, linenum             ; [4]
        andi    ZL, 7                   ; [5]
        lds     ZH, tilemap_hi          ; [6][7]
        add     ZH, ZL                  ; [0]
        ; cache it in GPIOR0
        out     IO_(GPIOR0), ZH         ; [1]
        ; set Z flag if this is an 8th row
        lds     ZL, tilemap_hi          ; [2][3]
        cp      ZH, ZL                  ; [4]
        ; only need one temporary
        push    r16                     ; [5][6]
        ; align to the shift register strobe
        nop                             ; [7]
        ; prep for the first character
        ld      ZL, X+                  ; [0][1] get first character (2 cycles)
        ld      ZL, Z                   ; [2][3] get 8 pixels for first character (2 cycles)
        ld      ZH, Y+                  ; [4][5] get color for first character (2 cycle)
        ; loop (16 cycles per character)
        .rept 39
                lpm     r16, Z          ; [6][7][0] get left 8 pixels from bitspread table (3 cycles)
                out     IO_(PORTC), r16 ; [1]* output left 8 pixels (1 cycle)
                swap    ZL              ; [2] select upper nibble (1 cycle)
                lpm     r16, Z          ; [3][4][5] get right 8 pixels from bitspread table (3 cycles)
                ld      ZL, X+          ; [6][7] get next character (2 cycles)
                in      ZH, IO_(GPIOR0) ; [0] Z points into tilemap (1 cycle)
                out     IO_(PORTC), r16 ; [1]* output right 8 pixels (1 cycle)
                ld      ZL, Z           ; [2][3] get 8 pixels for character (2 cycles)
                ld      ZH, Y+          ; [4][5] get color for character, Z now points into bitspread table (2 cycles)
        .endr
        ; tidy up the last character
        lpm     r16, Z                  ; [6][7][0] get left 8 pixels from bitspread table (3 cycles)
        out     IO_(PORTC), r16         ; [1]* output left 8 pixels (1 cycle)
        swap    ZL                      ; [2] select upper nibble (1 cycle)
        nop                             ; [3]
        lpm     ZH, Z                   ; [4][5][6] get right 8 pixels from bitspread table (3 cycles)
        ; use the spare cycles until the next shift register strobe to pop registers
        pop     r16                     ; [7][0]
        out     IO_(PORTC), ZH          ; [1]* output right 8 pixels (1 cycle)
        in      YL, IO_(GPIOR1)         ; [2]
        in      YH, IO_(GPIOR2)         ; [3]
        ldi     ZL, pm_lo8(9b)          ; [4]
        out     IO_(GPIOR1), ZL         ; [5]
        ldi     ZL, pm_hi8(9b)          ; [6]
        out     IO_(GPIOR2), ZL         ; [7]
        ; blank video
        clrnf   ZL                      ; [0]
        out     IO_(PORTC), ZL          ; [1]*
        ; epilogue is the same as mono mode
        rjmp    .linehandler_40x25_text_common_end



; Cycles until first PORTC write: 33
.global linehandler_80x100x4
        rjmp    framehandler
linehandler_80x100x4:
9:      in      isr_tmp, IO_(SREG)      ; [0] save SREG
        dec     linenum                 ; [1]
        movw    Z, vptr                 ; [2]
        movw    vptr, X                 ; [3] use vptr to save previous X value
        movw    X, Z                    ; [4] so we can use X to point into the bitmap
        lds     ZL, tilemap_hi          ; [5][6] specifies the bitspread table, i.e. the palette
        out     IO_(GPIOR0), ZL         ; [7] store in GPIOR0
; we need 3 temp registers; use GPIOR1/2 to save 2 of them
        out     IO_(GPIOR1), r16        ; [0]
        out     IO_(GPIOR2), r17        ; [1]
        push    r18                     ; [2][3]
        clr     r17                     ; [4]
        clr     r18                     ; [5]
.rept 20
        ld      ZL, X+                  ; [6][7] get 4 pixels
; expand 8 bits to 16 bits
        in      ZH, IO_(GPIOR0)         ; [0] ; Z points into bitspread table
        out     IO_(PORTC), r18         ; [1]* output third pixel from previous byte
        lpm     r16, Z                  ; [2][3][4] get left 2 pixels
        swap    ZL                      ; [5]
        lpm     r18, Z                  ; [6][7][0] get right 2 pixels
; expand 16 bits to 32 bits
        out     IO_(PORTC), r17         ; [1]* output fourth pixel from previous byte
        mov     ZL, r16                 ; [2] r16 contains left 2 pixels from this byte
        andi    ZL, 0b11110000          ; [3] duplicate upper nibble
        mov     ZH, ZL                  ; [4]
        swap    ZH                      ; [5]
        or      ZL, ZH                  ; [6] ZL now contains first pixel from this byte
        andi    r16, 0b00001111         ; [7] duplicate lower nibble
        mov     ZH, r16                 ; [0]
        out     IO_(PORTC), ZL          ; [1]* output first pixel from this byte
        swap    ZH                      ; [2]
        or      r16, ZH                 ; [3] r16 now contains second pixel from this byte
        mov     r17, r18                ; [4] r18 contains upper 2 pixels from this byte
        andi    r18, 0b11110000         ; [5] duplicate upper nibble
        mov     ZH, r18                 ; [6]
        swap    ZH                      ; [7]
        or      r18, ZH                 ; [0] r18 now contains third pixel from this byte
        out     IO_(PORTC), r16         ; [1]* output second pixel from this byte
        andi    r17, 0b00001111         ; [2] duplicate lower nibble
        mov     ZH, r17                 ; [3]
        swap    ZH                      ; [4]
        or      r17, ZH                 ; [5] r17 now contains fourth pixel from this byte
.endr
        ; finish up last 2 pixels from last byte, restore registers
        short_nop3                      ; [6][7][0]
        out     IO_(PORTC), r18         ; [1]* output third pixel from last byte
        pop     r18                     ; [2][3]
        in      r16, IO_(GPIOR1)        ; [4]
        ldi     ZL, pm_lo8(9b)          ; [5]
        out     IO_(GPIOR1), ZL         ; [6]
        short_nop2                      ; [7][0]
        out     IO_(PORTC), r17         ; [1]* output fourth pixel from last byte
        ; finish restoring registers and blank video
        in      r17, IO_(GPIOR2)        ; [2]
        ldi     ZL, pm_hi8(9b)          ; [3]
        out     IO_(GPIOR2), ZL         ; [4]
        short_nop3                      ; [5][6][7]
        clrnf   ZL                      ; [0]
        out     IO_(PORTC), ZL          ; [1]*
        sbrc    linenum, 0              ; rewind bitmap ptr every other line
        sbiw    X, 20
        ; restore SREG and X, save new bitmap ptr
        out     IO_(SREG), isr_tmp
        movw    Z, X
        movw    X, vptr                 ; restore previous X value
        movw    vptr, Z                 ; save updated bitmap ptr
        jmp     linehandler_end



; Cycles until first PORTC write: 17
.global linehandler_160x100_mono
        rjmp    framehandler
linehandler_160x100_mono:
        in      isr_tmp, IO_(SREG)      ; [0] save SREG
        dec     linenum                 ; [1]
        bst     linenum, 0              ; [2] every other line, rewind bitmap ptr
        rjmp    .linehandler_20_bytes_per_line_mono  ; [3][4]

; Cycles until first PORTC write: 17
.global linehandler_160x200_mono
        rjmp    framehandler
linehandler_160x200_mono:
        in      isr_tmp, IO_(SREG)      ; [0] save SREG
        dec     linenum                 ; [1]
        clt                             ; [2] T flag indicates if vertical resolution should be halved
        short_nop2                      ; [3][4]
.linehandler_20_bytes_per_line_mono:
        short_nop3                      ; [5][6][7]
        movw    Z, vptr                 ; [0]
        movw    vptr, r16               ; [1] use vptr to save two high registers
.rept 20
        ld      r16, Z+                 ; [2][3] get 8 pixels
        clr     r17                     ; [4]
        sbrc    r16, 6                  ; [5]
        ori     r17, 0b00001111         ; [6]
        sbrc    r16, 7                  ; [7]
        ori     r17, 0b11110000         ; [0]
        out     IO_(PORTC), r17         ; [1]* output two pixels
        short_nop2                      ; [2][3]
        clr     r17                     ; [4]
        sbrc    r16, 4                  ; [5]
        ori     r17, 0b00001111         ; [6]
        sbrc    r16, 5                  ; [7]
        ori     r17, 0b11110000         ; [0]
        out     IO_(PORTC), r17         ; [1]* output two pixels
        short_nop2                      ; [2][3]
        clr     r17                     ; [4]
        sbrc    r16, 2                  ; [5]
        ori     r17, 0b00001111         ; [6]
        sbrc    r16, 3                  ; [7]
        ori     r17, 0b11110000         ; [0]
        out     IO_(PORTC), r17         ; [1]* output two pixels
        short_nop2                      ; [2][3]
        clr     r17                     ; [4]
        sbrc    r16, 0                  ; [5]
        ori     r17, 0b00001111         ; [6]
        sbrc    r16, 1                  ; [7]
        ori     r17, 0b11110000         ; [0]
        out     IO_(PORTC), r17         ; [1]* output two pixels
.endr
        ; blank video
        clrnf   r17                     ; [0]
        nop3                            ; [1][2][3]
        nop3                            ; [5][6][7]
        out     IO_(PORTC), r17         ; [1]*
        ; restore registers
        movw    r16, vptr
        brtc    2f                      ; if T is set, rewind the bitmap ptr to the start of the line
        sbiw    Z, 20
2:      movw    vptr, Z
        out     IO_(SREG), isr_tmp
        jmp     linehandler_end


; 40x25 text mode. Hardcoded to ROM font and white-on-black. vscroll has no effect.
; Useful for split-screen modes.
; Cycles until first PORTC write: 25
.global linehandler_40x25_basictext
        rjmp    framehandler
linehandler_40x25_basictext:
9:      movw    Z, vptr                 ; [0]
        movw    vptr, X                 ; [1] use vptr to save previous X value
        movw    X, Z                    ; [2] so we can use X to point into the name table
        in      isr_tmp, IO_(SREG)      ; [3]
        dec     linenum                 ; [4]
        ; compute tilemap pointer high byte for this row
        ; (does not take vscroll into account)
        ldi     ZL, 7                   ; [5]
        and     ZL, linenum             ; [6]
        ldi     ZH, hi8(amscii_font_8x8) ; [7]
        add     ZH, ZL                  ; [0]
        ; cache it in GPIOR0
        out     IO_(GPIOR0), ZH         ; [1]
        ; save r16
        out     IO_(GPIOR1), r16        ; [2]
        short_nop5                      ; [3][4][5][6][7]
        ; prep for the first character
        ld      ZL, X+                  ; [0][1] get first character (2 cycles)
        lpm     ZL, Z                   ; [2][3][4] get 8 pixels for first character (3 cycles)
        ldi     ZH, hi8(bitspreadtable+(256*15)) ; [5] Z now points into bitspread table (1 cycle)
        ; loop (16 cycles per character)
        .rept 39
                lpm     r16, Z          ; [6][7][0] get right 8 pixels from bitspread table (3 cycles)
                out     IO_(PORTC), r16 ; [1]* output left 8 pixels (1 cycle)
                swap    ZL              ; [2] select upper nibble (1 cycle)
                lpm     r16, Z          ; [3][4][5] get left 8 pixels from bitspread table (3 cycles)
                ld      ZL, X+          ; [6][7] get next character (2 cycles)
                in      ZH, IO_(GPIOR0) ; [0] Z points into tilemap (1 cycle)
                out     IO_(PORTC), r16 ; [1]* output right 8 pixels (1 cycle)
                lpm     ZL, Z           ; [2][3][4] get 8 pixels for character (3 cycles)
                ldi     ZH, hi8(bitspreadtable+(256*15)) ; [5] Z now points into bitspread table (1 cycle)
        .endr
        ; tidy up the last character
        lpm     r16, Z                  ; [6][7][0] get left 8 pixels from bitspread table (3 cycles)
        out     IO_(PORTC), r16         ; [1]* output left 8 pixels (1 cycle)
        swap    ZL                      ; [2] select upper nibble (1 cycle)
        lpm     ZH, Z                   ; [3][4][5] get right 8 pixels from bitspread table (3 cycles)
        ; restore r16 and GPIOR1
        in      r16, IO_(GPIOR1)        ; [6]
        ldi     ZL, pm_lo8(9b)          ; [7]
        out     IO_(GPIOR1), ZL         ; [0]
        out     IO_(PORTC), ZH          ; [1]* output right 8 pixels (1 cycle)
        ; blank video
        short_nop2                      ; [2][3]
        short_nop2                      ; [4][5]
        in      ZH, IO_(GPIOR0)         ; [6]
        cpi     ZH, hi8(amscii_font_8x8) ; [7] set Z flag if this was the 8th for
        clrnf   ZL                      ; [0]
        out     IO_(PORTC), ZL          ; [1]*
        ; unless this was the 8th row, rewind X back to the start of the line
        breq    1f
        sbiw    X, 40
        ; restore registers, save new vptr
1:      movw    Z, X
        movw    X, vptr                 ; restore X register
        movw    vptr, Z                 ; save new vptr
        out     IO_(SREG), isr_tmp      ; restore flags
        jmp     linehandler_end

        ; ; use the spare cycles until the next shift register strobe to pop registers and restore RAMPZ
        ; bld     r17, 0                  ; [6]
        ; out     IO_(RAMPZ), r17         ; [7]
        ; in      r16, IO_(GPIOR1)        ; [0]
        ; out     IO_(PORTC), ZH          ; [1]* output right 8 pixels (1 cycle)
        ; in      r17, IO_(GPIOR2)        ; [2]
        ; ; restore proper values to GPIOR1/GPIOR2 and blank video
        ; ldi     ZL, pm_lo8(9b)          ; [3]
        ; out     IO_(GPIOR1), ZL         ; [4]
        ; ldi     ZL, pm_hi8(9b)          ; [5]
        ; out     IO_(GPIOR2), ZL         ; [6]
        ; clrnf   ZL                      ; [7]
        ; nop                             ; [0]
        ; out     IO_(PORTC), ZL          ; [1]* blank video
        ; unless this was the 8th row, rewind X back to the start of the line
        ; in      ZH, IO_(GPIOR0)
        ; lds     ZL, tilemap_hi
        ; cpse    ZH, ZL
        ; sbiw    X, 40
        ; restore registers, save new vptr
; .linehandler_40x25_text_common_end2:
        ; movw    Z, X
        ; movw    X, vptr                 ; restore X register
        ; movw    vptr, Z                 ; save new vptr
        ; out     IO_(SREG), isr_tmp      ; restore flags
        ; lhret

.section .text
.end
