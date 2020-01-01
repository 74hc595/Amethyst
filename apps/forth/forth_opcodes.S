; Ah yes, there's nothing like a 5000 line .S file.
; The location of each routine relative to the locations of the dispatch tables
; are important. A dispatch table must be able to reach every routine via RJMP.
; The linker can't be trusted to do this correctly. And it also likes to add
; *lots* of padding, for some reason the ".align 9" directive used to align each
; dispatch table forces 512-byte alignment for the *entire* object file.
; I suppose I could break it all down into smaller .inc files...
#include "forth_opcodes.h"
#include "forth_macros.inc"
#include "forth_defs.h"
#include "videodefs.h"

; Base address used for high-page memory access instructions
; (C#+H@) (C#+H!) (C#+HC@) (C#+HC!)
.global high_page_base
.equ high_page_base, (RAMEND+1)-0x100

; Drawing operation dispatch table aligned to 256-word boundary
#include "draw_ops.inc"

; Graphics/drawing routines
; Lots of code duplication here to avoid the penalty (speed and stack usage)
; of function calls and branching.
; I could macro-ize the duplicated parts I guess
opcode_txres:
opcode_hxres:
        pushd
        lds     TOSL, bytesperline
        clr     TOSH
        rnext
opcode_mxres:
        pushd
        lds     TOSL, bytesperline
        lsl     TOSL                    ; 2 pixels per byte
        clr     TOSH
        rnext
opcode_lxres:
        pushd
        lds     TOSL, bytesperline
        lsl     TOSL                    ; 4 pixels per byte
        lsl     TOSL
        clr     TOSH
        rnext
opcode_bxres:
        pushd
        lds     TOSL, bytesperline
        lsl     TOSL                    ; 8 pixels per byte
        clr     TOSH
        lsl     TOSL
        rol     TOSH
        lsl     TOSL
        rol     TOSH
        rnext

opcode_tgetcolor:
        pushd
        lds     TOSL, globalcolor
        ldi     TOSH, hi8(bitspreadtable)       ; TODO how to do this with one subi?
        sub     TOSL, TOSH
        clr     TOSH
        rnext
opcode_hgetcolor:
        pushd
        lds     TOSL, globalcolor
        clr     TOSH
        rnext
opcode_mgetcolor:
        pushd
        lds     TOSL, globalcolor
        andi    TOSL, 0b00001111
        clr     TOSH
        rnext
opcode_lgetcolor:
        pushd
        lds     TOSL, globalcolor
        andi    TOSL, 0b00000011
        clr     TOSH
        rnext
opcode_bgetcolor:
        pushd
        lds     TOSL, globalcolor
        andi    TOSL, 0b00000001
        clr     TOSH
        rnext


; TCOLOR! ( c -- ) set current text color to c
opcode_tsetcolor:
        cpi     TOSL, bitspreadtable_numpatterns
        cpc     TOSH, ZEROH
        brlo    1f
        ldi     TOSL, bitspreadtable_numpatterns-1
1:      ldi     TOSH, hi8(bitspreadtable)       ; TODO how to do this with one subi?
        add     TOSL, TOSH
        ; fall through

; HCOLOR! ( c -- ) set high-color graphics mode current color to c (0-255)
opcode_hsetcolor:
        sts     globalcolor, TOSL
        popd
        rnext   ; all draw ops have to restore Z

; MCOLOR! ( c -- ) set medium-color graphics mode current color to c (0-15)
opcode_msetcolor:
        ldi     ZH, hi8(lowernibbledoubletable)
        mov     ZL, TOSL
        lpm     TOSL, Z
        sts     globalcolor, TOSL
        popd
        rnext

; LCOLOR! ( c -- ) set low-color graphics mode current color to c (0-3)
opcode_lsetcolor:
        ; expand 2 bits to 8 bits
        ldi     ZH, hi8(crumbquadtable)
        mov     ZL, TOSL
        lpm     TOSL, Z
        sts     globalcolor, TOSL
        popd
        rnext

; BCOLOR! ( c -- ) set mono graphics mode color to 0 (black) or 1 (white)
opcode_bsetcolor:
        lsr     TOSL            ; lsb to carry bit
        clr     TOSL
        sbc     TOSL, TOSL      ; if carry set, TOSH=0xFF, otherwise TOSH=0x00
        sts     globalcolor, TOSL
        popd
        rnext

; MCLEAR ( c -- ) fill medium-color graphics screen with color c (0-15)
opcode_mclear:
        ; expand 4 bits to 8 bits
        ldi     ZH, hi8(lowernibbledoubletable)
        mov     ZL, TOSL
        lpm     TOSL, Z
        ; fall through

; HCLEAR ( c -- ) fill high-color graphics screen with color c (0-255)
; TCLEAR ( c -- ) fill text screen with character c (0-255)
opcode_hclear:
opcode_tclear:
        mov     TMPL, TOSL
        lds     r0, bytesperline
        lds     r1, linesperscreen
        mul     r0, r1          ; get byte count
        movw    TOS, r0         ; move to r25:24 so we can use sbiw
        lds     ZL, screen_ptr
        lds     ZH, screen_ptr+1
1:      st      Z+, TMPL
        sbiw    TOS, 1
        brne    1b
        movw    r0, TOS         ; restore r1:r0 to zero
        popd
        rnext

; LCLEAR ( c -- ) fill low-color graphics screen with color c (0-3)
opcode_lclear:
        ; expand 2 bits to 8 bits
        ldi     ZH, hi8(crumbquadtable)
        mov     ZL, TOSL
        lpm     TOSL, Z
        rjmp    opcode_hclear

; BCLEAR ( c -- ) fill black & white graphics screen with black (if lsb of c
; is 0) or white (if lsb of c is 1)
opcode_bclear:
        lsr     TOSL
        ldi     TOSL, 0
        sbc     TOSL, ZEROL
        rjmp    opcode_hclear


; HPLOT ( xy -- ) set pixel in high-color graphics screen to current color
opcode_hplot_chk:
        lds     TMPL, bytesperline      ; check x coord
        cp      TOSL, TMPL
        brsh    .nohplot
opcode_hplot:
        lds     TMPL, globalcolor
        rjmp    .xystore

opcode_hpset_chk:
opcode_tpset_chk:
        lds     TMPL, bytesperline      ; check x coord
        cp      TOSL, TMPL
        brsh    .nohpset
; XY!  ( c xy -- ) store byte at xy coordinate in screen buffer
; HPSET ( c xy -- ) set color of pixel at xy in high-color bitmap mode
; TSET ( c xy -- ) set character in text cell at xy (color is not affected)
; no bounds checking is performed
opcode_hpset:
opcode_tpset:
opcode_xystore:
        nip                             ; get char into TMPL
.xystore:
        movw    r20, ZERO               ; mul will clobber the zero regs
        lds     r0, bytesperline        ; multiply y coord * bytes per line
        mul     r0, TOSH
        add     r0, TOSL                ; add x coord
        adc     r1, r20                 ; will be zero
        lds     ZL, screen_ptr          ; add screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r20               ; restore zero regs
        st      Z, TMPL
.nohplot:
        popd
        rnext
.nohpset:
        drop2
        rnext

opcode_mpset_chk:
        lds     TMPL, bytesperline      ; check x coord
        lsl     TMPL
        cp      TOSL, TMPL
        brsh    .nompset
; MPSET ( c xy -- ) set pixel in medium-color graphics screen to c
opcode_mpset:
        ld      ZL, DSP+        ; nip color
        ld      TMPL, DSP+      ; discard high byte of NOS
        ldi     ZH, hi8(lowernibbledoubletable)
        lpm     TMPL, Z
        movw    r20, ZERO               ; mul will clobber the zero regs
        lds     r0, bytesperline        ; multiply y coord * bytes per line
        mul     r0, TOSH
        lsr     TOSL                    ; divide x coord by two--odd or even?
        brcs    .mplot_odd
        rjmp    .mplot_even
.nompset:
        drop2
        rnext

; MPLOT ( xy -- ) set pixel in medium-color graphics screen to current color
opcode_mplot_chk:
        lds     TMPL, bytesperline      ; check x coord
        lsl     TMPL
        cp      TOSL, TMPL
        brsh    .nomplot
opcode_mplot:
        lds     TMPL, globalcolor
        movw    r20, ZERO               ; mul will clobber the zero regs
        lds     r0, bytesperline        ; multiply y coord * bytes per line
        mul     r0, TOSH
        lsr     TOSL                    ; divide x coord by two--odd or even?
        brcs    .mplot_odd
; save a couple cycles by unrolling the odd/even branches
.mplot_even:
        add     r0, TOSL                ; add x byte index
        adc     r1, r20                 ; will be zero
        lds     ZL, screen_ptr          ; add screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r20               ; restore zero regs
        ld      TOSL, Z                 ; get existing byte
        andi    TOSL, 0b00001111        ; apply masks to existing and incoming byte values
        andi    TMPL, 0b11110000
        or      TOSL, TMPL              ; or in the new pixel value
        st      Z, TOSL                 ; store new byte
.nomplot:
        popd
        rnext
.mplot_odd:
        add     r0, TOSL                ; add x byte index
        adc     r1, r20                 ; will be zero
        lds     ZL, screen_ptr          ; add screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r20               ; restore zero regs
        ld      TOSL, Z                 ; get existing byte
        andi    TOSL, 0b11110000        ; apply masks to existing and incoming byte values
        andi    TMPL, 0b00001111
        or      TOSL, TMPL              ; or in the new pixel value
        st      Z, TOSL                 ; store new byte
        popd
        rnext


opcode_lpset_chk:
        lds     TMPL, bytesperline      ; check x coord
        lsl     TMPL
        lsl     TMPL
        cp      TOSL, TMPL
        brsh    .nolpset
; LPSET ( c xy -- ) set pixel in low-color graphics screen to c
opcode_lpset:
        ; expand 2 bits to 8 bits
        ld      ZL, DSP+        ; nip color
        ld      TMPL, DSP+      ; discard high byte of NOS
        ldi     ZH, hi8(crumbquadtable)
        lpm     TMPL, Z
        rjmp    .lplot
.nolpset:
        drop2
        rnext

; LPLOT ( xy -- ) set pixel in low-color graphics screen to current color
opcode_lplot_chk:
        lds     TMPL, bytesperline      ; check x coord
        lsl     TMPL
        lsl     TMPL
        cp      TOSL, TMPL
        brsh    .nolplot
opcode_lplot:
        lds     TMPL, globalcolor       ; get color
.lplot: mov     ZL, TOSL                ; get bit mask for x position
        ldi     ZH, hi8(crumbmasktable)
        lpm     TMPH, Z                 ; select the correct byte in color for this x pos
        and     TMPL, TMPH
        com     TMPH                    ; invert mask
        lsr     TOSL                    ; divide x coord by 4 to get byte address
        lsr     TOSL
        movw    r20, ZERO               ; mul will clobber the zero regs
        lds     r0, bytesperline        ; multiply y coord * bytes per line
        mul     r0, TOSH
        add     r0, TOSL                ; add x byte index
        adc     r1, r20                 ; will be zero
        lds     ZL, screen_ptr          ; add screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r20               ; restore zero regs
        ld      TOSL, Z                 ; get existing byte
        and     TOSL, TMPH              ; mask existing pixel value
        or      TOSL, TMPL              ; or in the new pixel value
        st      Z, TOSL                 ; store new byte
.nolplot:
        popd
        rnext

opcode_bpset_chk:
        movw    TMP, ZERO               ; mul will clobber the zero regs
        lds     r0, bytesperline        ; compute x width
        ldi     r18, 8
        mul     r0, r18
        cp      TOSL, r0                ; check x coord, zero-extended to 16 bits
        cpc     TMPL, r1                ; (TMPL will be zero)
        movw    ZERO, TMP               ; restore zero regs (doesn't affect flags)
        brsh    .nobpset
; BPSET ( c xy -- ) set pixel at xy (for x<=255) in mono graphics screen to
; color c (lsb indicates white or black)
; No bounds checking is performed.
; Does not work for x coordinates greater than 255
opcode_bpset:
        nip
        bst     TMPL, 0
        rjmp    .bplot
.nobpset:
        drop2
        rnext

opcode_bplot_chk:
        movw    TMP, ZERO               ; mul will clobber the zero regs
        lds     r0, bytesperline        ; compute x width
        ldi     r18, 8
        mul     r0, r18
        cp      TOSL, r0                ; check x coord, zero-extended to 16 bits
        cpc     TMPL, r1                ; (TMPL will be zero)
        movw    ZERO, TMP               ; restore zero regs (doesn't affect flags)
        brsh    .nobplot
; BPLOT ( xy -- ) set pixel at xy (for x<=255) in mono graphics screen to
; current color (lsb indicates white or black)
; No bounds checking is performed.
; Does not work for x coordinates greater than 255
opcode_bplot:
        lds     TMPL, globalcolor       ; move lsb of color to T flag
        bst     TMPL, 0
.bplot: mov     ZL, TOSL                ; get bit mask for current position
        ldi     ZH, hi8(bitmasktable)
        lpm     TMPH, Z
        lsr     TOSL                    ; divide x coord by 8 to get byte address
        lsr     TOSL
        lsr     TOSL
        movw    r20, ZERO               ; mul will clobber the zero regs
        lds     r0, bytesperline        ; multiply y coord * bytes per line
        mul     r0, TOSH
        add     r0, TOSL                ; add x byte index
        adc     r1, r20                 ; will be zero
        lds     ZL, screen_ptr          ; add screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r20               ; restore zero regs
        ld      TOSL, Z                 ; get existing byte
        brts    .bplot_setpixel
.bplot_clearpixel:
        com     TMPH
        and     TOSL, TMPH
        st      Z, TOSL
.nobplot:
        popd
        rnext
.bplot_setpixel:
        or      TOSL, TMPH              ; or in the new pixel value
        st      Z, TOSL                 ; store new byte
        popd
        rnext

; assumes: stack is ( h -- ) and xy in TMPL/TMPH
opcode_hvlin_chk:
        lds     TOSH, bytesperline      ; check x coord
        cp      TMPL, TOSH
        brlo    1f
.nohvlin:
        popd
        rnext
; HVLIN ( xy h -- ) draw vertical line starting at xy and extending h pixels
; downward in current color (high color modes)
; No bounds checking is performed, and behavior is undefined if h=0.
; The argument order follows the pattern used by ! ( what where -- )
opcode_hvlin:
        nip                             ; get xy into TMP
        lds     TOSH, bytesperline
1:      movw    r20, ZERO               ; mul will clobber the zero regs
        mul     TOSH, TMPH              ; multiply y coord * bytes per line
        add     r0, TMPL                ; add x coord
        adc     r1, r20                 ; will be zero
        lds     ZL, screen_ptr          ; add screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r20               ; restore zero regs
        lds     r20, globalcolor
1:      st      Z, r20
        add     ZL, TOSH                ; advance 1 line
        adc     ZH, ZERO
        dec     TOSL                    ; decrement remaining height
        brne    1b
        popd
        rnext

; assumes: stack is ( h -- ) and xy in TMPL/TMPH
opcode_mvlin_chk:
        lds     TOSH, bytesperline      ; check x coord
        mov     r20, TOSH               ; multiply bytesperline by 2
        lsl     r20
        cp      TMPL, r20
        brlo    1f
.nomvlin:
        popd
        rnext
; MVLIN ( xy h -- ) draw vertical line starting at xy and extending h pixels
; downward in current color (medium color modes)
; No bounds checking is performed, and behavior is undefined if h=0.
opcode_mvlin:
        nip                             ; get xy into TMP
        lds     TOSH, bytesperline
1:      movw    r20, ZERO               ; mul will clobber the zero regs
        mul     TOSH, TMPH              ; multiply y coord * bytes per line
        bst     TMPL, 0                 ; odd pixel or even?
        lsr     TMPL                    ; divide x by 2 to get byte offset
        add     r0, TMPL                ; add x byte index
        adc     r1, r20                 ; will be zero
        lds     ZL, screen_ptr          ; add screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r20               ; restore zero regs
        lds     r20, globalcolor
        brts    .mvlin_odd
.mvlin_even:
        andi    r20, 0b11110000         ; apply mask to existing color
1:      ld      TMPL, Z                 ; get existing byte
        andi    TMPL, 0b00001111        ; apply mask to existing byte value
        or      TMPL, r20               ; or in the new pixel value
        st      Z, TMPL                 ; store new byte
        add     ZL, TOSH                ; advance 1 line
        adc     ZH, ZERO
        dec     TOSL                    ; decrement remaining height
        brne    1b
        popd
        rnext
.mvlin_odd:
        andi    r20, 0b00001111         ; apply mask to existing color
1:      ld      TMPL, Z                 ; get existing byte
        andi    TMPL, 0b11110000        ; apply mask to existing byte value
        or      TMPL, r20               ; or in the new pixel value
        st      Z, TMPL                 ; store new byte
        add     ZL, TOSH                ; advance 1 line
        adc     ZH, ZERO
        dec     TOSL                    ; decrement remaining height
        brne    1b
        popd
        rnext

; assumes: stack is ( h -- ) and xy in TMPL/TMPH
opcode_lvlin_chk:
        lds     r18, bytesperline      ; check x coord
        mov     r20, r18               ; multiply bytesperline by 4
        lsl     r20
        lsl     r20
        cp      TMPL, r20
        brlo    1f
.nolvlin:
        popd
        rnext
; LVLIN ( xy h -- ) draw vertical line starting at xy and extending h pixels
; downward in current color (low color modes)
; No bounds checking is performed, and behavior is undefined if h=0.
opcode_lvlin:
        nip                             ; get xy into TMP
        lds     r18, bytesperline       ; multiply y coord * bytes per line
1:      lds     r19, globalcolor        ; get color
        mov     ZL, TMPL                ; get bit mask for x position
        ldi     ZH, hi8(crumbmasktable)
        lpm     r20, Z
        and     r19, r20                ; select the correct bits in color for this x pos
        com     r20                     ; invert mask
        lsr     TMPL                    ; divide x coord by 4 to get byte address
        lsr     TMPL
        movw    XSAV, ZERO              ; mul will clobber the zero regs
        mul     r18, TMPH               ; multiply y coord * bytes per line
        add     r0, TMPL                ; add x byte offset
        adc     r1, XSAVL               ; will be zero
        lds     ZL, screen_ptr          ; add screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, XSAV              ; restore zero regs
1:      ld      TOSH, Z                 ; get existing byte
        and     TOSH, r20               ; mask existing pixel value
        or      TOSH, r19               ; or in the new pixel value
        st      Z, TOSH                 ; store new byte
        add     ZL, r18                 ; advance 1 line
        adc     ZH, ZERO
        dec     TOSL
        brne    1b
        popd
        rnext

; assumes: stack is ( h -- ) and xy in TMPL/TMPH
opcode_bvlin_chk:
        movw    XSAV, ZERO              ; mul will clobber the zero regs
        lds     r18, bytesperline       ; compute x width
        ldi     r19, 8
        mul     r18, r19
        cp      TMPL, r0                ; check x coord, zero-extended to 16 bits
        cpc     XSAVL, r1               ; (XSAVL will be zero)
        brlo    1f
.nobvlin:
        movw    ZERO, XSAV              ; restore zero regs
        popd
        rnext
; BVLIN ( xy h -- ) draw vertical line starting at xy (for x<=255) and extending
; h pixels downward in current color (mono modes)
; No bounds checking is performed, and behavior is undefined if h=0.
; Does not work for x coordinates greater than 255
opcode_bvlin:
        nip                             ; get xy into TMP
        movw    XSAV, ZERO              ; mul will clobber the zero regs
        lds     r18, bytesperline
1:      lds     r20, globalcolor        ; move lsb of color to T flag
        bst     r20, 0
        mov     ZL, TMPL                ; get bit mask for x position
        ldi     ZH, hi8(bitmasktable)
        lpm     r20, Z                  ; bitmask is now in r20
        lsr     TMPL                    ; divide x coord by 8 to get byte address
        lsr     TMPL
        lsr     TMPL
        mul     r18, TMPH               ; multiply y coord * bytes per line
        add     r0, TMPL                ; add x byte index
        adc     r1, XSAVL               ; will be zero
        lds     ZL, screen_ptr          ; add screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, XSAV              ; restore zero regs
        brts    .bvlin_white
.bvlin_black:
        com     r20                     ; invert mask
1:      ld      r21, Z                  ; get existing byte
        and     r21, r20
        st      Z, r21
        add     ZL, r18                 ; advance 1 line
        adc     ZH, ZERO
        dec     TOSL
        brne    1b
        popd
        rnext
.bvlin_white:
1:      ld      r21, Z                  ; get existing byte
        or      r21, r20                ; or in the new pixel value
        st      Z, r21                  ; store new byte
        add     ZL, r18                 ; advance 1 line
        adc     ZH, ZERO
        dec     TOSL
        brne    1b
        popd
        rnext

; assumes: stack is ( w -- ) and xy in TMPL/TMPH
opcode_hhlin_chk:
        lds     r20, bytesperline      ; check x coord
        cp      TMPL, r20
        brsh    .nohhlin
        tst     TOSL                    ; width can't be zero
        breq    .nohhlin
; clip width to screen bounds
        mov     r21, TMPL               ; add width to x origin
        add     r21, TOSL
        brcs    .cliphlin               ; check for wraparound
        cp      r20, r21                ; check if endpoint > screen width
        brsh    1f                      ; if in bounds, draw line
.cliphlin:                              ; otherwise, clip
        mov     TOSL, r20
        sub     TOSL, TMPL              ; set line width to (screen width - x origin)
        rjmp    1f
; HHLIN ( w xy -- ) draw horizontal line starting at xy and extending w pixels
; to the right, in current color
; No bounds checking is performed, and behavior is undefined if w=0.
opcode_hhlin:
        nip                             ; get xy into TMP
        lds     r20, bytesperline
1:      movw    r18, ZERO               ; mul will clobber the zero regs
        mul     r20, TMPH               ; multiply y coord * bytes per line
        add     r0, TMPL                ; add x coord
        adc     r1, r18                 ; will be zero
        lds     ZL, screen_ptr          ; add screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r18               ; restore zero regs
        lds     r20, globalcolor        ; get color
2:      st      Z+, r20
        dec     TOSL
        brne    2b
.nohhlin:
        popd
        rnext

; assumes: stack is ( w -- ) and xy in TMPL/TMPH
opcode_mhlin_chk:
        lds     r20, bytesperline
        mov     r18, r20                ; multiply bytes per line by 2
        lsl     r18
        cp      TMPL, r18               ; check x coord
        brsh    .nomhlin
        tst     TOSL                    ; width can't be zero
        breq    .nomhlin
; clip width to screen bounds
        mov     r21, TMPL               ; add width to x origin
        add     r21, TOSL
        brcs    .clipmhlin              ; check for wraparound
        cp      r18, r21                ; check if endpoint > screen width
        brsh    1f                      ; if in bounds, draw line
.clipmhlin:                             ; otherwise, clip
        mov     TOSL, r18
        sub     TOSL, TMPL              ; set line width to (screen width - x origin)
        rjmp    1f
; MHLIN
; multiply y by bytesperline
; left shift x
; if carry set, write head byte and advance to next pixel, decrement width
; subtract 2 from width
; if width=0xFF, do the residual last column
opcode_mhlin:
        nip                             ; get xy into TMP
        lds     r20, bytesperline
1:      lds     ZL, screen_ptr          ; get screen offset
        lds     ZH, screen_ptr+1
        movw    r18, ZERO               ; mul will clobber the zero regs
        mul     r20, TMPH               ; multiply y coord * bytes per line
        add     ZL, r0                  ; add row offset
        adc     ZH, r1
        movw    ZERO, r18               ; restore zero regs
        lds     r20, globalcolor        ; get color
        lsr     TMPL                    ; x coord not byte-aligned at start?
        brcc    .mhlin_aligned
.mhlin_unaligned:
        add     ZL, TMPL                ; add x offset to pointer
        adc     ZH, ZERO
        ld      r21, Z
        andi    r21, 0b11110000
        mov     r18, r20
        andi    r18, 0b00001111         ; apply masks
        or      r21, r18
        st      Z+, r21
        dec     TOSL                    ; decrement width
        breq    .mhlin_done             ; we could already be done!
        rjmp    2f
.mhlin_aligned:
        add     ZL, TMPL                ; add x offset to pointer
        adc     ZH, ZERO
2:      subi    TOSL, 2                 ; subtract 2 pixels from remaining width
        brcs    .mhline_residual
        st      Z+, r20
        brne    2b
.nomhlin:
.mhlin_done:
        popd
        rnext
; one last pixel to clean up?
.mhline_residual:
        ld      r21, Z
        andi    r21, 0b00001111
        mov     r18, r20
        andi    r18, 0b11110000
        or      r21, r18
        st      Z+, r21
        popd
        rnext

; assumes: stack is ( w -- ) and xy in TMPL/TMPH
opcode_lhlin_chk:
        lds     r20, bytesperline
        mov     r18, r20                ; multiply bytes per line by 4
        lsl     r18
        lsl     r18
        cp      TMPL, r18               ; check x coord
        brsh    .nolhlin
        tst     TOSL                    ; width can't be zero
        breq    .nomhlin
; clip width to screen bounds
        mov     r21, TMPL               ; add width to x origin
        add     r21, TOSL
        brcs    .cliplhlin              ; check for wraparound
        cp      r18, r21                ; check if endpoint > screen width
        brsh    1f                      ; if in bounds, draw line
.cliplhlin:                             ; otherwise, clip
        mov     TOSL, r18
        sub     TOSL, TMPL              ; set line width to (screen width - x origin)
        rjmp    1f
.nolhlin:
        popd
        rnext
; LHLIN ( w xy -- )
; preconditions: w > 0, 0 <= x <= 255
opcode_lhlin:
        nip                             ; get xy into TMP
        lds     r20, bytesperline       ; multiply y coord * bytes per line
1:      lds     ZL, screen_ptr          ; get screen offset
        lds     ZH, screen_ptr+1
        movw    r18, ZERO               ; mul will clobber the zero regs
        mul     r20, TMPH
        add     ZL, r0                  ; add row offset
        adc     ZH, r1
        movw    ZERO, r18               ; restore zero regs
        lds     r20, globalcolor        ; get color
        mov     TMPH, TMPL              ; add width-1 to x coord
        add     TMPH, TOSL
        dec     TMPH
; TMPL: x coordinate of first pixel
; TMPH: x coordinate of last pixel
        movw    r18, TMP
        andi    r18, 0b00000011
        andi    r19, 0b00000011
; r18: lower 2 bits of first pixel x coordinate
; r19: lower 2 bits of last pixel x coordinate
        lsr     TMPH                    ; divide last x by 4 to get last byte offset
        lsr     TMPH
        add     TMPH, ZL                ; lower 8 bits of the address of the last pixel's byte
        lsr     TMPL                    ; divide first x by 4
        lsr     TMPL
        add     ZL, TMPL                ; add byte index to pointer
        adc     ZH, ZERO
; ZL: lower 8 bits of first pixel's byte
; TMPH: lower 8 bits of last pixel's byte
; if ZL==TOSH, the line starts and ends within the same byte
        cp      ZL, TMPH
        breq    .lhlin_tail
; if first pixel's start bit is not 0, we need to draw a "head" fragment
        tst     r18
        breq    .lhlin_body
.lhlin_head:
        movw    TOS, Z          ; save Z because we have to do a table lookup
        ldi     ZH, hi8(lhlin_fragments)
        mov     ZL, r18         ; lower nibble: start bit
        ori     ZL, 0b00110000
        lpm     r21, Z
; draw head fragment into underlying byte
        movw    Z, TOS
        ld      TOSL, Z         ; get underlying byte
        mov     TOSH, r20       ; get pixel color
        and     TOSH, r21       ; apply mask
        com     r21
        and     TOSL, r21       ; mask off bits we don't care about
        or      TOSL, TOSH      ; or in the bits we want to set
        st      Z+, TOSL
        ldi     r18, 0
        cp      ZL, TMPH
        breq    .lhlin_tail
; body loop: write full bytes (4-pixel strides) until we reach the last byte
.lhlin_body:
        ldi     r18, 0
1:      st      Z+, r20
        cp      ZL, TMPH
        brne    1b
.lhlin_tail:
; look at the last 2 bits of the start and end x coordinates, and look up the
; appropriate line fragment from a table
        movw    TOS, Z          ; save Z because we have to do a table lookup
        ldi     ZH, hi8(lhlin_fragments)
        mov     ZL, r18         ; lower nibble: start bit
        swap    r19
        or      ZL, r19         ; upper nibble: end bit
        lpm     r21, Z          ; get mask from table
; draw tail fragment into underlying byte
        movw    Z, TOS
        ld      TOSL, Z         ; get underlying byte
        and     r20, r21        ; apply mask
        com     r21
        and     TOSL, r21       ; mask off bits we don't care about
        or      TOSL, r20       ; or in the bits we want to set
        st      Z, TOSL
3:      popd
        rnext

; assumes: stack is ( w -- ) and xy in TMPL/TMPH
opcode_bhlin_chk:
        movw    r18, ZERO               ; mul will clobber the zero regs
        lds     r20, bytesperline       ; compute x width
        ldi     r21, 8
        mul     r20, r21
        cp      TMPL, r0                ; check x coord, zero-extended to 16 bits
        cpc     r18, r1                 ; (r18 will be zero)
        brsh    .nobhlin
        cpi     TOSL, 0                 ; width can't be zero
        cpc     TOSH, r18               ; (r18 will be zero)
        breq    .nobhlin
; clip width to screen bounds
        mov     r21, TMPL               ; add width to x origin
        add     r21, TOSL
        brcs    .clipbhlin              ; check for wraparound
        cp      r0, r21                 ; check if endpoint > screen width
        cpc     r1, r18                 ; (r18 will be zero)
        brsh    1f                      ; if in bounds, draw line
.clipbhlin:                             ; otherwise, clip
        mov     TOSL, r0
        sub     TOSL, TMPL              ; set line width to (screen width - x origin)
        rjmp    1f
.nobhlin:
        movw    ZERO, r18               ; restore zero regs
        popd
        rnext
; BHLIN ( w xy -- )
; preconditions: w > 0, 0 <= x <= 255
; if w=0, line will be 256 pixels wide
opcode_bhlin:
        nip                             ; get xy into TMP
        lds     r20, bytesperline       ; multiply y coord * bytes per line
        movw    r18, ZERO               ; mul will clobber the zero regs
1:      lds     ZL, screen_ptr          ; get screen offset
        lds     ZH, screen_ptr+1
        mul     r20, TMPH
        add     ZL, r0                  ; add row offset
        adc     ZH, r1
        movw    ZERO, r18               ; restore zero regs
        lds     r20, globalcolor        ; get color
        mov     TMPH, TMPL              ; add width-1 to x coord
        add     TMPH, TOSL
        dec     TMPH
; TMPL: x coordinate of first pixel
; TMPH: x coordinate of last pixel
        movw    r18, TMP
        andi    r18, 0b00000111
        andi    r19, 0b00000111
; r18: lower 3 bits of first pixel x coordinate
; r19: lower 3 bits of last pixel x coordinate
        lsr     TMPH                    ; divide last x by 8 to get last byte offset
        lsr     TMPH
        lsr     TMPH
        add     TMPH, ZL                ; lower 8 bits of the address of the last pixel's byte
        lsr     TMPL                    ; divide first x by 8
        lsr     TMPL
        lsr     TMPL
        add     ZL, TMPL                ; add byte index to pointer
        adc     ZH, ZERO
;!!! TODO: use separate branches for black and white?
; ZL: lower 8 bits of first pixel's byte
; TMPH: lower 8 bits of last pixel's byte
; if ZL==TMPH, the line starts and ends within the same byte
        cp      ZL, TMPH
        breq    .bhlin_tail
; if first pixel's start bit is not 0, we need to draw a "head" fragment
        tst     r18
        breq    .bhlin_body
.bhlin_head:
        movw    TOS, Z          ; save Z because we have to do a table lookup
        ldi     ZH, hi8(bhlin_fragments)
        mov     ZL, r18         ; lower nibble: start bit
        ori     ZL, 0b01110000
        lpm     r21, Z
; draw head fragment into underlying byte
        movw    Z, TOS
        ld      TOSL, Z         ; get underlying byte
        mov     TOSH, r20       ; get pixel color
        and     TOSH, r21       ; apply mask
        com     r21
        and     TOSL, r21       ; mask off bits we don't care about
        or      TOSL, TOSH      ; or in the bits we want to set
        st      Z+, TOSL
        ldi     r18, 0
        cp      ZL, TMPH
        breq    .bhlin_tail
; body loop: write full bytes (8-pixel strides) until we reach the last byte
.bhlin_body:
        ldi     r18, 0
1:      st      Z+, r20
        cp      ZL, TMPH
        brne    1b
.bhlin_tail:
; look at the last 3 bits of the start and end x coordinates, and look up the
; appropriate line fragment from a table
        movw    TOS, Z          ; save Z because we have to do a table lookup
        ldi     ZH, hi8(bhlin_fragments)
        mov     ZL, r18         ; lower nibble: start bit
        swap    r19
        or      ZL, r19         ; upper nibble: end bit
        lpm     r21, Z          ; get mask from table
; draw tail fragment into underlying byte
        movw    Z, TOS
        ld      TOSL, Z         ; get underlying byte
        and     r20, r21        ; apply mask
        com     r21
        and     TOSL, r21       ; mask off bits we don't care about
        or      TOSL, r20       ; or in the bits we want to set
        st      Z, TOSL
        popd
        rnext

; assumes: stack is ( wh -- ) and xy in TMPL/TMPH
opcode_hrect_chk:
        lds     r21, bytesperline       ; check x coord
        cp      TMPL, r21
        brsh    .nohrect
        tst     TOSL                    ; width can't be zero
        breq    .nohrect
; clip width to screen bounds
        mov     r20, TMPL               ; add width to x origin
        add     r20, TOSL
        brcs    .cliphrecth             ; check for wraparound
        cp      r21, r20                ; check if endpoint > screen width
        brsh    1f                      ; if in bounds, draw rect
.cliphrecth:                            ; otherwise, clip
        mov     TOSL, r21
        sub     TOSL, TMPL              ; set line width to (screen width - x origin)
        rjmp    1f
.nohrect:
.nomrect:
        popd
        rnext
; HRECT ( xy wh -- )
opcode_hrect:
        nip                             ; get xy into TMP
        lds     r21, bytesperline       ; multiply y coord * bytes per line
1:      movw    r18, ZERO               ; mul will clobber the zero regs
        mul     r21, TMPH
        add     r0, TMPL                ; add x coord
        adc     r1, r18                 ; will be zero
        lds     ZL, screen_ptr          ; add screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r18               ; restore zero regs
        lds     r20, globalcolor
        sub     r21, TOSL               ; subtract rect width from screen width, giving byteskip btwn lines
; loop
; TOSL - width
; TOSH - height (y counter)
; r21  - number of bytes to skip to get to next row
; r20  - value to fill with
; r19  - x counter
.yloop: mov     r19, TOSL               ; reset x counter
.xloop: st      Z+, r20
        dec     r19
        brne    .xloop
; end of x loop
        add     ZL, r21
        adc     ZH, ZEROL
        dec     TOSH
        brne    .yloop
3:      popd
        rnext

; assumes: stack is ( wh -- ) and xy in TMPL/TMPH
opcode_mrect_chk:
        lds     r21, bytesperline
        mov     r18, r21                ; multiply bytes per line by 2
        lsl     r18
        cp      TMPL, r18               ; check x coord
        brsh    .nomrect
        tst     TOSL                    ; width can't be zero
        breq    3b
; clip width to screen bounds
        mov     r20, TMPL               ; add width to x origin
        add     r20, TOSL
        brcs    .clipmrecth             ; check for wraparound
        cp      r18, r20                ; check if endpoint > screen width
        brsh    1f                      ; if in bounds, draw line
.clipmrecth:                            ; otherwise, clip
        mov     TOSL, r18
        sub     TOSL, TMPL              ; set rect width to (screen width - x origin)
        rjmp    1f
; MRECT ( xy wh -- )
; Drawn one row at a time, in (up to) three sections: head, body, tail
; Basically adds a loop around "MHLIN"
opcode_mrect:
        nip                             ; get xy into TMP
        lds     r21, bytesperline
1:      lds     ZL, screen_ptr          ; get screen offset
        lds     ZH, screen_ptr+1
        movw    r18, ZERO               ; mul will clobber the zero regs
        mul     r21, TMPH               ; multiply y coord * bytes per line
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r18               ; restore zero regs
        movw    TSAV, TOS               ; save copies of width/height
        lds     r20, globalcolor        ; get fill color
        bst     TMPL, 0                 ; set T flag if not byte-aligned
        lsr     TMPL                    ; divide width by 2, giving byte offset
; TSAVL  - width in pixels
; TSAVH  - height in pixels
; r18    - free
; r19    - free
; r20    - color
; r21    - bytes per line
; TOSL   - width in pixels (horizontal loop counter)
; TOSH   - height in pixels (vertical loop counter)
; XSAVL  - saves ZL
; XSAVH  - saves ZH
        add     ZL, TMPL                ; add x offset to pointer
        adc     ZH, ZERO                ; TMPL/TMPH are now free
        rjmp    .mrect_body_firstlinestart
.mrect_body_linestart:
        mov     TOSL, TSAVL             ; initialize x counter
        movw    Z, XSAV                 ; rewind to start of line
        add     ZL, r21                 ; advance to next line
        adc     ZH, ZERO
.mrect_body_firstlinestart:
        movw    XSAV, Z                 ; save address of line start
        brtc    .mrect_no_head          ; not byte-aligned?
        ld      r19, Z
        andi    r19, 0b11110000
        mov     r18, r20
        andi    r18, 0b00001111         ; apply masks
        or      r19, r18
        st      Z+, r19
        dec     TOSL                    ; decrement width
        breq    1f                      ; we could already be done (if width=1)
.mrect_no_head:
.mrect_body_innerloop:
        subi    TOSL, 2
        brcs    .mrect_tail             ; end of line (with a pixel left over)?
        st      Z+, r20
        brne    .mrect_body_innerloop
1:      dec     TOSH                    ; decrement vertical count
        brne    .mrect_body_linestart
.mrect_done:
        popd
        rnext
.mrect_tail:
        ld      r19, Z
        andi    r19, 0b00001111
        mov     r18, r20
        andi    r18, 0b11110000
        or      r19, r18
        st      Z+, r19
        dec     TOSH
        brne    .mrect_body_linestart
.nolrect:
3:      popd
        rnext

; assumes: stack is ( wh -- ) and xy in TMPL/TMPH
opcode_lrect_chk:
        lds     r21, bytesperline
        mov     r18, r21                ; multiply bytes per line by 4
        lsl     r18
        lsl     r18
        cp      TMPL, r18               ; check x coord
        brsh    .nolrect
        tst     TOSL                    ; width can't be zero
        breq    3b
; clip width to screen bounds
        mov     r20, TMPL               ; add width to x origin
        add     r20, TOSL
        brcs    .cliplrecth             ; check for wraparound
        cp      r18, r20                ; check if endpoint > screen width
        brsh    1f                      ; if in bounds, draw line
.cliplrecth:                            ; otherwise, clip
        mov     TOSL, r18
        sub     TOSL, TMPL              ; set line width to (screen width - x origin)
        rjmp    1f
; LRECT ( xy wh -- )
; preconditions: w > 0, h > 0, 0 <= x <= 255
; Drawn in (at most) three vertical strips: head (if left edge occurs in the
; middle of a byte), body (written as full bytes--4 pixels--at a time), and tail
; (if right edge occurs in the middle of a byte)
opcode_lrect:
        nip                             ; get xy into TMP
        lds     r21, bytesperline       ; multiply y coord * bytes per line
1:      lds     ZL, screen_ptr          ; get screen offset
        lds     ZH, screen_ptr+1
        movw    r18, ZERO               ; mul will clobber the zero regs
        mul     r21, TMPH
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r18               ; restore zero regs
        lds     r20, globalcolor        ; get color
        movw    TSAV, TOS               ; save copies of width/height
        mov     TMPH, TMPL              ; add width-1 to x coord
        add     TMPH, TOSL
        dec     TMPH
; TMPL: x coordinate of first pixel
; TMPH: x coordinate of last pixel
        movw    r18, TMP
        andi    r18, 0b00000011
        andi    r19, 0b00000011
; r18: lower 2 bits of first pixel x coordinate
; r19: lower 2 bits of last pixel x coordinate
        lsr     TMPH                    ; divide last x by 4 to get last byte offset
        lsr     TMPH
        lsr     TMPL                    ; divide first x by 4
        lsr     TMPL
        add     ZL, TMPL                ; add byte index to pointer
        adc     ZH, ZERO
        sub     TMPH, TMPL              ; number of full bytes in each line
        breq    .lrect_tail             ; if 0, rect's x extent starts and ends within the same byte
; if first pixel's start bit is not 0, we need to draw a "head" strip
        tst     r18
        breq    .lrect_body
.lrect_head:
        movw    XSAV, Z         ; save origin (the table lookup will clobber Z)
        ldi     ZH, hi8(lhlin_fragments)
        mov     ZL, r18         ; lower nibble: start bit
        ori     ZL, 0b00110000
        lpm     TSAVL, Z
        movw    Z, XSAV         ; restore Z since it's clobbered by lpm
        mov     TOSH, r20       ; get pixel color
        and     TOSH, TSAVL     ; apply mask
        com     TSAVL
        mov     r18, TSAVH      ; reuse r18 as a loop count, we need to zero it anyway
1:      ld      TOSL, Z         ; get underlying byte
        and     TOSL, TSAVL     ; mask off bits we don't care about
        or      TOSL, TOSH      ; or in the bits we want to set
        st      Z, TOSL
        add     ZL, r21         ; advance one line
        adc     ZH, ZERO
        dec     r18
        brne    1b
        movw    Z, XSAV         ; restore Z to origin
        adiw    Z, 1            ; advance to next byte
        dec     TMPH            ; one less byte in body
        breq    .lrect_tail
.lrect_body:
        movw    TOS, Z          ; save origin
        mov     r18, TSAVH      ; reuse r18 as a loop count, we need to zero it anyway
        mov     TSAVL, r21      ; compute number of bytes to skip to get to next line
        sub     TSAVL, TMPH
1:      mov     TMPL, TMPH      ; initalize horizontal loop count
2:      st      Z+, r20
        dec     TMPL
        brne    2b
        add     ZL, TSAVL       ; advance to next line
        adc     ZH, ZERO
        dec     r18
        brne    1b
        movw    Z, TOS          ; restore origin
        add     ZL, TMPH        ; advance to byte after the first line
        adc     ZH, ZERO
.lrect_tail:
; look at the last 2 bits of the start and end x coordinates, and look up the
; appropriate line fragment from a table
        movw    TOS, Z          ; save Z because we have to do a table lookup
        ldi     ZH, hi8(lhlin_fragments)
        mov     ZL, r18         ; lower nibble: start bit
        swap    r19
        or      ZL, r19         ; upper nibble: end bit
        lpm     r19, Z          ; get mask from table
        movw    Z, TOS
        and     r20, r19        ; apply mask to color
        com     r19             ; invert mask
        mov     r18, TSAVH      ; initialize loop counter to height
; draw tail fragment into underlying byte
1:      ld      TOSL, Z         ; get underlying byte
        and     TOSL, r19       ; mask off bits we don't care about
        or      TOSL, r20       ; or in the bits we want to set
        st      Z, TOSL
        add     ZL, r21         ; advance one line
        adc     ZH, ZERO
        dec     r18
        brne    1b
        popd
        rnext

; assumes: stack is ( wh -- ) and xy in TMPL/TMPH
opcode_brect_chk:
        movw    r18, ZERO               ; mul will clobber the zero regs
        lds     r21, bytesperline       ; compute x width
        ldi     r20, 8
        mul     r21, r20
        cp      TMPL, r0                ; check x coord, zero-extended to 16 bits
        cpc     r18, r1                 ; (r18 will be zero)
        brsh    .nobrect
        tst     TOSL                    ; width can't be zero
        breq    .nobrect
; clip width to screen bounds
        mov     r20, TMPL               ; add width to x origin
        add     r20, TOSL
        brcs    .clipbrecth             ; check for wraparound
        cp      r0, r20                 ; check if endpoint > screen width
        cpc     r1, r18                 ; (r18 will be zero)
        brsh    1f                      ; if in bounds, draw line
.clipbrecth:                            ; otherwise, clip
        mov     TOSL, r0
        sub     TOSL, TMPL              ; set line width to (screen width - x origin)
        rjmp    1f
.nobrect:
        movw    ZERO, r18               ; restore zero regs
        popd
        rnext
opcode_brect:
        nip                             ; get width into TMPL and height into TMPH
        lds     r21, bytesperline       ; multiply y coord * bytes per line
        movw    r18, ZERO               ; mul will clobber the zero regs
1:      lds     ZL, screen_ptr          ; get screen offset
        lds     ZH, screen_ptr+1
        mul     r21, TMPH
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r18               ; restore zero regs
        lds     r20, globalcolor        ; get color
        movw    TSAV, TOS               ; save copies of width/height
        mov     TMPH, TMPL              ; add width-1 to x coord
        add     TMPH, TOSL
        dec     TMPH
; TMPL: x coordinate of first pixel
; TMPH: x coordinate of last pixel
        movw    r18, TMP
        andi    r18, 0b00000111
        andi    r19, 0b00000111
; r18: lower 3 bits of first pixel x coordinate
; r19: lower 3 bits of last pixel x coordinate
        lsr     TMPH                    ; divide last x by 8 to get last byte offset
        lsr     TMPH
        lsr     TMPH
        lsr     TMPL                    ; divide first x by 8
        lsr     TMPL
        lsr     TMPL
        add     ZL, TMPL                ; add byte index to pointer
        adc     ZH, ZERO
        sub     TMPH, TMPL              ; number of full bytes in each line
        breq    .brect_tail             ; if 0, rect's x extent starts and ends within the same byte
; if first pixel's start bit is not 0, we need to draw a "head" strip
        tst     r18
        breq    .brect_body
;!!! TODO: use separate branches for black and white heads/tails?
.brect_head:
        movw    XSAV, Z         ; save origin (the table lookup will clobber Z)
        ldi     ZH, hi8(bhlin_fragments)
        mov     ZL, r18         ; lower nibble: start bit
        ori     ZL, 0b01110000
        lpm     TSAVL, Z
        movw    Z, XSAV         ; restore Z since it's clobbered by lpm
        mov     TOSH, r20       ; get pixel color
        and     TOSH, TSAVL     ; apply mask
        com     TSAVL
        mov     r18, TSAVH      ; reuse r18 as a loop count, we need to zero it anyway
1:      ld      TOSL, Z         ; get underlying byte
        and     TOSL, TSAVL     ; mask off bits we don't care about
        or      TOSL, TOSH      ; or in the bits we want to set
        st      Z, TOSL
        add     ZL, r21         ; advance one line
        adc     ZH, ZERO
        dec     r18
        brne    1b
        movw    Z, XSAV         ; restore Z to origin
        adiw    Z, 1            ; advance to next byte
        dec     TMPH            ; one less byte in body
        breq    .brect_tail
.brect_body:
        movw    TOS, Z          ; save origin
        mov     r18, TSAVH      ; reuse r18 as a loop count, we need to zero it anyway
        mov     TSAVL, r21      ; compute number of bytes to skip to get to next line
        sub     TSAVL, TMPH
1:      mov     TMPL, TMPH      ; initalize horizontal loop count
2:      st      Z+, r20
        dec     TMPL
        brne    2b
        add     ZL, TSAVL       ; advance to next line
        adc     ZH, ZERO
        dec     r18
        brne    1b
        movw    Z, TOS          ; restore origin
        add     ZL, TMPH        ; advance to byte after the first line
        adc     ZH, ZERO
.brect_tail:
; look at the last 2 bits of the start and end x coordinates, and look up the
; appropriate line fragment from a table
        movw    TOS, Z          ; save Z because we have to do a table lookup
        ldi     ZH, hi8(bhlin_fragments)
        mov     ZL, r18         ; lower nibble: start bit
        swap    r19
        or      ZL, r19         ; upper nibble: end bit
        lpm     r19, Z          ; get mask from table
        movw    Z, TOS
        and     r20, r19        ; apply mask to color
        com     r19             ; invert mask
        mov     r18, TSAVH      ; initialize loop counter to height
; draw tail fragment into underlying byte
1:      ld      TOSL, Z         ; get underlying byte
        and     TOSL, r19       ; mask off bits we don't care about
        or      TOSL, r20       ; or in the bits we want to set
        st      Z, TOSL
        add     ZL, r21         ; advance one line
        adc     ZH, ZERO
        dec     r18
        brne    1b
        popd
        rnext

; HLINE ( xy1 xy2 -- ) draw a line from point xy1 to point xy2 using Bresenham's
; line algorithm (high-color bitmap modes)
; Lines are always drawn left-to-right.
opcode_hline_chk:
opcode_hline:
        nip                     ; get xy1 into TMP
; TMPL=x1, TMPH=y1, TOSL=x2, TOSH=y2. if x2 < x1, swap xy1 and xy2.
        cp      TOSL, TMPL
        brsh    1f
        movw    r20, TMP
        movw    TMP, TOS
        movw    TOS, r20
; at this point, x1 <= x2. TMP is the starting coordinate
1:      movw    r18, ZERO               ; mul will clobber the zero regs
        clr     r21                     ; r21:r20 - signed number of bytes to advance each line
        lds     r20, bytesperline       ; multiply y coord * bytes per line
        mul     TMPH, r20               ; get byte offset for y coord
        add     r0, TMPL                ; add x coord
        adc     r1, r18                 ; will be zero
        lds     ZL, screen_ptr          ; add to screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r18               ; restore zero regs
        lds     r19, globalcolor
; Z is now the location of the first byte
; TOSL-TMPL is the line width (always nonnegative)
        sub     TOSL, TMPL
; compute absolute difference between y coordinates to determine line slope
        sub     TOSH, TMPH
        brcc    1f
        com     r21                     ;if negative dy, negate byte offset between rows
        neg     r20
        sbci    r21, -1
        neg     TOSH
1:
; TOSL is dx = abs(x2-x1). TOSH is dy = abs(y2-y1)
        cp      TOSH, TOSL
        brlo    .hshallow
; "steep" lines (dy > dx)
.hsteep:
; TMP holds 2*dx. (signed 16-bit)
        mov     TMPL, TOSL
        clr     TMPH
        lsl     TMPL
        rol     TMPH
; r18 is the loop counter (initialized to dy)
        mov     r18, TOSH
; XSAV holds 2*dy
        mov     XSAVL, TOSH
        clr     XSAVH
        lsl     XSAVL
        rol     XSAVH
; TOS is the error accumulator, initialized to 2*dx - dy
        movw    TOS, TMP
        sub     TOSL, r18
        sbc     TOSH, ZEROL
; Now draw the pixels. One iteration per row.
1:      st      Z, r19
; is D (error accumulator) > 0?
        tstw    TOS             ; will always clear carry
        breq    2f
        brmi    2f
; if so, subtract 2*dy from error accumulator and advance to next column
        sub     TOSL, XSAVL
        sbc     TOSH, XSAVH
        sec
; advance to next row
2:      adc     ZL, r20         ; explicitly add the carry flag, it indicates if we need to move right one column
        adc     ZH, r21
; add 2*dx to error accumulator
2:      add     TOSL, TMPL
        adc     TOSH, TMPH
; decrement loop count
        subi    r18, 1          ; don't use dec--it doesn't set the carry flag!
        brcc    1b
3:      popd
        rnext
; "shallow" lines (dy <= dx)
.hshallow:
; TMP holds 2*dy. (signed 16-bit)
        mov     TMPL, TOSH
        clr     TMPH
        lsl     TMPL
        rol     TMPH
; r18 is the loop counter (initialized to dx)
        mov     r18, TOSL
; XSAV holds 2*dx
        mov     XSAVL, TOSL
        clr     XSAVH
        lsl     XSAVL
        rol     XSAVH
; TOS is the error accumulator, initialized to 2*dy - dx
        movw    TOS, TMP
        sub     TOSL, r18
        sbc     TOSH, ZEROL
; Now draw the pixels. One iteration per column.
1:      st      Z+, r19         ; write pixel and advance right
; is D (error accumulator) > 0?
        tstw    TOS
        breq    2f
        brmi    2f
; if so, subtract 2*dx from error accumulator and advance to next row
        sub     TOSL, XSAVL
        sbc     TOSH, XSAVH
        add     ZL, r20
        adc     ZH, r21
; add 2*dy to error accumulator
2:      add     TOSL, TMPL
        adc     TOSH, TMPH
; decrement loop count
        subi    r18, 1          ; don't use dec--it doesn't set the carry flag!
        brcc    1b
3:      popd
        rnext


opcode_mline_chk:
opcode_mline:
        nip                     ; get xy1 into TMP
; TMPL=x1, TMPH=y1, TOSL=x2, TOSH=y2. if x2 < x1, swap xy1 and xy2.
        cp      TOSL, TMPL
        brsh    1f
        movw    r20, TMP
        movw    TMP, TOS
        movw    TOS, r20
; at this point, x1 <= x2. TMP is the starting coordinate
1:      movw    r18, ZERO               ; mul will clobber the zero regs
        clr     r21                     ; r21:r20 - signed number of bytes to advance each line
        lds     r20, bytesperline       ; multiply y coord * bytes per line
        mul     TMPH, r20               ; get byte offset for y coord
        bst     TMPL, 0                 ; starting on an odd or even pixel?
        mov     ZL, TMPL                ; divide x coord by 2 to get x byte offset
        lsr     ZL
        add     r0, ZL                  ; add x byte offset
        adc     r1, r18                 ; r18 will be zero
        lds     ZL, screen_ptr          ; add to screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, r18               ; restore zero regs
; Z is now the location of the first byte
; TOSL-TMPL is the line width (always nonnegative)
        sub     TOSL, TMPL
; compute absolute difference between y coordinates to determine line slope
        sub     TOSH, TMPH
        brcc    1f
; r21:r20 - signed number of bytes to advance each line
        com     r21                     ;if negative dy, negate byte offset between rows
        neg     r20
        sbci    r21, -1
        neg     TOSH
1:
; TOSL is dx = abs(x2-x1). TOSH is dy = abs(y2-y1)
        cp      TOSH, TOSL
        brlo    .mshallow
; "steep" lines (dy > dx)
.msteep:
; r18 holds the mask to apply to the existing byte
; r19 holds the pattern to be or-ed into the existing byte
        lds     r19, globalcolor
        brtc    2f                      ; starting on an odd or even column?
        ldi     r18, 0b11110000
        andi    r19, 0b00001111
        rjmp    3f
2:      ldi     r18, 0b00001111
        andi    r19, 0b11110000
; TMP holds 2*dx. (signed 16-bit)
3:      mov     TMPL, TOSL
        clr     TMPH
        lsl     TMPL
        rol     TMPH
; r16 is the loop counter (initialized to dy)
        mov     r16, TOSH
; XSAV holds 2*dy
        mov     XSAVL, TOSH
        clr     XSAVH
        lsl     XSAVL
        rol     XSAVH
; TOS is the error accumulator, initialized to 2*dx - dy
        movw    TOS, TMP
        sub     TOSL, r16
        sbc     TOSH, ZEROL
; This is the simpler case. Since the line is one pixel wide, and only one
; byte per row is modified, we will only need to modify one nibble per row.
; Thus, we use the same read-modify-write process for each byte.
4:      ld      TSAVL, Z        ; read existing pixel, use TSAVL as a temporary
        and     TSAVL, r18      ; apply mask
        or      TSAVL, r19      ; or in the new pixels
        st      Z, TSAVL
; is D (error accumulator) > 0?
        tstw    TOS             ; will always clear carry
        breq    5f
        brmi    5f
; if so, subtract 2*dy from error accumulator and advance to next column
        sub     TOSL, XSAVL
        sbc     TOSH, XSAVH
        swap    r18             ; swap nibbles of pixel masks
        swap    r19
        cpi     r18, 0b11110000 ; set carry flag if we need to advance one byte to the right
; advance to next row
5:      adc     ZL, r20         ; explicitly add the carry flag, it indicates if we need to move right one column
        adc     ZH, r21
; add 2*dx to error accumulator
        add     TOSL, TMPL
        adc     TOSH, TMPH
; decrement loop count
        subi    r16, 1          ; don't use dec--it doesn't set the carry flag!
        brcc    4b
        popd
        rnext
; "shallow" lines (dy <= dx)
.mshallow:
; TSAV holds 2*dy. (signed 16-bit)
        mov     TSAVL, TOSH
        clr     TSAVH
        lsl     TSAVL
        rol     TSAVH
; r16 is the loop counter (initialized to dx)
        mov     r16, TOSL
; XSAV holds 2*dx
        mov     XSAVL, TOSL
        clr     XSAVH
        lsl     XSAVL
        rol     XSAVH
; TOS is the error accumulator, initialized to 2*dy - dx
        movw    TOS, TSAV
        sub     TOSL, r16
        sbc     TOSH, ZEROL
; This is more complicated. For a given byte, we may need to set either the left
; pixel, the right pixel, or both. In the case where both pixels need to be set
; (i.e. we are drawing a run of 2 or more pixels) we'd like to avoid doing two
; read-modify-writes, because all we really need to do is a single write, setting
; both nibbles to the new color value.
        ser     r19                     ; initialize mask indicating which nibbles of the current byte will be written
        lds     LINKL, globalcolor
        mov     TMPL, LINKL
        andi    TMPL, 0b11110000        ; cache current color in left/right nibbles
        mov     TMPH, LINKL
        andi    TMPH, 0b00001111
        brts    3f              ; starting on an even or odd column?
; Left pixel.
1:      ldi     r19, 0b00001111 ; initialize mask
; is D (error accumulator) > 0?
        tstw    TOS
        breq    2f
        brmi    2f
; if so, update left pixel of current byte only, reset mask
        ld      r18, Z
        and     r18, r19        ; apply mask to existing byte
        or      r18, TMPL       ; or in the current color
        st      Z, r18          ; update byte
        ser     r19
; subtract 2*dx from error accumulator and advance to next row
        sub     TOSL, XSAVL
        sbc     TOSH, XSAVH
        add     ZL, r20
        adc     ZH, r21
; add 2*dy to error accumulator
2:      add     TOSL, TSAVL
        adc     TOSH, TSAVH
; decrement loop count
        subi    r16, 1          ; don't use dec--it doesn't set the carry flag!
        brcs    5f
; Right pixel.
3:      andi    r19, 0b11110000 ; mask
        mov     r18, LINKL      ; byte to be written (initially filled with current color)
        breq    9f              ; avoid read-modify-write if we are updating the entire byte
        ld      r18, Z
        and     r18, r19
        or      r18, TMPH
9:      st      Z+, r18
; is D (error accumulator) > 0?
        tstw    TOS
        breq    4f
        brmi    4f
; if so, update right pixel,
; subtract 2*dx from error accumulator and advance to next row
        sub     TOSL, XSAVL
        sbc     TOSH, XSAVH
        add     ZL, r20
        adc     ZH, r21
; add 2*dy to error accumulator
4:      add     TOSL, TSAVL
        adc     TOSH, TSAVH
; decrement loop count
        subi    r16, 1          ; don't use dec--it doesn't set the carry flag!
        brcc    1b
        popd
        rnext
; finish up last byte (if line ends on an even column and that byte hasn't been written to)
5:      cpi     r19, 0xFF       ; end right away if mask indicates neither pixel should be touched
        breq    6f
        ld      r18, Z
        andi    r18, 0b00001111
        or      r18, TMPL
        st      Z, r18
6:      popd
        rnext

opcode_lline_chk:
; LLINE ( xy1 xy2 -- ) draw a line from point xy1 to point xy2 (low-color bitmap
; modes)
opcode_lline:
        nip                     ; get xy1 into TMP
; TMPL=x1, TMPH=y1, TOSL=x2, TOSH=y2. if x2 < x1, swap xy1 and xy2.
        cp      TOSL, TMPL
        brsh    1f
        movw    r20, TMP
        movw    TMP, TOS
        movw    TOS, r20
; at this point, x1 <= x2. TMP is the starting coordinate
1:      mov     ZL, TMPL                ; get bit mask for x position
        ldi     ZH, hi8(crumbmasktable)
        lpm     TSAVH, Z                ; select the correct bit mask for this x pos (before we need to use Z)
        movw    XSAV, ZERO              ; mul will clobber the zero regs
        clr     r21                     ; r21:r20 - signed number of bytes to advance each line
        lds     r20, bytesperline       ; multiply y coord * bytes per line
        mul     TMPH, r20               ; get byte offset for y coord
        mov     ZL, TMPL                ; divide x coord by 4 to get x byte offset
        lsr     ZL
        lsr     ZL
        add     r0, ZL                  ; add x byte offset
        adc     r1, XSAVL               ; XSAVL will be zero
        lds     ZL, screen_ptr          ; add to screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, XSAV              ; restore zero regs
; Z is now the location of the first byte
; TOSL-TMPL is the line width (always nonnegative)
        sub     TOSL, TMPL
; compute absolute difference between y coordinates to determine line slope
        sub     TOSH, TMPH
        brcc    1f
; r21:r20 - signed number of bytes to advance each line
        com     r21                     ;if negative dy, negate byte offset between rows
        neg     r20
        sbci    r21, -1
        neg     TOSH
1:
; TOSL is dx = abs(x2-x1). TOSH is dy = abs(y2-y1)
        cp      TOSH, TOSL
        brlo    .lshallow
; "steep" lines (dy > dx)
.lsteep:
; initialize bitmasks
        lds     r19, globalcolor        ; initialize color
        mov     r18, r19
        andi    r18, 0b11000000         ; r18 holds a copy of the color for every 4th column
        and     r19, TSAVH              ; r19 is the color or'd in
        com     TSAVH                   ; TSAVH is the mask applied to existing bytes
; TMP holds 2*dx. (signed 16-bit)
        mov     TMPL, TOSL
        clr     TMPH
        lsl     TMPL
        rol     TMPH
; r16 is the loop counter (initialized to dy)
        mov     r16, TOSH
; XSAV holds 2*dy
        mov     XSAVL, TOSH
        clr     XSAVH
        lsl     XSAVL
        rol     XSAVH
; TOS is the error accumulator, initialized to 2*dx - dy
        movw    TOS, TMP
        sub     TOSL, r16
        sbc     TOSH, ZEROL
; This is the simpler case. Since the line is one pixel wide, and only one
; byte per row is modified, we will only need to modify one crumb per row.
; Thus, we use the same read-modify-write process for each byte.
1:      ld      TSAVL, Z        ; read existing pixel, use TSAVL as a temporary
        and     TSAVL, TSAVH    ; apply mask
        or      TSAVL, r19      ; or in the new pixels
        st      Z, TSAVL
; is D (error accumulator) > 0?
        tstw    TOS             ; will always clear carry
        breq    2f
        brmi    2f
; if so, subtract 2*dy from error accumulator and advance to next column
        sub     TOSL, XSAVL
        sbc     TOSH, XSAVH
        ; shift color to the right
        lsr     r19
        lsr     r19
        ; rotate mask to the right
        bst     TSAVH, 0
        lsr     TSAVH
        bld     TSAVH, 7
        asr     TSAVH           ; duplicate msb
        brcs    2f
        ; if carry was cleared, mask is 0b00111111. color needs to be reset and Z needs to advance 1 byte
        mov     r19, r18
        adiw    Z, 1
; advance to next row
2:      add     ZL, r20
        adc     ZH, r21
; add 2*dx to error accumulator
        add     TOSL, TMPL
        adc     TOSH, TMPH
; decrement loop count
        subi    r16, 1          ; don't use dec--it doesn't set the carry flag!
        brcc    1b
        popd
        rnext
; "shallow" lines (dy <= dx)
.lshallow:
; TMP holds 2*dy. (signed 16-bit)
        mov     TMPL, TOSH
        clr     TMPH
        lsl     TMPL
        rol     TMPH
; r16 is the loop counter (initialized to dx)
        mov     r16, TOSL
; XSAV holds 2*dx
        mov     XSAVL, TOSL
        clr     XSAVH
        lsl     XSAVL
        rol     XSAVH
; TOS is the error accumulator, initialized to 2*dy - dx
        movw    TOS, TMP
        sub     TOSL, r16
        sbc     TOSH, ZEROL
; initialize masks
        lds     r18, globalcolor
        com     TSAVH           ; mask to apply for this pixel (2 bits always zero)
        ser     r19             ; mask to apply for this byte
; This is more complicated. For a given byte, we may need to set between one and
; four crumbs. We'd like to avoid redundant read-modify-writes as much as possible,
; ideally using a single read-modify-write in cases where one, two, or three
; crumbs need to be set, and ideally use a single write in the case where all
; four crumbs need to be set to the new color value.
1:      bst     TSAVH, 0        ; save lsb of pixel mask
        mov     TSAVL, r18      ; get color
        and     r19, TSAVH      ; update byte mask
        breq    2f              ; if byte mask is all zeros, don't need read-modify-write
        com     TSAVH           ; invert mask
        and     TSAVL, TSAVH    ; isolate pixels we want to write
        com     TSAVH           ; put mask back
        ld      LINKL, Z        ; read existing pixel
        and     LINKL, TSAVH    ; mask out pixels we don't want to write
        or      TSAVL, LINKL    ; combine
2:      st      Z, TSAVL        ; store
        brts    3f              ; add 1 to Z and reset byte mask if we crossed a byte boundary
        ser     r19
        adiw    Z, 1
3:      lsr     TSAVH           ; rotate pixel mask
        bld     TSAVH, 7
        asr     TSAVH           ; duplicate msb. if T is clear, we crossed a byte boundary
; is D (error accumulator) > 0?
        tstw    TOS
        breq    4f
        brmi    4f
; if so, subtract 2*dx from error accumulator and advance to next row
        sub     TOSL, XSAVL
        sbc     TOSH, XSAVH
        add     ZL, r20
        adc     ZH, r21
        ser     r19             ; reset byte mask if we advance down a row
; add 2*dy to error accumulator
4:      add     TOSL, TMPL
        adc     TOSH, TMPH
; decrement loop count
        subi    r16, 1          ; don't use dec--it doesn't set the carry flag!
        brcc    1b
        popd
        rnext


opcode_bline_chk:
; BLINE ( xy1 xy2 -- ) draw a line from point xy1 to point xy2 (monochrome bitmap
; modes <= 256 pixels wide)
opcode_bline:
        nip                             ; get xy1 into TMP
; TMPL=x1, TMPH=y1, TOSL=x2, TOSH=y2. if x2 < x1, swap xy1 and xy2.
        cp      TOSL, TMPL
        brsh    1f
        movw    r20, TMP
        movw    TMP, TOS
        movw    TOS, r20
; at this point, x1 <= x2. TMP is the starting coordinate
1:      mov     ZL, TMPL                ; get bit mask for x position
        ldi     ZH, hi8(bitmasktable)
        lpm     r19, Z                  ; select the correct bit mask for this x pos (before we need to use Z)
        movw    XSAV, ZERO              ; mul will clobber the zero regs
        clr     r21                     ; r21:r20 - signed number of bytes to advance each line
        lds     r20, bytesperline       ; multiply y coord * bytes per line
        mul     TMPH, r20               ; get byte offset for y coord
        mov     ZL, TMPL                ; divide x coord by 8 to get x byte offset
        lsr     ZL
        lsr     ZL
        lsr     ZL
        add     r0, ZL                  ; add x byte offset
        adc     r1, XSAVL               ; XSAVL will be zero
        lds     ZL, screen_ptr          ; add to screen offset
        lds     ZH, screen_ptr+1
        add     ZL, r0
        adc     ZH, r1
        movw    ZERO, XSAV              ; restore zero regs
; Z is now the location of the first byte
; TOSL-TMPL is the line width (always nonnegative)
        sub     TOSL, TMPL
; compute absolute difference between y coordinates to determine line slope
        sub     TOSH, TMPH
        brcc    1f
; r21:r20 - signed number of bytes to advance each line
        com     r21                     ;if negative dy, negate byte offset between rows
        neg     r20
        sbci    r21, -1
        neg     TOSH
1:
; TOSL is dx = abs(x2-x1). TOSH is dy = abs(y2-y1)
        cp      TOSH, TOSL
        brlo    .bshallow
; "steep" lines (dy > dx)
.bsteep:
; TMP holds 2*dx. (signed 16-bit)
        mov     TMPL, TOSL
        clr     TMPH
        lsl     TMPL
        rol     TMPH
; r16 is the loop counter (initialized to dy)
        mov     r16, TOSH
; XSAV holds 2*dy
        mov     XSAVL, TOSH
        clr     XSAVH
        lsl     XSAVL
        rol     XSAVH
; TOS is the error accumulator, initialized to 2*dx - dy
        movw    TOS, TMP
        sub     TOSL, r16
        sbc     TOSH, ZEROL
; check lsb of color--white or black?
        lds     r18, globalcolor
        sbrs    r18, 0
        rjmp    .bsteep_black
.bsteep_white:
1:      ld      r18, Z          ; read existing pixel
        or      r18, r19        ; or in the new pixel
        st      Z, r18
; is D (error accumulator) > 0?
        tstw    TOS             ; will always clear carry
        breq    2f
        brmi    2f
; if so, subtract 2*dy from error accumulator and advance to next column
        sub     TOSL, XSAVL
        sbc     TOSH, XSAVH
        ; rotate mask to the right
        ; sets carry flag when when the pattern repeats (i.e. a 1 is shifted out)
        bst     r19, 0
        lsr     r19
        bld     r19, 7
; advance to next row
2:      adc     ZL, r20         ; if carry flag is set, also advance 1 column to the right
        adc     ZH, r21
; add 2*dx to error accumulator
        add     TOSL, TMPL
        adc     TOSH, TMPH
; decrement loop count
        subi    r16, 1          ; don't use dec--it doesn't set the carry flag!
        brcc    1b
        popd
        rnext
; "shallow" lines (dy <= dx)
.bshallow:
; TMP holds 2*dy. (signed 16-bit)
        mov     TMPL, TOSH
        clr     TMPH
        lsl     TMPL
        rol     TMPH
; r16 is the loop counter (initialized to dx)
        mov     r16, TOSL
; XSAV holds 2*dx
        mov     XSAVL, TOSL
        clr     XSAVH
        lsl     XSAVL
        rol     XSAVH
; TOS is the error accumulator, initialized to 2*dy - dx
        movw    TOS, TMP
        sub     TOSL, r16
        sbc     TOSH, ZEROL
; check lsb of color--white or black?
        lds     r18, globalcolor
        sbrs    r18, 0
        rjmp    .bshallow_black
.bshallow_white:
; initialize bitmasks
        clr     r18             ; bits to or in for this byte
; draw pixels
; Unlike with other color depths, it's faster to always read-modify-write than
; check if r18=0xFF and branch
1:      bst     r19, 0          ; save lsb of pixel mask
        or      r18, r19        ; update byte mask
        ld      TSAVL, Z        ; read existing byte
        or      TSAVL, r18      ; set pixels
        st      Z, TSAVL        ; store
        brtc    2f              ; add 1 to Z and reset byte mask if we crossed a byte boundary
        clr     r18
        adiw    Z, 1
2:      lsr     r19             ; rotate pixel mask
        bld     r19, 7
; is D (error accumulator) > 0?
        tstw    TOS
        breq    3f
        brmi    3f
; if so, subtract 2*dx from error accumulator and advance to next row
        sub     TOSL, XSAVL
        sbc     TOSH, XSAVH
        add     ZL, r20
        adc     ZH, r21
        clr     r18             ; reset byte mask if we advance down a row
; add 2*dy to error accumulator
3:      add     TOSL, TMPL
        adc     TOSH, TMPH
; decrement loop count
        subi    r16, 1          ; don't use dec--it doesn't set the carry flag!
        brcc    1b
        popd
        rnext
.bsteep_black:
        com     r19             ; invert pixel mask
1:      ld      r18, Z          ; read existing pixel
        and     r18, r19        ; turn off the new pixel
        st      Z, r18
; is D (error accumulator) > 0?
        tstw    TOS             ; will always clear carry
        breq    2f
        brmi    2f
; if so, subtract 2*dy from error accumulator and advance to next column
        sub     TOSL, XSAVL
        sbc     TOSH, XSAVH
        ; rotate mask to the right
        ; clears carry flag when when the pattern repeats (i.e. a 0 is shifted out)
        bst     r19, 0
        lsr     r19
        bld     r19, 7
        cpi     r19, 0b10000000 ; set carry flag if msb of r19 is 0 (i.e. a 0 was just shifted out)
; advance to next row
2:      adc     ZL, r20         ; if carry flag is set, also advance 1 column to the right
        adc     ZH, r21
; add 2*dx to error accumulator
        add     TOSL, TMPL
        adc     TOSH, TMPH
; decrement loop count
        subi    r16, 1          ; don't use dec--it doesn't set the carry flag!
        brcc    1b
        popd
        rnext
.bshallow_black:
; initialize bitmasks
        com     r19             ; invert pixel mask
        ser     r18             ; initialize byte mask
; draw pixels
; Unlike with other color depths, it's faster to always read-modify-write than
; check if r18=0xFF and branch
1:      bst     r19, 0          ; save lsb of pixel mask
        and     r18, r19        ; update byte mask
        ld      TSAVL, Z        ; read existing byte
        and     TSAVL, r18      ; clear pixels
        st      Z, TSAVL        ; store
        brts    2f              ; add 1 to Z and reset byte mask if we crossed a byte boundary (a 0 was shifted out)
        ser     r18
        adiw    Z, 1
2:      lsr     r19             ; rotate pixel mask
        bld     r19, 7
; is D (error accumulator) > 0?
        tstw    TOS
        breq    3f
        brmi    3f
; if so, subtract 2*dx from error accumulator and advance to next row
        sub     TOSL, XSAVL
        sbc     TOSH, XSAVH
        add     ZL, r20
        adc     ZH, r21
        ser     r18             ; reset byte mask if we advance down a row
; add 2*dy to error accumulator
3:      add     TOSL, TMPL
        adc     TOSH, TMPH
; decrement loop count
        subi    r16, 1          ; don't use dec--it doesn't set the carry flag!
        brcc    1b
        popd
        rnext

; Interpreter dispatch table aligned to a 256-word (512-byte) boundary.
.align 9
.global vm_dispatch_table
vm_dispatch_table:
        create_dispatch_table   .opcode_, 255
.align 0

; 0xFF: (EXT) extended instruction
opcode_ext:
        ldi     DTH, pm_hi8(vm_ext_dispatch_table)
        next


; (CALL) ( R: -- a ) push IP onto return stack and jump to address
opcode_call:
        ld      TMPL, IP+       ; get lower byte of new IP
        pushr                   ; push return address
        mov     IPL, TMPL       ; set new IP lower byte
        mov     IPH, IR         ; set new IP upper byte
        next

; callable from C (error code in r24)
.global forth_throw_if
forth_throw_if:
        tstw    r24
        brne    forth_throw
        ret
; callable from C (error code in r24)
.global forth_throw
forth_throw:
; move error code into r21:r20
        movw    r20, r24
; restore the constant registers. the exception may have been thrown from within
; a C function that clobbered them.
        clr     ZEROL
        clr     ZEROH
        movw    TRUE, ZERO
        com     TRUEL
        com     TRUEH
        ldi     CDTH, pm_hi8(vm_dispatch_table)
        lds     ZL, forth_flags
        sbrc    ZL, FF_TRACE_BIT
        ldi     CDTH, pm_hi8(vm_debug_dispatch_table)

; exception number in r21:r20
.global do_throw
do_throw:
; set T flag, indicating an exception code is present
        set
; get current exception frame
        lds     ZL, forth_exception_frame
        lds     ZH, forth_exception_frame+1
; invoke the global exception handler if no current frame
        sbrc    ZH, 7
        jmp     forth_global_exception_handler
; roll the return stack back to the top of the exception frame
        z_to_rsp
; pop and restore the address of the previous frame
        pop     ZL
        pop     ZH
        sts     forth_exception_frame, ZL
        sts     forth_exception_frame+1, ZH
; pop and jump to the exception handler
        pop     ZL
        pop     ZH
        ijmp

; BREAK ( -- ) jump to the current exception handler, without popping the
; exception frame or pushing an error code
opcode_break:
; clear T flag, indicating there is no exception code
        clt
; get current exception frame
        lds     ZL, forth_exception_frame
        lds     ZH, forth_exception_frame+1
; invoke the global exception handler if no current frame
        sbrc    ZH, 7
        jmp     forth_global_exception_handler
; otherwise, read the exception handler address out of the current context
        ldd     TMPL, Z+3
        ldd     TMPH, Z+4
        movw    Z, TMP
        ijmp

; 0 ( -- 0 ) push zero on stack
opcode_zero:
        pushd
        movw    TOS, ZERO
        next

; -1 ( -- 0xFFFF ) push -1 on stack
opcode_minus1:
        pushd
        movw    TOS, TRUE
        next

; (C#) ( -- c ) push 8-bit (unsigned) literal from instruction stream
opcode_lit8:
        pushd
        ld      TOSL, IP+
        ldi     TOSH, 0
        next

; (#) ( -- n ) push 16-bit (little-endian) literal from instruction stream
opcode_lit16:
        pushd
        ld      TOSL, IP+
        ld      TOSH, IP+
        next

; (S") ( -- addr len ) literal string in instruction stream (0-255 bytes); push address and length, then skip over string
opcode_litstr:
        ld      TMPL, IP+       ; get string length from instruction stream
        clr     TMPH
        pushd
        movw    TOS, IP         ; push address of string
        pushd
        movw    TOS, TMP        ; push length
        add     IPL, TOSL       ; skip over string
        adc     IPH, TOSH
        next

; DUP ( n -- n n ) duplicate TOS
opcode_dup:
        pushd
        next

; DROP ( n -- ) remove from data stack
opcode_drop:
        popd
        next

; NIP ( n1 n2 -- n2 ) delete item below top of stack
opcode_nip:
        adiw    DSP, 2
        rnext

; SWAP ( n1 n2 -- n2 n1 ) exchange top of stack with item below
opcode_swap:
        ld      TMPL, DSP       ; get second item on stack
        ldd     TMPH, DSP+1
        st      DSP, TOSL       ; put top item into second item
        std     DSP+1, TOSH     ; put top item into second item
        movw    TOS, TMP        ; put previous second item into top item
        next

; OVER ( n1 n2 -- n1 n2 n1 ) copy second item on stack to top of stack
opcode_over:
        pushd
        ldd     TOSL, DSP+2
        ldd     TOSH, DSP+3
        next

; ROT ( n1 n2 n3 -- n2 n3 n1 ) rotate 3rd element to top of stack
; n1 = TOSH:TOSL
; n2 = [DSP+1]:[DSP+0]
; n3 = [DSP+2]:[DSP+3]
opcode_rot:
        movw    TMP, TOS
        ldd     TOSL, DSP+2
        ldd     TOSH, DSP+3
        ld      IR, DSP
        std     DSP+2, IR
        ldd     IR, DSP+1
        std     DSP+3, IR
        st      DSP, TMPL
        std     DSP+1, TMPH
        next

; 2DUP ( n1 n2 -- n1 n2 n1 n2 ) copy top two items on stack
opcode_twodup:
        movw    TMP, TOS        ; save n2
        pushd
        ldd     TOSL, DSP+2     ; get n1
        ldd     TOSH, DSP+3
        pushd                   ; push n2
        movw    TOS, TMP
        next

; 2DROP ( n1 n2 -- ) drop top two items on stack
opcode_twodrop:
        drop2
        next

; R@ ( -- n   R: -- ) push top of return stack onto data stack
; also can be used as "I"
opcode_rfetch:
        pushd
        rsp_to_z
        ldd     TOSL, Z+1
        ldd     TOSH, Z+2
        rnext

; (C#+R@) ( -- n ) fetch word from top of return stack plus literal 8-bit unsigned displacement
; RSP points to the next *free* location in the return stack,
; so (C#+R@) 0x01 is equivalent to R@, (C#+R@) 0x03 gets second item on return stack, etc.
opcode_rfetchlitoffset:
        rsp_to_z
        ld      TMPL, IP+       ; get displacement from instruction stream
        add     ZL, TMPL        ; add displacement
        adc     ZH, ZERO
        pushd
        ld      TOSL, Z         ; get word
        ldd     TOSH, Z+1
        rnext

; >R ( n --   R: -- n ) pop from data stack and push onto return stack
opcode_tor:
        push    TOSH
        push    TOSL
        popd
        next

; R> ( -- n   R: n -- ) pop from return stack and push onto data stack
opcode_rfrom:
        pushd
        pop     TOSL
        pop     TOSH
        next

; RDROP ( R: n -- ) drop item from return stack
opcode_rdrop:
        pop     TMPL    ; ignore the value
        pop     TMPH
        next

; 2RDROP ( R: n1 n2 -- ) drop top two items from return stack (aka UNLOOP)
opcode_twordrop:
; 4 pops is 8 cycles, but incrementing RSP is only 6
        rsp_to_z
        adiw    Z, 4
        z_to_rsp
        rnext

; 2R@ ( -- x1 x2 ) ( R: x1 x2 -- x1 x2 ) copy cell pair from TOR to TOS
opcode_tworfetch:
        rsp_to_z
        pushd
        ldd     TOSL, Z+3
        ldd     TOSH, Z+4
        pushd
        ldd     TOSL, Z+1
        ldd     TOSH, Z+2
        rnext

; 2>R ( limit index -- R: -- limit index ) runtime of DO
; pop index from stack
; pop limit from stack
; push limit onto return stack
; push index onto return stack
opcode_twotor:
        nip             ; get limit in TMP
        push    TMPH    ; push limit on return stack
        push    TMPL
        push    TOSH    ; push index on return stack
        push    TOSL
        popd
        next

; 2R> ( R: n1 n2 -- ) ( -- n1 n2 ) drop top two items from return stack (aka UNLOOP)
opcode_tworfrom:
        pushd
        pop     TMPL    ; get n2
        pop     TMPH
        pop     TOSL    ; get n1
        pop     TOSH
        pushd
        movw    TOS, TMP
        next

; (R>LINK) ( R: n -- ) ( -- ) move top of return stack to the temporary link register
; Can be used to preserve the return address in words that modify or inspect the
; return stack.
opcode_link:
        pop     LINKL
        pop     LINKH
        next

; (LINK>IP) ( R: n1 -- ) ( -- ) jump to the address in the temporary link register
; Can be used to exit properly from a word that has modified the return stack.
opcode_unlink:
        movw    IP, LINKL
        next

; 0= ( n -- flag ) is TOS equal to 0?
opcode_eq0:
        tstw    TOS
        breq    .flag_true
        movw    TOS, ZERO
        next

.flag_true:
        movw    TOS, TRUE
        next

; 0< ( n -- flag ) is TOS less than 0? (sign bit set)
opcode_lt0:
        sbrc    TOSH, 7
        rjmp    .flag_true
        movw    TOS, ZERO
        next

; 0> ( n -- flag ) is TOS greater than 0? (signed comparison)
opcode_gt0:
        cp      ZERO, TOSL
        cpc     ZERO, TOSH
        brlt    .flag_true
        movw    TOS, ZERO
        next

; = ( n1 n2 -- flag ) compare top two stack items for equality
opcode_eq:
        nip
        cp      TOSL, TMPL
        cpc     TOSH, TMPH
        breq    .flag_true
        movw    TOS, ZERO
        next

; < ( n1 n2 -- flag ) is n1 less than n2? (signed)
opcode_lt:
        nip
        cp      TMPL, TOSL
        cpc     TMPH, TOSH
        brlt    .flag_true
        movw    TOS, ZERO
        next

; > ( n1 n2 -- flag ) is n1 greater than n2? (signed)
opcode_gt:
        nip
        cp      TOSL, TMPL
        cpc     TOSH, TMPH
        brlt    .flag_true
        movw    TOS, ZERO
        next

; U< ( n1 n2 -- flag ) is n1 less than n2? (unsigned)
opcode_ult:
        nip
        cp      TMPL, TOSL
        cpc     TMPH, TOSH
        brlo    .flag_true
        movw    TOS, ZERO
        next

; U> ( n1 n2 -- flag ) is n1 greater than n2? (unsigned)
opcode_ugt:
        nip
        cp      TOSL, TMPL
        cpc     TOSH, TMPH
        brlo    .flag_true
        movw    TOS, ZERO
        next

; 1+ ( n -- n ) add 1 to TOS
opcode_oneplus:
        adiw    TOS, 1
        next

; 1- ( n -- n ) subtract 1 from TOS
opcode_oneminus:
        sbiw    TOS, 1
        next

; 2+ ( n -- n ) add 2 (the size of a cell) to TOS
opcode_twoplus:
        adiw    TOS, 2
        next

; 2* ( n -- n ) logical left shift TOS
opcode_twostar:
        lsl     TOSL
        rol     TOSH
        next

; 2/ ( n -- n ) arithmetic right shift TOS
opcode_twoslash:
        asr     TOSH
        ror     TOSL
        next

; U2/ ( n -- n ) logical right shift TOS
opcode_utwoslash:
        lsr     TOSH
        ror     TOSL
        next

; ABS ( n -- u ) absolute value of TOS
opcode_abs:
        tst     TOSH
        brpl    1f      ; don't negate if positive

; NEGATE ( n -- n ) two's complement of TOS
opcode_negate:
        com     TOSH
        neg     TOSL
        sbci    TOSH, -1
1:      next

; + ( n1 n2 -- n1+n2 ) add top two stack items
opcode_plus:
        nip
        add     TOSL, TMPL
        adc     TOSH, TMPH
        next

; - ( n1 n2 -- n1-n2 ) subtract top two stack items
opcode_minus:
        nip
        sub     TMPL, TOSL
        sbc     TMPH, TOSH
        movw    TOS, TMP
        next

; INVERT ( n -- n ) ones' complement of TOS
opcode_invert:
        com     TOSL
        com     TOSH
        next

; AND ( n1 n2 -- n1&n2 ) bitwise AND of top two stack items
opcode_and:
        nip
        and     TOSL, TMPL
        and     TOSH, TMPH
        next

; (C#AND) ( n -- c ) read byte from bytecode stream and bitwise AND with top of stack
opcode_candlit:
        ld      TMPL, X+
        and     TOSL, TMPL
        clr     TOSH
        next

; OR ( n1 n2 -- n1|n2 ) bitwise OR of top two stack items
opcode_or:
        nip
        or      TOSL, TMPL
        or      TOSH, TMPH
        next

; XOR ( n1 n2 -- n1^n2 ) bitwise XOR of top two stack items
opcode_xor:
        nip
        eor     TOSL, TMPL
        eor     TOSH, TMPH
        next


; C@ ( c-addr -- c ) read byte from address and push onto stack
opcode_cfetch:
        movw    Z, TOS
        ld      TOSL, Z
        clr     TOSH
        rnext

; C! ( c c-addr -- ) store value to byte address
opcode_cstore:
        movw    Z, TOS
        popd
        st      Z, TOSL
        popd
        rnext

; (C#@) ( -- c ) fetch and push byte from literal address
opcode_cfetchlit:
        ld      ZL, IP+         ; get literal address from instruction stream
        ld      ZH, IP+
        pushd
        ld      TOSL, Z+        ; get byte
        clr     TOSH
        rnext

; (C#!) ( c -- ) store byte to literal address
opcode_cstorelit:
        ld      ZL, IP+         ; get address from instruction stream
        ld      ZH, IP+
        st      Z+, TOSL        ; store byte
        popd
        rnext

; (C#+HC@) ( -- c ) fetch byte from 0x4000 plus literal 8-bit unsigned displacement (in bytes)
opcode_cfetchlithi:
        pushd
        ld      ZL, IP+         ; get displacement from instruction stream
        ldi     ZH, hi8(high_page_base)
        ld      TOSL, Z         ; get byte
        clr     TOSH
        rnext

; (C#+HC!) ( c -- ) store byte to 0x4000 plus literal 8-bit unsigned displacement (in bytes)
opcode_cstorelithi:
        ld      ZL, IP+         ; get displacement from instruction stream
        ldi     ZH, hi8(high_page_base)
        st      Z, TOSL         ; store byte
        popd
        rnext

; C@+ ( addr -- addr+1 c ) read character at addr and postincrement addr
opcode_cfetchplus:
        movw    Z, TOS
        ld      TMPL, Z+
        movw    TOS, Z
        pushd
        mov     TOSL, TMPL
        clr     TOSH
        rnext

; C!+ ( addr c -- addr+1 ) store character to addr and postincrement addr
; note reversed argument order!
opcode_cstoreplus:
        ld      ZL, DSP+        ; nip address
        ld      ZH, DSP+
        st      Z+, TOSL
        movw    TOS, Z
        rnext

; (C#+HCBITS@) ( -- c ) fetch bits from 0x4000 plus literal 8-bit unsigned displacement
opcode_cfetchbitshi:
        pushd
        ld      TMPL, IP+       ; get mask from instruction stream
        ld      ZL, IP+         ; get displacement from instruction stream
        ldi     ZH, hi8(high_page_base)
        ld      TOSL, Z         ; get byte
        and     TOSL, TMPL      ; apply mask
        clr     TOSH
        rnext

; (C#+HCBITS!) ( c -- ) store bits to 0x4000 plus literal 8-bit unsigned displacement
opcode_cstorebitshi:
        ld      TMPL, IP+       ; get mask from instruction stream
        and     TOSL, TMPL      ; apply mask to incoming value
        com     TMPL            ; complement mask
        ld      ZL, IP+         ; get displacement from instruction stream
        ldi     ZH, hi8(high_page_base)
        ld      TMPH, Z         ; get byte
        and     TMPH, TMPL      ; apply mask
        or      TMPH, TOSL      ; set bits
        st      Z, TMPH         ; store byte
        popd
        rnext

; @ ( addr -- n ) read word from address and push onto stack
opcode_fetch:
        movw    Z, TOS
        ld      TOSL, Z
        ldd     TOSH, Z+1
        rnext

; ! ( n addr -- ) store value to address
opcode_store:
        movw    Z, TOS
        popd
        st      Z, TOSL
        std     Z+1, TOSH
        popd
        rnext

; +! ( n addr -- ) add n to word at address
opcode_plusstore:
        movw    Z, TOS
        ld      TMPL, Z
        ldd     TMPH, Z+1
        popd
        add     TMPL, TOSL
        adc     TMPH, TOSH
        st      Z, TMPL
        std     Z+1, TMPH
        popd
        rnext

; (#@) ( -- n ) fetch and push word from literal address
opcode_fetchlit:
        ld      ZL, IP+         ; get address from instruction stream
        ld      ZH, IP+
        pushd
        ld      TOSL, Z+        ; get word
        ld      TOSH, Z+
        rnext

; (#!) ( n -- ) store value to literal address
opcode_storelit:
        ld      ZL, IP+         ; get address from instruction stream
        ld      ZH, IP+
        st      Z+, TOSL        ; store word
        st      Z+, TOSH
        popd
        rnext

; (C#+H@) ( -- n ) fetch word from 0x4000 plus literal 8-bit unsigned displacement (in bytes)
opcode_fetchlithi:
        pushd
        ld      ZL, IP+         ; get displacement from instruction stream
        ldi     ZH, hi8(high_page_base)
        ld      TOSL, Z         ; get word
        ldd     TOSH, Z+1
        rnext

; (C#+!) ( n -- ) store word to 0x4000 plus literal 8-bit unsigned displacement (in bytes)
opcode_storelithi:
        movw    Z, TOS          ; get address from stack
        ld      ZL, IP+         ; get displacement from instruction stream
        ldi     ZH, hi8(high_page_base)
        st      Z, TOSL         ; store word
        std     Z+1, TOSH
        popd
        rnext

; 2@ ( addr -- x1 x2 ) fetch two cells from address in TOS
; x2 is the word at addr, x1 is the word at addr+2.
opcode_twofetch:
        movw    Z, TOS          ; need an index register
        ldd     TOSL, Z+2       ; push value of high cell
        ldd     TOSH, Z+3
        pushd
        ld      TOSL, Z         ; then value of low cell
        ldd     TOSH, Z+1
        rnext

; 2! ( x1 x2 addr -- ) store two cells to address in TOS
; x2 is stored at addr and x1 at addr+2.
opcode_twostore:
        movw    Z, TOS          ; need an index register
        popd
        st      Z, TOSL
        std     Z+1, TOSH
        popd
        std     Z+2, TOSL
        std     Z+3, TOSH
        popd
        rnext

; SELECT ( true-n false-n flag -- n ) IF DROP ELSE NIP THEN
; pop flag from stack, if zero, discard second stack item leaving false-n,
; if nonzero, drop top of stack, leaving true-n
; like the ternary operator in other languages, but does not short-circuit
opcode_select:
        tstw    TOS
        popd
        brne    1f
        adiw    DSP, 2
        next
1:      popd
        next

; (?DO) ( limit index -- ) ( R: -- ) if limit==index
;       ( limit index -- ) ( R: -- limit index ) if limit!=index
opcode_qdo:
        nip                     ; get limit in TMP
        cp      TOSL, TMPL      ; limit equal to index?
        cpc     TOSH, TMPH
        breq    .qdo_noloop
        push    TMPH            ; push limit on return stack
        push    TMPL
        push    TOSH            ; push index on return stack
        push    TOSL
        adiw    IP, 1           ; skip branch offset
        popd
        next
.qdo_noloop:
        ld      TMPL, X+        ; get branch offset
        add     IPL, TMPL
        adc     IPH, ZERO
        popd                    ; drop index
        next

; (LOOP) ( -- R: -- limit index | -- ) runtime of LOOP
; add 1 to index on return stack
; compare index with limit, push flag on stack
; if index == limit, drop index and limit from return stack
opcode_loop:
        rsp_to_z
        ldd     TMPL, Z+3       ; get limit
        ldd     TMPH, Z+4
        pop     ZL              ; get index
        pop     ZH
        adiw    Z, 1            ; increment index
.loop_compare:
        cp      ZL, TMPL        ; compare with limit
        cpc     ZH, TMPH
        breq    .loop_terminate
.loop_continue:
; continue--put new index back on rstack
        push    ZH
        push    ZL
; read branch offset and jump back
        ld      TMPL, IP        ; compensate for 2 bytes of instruction
        sec
        sbc     IPL, TMPL
        sbc     IPH, ZERO
        rnext
.loop_terminate:
; discard remainder of loop context
        pop     TMPL
        pop     TMPL
; skip branch offset, we don't need it
        adiw    IP, 1
        rnext

; (+LOOP) ( n --   R: -- limit index | -- ) runtime of +LOOP
; pop from TOS and add to index on return stack
; If the loop index did not cross the boundary between the loop limit minus one
; and the loop limit, continue execution at the beginning of the loop.
; Otherwise, discard the current loop control parameters and continue execution
; immediately following the loop.
opcode_plusloop:
        rsp_to_z
        ; in      ZL, RSPL
        ; in      ZH, RSPH
        ldd     TMPL, Z+3       ; get limit
        ldd     TMPH, Z+4
        pop     ZL              ; get index
        pop     ZH
        movw    r20, Z          ; save index
        add     ZL, TOSL        ; add increment
        adc     ZH, TOSH        ; Z contains new index value
; Anton Ertl says:
; "Note that the loop control parameters can be either signed or unsigned, and
; +LOOP has to work for both. For systems with 2s-complement representation for
; signed numbers, the way to go is to use circular arithmetic:
; compute x=(index-limit)+minint, and observe if the addition x+n crosses the
; boundary between minint and maxint. Many architectures report this through the
; overflow flag."
        sub     r20, TMPL
        sbc     r21, TMPH
; this can be optimized out by adding 0x8000 to the loop index and limit only
; once during (DO) (this is what amforth does) but that no longer means I is
; equivalent to R@ and I'd have to add another opcode
        subi    r20, lo8(-0x8000)
        sbci    r21, hi8(-0x8000)
; add increment
        add     r20, TOSL
        adc     r21, TOSH
        popd                    ; pop increment
; test for overflow
        brvs    .loop_terminate
        rjmp    .loop_continue

; (XY-LOOP) ( -- R: -- xy-limit xy-index | -- ) runtime of XY-LOOP
; add 1 to LSB of index on return stack
; compare index LSB with limit LSB. if equal, set index LSB to 0 and
; add 1 to index MSB.
; compare index MSB with limit MSB. push flag on stack
; if index MSB == limit MSB, drop index and limit from return stack
;!!! NOTE: will only work correctly if initial x-index value is 0. So it's NOT
; identical to two nested DO loops.
opcode_xyloop:
        rsp_to_z
        ldd     TMPL, Z+1       ; get X index
        ldd     TMPH, Z+3       ; get X limit
        inc     TMPL            ; increment X index
        cp      TMPL, TMPH      ; compare with X limit
        breq    .xyloop_inc_y
; continue--put X back on rstack
        std     Z+1, TMPL
; read branch offset and jump back
        ld      TMPL, IP        ; compensate for 2 bytes of instruction
        sec
        sbc     IPL, TMPL
        sbc     IPH, ZERO
        rnext
.xyloop_inc_y:
        ldd     TMPL, Z+2       ; get Y index
        ldd     TMPH, Z+4       ; get Y limit
        inc     TMPL            ; increment Y index
        cp      TMPL, TMPH      ; compare with Y limit
        breq    .xyloop_terminate
; continue--reset X to 0 and write new X and Y to rstack
        std     Z+1, ZERO       ; reset X index
        std     Z+2, TMPL       ; store new Y index
; read branch offset and jump back
        ld      TMPL, IP        ; compensate for 2 bytes of instruction
        sec
        sbc     IPL, TMPL
        sbc     IPH, ZERO
        rnext
.xyloop_terminate:
; discard loop context
        adiw    Z, 4
        z_to_rsp
; skip branch offset, we don't need it
        adiw    IP, 1
        rnext


; (OF) ( x1 x2 -- | x1 ) runtime of OF
; If x1 and x2 are not equal, discard x2, read forward displacement value from
; bytecode stream, and jump forward.
; If x1 and x2 are equal, discard both and continue execution.
opcode_of:
        nip                     ; nip x1 into TMP
        cp      TMPL, TOSL
        cpc     TMPH, TOSH
        brne    .of_notequal
; two cells equal? skip branch offset, drop TOS, and continue
        ld      TMPL, IP+       ; ignore
        popd
        next
; not equal? keep x1 and branch
.of_notequal:
        movw    TOS, TMP
        rjmp    opcode_branchfwd

; (0BRANCH<) pop and branch backward (0-255 bytes) if zero
opcode_zbranchback:
        tstw    TOS
        popd
        brne    .nobranch
; (BRANCH<) backward branch (0-255 bytes)
opcode_branchback:
        ld      TMPL, IP        ; compensate for 2 bytes of instruction
        sec
        sbc     IPL, TMPL
        sbc     IPH, ZERO
        next
.nobranch:
        ld      TMPL, IP+       ; discard branch offset
        next

; (0BRANCH>) pop and branch forward (0-255 bytes) if zero
opcode_zbranchfwd:
        tstw    TOS
        popd
        brne    .nobranch
; (BRANCH<) forward branch (0-255 bytes)
opcode_branchfwd:
        ld      TMPL, IP+
        add     IPL, TMPL
        adc     IPH, ZERO
        next

; (NP0+CALL) add value to np0 and call
opcode_np0relcall:
        ld      TMPL, IP+       ; get lower byte of offset
        ld      TMPH, IP+       ; get upper byte of offset
        pushr                   ; push return address
        lds     IPL, forth_np0
        lds     IPH, forth_np0+1
        add     IPL, TMPL       ; add offset to current IP
        adc     IPH, TMPH
        next

; (CCALL) ( i*x -- j*x ) call C or assembler function in flash
; Assumes the function takes from one to four cell-sized arguments.
; TOS is overwritten with return value, no other stack elements are affected.
; First argument is TOS
; Second argument is [DSP+1:DSP]
; Third argument is [DSP+3:DSP+2]
; Fourth argument is [DSP+5:DSP+4]
; Should only be used in ROM words!
opcode_ccall:
        ld      ZL, IP+
        ld      ZH, IP+
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        ldd     r20, DSP+2
        ldd     r21, DSP+3
        ldd     r18, DSP+4
        ldd     r19, DSP+5
        callc_prologue
        icall
        callc_epilogue
        rnext

; (JUMP) set IP to absolute address
opcode_jump:
        ld      TMPL, IP+
        ld      TMPH, IP+
        movw    IP, TMP
        next

; (NP0JUMP) set IP to NP0-relative address
opcode_np0reljump:
        ld      TMPL, IP+
        ld      TMPH, IP+
        lds     IPL, forth_np0
        lds     IPH, forth_np0+1
        add     IPL, TMPL
        adc     IPH, TMPH
        next

.global .throw_forth_err_code
.throw_forth_err_code:
; THROW ( n -- ) pop n and abort if nonzero
opcode_throw:
        tstw    TOS
        movw    r20, TOS
        popd
        breq    1f
        rthrow_r21r20
1:      rnext

; EMIT ( c -- ) write character at TOS to output device
opcode_emit:
        callc_prologue
        lds     ZL, mio_putc
        lds     ZH, mio_putc+1
        icall
        popd
        callc_epilogue

; (C#EMIT) read byte from instruction stream and write to output device
opcode_emitlit:
        movw    TSAV, TOS       ; need to save TOS
        ld      r24, IP+        ; then read char from bytecode stream
        movw    XSAV, IP        ; *now* save IP
        lds     ZL, mio_putc
        lds     ZH, mio_putc+1
        icall
        callc_0arg_epilogue

; H. ( u -- ) write single-cell number to output device as four hex digits
; (with leading zeros). Does not use the pictured numeric output buffer and does
; NOT print a trailing space. Thus, multiple consecutive invocations can be used
; to print larger numbers. (e.g. H. H. will print a double-cell as 8 hex digits)
opcode_hdot:
        callc_prologue
        call    mio_x16
        popd
        callc_epilogue

; HOLD ( c -- ) add character to the pictured numeric output buffer
opcode_hold:
        callc_prologue
        call    forth_hold
        popd
        callc_epilogue

; # ( ud1 -- ud2 ) extract digit to the pictured numeric output buffer
opcode_num:
        callc_prologue
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        call    forth_extract_digit
        st      DSP, TMPL
        std     DSP+1, TMPH
        callc_epilogue

; TYPE ( c-addr len -- ) write len characters from c-addr to output
opcode_type:
; print_fstr() takes args in this order:
; r25:r24 - len (and RAM/ROM flag in msb)
; r23:r22 - ptr (RAM or ROM)
        callc_prologue
        nip                     ; nip address into r23:r22
        call    print_fstr
        popd                    ; drop remaining item on stack
        callc_epilogue

; KEY ( -- c ) receive a character from the input source
opcode_key:
        pushd
        callc_prologue
        clr     TOSL    ; block until char is received
        lds     ZL, mio_getc
        lds     ZH, mio_getc+1
        icall
        callc_epilogue

; ALLOT ( n -- ) allocate n bytes in the body of the current definition.
; (If LATEST is defined as a compiler word, this advances the name space pointer
; by n. Otherwise, advances the code space pointer by n.)
; throws an exception if insufficient space
opcode_allot:
        callc_prologue
        call    allot
        callc_restore
        rjmp    .throw_forth_err_code

; HERE ( -- addr ) return the next free address in the current definition
opcode_here:
        pushd
        callc_prologue
        call    here
        callc_epilogue

; C, ( c -- ) store low byte of TOS in the body of the current definition
opcode_ccomma:
        callc_prologue
        call    ccomma
        popd
        callc_epilogue

; , ( n -- ) store TOS in the body of the current definition
opcode_comma:
        callc_prologue
        call    comma
        popd
        callc_epilogue

; X, ( n -- ) store TOS into code space
opcode_xcomma:
        callc_prologue
        call    codespace_comma
        popd
        callc_epilogue

; XALLOT ( n -- ) allocate n bytes in code space.
opcode_xallot:
        callc_prologue
        call    codespace_allot
        callc_restore
        rjmp    .throw_forth_err_code

; (C#,) ( -- ) read byte from bytecode stream and store to body of current definition
opcode_litccomma:
        ld      TMPL, X+
        callc_0arg_prologue
        mov     TOSL, TMPL
        call    ccomma
        callc_0arg_epilogue

; ; (#,) ( -- ) read word from bytecode stream and store to body of current definition
; opcode_litcomma:
;         ld      TMPL, X+
;         ld      TMPH, X+
;         callc_0arg_prologue
;         movw    TOS, TMP
;         call    comma
;         callc_0arg_epilogue

; (HEADER) ( -- ) read a name from the input stream and create a header for it.
; Read the flags from the next byte in the bytecode stream. Creates a data field
; based on the type specified in the flags.
opcode_header:
        callc_0arg_prologue
        call    forth_parse_name
        movw    IP, XSAV
        ld      r20, IP+
        sbrc    r20, 1  /* check for FL_SAVE_STACK_PTRS */
        rjmp    .save_stack_ptrs
3:      movw    XSAV, IP
        call    namespace_create
        callc_0arg_epilogue

.save_stack_ptrs:
        sts     forth_saved_dsp, DSPL
        sts     forth_saved_dsp+1, DSPH
        rsp_to_z
        sts     forth_saved_rsp, ZL
        sts     forth_saved_rsp+1, ZH
        rjmp    3b

; (C::DOES>) ( -- ) creates a new compiler-child word and sets its runtime
; behavior to the literal xt in the bytecode stream
opcode_makechild:
        callc_0arg_prologue
        call    forth_parse_name
        ldi     r20, FL_COMPILER_CHILD
        call    namespace_create
        callc_0arg_restore
        ; fall through to (DOES>)

; (DOES>) ( -- ) change the latest word's runtime behavior to that of the
; literal nt in the bytecode stream
opcode_does:
        ld      TMPL, IP+       ; read nt
        ld      TMPH, IP+
        callc_0arg_prologue
        movw    TOS, TMP
        call    interpret_does
        callc_0arg_epilogue

; PARSE-NAME ( -- c-addr len ) obtain the next word from the TIB, skipping any
; leading spaces, and stopping after the next space.
opcode_parsename:
        pushd
        callc_prologue
        call    forth_parse_name
        ; upon return: addr in r23:r22, len in r25:r24
        st      -DSP, TMPH
        st      -DSP, TMPL
        callc_epilogue

; PARSE ( delim -- c-addr len ) obtain the next word from the TIB, stopping
; after the next occurrence of delim.
.global opcode_parse
opcode_parse:
        callc_prologue
        call    forth_parse
        ; upon return: addr in r23:r22, len in r25:r24
        st      -DSP, TMPH
        st      -DSP, TMPL
        callc_epilogue

; ' ( -- xt ) read a name from the input stream and push its execution token
opcode_tick:
        pushd
        callc_prologue
        call    forth_parse_name
        tstw    TOS
        breq    1f
        call    find_xt
        ; 0 indicates no xt
        tstw    TOS
        breq    2f
        callc_epilogue
1:      rthrow  FE_ZERO_LENGTH_NAME
2:      rthrow  FE_UNDEFINED_WORD

; (DODOES) ( -- addr ) runtime of DOES> words. Push body address on stack,
; then read inline xt from bytecode stream and execute it.
opcode_dodoes:
        ; get inline xt
        ld      TMPL, IP+
        ld      TMPH, IP+
        ; push current IP value (the body address) on stack
        pushd
        movw    TOS, IP
        ; invoke the xt in TMP
        rjmp    .execute_tmp

; EXECUTE ( i*x xt -- j*x ) get execution token from top of stack and perform its
; execution semantics
.global opcode_execute
opcode_execute:
        movw    TMP, TOS        ; drop xt from stack
        popd
.execute_tmp:
        clr     r18             ; tell forth_execute_xt to push a return addr
        jmp     forth_execute_xt

; COMPILE, ( i*x ct -- j*x ) get execution token from top of stack and perform
; its compilation semantics
opcode_compile:
; pop compilation token
        movw    TMP, TOS
        popd
        jmp     forth_compile_xt

; (MARK) ( -- addr ) compile dummy branch instruction and push address of offset on stack
opcode_mark:
        ld      TMPL, X+        ; get branch instruction to compile
        pushd
        callc_prologue
        movw    TOSL, TMPL
        clr     TOSH            ; dummy offset
        call    comma
        call    here            ; get address where we compiled the branch offset
        sbiw    TOS, 1          ; (need to subtract 1)
        callc_epilogue

; FILL ( c-addr u c -- ) store c in u bytes starting at c-addr
opcode_fill:
        mov     TMPL, TOSL      ; get character
        ld      TOSL, DSP+      ; get count in a register we can use sbiw on
        ld      TOSH, DSP+
        tstw    TOS
        breq    .nofill
        ld      ZL, DSP+        ; get address
        ld      ZH, DSP+
; fill loop
1:      st      Z+, TMPL
        sbiw    TOS, 1
        brne    1b
; drop last arg and return
        popd
        rnext
.nofill:
; drop remainder of args and return
        drop2
        next

; (EXIT) ( R: a -- ) pop IP from return stack, also a special end-of-word marker
.global opcode_endword
opcode_endword:
        popr
; don't return into the temporary rom buffer!
; (this check is only for debugging)
        ldi     ZH, hi8(forth_rom_word_buf)
        cpi     IPL, lo8(forth_rom_word_buf)
        cpc     IPH, ZH
        brlo    1f
        ; save a few cycles by assuming the rom word buf doesn't span a 256-byte boundary
        cpi     IPL, lo8(forth_rom_word_buf+MAX_ROMDICT_ENTRY_SIZE+1)
        cpc     IPH, ZH
        brsh    1f
        throw   -29
1:      rnext

; PUTC ( c -- ) write character at current cursor location without interpreting
; escape sequences and newlines
opcode_putc:
        movw    TMP, TOS        ; get character into r22
        ldi     r24, lo8(fcon)
        ldi     r25, hi8(fcon)
        callc_prologue
        call    console_addch_raw
        callc_restore
        popd
        rnext


; >< ( hl -- lh ) swap bytes of TOS
opcode_byteswap:
        mov     TMPL, TOSH
        mov     TOSH, TOSL
        mov     TOSL, TMPL
        next

; LH> ( u -- cl ch ) split cell into bytes
opcode_splitbytes:
        mov     TMPL, TOSH
        clr     TOSH
        pushd
        mov     TOSL, TMPL
        next

; >LH ( cl ch -- u ) combine bytes into cell
opcode_mergebytes:
        mov     TOSH, TOSL      ; move low byte of TOS to high byte
        ld      TOSL, DSP+      ; nip low byte
        ld      TMPH, DSP+      ; discard
        next

; >H ( u1 c -- u2 ) replace high byte of u1 with c
opcode_tohighbyte:
        mov     TOSH, TOSL      ; low byte is new high byte
        ld      TOSL, DSP+      ; old high byte stays the same
        ld      TMPL, DSP+      ; discard
        next

; >L ( u1 c -- u2 ) replace low byte of u1 with c
opcode_tolowbyte:
        ld      TOSH, DSP+      ; old high byte stays the same
        ld      TMPL, DSP+      ; discard
        next

; HI ( u -- c ) logical AND with 0xFF00 and shift right 8 bits
opcode_highbyte:
        mov     TOSL, TOSH
        ; fall through

; LO ( u -- c ) logical AND with 0x00FF
opcode_lowbyte:
        clr     TOSH
        next

; LH+ ( u1 u2 -- u ) bytewise sum of u1 and u2 without interbyte carry
opcode_bytepairplus:
        nip
        add     TOSL, TMPL
        add     TOSH, TMPH
        next

; LH- ( u1 u2 -- u ) bytewise difference of u1 and u2 without interbyte carry
opcode_bytepairminus:
        nip
        sub     TMPL, TOSL
        sub     TMPH, TOSH
        movw    TOS, TMP
        next

; L+ ( u1 c -- u2 ) add c to low byte of u1 without affecting high byte
opcode_lplus:
        mov     TMPL, TOSL
        popd
        add     TOSL, TMPL
        next

; H+ ( u1 c -- u2 ) add c to high byte of u1 without affecting low byte
opcode_hplus:
        mov     TMPL, TOSL
        popd
        add     TOSH, TMPL
        next

; LNEGATE ( lh -- lh ) negate lower byte of TOS leaving upper byte alone
opcode_lnegate:
        neg     TOSL
        next

; HNEGATE ( lh -- lh ) negate upper byte of TOS leaving lower byte alone
opcode_hnegate:
        neg     TOSH
        next

; LH*/ ( lh1 lh2 -- lh3 )
opcode_lhscale:
        nip                     ; get lh1
        muls    TMPL, TOSL      ; product of lsbs
        mov     TOSL, r1        ; take msb of result
        muls    TMPH, TOSH      ; product of msbs
        mov     TOSH, r1
        clr     ZEROL
        clr     ZEROH
        next

; L+H ( lh -- n ) sum of low and high bytes of TOS
opcode_bytesum:
        add     TOSL, TOSH
        ldi     TOSH, 0
        adc     TOSH, ZERO
        next

; L-H ( lh -- n ) low byte of TOS minus high byte of TOS
opcode_bytediff:
        sub     TOSL, TOSH
        ldi     TOSH, 0
        sbc     TOSH, ZERO
        next

; 256* ( u1 -- u2 ) multiply u1 by 256 (left shift 8 bits)
; i.e. move low byte of TOS into high byte and clear low byte
opcode_twofivesixstar:
        mov     TOSH, TOSL
        clr     TOSL
        next

; RANDOM ( -- u ) push a pseudorandom cell-sized integer
opcode_random:
; random number generator from Starting Forth
; seed = (seed * 31421) + 6927
        pushd
        lds     r20, seed
        lds     r21, seed+1
        ldi     TMPL, lo8(31421)
        ldi     TMPH, hi8(31421)
        mul     r20, TMPL
        movw    TOS, r0
        mul     r20, TMPH
        add     TOSH, r0
        mul     r21, TMPL
        add     TOSH, r0
        subi    TOSL, lo8(-6927)
        sbci    TOSH, hi8(-6927)
        clr     ZEROL
        clr     ZEROH
        sts     seed, TOSL
        sts     seed+1, TOSH
        next
; ; 16-bit xorshift random number generator:
; ; http://www.retroprogramming.com/2017/07/xorshift-pseudorandom-numbers-in-z80.html
;         pushd
;         lds     TOSL, seed
;         lds     TOSH, seed+1
; ; xs ^= xs << 7
;         movw    TMPL, TOSL
;         lsr     TMPH
;         mov     TMPH, TMPL
;         eor     TMPL, TMPL
;         ror     TMPH
;         ror     TMPL
;         eor     TMPL, TOSL
;         eor     TMPH, TOSH
; ; xs ^= xs >> 9
;         movw    TOSL, TMPL
;         mov     TOSL, TOSH
;         eor     TOSH, TOSH
;         lsr     TOSL
;         eor     TMPL, TOSL
;         eor     TMPH, TOSH
; ; xs ^= xs << 8
;         mov     TOSH, TMPL
;         eor     TOSL, TOSL
;         eor     TOSL, TMPL
;         eor     TOSH, TMPH
;         sts     seed, TOSL
;         sts     seed+1, TOSH
;         next

; XRES ( -- n ) screen width, in pixels (for bitmap modes) or cells (for
; text/tiled modes)
opcode_xres:
        invoke_drawop xres

; GLYPH! ( d1 d2 c -- ) set glyph c in current font to 8 bytes in d1 d2
; raises an exception if current font is in ROM
opcode_setglyph:
        lds     TOSH, tilemap_hh
        sbrs    TOSH, TILEMAP_IS_RAM_BIT
        rjmp    .setglyph_readonly
        mov     ZL, TOSL
        lds     ZH, tilemap_hi
        subi    ZH, -7          ; start with last row first
        .rept 4
        popd                    ; pop 8 bytes off stack and store into font
        st      Z, TOSH
        dec     ZH
        st      Z, TOSL
        dec     ZH
        .endr
        rnext
.setglyph_readonly:
        throw   FE_READ_ONLY

; CGS ( -- ) clear graphics screen to black and set current color to white
opcode_cgs:
        movw    TSAV, TOS       ; free up r25:r24 so we can use sbiw
        clr     TMPL
        lds     r0, bytesperline
        lds     r1, linesperscreen
        mul     r0, r1          ; get byte count
        movw    TOS, r0
        lds     ZL, screen_ptr
        lds     ZH, screen_ptr+1
1:      st      Z+, TMPL
        sbiw    TOS, 1
        brne    1b
        movw    ZERO, TOS       ; restore r1:r0 to zero
        movw    TOS, TSAV       ; restore TOS
        sts     globalcolor, TRUEL
        rnext


; CLEAR ( c -- ) fill graphics screen with color c (any bitmap or text mode)
opcode_clear:
        invoke_drawop clear

; COLOR ( -- c ) get current color (any bitmap or text mode)
opcode_getcolor:
        invoke_drawop getcolor

; COLOR! ( c -- ) set current color (any bitmap or text mode)
opcode_setcolor:
        invoke_drawop setcolor

; PLOT ( xy -- ) set pixel in graphics screen to current color (any bitmap mode)
opcode_plot:
        lds     r20, linesperscreen     ; check y coordinate
        cp      TOSH, r20
        brsh    .noplot
        invoke_drawop plot
.noplot:
        popd
        next

; PSET ( c xy -- ) set color of pixel in graphics screen (any bitmap mode)
; TSET ( c xy -- ) set character at location in text screen (text modes)
opcode_pset:
        lds     r20, linesperscreen     ; check y coordinate
        cp      TOSH, r20
        brsh    .nopset
        invoke_drawop pset
.nopset:
        drop2
        next

; VLIN ( xy h -- ) draw vertical line at xy extending down h pixels (any bitmap mode)
; ("VLIN" is the command used by Applesoft Basic)
opcode_vlin:
        nip                             ; get xy into TMPL/TMPH
; check y origin
        lds     r20, linesperscreen     ; check y coordinate
        cp      TMPH, r20
        brsh    .novlin
; height must be nonzero
        mov     r21, TOSL
        tst     r21
        breq    .novlin
; clip height to screen bounds
        add     r21, TMPH               ; compute ending y coordinate
        brcs    .clipvlin               ; check for wraparound
        cp      r20, r21                ; check if endpoint > screen height
        brlo    .clipvlin
        invoke_drawop vlin
.clipvlin:
        sub     r20, TMPH               ; set line height to (screen height - y origin)
        mov     TOSL, r20
        invoke_drawop vlin
.novlin:
.nohlin:
        popd
        next

; HLIN ( xy w -- ) draw horizontal line at xy extending right w pixels (any bitmap mode)
; ("HLIN" is the command used by Applesoft Basic)
opcode_hlin:
        nip                             ; get xy into TMPL/XMPH
; check y origin
        lds     r20, linesperscreen     ; check y coordinate
        cp      TMPH, r20
        brsh    .nohlin
        invoke_drawop hlin

; RECT ( xy wh -- ) draw filled rectangle its upper left corner positioned at xy
; and extending w pixels to the right and h pixels down.
opcode_rect:
        nip                             ; get xy into TMPL/XMPH
; check y origin
        lds     r20, linesperscreen     ; check y coordinate
        cp      TMPH, r20
        brsh    .norect
; height must be nonzero
        tst     TOSH
        breq    .norect
; clip height to screen bounds
        mov     r21, TOSH
        add     r21, TMPH               ; compute ending y coordinate
        brcs    .cliprectv              ; check for wraparound
        cp      r20, r21                ; check if bottom edge > screen height
        brlo    .cliprectv
        invoke_drawop rect
.cliprectv:
        sub     r20, TMPH               ; set rect height to (screen height - y origin)
        mov     TOSH, r20
        invoke_drawop rect
.norect:
        popd
        next

;!!! TODO: clipping
opcode_line:
        invoke_drawop line

; COS ( c -- fx ) cosine of angle c (in binary radians) as a signed 1.15
; fixed-point value
opcode_cos:
        ldi     ZH, hi8(cosinetable)
        mov     ZL, TOSL
        lpm     TOSL, Z         ; get lsb
        inc     ZH
        lpm     TOSH, Z         ; get msb
        rnext

; SIN ( c -- n ) sine of angle c (in binary radians) as a signed 1.15
; fixed-point value
opcode_sin:
        ldi     ZH, hi8(cosinetable)
        ldi     ZL, 0x40
        sub     ZL, TOSL
        lpm     TOSL, Z         ; get lsb
        inc     ZH
        lpm     TOSH, Z         ; get msb
        rnext

; CCOS ( c -- c ) cosine of angle c (in binary radians) as a signed 1.7
; fixed-point value
opcode_ccos:
        ldi     ZH, hi8(cosinetable_msb)
        mov     ZL, TOSL
        lpm     TOSL, Z
        clr     TOSH            ; sign-extend
        sbrc    TOSL, 7
        ser     TOSH
        rnext

; CSIN ( c -- c ) sine of angle c (in binary radians) as a signed 1.7
; fixed-point value
opcode_csin:
        ldi     ZH, hi8(cosinetable_msb)
        ldi     ZL, 0x40
        sub     ZL, TOSL
        lpm     TOSL, Z
        clr     TOSH            ; sign-extend
        sbrc    TOSL, 7
        ser     TOSH
        rnext

; LH-COS/SIN ( lh1 -- lh2 )
opcode_lhcossin:
        ldi     ZH, hi8(cosinetable_msb)
        movw    TMP, TOS
        mov     ZL, TMPL
        lpm     TOSL, Z
        ldi     ZL, 0x40
        sub     ZL, TMPH
        lpm     TOSH, Z
        rnext

; HSV ( c1 -- c2 ) convert HSV color value 0bVVSSHHHH (2 bit value, 2 bit
; saturation, 4 bit hue) to nearest color index (high-color bitmap modes)
opcode_hsv:
        ldi     ZH, hi8(hsvtable)
        mov     ZL, TOSL
        lpm     TOSL, Z
        clr     TOSH
        rnext

; VSYNC ( -- ) wait until the start of the next frame of video
; this will hang the system if interrupts are disabled
opcode_vsync:
1:      sbis    new_frame, new_frame_bit
        rjmp    1b
        cbi     new_frame, new_frame_bit
        next

opcode_.unimplemented:
opcode_invalid:
        throw   FE_UNSUPPORTED_OPERATION
