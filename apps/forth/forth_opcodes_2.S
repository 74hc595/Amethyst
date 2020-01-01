#include "forth_opcodes.h"
#include "forth_macros.inc"
#include "forth_defs.h"
#include "spi.h"

; ?DUP ( n -- 0 | n n ) duplicate TOS if non-zero
ext_opcode_qdup:
        tstw    TOS
        breq    1f
        pushd
1:      rnext

; -ROT ( n1 n2 n3 -- n3 n1 n2 ) push top element down to 3rd position
ext_opcode_nrot:
        movw    TMP, TOS
        ld      TOSL, DSP
        ldd     TOSH, DSP+1
        ldd     IR, DSP+2
        st      DSP, IR
        ldd     IR, DSP+3
        std     DSP+1, IR
        std     DSP+2, TMPL
        std     DSP+3, TMPH
        rnext

; TUCK ( n1 n2 -- n2 n1 n2 ) copy top item below second item
ext_opcode_tuck:
        movw    TMP, TOS        ; save n2
        ld      r20, DSP        ; save n1
        ldd     r21, DSP+1
        pushd                   ; duplicate n2 ( -- n1 n2 n2 )
        st      DSP, r20        ; store n1 where n2 was
        std     DSP+1, r21
        std     DSP+2, TMPL     ; store n2 where n1 was
        std     DSP+3, TMPH
        rnext

; PICK ( xu...x1 x0 u -- xu...x1 x0 xu ) replace u with xu
ext_opcode_pick:
        lsl     TOSL            ; multiply TOS by cell size
        rol     TOSH
        movw    Z, DSP
        add     ZL, TOSL        ; add cell offset to stack pointer
        adc     ZH, TOSH
        ld      TOSL, Z         ; get value from stack
        ldd     TOSH, Z+1
        rnext

; ROLL ( xu xu-1 ... x0 u -- xu-1 ... x0 xu ) rotate u+1 items on top of stack
ext_opcode_roll:
        tstw    TOS             ; skip if TOS=0
        breq    2f
        movw    Z, DSP          ; get stack pointer
        ld      TMPL, Z+        ; get the "replacer" value
        ld      TMPH, Z+
1:      ld      r20, Z          ; get the value to be replaced
        ldd     r21, Z+1
        st      Z+, TMPL
        st      Z+, TMPH
        movw    TMP, r20        ; replacee now becomes the replacer
        sbiw    TOS, 1
        brne    1b
        st      DSP, r20
        std     DSP+1, r21
2:      popd                    ; drop count
        rnext

; 2SWAP ( x1 x2 x3 x4 -- x3 x4 x1 x2 )
; x4 - TOS (r25:24) -> DSP+3:DSP+2
; x3 - DSP+1:DSP    -> DSP+5:DSP+4
; x2 - DSP+3:DSP+2  -> TOS (r25:r24)
; x1 - DSP+5:DSP+4  -> DSP+1:DSP
ext_opcode_twoswap:
        ld      TMPL, DSP       ; get x3
        ldd     TMPH, DSP+1
        ldd     r20, DSP+2      ; get x2
        ldd     r21, DSP+3
        ldd     r18, DSP+4      ; get x1
        ldd     r19, DSP+5
        std     DSP+2, TOSL     ; store x4
        std     DSP+3, TOSH
        std     DSP+4, TMPL     ; store x3
        std     DSP+5, TMPH
        movw    TOS, r20        ; store x2
        st      DSP, r18        ; store x1
        std     DSP+1, r19
        rnext

; 2OVER ( x1 x2 x3 x4 -- x1 x2 x3 x4 x1 x2 )
ext_opcode_twoover:
        pushd
        ldd     TOSL, DSP+6     ; push x1
        ldd     TOSH, DSP+7
        pushd
        ldd     TOSL, DSP+6     ; push x2
        ldd     TOSH, DSP+7
        rnext

; 2ROT ( x1 x2 x3 x4 x5 x6 -- x3 x4 x5 x6 x1 x2 )
; x6 - TOS (r25:r24) -> DSP+3:DSP+2
; x5 - DSP+1:DSP     -> DSP+5:DSP+4
; x4 - DSP+3:DSP+2   -> DSP+9:DSP+8
; x3 - DSP+5:DSP+4   -> DSP+7:DSP+6
; x2 - DSP+7:DSP+6   -> TOS (r25:r24)
; x1 - DSP+9:DSP+8   -> DSP+1:DSP
ext_opcode_tworot:
        ldd     TMPL, DSP+8     ; get x1
        ldd     TMPH, DSP+9
        ldd     r20, DSP+6      ; get x2
        ldd     r21, DSP+7
        ldd     r18, DSP+4      ; get x3
        ldd     r19, DSP+5
        std     DSP+8, r18      ; store x3
        std     DSP+9, r19
        ldd     r18, DSP+2      ; get x4
        ldd     r19, DSP+3
        std     DSP+6, r18      ; store x4
        std     DSP+7, r19
        ld      r18, DSP        ; get x5
        ldd     r19, DSP+1
        std     DSP+4, r18      ; store x5
        std     DSP+5, r19
        std     DSP+2, TOSL     ; store x6
        std     DSP+3, TOSH
        st      DSP, TMPL       ; store x1
        std     DSP+1, TMPH
        movw    TOS, r20        ; store x2
        rnext

; 2SELECT ( true-d false-d flag -- d ) IF 2DROP ELSE 2NIP THEN
; pop flag from stack, if zero, discard second stack item leaving false-n,
; if nonzero, drop top of stack, leaving true-n
; like the ternary operator in other languages, but does not short-circuit
; n1 n2 n3 n4 -- n1 n2 .. .. 2DROP
; n1 n2 n3 n4 | -- .. .. n3 n4 2NIP
ext_opcode_twoselect:
        tstw    TOS
        popd
        brne    1f
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        adiw    DSP, 4
        st      DSP, TMPL
        std     DSP+1, TMPH
        rnext
1:      drop2
        rnext

; C>S ( c -- n ) sign-extend byte in TOS to word width
ext_opcode_signextend:
        clr     TOSH
        sbrc    TOSL, 7
        ser     TOSH
        rnext

; MIN ( n1 n2 -- n ) keep smaller of top two items on stack
ext_opcode_min:
        nip
        cp      TOSL, TMPL      ; is TOS < TMP?
        cpc     TOSH, TMPH
        brlt    1f              ; if so, TOS is smallest, keep it
        movw    TOS, TMP        ; otherwise, move TMP to TOS
1:      rnext

; MAX ( n1 n2 -- n ) keep larger of top two items on stack
ext_opcode_max:
        nip
        cp      TMPL, TOSL      ; is TMP < TOS?
        cpc     TMPH, TOSH
        brlt    1f              ; if so, TOS is largest, keep it
        movw    TOS, TMP        ; otherwise, move TMP to TOS
1:      rnext

; UMIN ( u1 u2 -- u ) keep smaller of top two items on stack, unsigned
ext_opcode_umin:
        nip
        cp      TOSL, TMPL      ; is TOS < TMP?
        cpc     TOSH, TMPH
        brlo    1f              ; if so, TOS is smallest, keep it
        movw    TOS, TMP        ; otherwise, move TMP to TOS
1:      rnext

; MAX ( u1 u2 -- u ) keep larger of top two items on stack, unsigned
ext_opcode_umax:
        nip
        cp      TMPL, TOSL      ; is TMP < TOS?
        cpc     TMPH, TOSH
        brlo    1f              ; if so, TOS is largest, keep it
        movw    TOS, TMP        ; otherwise, move TMP to TOS
1:      rnext

; WITHIN ( n1|u1 n2|u2 n3|u3 -- flag ) is n2 <= n1 < n3?
; the reference implementation (that works for signed and unsigned values) is:
;  : WITHIN ( test low high -- flag ) OVER - >R - R> U< ;
; in C syntax, this is:
;  within(test,low,high) { return (uint16_t)(test-low) < (uint16_t)(high-low); }
ext_opcode_within:
        ; upper limit is already in r25:r24
        nip                     ; lower limit in r23:r22
        ld      r20, DSP+       ; test value in r21:r20
        ld      r21, DSP+
        sub     r20, TMPL       ; compute (test-low)
        sbc     r21, TMPH
        sub     TOSL, TMPL      ; compute (high-low)
        sbc     TOSH, TMPH
        cp      r20, TOSL
        cpc     r21, TOSH
        brlo    1f
        movw    TOS, ZERO
        rnext
1:      movw    TOS, TRUE
        rnext

; LSHIFT ( n1 u -- n2 ) logical shift left by u bits
ext_opcode_lshift:
        nip
        rjmp    2f
1:      lsl     TMPL
        rol     TMPH
2:      dec     TOSL
        brpl    1b
        movw    TOS, TMP
        rnext

; RSHIFT ( u1 u -- u2 ) logical shift right by u bits
ext_opcode_rshift:
        nip
        rjmp    2f
1:      lsr     TMPH
        ror     TMPL
2:      dec     TOSL
        brpl    1b
        movw    TOS, TMP
        rnext

; ARSHIFT ( n1 u -- n2 ) arithetic shift right by u bits
ext_opcode_arshift:
        nip
        rjmp    2f
1:      asr     TMPH
        ror     TMPL
2:      dec     TOSL
        brpl    1b
        movw    TOS, TMP
        rnext

; C+! ( c c-addr -- ) add c to byte at address
ext_opcode_caddstore:
        movw    Z, TOS
        ld      TMPL, Z
        popd
        add     TMPL, TOSL
        st      Z, TMPL
        popd
        rnext

; CBIT@ ( c c-addr -- c ) test bits in byte at address
ext_opcode_cbittest:
        movw    Z, TOS
        ld      TMPL, Z
        popd
        and     TOSL, TMPL
        clr     TOSH
        rnext

; CBIC! ( c c-addr -- ) clear bits in byte at address
ext_opcode_cbitclear:
        movw    Z, TOS
        ld      TMPL, Z
        popd
        com     TOSL
        and     TMPL, TOSL
        st      Z, TMPL
        popd
        rnext

; CBIS! ( c c-addr -- ) set bits in byte at address
ext_opcode_cbitset:
        movw    Z, TOS
        ld      TMPL, Z
        popd
        or      TMPL, TOSL
        st      Z, TMPL
        popd
        rnext

; CXOR! ( c c-addr -- ) flip bits in byte at address (xor)
ext_opcode_cbitflip:
        movw    Z, TOS
        ld      TMPL, Z
        popd
        eor     TMPL, TOSL
        st      Z, TMPL
        popd
        rnext


; S>D ( n -- d ) sign-extend single cell value to double cell
ext_opcode_stod:
        pushd   ; duplicate TOS
        lsl     TOSH            ; get sign bit
        movw    TOS, ZERO       ; clear TOS
        sbc     TOSL, ZEROL     ; invert TOS if sign bit was set
        sbc     TOSH, ZEROH
        rnext

; D0= ( xd -- flag ) check if double-cell is equal to zero
; r25:r24:r23:r22 = xd
ext_opcode_deq0:
        nip
        or      TOSH, TOSL
        or      TOSH, TMPH
        or      TOSH, TMPL
        breq    .dflag_true
        movw    TOS, ZERO
        rnext

; D0<> ( xd -- flag ) check if double-cell is not equal to zero
; r25:r24:r23:r22 = xd
ext_opcode_dne0:
        nip
        or      TOSH, TOSL
        or      TOSH, TMPH
        or      TOSH, TMPL
        brne    .dflag_true
        movw    TOS, ZERO
        rnext

; D= ( xd1 xd2 -- flag ) compare two double-cells for equality
; r25:r24:r23:r22 = xd1
; r21:r20:r19:r18 = xd2
ext_opcode_deq:
        ld      TMPL, DSP+      ; byte 0 (LSB) of xd2
        ld      TMPH, DSP+      ; byte 1 of xd2
        ld      r20, DSP+       ; byte 2 of xd1
        ld      r21, DSP+       ; byte 3 (MSB) of xd1
        ld      r18, DSP+       ; byte 0 (LSB) of xd1
        ld      r19, DSP+       ; byte 1 of xd1
        cp      TMPL, r18
        cpc     TMPH, r19
        cpc     TOSL, r20
        cpc     TOSH, r21
        breq    .dflag_true
        movw    TOS, ZERO
        rnext

; D<> ( xd1 xd2 -- flag ) compare two double-cells for inequality
; r25:r24:r23:r22 = xd1
; r21:r20:r19:r18 = xd2
ext_opcode_dne:
        ld      TMPL, DSP+      ; byte 0 (LSB) of xd2
        ld      TMPH, DSP+      ; byte 1 of xd2
        ld      r20, DSP+       ; byte 2 of xd1
        ld      r21, DSP+       ; byte 3 (MSB) of xd1
        ld      r18, DSP+       ; byte 0 (LSB) of xd1
        ld      r19, DSP+       ; byte 1 of xd1
        cp      TMPL, r18
        cpc     TMPH, r19
        cpc     TOSL, r20
        cpc     TOSH, r21
        brne    .dflag_true
        movw    TOS, ZERO
        rnext

; D0< ( xd -- flag ) is xd less than zero?
; r25:r24:r23:r22 = xd
ext_opcode_dlt0:
        nip
        tst     TOSH
        brmi    .dflag_true
        movw    TOS, ZERO
        rnext

; D0>= ( xd -- flag ) is xd greater than or equal to zero?
; r25:r24:r23:r22 = xd
ext_opcode_dge0:
        nip
        tst     TOSH
        brpl    .dflag_true
        movw    TOS, ZERO
        rnext
.dflag_true:
        movw    TOS, TRUE
        rnext

; D0> ( xd -- flag ) is xd greater than zero? (signed comparison)
; r25:r24:r23:r22 = xd
ext_opcode_dgt0:
        nip
        cp      ZERO, TMPL
        cpc     ZERO, TMPH
        cpc     ZERO, TOSL
        cpc     ZERO, TOSH
        brlt    .dflag_true
        movw    TOS, ZERO
        rnext

; D0<= ( xd -- flag ) is xd less than or equal to zero? (signed comparison)
; r25:r24:r23:r22 = xd
ext_opcode_dle0:
        nip
        cp      ZERO, TMPL
        cpc     ZERO, TMPH
        cpc     ZERO, TOSL
        cpc     ZERO, TOSH
        brge    .dflag_true
        movw    TOS, ZERO
        rnext

; D< ( xd1 xd2 -- flag ) is xd1 less than xd2? (signed)
ext_opcode_dlt:
        ld      TMPL, DSP+      ; byte 0 (LSB) of xd2
        ld      TMPH, DSP+      ; byte 1 of xd2
        ld      r20, DSP+       ; byte 2 of xd1
        ld      r21, DSP+       ; byte 3 (MSB) of xd1
        ld      r18, DSP+       ; byte 0 (LSB) of xd1
        ld      r19, DSP+       ; byte 1 of xd1
        cp      r18, TMPL
        cpc     r19, TMPH
        cpc     r20, TOSL
        cpc     r21, TOSH
        brlt    .dflag_true
        movw    TOS, ZERO
        rnext

; D>= ( xd1 xd2 -- flag ) is xd1 greater than or equal to than xd2? (signed)
ext_opcode_dge:
        ld      TMPL, DSP+      ; byte 0 (LSB) of xd2
        ld      TMPH, DSP+      ; byte 1 of xd2
        ld      r20, DSP+       ; byte 2 of xd1
        ld      r21, DSP+       ; byte 3 (MSB) of xd1
        ld      r18, DSP+       ; byte 0 (LSB) of xd1
        ld      r19, DSP+       ; byte 1 of xd1
        cp      r18, TMPL
        cpc     r19, TMPH
        cpc     r20, TOSL
        cpc     r21, TOSH
        brge    .dflag_true
        movw    TOS, ZERO
        rnext

; D> ( xd1 xd2 -- flag ) is xd1 greater than xd2? (signed)
ext_opcode_dgt:
        ld      TMPL, DSP+      ; byte 0 (LSB) of xd2
        ld      TMPH, DSP+      ; byte 1 of xd2
        ld      r20, DSP+       ; byte 2 of xd1
        ld      r21, DSP+       ; byte 3 (MSB) of xd1
        ld      r18, DSP+       ; byte 0 (LSB) of xd1
        ld      r19, DSP+       ; byte 1 of xd1
        cp      TMPL, r18
        cpc     TMPH, r19
        cpc     TOSL, r20
        cpc     TOSH, r21
        brlt    .dflag_true2
        movw    TOS, ZERO
        rnext

; D<= ( xd1 xd2 -- flag ) is xd1 less than or equal to xd2? (signed)
ext_opcode_dle:
        ld      TMPL, DSP+      ; byte 0 (LSB) of xd2
        ld      TMPH, DSP+      ; byte 1 of xd2
        ld      r20, DSP+       ; byte 2 of xd1
        ld      r21, DSP+       ; byte 3 (MSB) of xd1
        ld      r18, DSP+       ; byte 0 (LSB) of xd1
        ld      r19, DSP+       ; byte 1 of xd1
        cp      TMPL, r18
        cpc     TMPH, r19
        cpc     TOSL, r20
        cpc     TOSH, r21
        brge    .dflag_true2
        movw    TOS, ZERO
        rnext

; DU< ( xd1 xd2 -- flag ) is xd1 less than xd2? (unsigned)
ext_opcode_dult:
        ld      TMPL, DSP+      ; byte 0 (LSB) of xd2
        ld      TMPH, DSP+      ; byte 1 of xd2
        ld      r20, DSP+       ; byte 2 of xd1
        ld      r21, DSP+       ; byte 3 (MSB) of xd1
        ld      r18, DSP+       ; byte 0 (LSB) of xd1
        ld      r19, DSP+       ; byte 1 of xd1
        cp      r18, TMPL
        cpc     r19, TMPH
        cpc     r20, TOSL
        cpc     r21, TOSH
        brlo    .dflag_true2
        movw    TOS, ZERO
        rnext
.dflag_true2:
        movw    TOS, TRUE
        rnext

; DU>= ( xd1 xd2 -- flag ) is xd1 greater than or equal to xd2? (unsigned)
ext_opcode_duge:
        ld      TMPL, DSP+      ; byte 0 (LSB) of xd2
        ld      TMPH, DSP+      ; byte 1 of xd2
        ld      r20, DSP+       ; byte 2 of xd1
        ld      r21, DSP+       ; byte 3 (MSB) of xd1
        ld      r18, DSP+       ; byte 0 (LSB) of xd1
        ld      r19, DSP+       ; byte 1 of xd1
        cp      r18, TMPL
        cpc     r19, TMPH
        cpc     r20, TOSL
        cpc     r21, TOSH
        brsh    .dflag_true2
        movw    TOS, ZERO
        rnext

; DU> ( xd1 xd2 -- flag ) is xd1 greater than xd2? (unsigned)
ext_opcode_dugt:
        ld      TMPL, DSP+      ; byte 0 (LSB) of xd2
        ld      TMPH, DSP+      ; byte 1 of xd2
        ld      r20, DSP+       ; byte 2 of xd1
        ld      r21, DSP+       ; byte 3 (MSB) of xd1
        ld      r18, DSP+       ; byte 0 (LSB) of xd1
        ld      r19, DSP+       ; byte 1 of xd1
        cp      TMPL, r18
        cpc     TMPH, r19
        cpc     TOSL, r20
        cpc     TOSH, r21
        brlo    .dflag_true2
        movw    TOS, ZERO
        rnext

; DU<= ( xd1 xd2 -- flag ) is xd1 less than or equal to xd2? (unsigned)
ext_opcode_dule:
        ld      TMPL, DSP+      ; byte 0 (LSB) of xd2
        ld      TMPH, DSP+      ; byte 1 of xd2
        ld      r20, DSP+       ; byte 2 of xd1
        ld      r21, DSP+       ; byte 3 (MSB) of xd1
        ld      r18, DSP+       ; byte 0 (LSB) of xd1
        ld      r19, DSP+       ; byte 1 of xd1
        cp      TMPL, r18
        cpc     TMPH, r19
        cpc     TOSL, r20
        cpc     TOSH, r21
        brsh    .dflag_true2
        movw    TOS, ZERO
        rnext

; D2* ( xd1 -- xd2 ) double-cell logical shift left
; TOSH is byte 3 (MSB)
; TOSL is byte 2 (low byte of high word)
; [DSP+1] is byte 1 (high byte of low word)
; [DSP] is byte 0 (LSB)
ext_opcode_dtwostar:
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        lsl     TMPL
        rol     TMPH
        rol     TOSL
        rol     TOSH
        st      DSP, TMPL
        std     DSP+1, TMPH
        rnext

; D2/ ( xd1 -- xd2 ) double-cell arithmetic shift right
ext_opcode_dtwoslash:
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        asr     TOSH
        ror     TOSL
        ror     TMPH
        ror     TMPL
        st      DSP, TMPL
        std     DSP+1, TMPH
        rnext

; DU2/ ( xd1 -- xd2 ) double-cell logical shift right
ext_opcode_dutwoslash:
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        lsr     TOSH
        ror     TOSL
        ror     TMPH
        ror     TMPL
        st      DSP, TMPL
        std     DSP+1, TMPH
        rnext

; DABS ( xd1 -- ud1 ) absolute value of double-cell
ext_opcode_dabs:
        tst     TOSH
        brpl    1f      ; don't negate if positive

; DNEGATE ( xd1 -- xd2 ) twos complement of double-cell
ext_opcode_dnegate:
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        com     TOSH
        com     TOSL
        com     TMPH
        neg     TMPL
        sbci    TMPH, -1
        sbci    TOSL, -1
        sbci    TOSH, -1
        st      DSP, TMPL
        std     DSP+1, TMPH
1:      rnext

; DMIN ( d1 d2 -- d3 ) keep smaller of d1 and d2
ext_opcode_dmin:
        movw    r20, TOS
        ld      r18, DSP+
        ld      r19, DSP+
        ld      TOSL, DSP+
        ld      TOSH, DSP+
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        cp      r18, TMPL
        cpc     r19, TMPH
        cpc     r20, TOSL
        cpc     r21, TOSH
        brlt    1f
        rnext
1:      movw    TOS, r20
        st      DSP, r18
        std     DSP+1, r19
        rnext

; DMAX ( d1 d2 -- d3 ) keep larger of d1 and d2
ext_opcode_dmax:
        movw    r20, TOS
        ld      r18, DSP+
        ld      r19, DSP+
        ld      TOSL, DSP+
        ld      TOSH, DSP+
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        cp      TMPL, r18
        cpc     TMPH, r19
        cpc     TOSL, r20
        cpc     TOSH, r21
        brlt    1f
        rnext
1:      movw    TOS, r20
        st      DSP, r18
        std     DSP+1, r19
        rnext

; M+ ( d1 n1 -- d2 ) add single cell to double cell
ext_opcode_mplus:
        movw    r18, TOS        ; get the single-cell value
        movw    r20, TOS        ; and sign-extend it the way gcc does:
        lsl     r20             ; - get msb into carry flag
        sbc     r20, r20        ; - if carry clear, r20 and r21 are zeroed
        sbc     r21, r21        ; - if carry set, r20 and r21 will contain 0xFFFF
        popd
        ld      TMPL, DSP       ; now the double-cell value is in r25-r22
        ldd     TMPH, DSP+1
        add     TMPL, r18       ; add single-cell to low word
        adc     TMPH, r19
        adc     TOSL, r20
        adc     TOSH, r21
        st      DSP, TMPL
        std     DSP+1, TMPH
        rnext

; D+ ( xd1 xd2 -- xd3 ) add xd1 and xd2 giving xd3
; r25:r24:r23:r22 = xd2 and xd3
; r21:r20:r19:r18 = xd1
ext_opcode_dplus:
        ld      TMPL, DSP+      ; byte 0 (LSB) of xd2
        ld      TMPH, DSP+      ; byte 1 of xd2
        ld      r20, DSP+       ; byte 2 of xd1
        ld      r21, DSP+       ; byte 3 (MSB) of xd1
        ld      r18, DSP        ; byte 0 (LSB) of xd1
        ldd     r19, DSP+1      ; byte 1 of xd1
        add     TMPL, r18
        adc     TMPH, r19
        adc     TOSL, r20       ; upper word of sum is already in TOS
        adc     TOSH, r21
        st      DSP, TMPL       ; store lower word of sum back to stack
        std     DSP+1, TMPH
        rnext

; D- ( xd1 xd2 -- xd3 ) subtract xd1 and xd2 giving xd3
; r25:r24:r23:r22 = xd1 and xd3
; r21:r20:r19:r18 = xd2
ext_opcode_dminus:
        movw    r20, TOS        ; bytes 2 and 3 (MSB) of xd2
        ld      r18, DSP+       ; byte 0 (LSB) of xd2
        ld      r19, DSP+       ; byte 1 of xd2
        ld      TOSL, DSP+      ; byte 2 of xd1
        ld      TOSH, DSP+      ; byte 3 (MSB) of xd1
        ld      TMPL, DSP       ; byte 0 (LSB) of xd1
        ldd     TMPH, DSP+1     ; byte 1 of xd1
        sub     TMPL, r18
        sbc     TMPH, r19
        sbc     TOSL, r20       ; upper word of difference is already in TOS
        sbc     TOSH, r21
        st      DSP, TMPL       ; store lower word of difference back to stack
        std     DSP+1, TMPH
        rnext


; * ( n1 n2 -- n ) 16x16=16 multiplication
ext_opcode_star:
        nip
        ; same code emitted by gcc
        movw    r20, TOS
        mul     r20, TMPL
        movw    TOS, r0
        mul     r20, TMPH
        add     TOSH, r0
        mul     r21, TMPL
        add     TOSH, r0
        clr     ZEROL
        clr     ZEROH
        rnext

; */ ( n1 n2 n3 -- n4 ) compute (n1*n2) producing a double cell immediate,
; then divide by n3. (signed)
ext_opcode_starslash:
        tstw    TOS             ; don't divide by zero
        breq    .div0
        callc_prologue
        ld      TMPL, DSP+      ; LSB of n2
        ld      TMPH, DSP+      ; MSB of n2
        ld      r20, DSP+       ; LSB of n1
        ld      r21, DSP+       ; MSB of n1
        call    starslash
        callc_epilogue

; */MOD ( n1 n2 n3 -- n4 n5 ) compute (n1*n2) producing a double-cell intermediate,
; then divide by n3, returning remainder as n4 and quotient as n5
ext_opcode_starslashmod:
        tstw    TOS             ; don't divide by zero
        breq    .div0
        callc_prologue
        ld      TMPL, DSP+      ; LSB of n2
        ld      TMPH, DSP+      ; MSB of n2
        ld      r20, DSP        ; LSB of n1
        ldd     r21, DSP+1      ; MSB of n1
        call    starslashmod
        st      DSP, TMPL
        std     DSP+1, TMPH
        callc_epilogue

; /MOD ( n1 n2 -- rem quot )
ext_opcode_slashmod:
        tstw    TOS     ; don't divide by zero
        breq    .div0
        movw    TMP, TOS        ; move divisor to TMP
        ld      TOSL, DSP       ; get dividend into TOS
        ldd     TOSH, DSP+1
        callc_prologue
        call    __divmodhi4     ; remainder in r24:r25, quotient in r22:r23
        st      DSP, TOSL       ; put remainder into second item on stack
        std     DSP+1, TOSH
        movw    TOS, TMP        ; put quotient into TOS
        callc_epilogue

; U/MOD ( u1 u2 -- rem quot )
ext_opcode_uslashmod:
        tstw    TOS     ; don't divide by zero
        breq    .div0
        movw    TMP, TOS        ; move divisor to TMP
        ld      TOSL, DSP       ; get dividend into TOS
        ldd     TOSH, DSP+1
        callc_prologue
        call    __udivmodhi4    ; remainder in r24:r25, quotient in r22:r23
        st      DSP, TOSL       ; put remainder into second item on stack
        std     DSP+1, TOSH
        movw    TOS, TMP        ; put quotient into TOS
        callc_epilogue
.div0:
        throw   FE_DIVIDE_BY_ZERO

; UM/MOD ( ud u1 -- u2 u3 ) divide double-cell ud by single-cell u1, giving
; single-cell remainder u2 and single-cell quotient u3. All values unsigned.
ext_opcode_umslashmod:
        tstw    TOS             ; don't divide by zero
        breq    .div0
        callc_prologue
        ld      TMPL, DSP+      ; byte 2 of ud
        ld      TMPH, DSP+      ; byte 3 (MSB) of ud
        ld      r20, DSP        ; byte 0 (LSB) of ud
        ldd     r21, DSP+1      ; byte 1 of ud
        call    umslashmod
        st      DSP, TMPL
        std     DSP+1, TMPH
        callc_epilogue

; SM/REM ( d1 n1 -- n2 n3 ) divide double-cell d1 by single-cell n1 using
; symmetric division (rounding towards zero), giving single-cell remainder n2
; and single-cell quotient n3. All values signed.
ext_opcode_smslashrem:
        tstw    TOS             ; don't divide by zero
        breq    .div0
        callc_prologue
        ld      TMPL, DSP+      ; byte 2 of d1
        ld      TMPH, DSP+      ; byte 3 (MSB) of d1
        ld      r20, DSP        ; byte 0 (LSB) of d1
        ldd     r21, DSP+1      ; byte 1 of d1
        call    smslashrem
        st      DSP, TMPL
        std     DSP+1, TMPH
        callc_epilogue

; FM/MOD ( d1 n1 -- n2 n3 ) divide double-cell d1 by single-cell n1 using
; floored division (rounding towards negative infinity), giving single-cell
; remainder n2 and single-cell quotient n3. All values signed.
ext_opcode_fmslashmod:
        tstw    TOS             ; don't divide by zero
        breq    .div0
        callc_prologue
        ld      TMPL, DSP+      ; byte 2 of d1
        ld      TMPH, DSP+      ; byte 3 (MSB) of d1
        ld      r20, DSP        ; byte 0 (LSB) of d1
        ldd     r21, DSP+1      ; byte 1 of d1
        call    fmslashmod
        st      DSP, TMPL
        std     DSP+1, TMPH
        callc_epilogue

; / ( n1 n2 -- n1/n2 ) 16/16=16 signed division
ext_opcode_slash:
        tstw    TOS     ; don't divide by zero
        breq    .div0
        movw    TMP, TOS        ; move divisor to TMP
        popd                    ; get dividend into TOS
        callc_prologue
        call    __divmodhi4
        movw    TOS, TMP        ; move quotient to TOS
        callc_epilogue

; MOD ( n1 n2 -- n1%n2 ) 16%16=16 signed remainder after division.
; Result has the sign of the dividend (n1).
ext_opcode_mod:
        tstw    TOS     ; don't divide by zero
        breq    .div0a
        movw    TMP, TOS        ; move divisor to TMP
        popd                    ; get dividend into TOS
        callc_prologue
        call    __divmodhi4     ; leaves remainder in TOS
        callc_epilogue

; U/ ( u1 u2 -- u1/u2 ) 16/16=16 unsigned division
ext_opcode_uslash:
        tstw    TOS     ; don't divide by zero
        breq    .div0a
        movw    TMP, TOS        ; move divisor to TMP
        popd                    ; get dividend into TOS
        callc_prologue
        call    __udivmodhi4
        movw    TOS, TMP        ; move quotient to TOS
        callc_epilogue
.div0a:
        throw   FE_DIVIDE_BY_ZERO

; UMOD ( u1 u2 -- u1%u2 ) 16%16=16 unsigned remainder after division.
ext_opcode_umod:
        tstw    TOS     ; don't divide by zero
        breq    .div0a
        movw    TMP, TOS        ; move divisor to TMP
        popd                    ; get dividend into TOS
        callc_prologue
        call    __udivmodhi4    ; leaves remainder in TOS
        callc_epilogue

; M*/ ( d1 n1 +n2 -- d2 ) double-single multiply/divide with triple-cell intermediate
ext_opcode_mstarslash:
        tstw    TOS             ; don't divide by zero
        breq    .div0a
        ; n2 is already in r25:r24
        ld      TMPL, DSP+      ; nip n1 into r23:r22
        ld      TMPH, DSP+
        ld      r20, DSP+       ; nip high word of d1 into r21:r20
        ld      r21, DSP+
        ld      r18, DSP        ; get low word of d1 into r19:r18
        ldd     r19, DSP+1
        callc_prologue
        call    mstarslash
        ; put lower word of result back on the stack
        st      DSP, TMPL
        std     DSP+1, TMPH
        callc_epilogue

; UM*/ ( ud1 u1 u2 -- ud2 ) double-single unsigned multiply/divide with triple-cell intermediate
ext_opcode_umstarslash:
        tstw    TOS             ; don't divide by zero
        breq    .div0a
        ; n2 is already in r25:r24
        ld      TMPL, DSP+      ; nip n1 into r23:r22
        ld      TMPH, DSP+
        ld      r20, DSP+       ; nip high word of d1 into r21:r20
        ld      r21, DSP+
        ld      r18, DSP        ; get low word of d1 into r19:r18
        ldd     r19, DSP+1
        callc_prologue
        call    umstarslash
        ; put lower word of result back on the stack
        st      DSP, TMPL
        std     DSP+1, TMPH
        callc_epilogue


; M* ( n1 n2 -- d ) multiply two single-cell numbers giving signed double-cell result
ext_opcode_mstar:
        ; __mulhisi3 is an exception to the stanard C calling convention:
        ; SI:22 = HI:26 * HI:18
        callc_prologue          ; need to save r27:r26
        ld      r18, DSP        ; LSB of n1
        ldd     r19, DSP+1      ; MSB of n1
        movw    r26, TOS
        call    __mulhisi3
        ; put lower word of product back on the stack
        st      DSP, TMPL
        std     DSP+1, TMPH
        callc_epilogue          ; restore r27:r26 and clear ZEROL

; UM* ( u1 u2 -- ud ) multiply two unsigned single-cell numbers giving unsigned double-cell result
ext_opcode_umstar:
        ; __umulhisi3 is an exception to the stanard C calling convention:
        ; SI:22 = HI:26 * HI:18
        callc_prologue          ; need to save r27:r26
        ld      r18, DSP        ; LSB of n1
        ldd     r19, DSP+1      ; MSB of n1
        movw    r26, TOS
        call    __umulhisi3
        ; put lower word of product back on the stack
        st      DSP, TMPL
        std     DSP+1, TMPH
        callc_epilogue          ; restore r27:r26 and clear ZEROL

.fwd_resolve:
        callc_0arg_prologue
        call    here
        movw    TMP, TOS        ; get value of HERE into TMP
        callc_0arg_restore
        sec
        sbc     TMPL, TOSL
        sbc     TMPH, TOSH
        brmi    .resolve_out_of_range   ; fail if displacement is negative
        cpse    TMPH, ZERO
        rjmp    .resolve_out_of_range   ; fail if displacement >= 256
        movw    Z, TOS
        st      Z, TMPL
        popd
        ret

; >RESOLVE ( addr -- ) resolve forward relative branch from addr to HERE
ext_opcode_resolve:
        rcall   .fwd_resolve
        rnext
.resolve_out_of_range:
        throw   FE_BRANCH_OUT_OF_RANGE

; <RESOLVE ( addr branch-offset-addr -- ) resolve backward relative branch
ext_opcode_bresolve:
        movw    TMP, TOS        ; get address of dummy branch offset
        movw    Z, TOS          ; and get another copy
        popd                    ; TOS is now address of branch destination
        sec
        sbc     TMPL, TOSL
        sbc     TMPH, TOSH
        brmi    .resolve_out_of_range   ; fail if displacement is negative
        cpse    TMPH, ZERO
        rjmp    .resolve_out_of_range   ; fail if displacement >= 256
        st      Z, TMPL
        popd
        rnext

; (RAKE) ( -- ) ( R: i*n 0 -- ) resolve LEAVEs in a DO loop
; it's called RAKE because it cleans up the LEAVEs
; i didn't make that up, thank Wonyong Koh (author of hForth) for it
ext_opcode_rake:
        pop     ZL
        pop     ZH
        tstw    Z
        breq    .rake_done      ; check for sentinel value (0)
        pushd
        movw    TOS, Z
        rcall   .fwd_resolve    ; otherwise, resolve branch at that address
        rjmp    ext_opcode_rake
.rake_done:
        rnext

; EXIT ( R: a -- ) pop IP from return stack
; does the same as (EXIT) but is used for returns in the middle of a word.
; the (EXIT) opcode is recognizes as an end-of-word marker by the disassembler
ext_opcode_exit:
        jmp     opcode_endword

; ; ( -- ) finalize current word definition and return to interpretation state
ext_opcode_semicolon:
; check stack pointers (the ancient Forth term for this is "compiler security")
        lds     ZL, forth_saved_rsp
        lds     ZH, forth_saved_rsp+1
        rsp_to_tmp
        cp      ZL, TMPL
        cpc     ZH, TMPH
        brne    2f
        lds     ZL, forth_saved_dsp
        lds     ZH, forth_saved_dsp+1
        cp      ZL, DSPL
        cpc     ZH, DSPH
        breq    1f
        throw   FE_CTRL_STRUCT_MISMATCH
1:      callc_0arg_prologue
        call    do_semicolon
        callc_0arg_epilogue
2:      throw   FE_RSTACK_IMBALANCE

; ]; ( xt -- ) finish current temporary definition, remove it from the dictionary,
; and execute the xt on the stack pushed by :NONAME/:[, and execute it immediately.
; TODO reduce code duplication here
ext_opcode_execsemicolon:
        lds     ZL, forth_saved_rsp
        lds     ZH, forth_saved_rsp+1
        rsp_to_tmp
        cp      ZL, TMPL
        cpc     ZH, TMPH
        brne    1b
        lds     ZL, forth_saved_dsp
        lds     ZH, forth_saved_dsp+1
        cp      ZL, DSPL
        cpc     ZH, DSPH
        breq    3f
        throw   FE_CTRL_STRUCT_MISMATCH
3:      callc_0arg_prologue
        call    do_semicolon
; forget latest word (but we should still have its xt on the stack from :NONAME)
        lds     TOSL, forth_latest
        lds     TOSH, forth_latest+1
        call    ramdict_forget_lfa
        callc_0arg_restore
; now execute the word immediately
        jmp     opcode_execute

; CH. ( c -- ) write low byte to output device as 2 hexadecimal digits, without
; a trailing space (to allow printing of larger words)
ext_opcode_chdot:
        callc_prologue
        call    mio_x8
        popd
        callc_epilogue

; U>HEX ( u -- d ) convert TOS to four ASCII characters (0-9A-F) returned on
; the stack as a double-cell.
; Equivalent to:                DUP C>HEX SWAP >< C>HEX
ext_opcode_utohex:
        mov     r20, TOSH       ; save high byte
; convert low byte
        mov     TOSH, TOSL      ; copy into high byte
        andi    TOSH, 0x0F      ; high byte will contain ASCII of low nibble
        swap    TOSL
        andi    TOSL, 0x0F      ; low byte will contain ASCII of high nibble
; convert low nibble to ASCII
        ldi     TMPL, '0'
        cpi     TOSH, 10
        brlo    1f
        ldi     TMPL, 'A'-10
1:      add     TOSH, TMPL
; convert high nibble to ASCII
        ldi     TMPL, '0'
        cpi     TOSL, 10
        brlo    2f
        ldi     TMPL, 'A'-10
2:      add     TOSL, TMPL
; push, then fall through and convert high byte
        pushd
        mov     TOSL, r20

; C>HEX ( c -- u ) convert low byte of TOS to two ASCII characters (0-9A-F)
; in display order (low byte of TOS contains ASCII of high nibble, high byte of
; TOS contains ASCII of low nibble)
; The equivalent of CH. is:     C>HEX DUP EMIT >< EMIT
ext_opcode_ctohex:
        mov     TOSH, TOSL      ; copy into high byte
        andi    TOSH, 0x0F      ; high byte will contain ASCII of low nibble
        swap    TOSL
        andi    TOSL, 0x0F      ; low byte will contain ASCII of high nibble
; convert low nibble to ASCII
        ldi     TMPL, '0'
        cpi     TOSH, 10
        brlo    1f
        ldi     TMPL, 'A'-10
1:      add     TOSH, TMPL
; convert high nibble to ASCII
        ldi     TMPL, '0'
        cpi     TOSL, 10
        brlo    2f
        ldi     TMPL, 'A'-10
2:      add     TOSL, TMPL
        rnext

; . ( n -- ) write single-cell number to output device in current base
ext_opcode_dot:
        callc_prologue
        movw    TMP, ZERO       ; no width limit
        call    forth_dot
        popd
        callc_epilogue

; U. ( u -- ) write unsigned single-cell number to output device in current base
ext_opcode_udot:
        callc_prologue
        movw    TMP, ZERO       ; no width limit
        call    forth_udot
        popd
        callc_epilogue

; D. ( d -- ) write double-cell number to output device in current base
ext_opcode_ddot:
        callc_prologue
        nip
        movw    r20, ZERO       ; no width limit
        call    forth_ddot
        popd
        callc_epilogue

; UD. ( d -- ) write unsigned double-cell number to output in current base
ext_opcode_uddot:
        callc_prologue
        nip
        movw    r20, ZERO       ; no width limit
        call    forth_uddot
        popd
        callc_epilogue

; .R ( n1 n2 -- ) write single-cell number to output device, right-aligned in
; a field n2 characters wide
ext_opcode_dotr:
        callc_prologue
        movw    TMP, TOS        ; move width into tmp
        popd                    ; move value to TOS
        call    forth_dot
        popd
        callc_epilogue

; U.R ( u n -- ) write single-cell unsigned number to output device,
; right-aligned in a field n characters wide
ext_opcode_udotr:
        callc_prologue
        movw    TMP, TOS        ; move width into tmp
        popd                    ; move value to TOS
        call    forth_udot
        popd
        callc_epilogue

; D.R ( d n -- ) write double-cell number to output device, right-aligned in a
; field n characters wide
ext_opcode_ddotr:
        callc_prologue
        movw    r20, TOS        ; pop width
        popd
        nip                     ; get lower cell into r23:r22
        call    forth_ddot
        popd
        callc_epilogue

; LH. ( lh -- ) write byte-pair to output device in current base
ext_opcode_lhdot:
        callc_prologue
        call    forth_lhdot
        popd
        callc_epilogue

; # ( ud1 -- ud2 ) extract digits to the pictured numeric output buffer
ext_opcode_nums:
        callc_prologue
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        call    forth_extract_digits
        st      DSP, TMPL
        std     DSP+1, TMPH
        callc_epilogue

; SIGN ( n -- ) add minus sign to pictured numeric output is TOS is negative
ext_opcode_sign:
        tstw    TOS
        popd
        brpl    1f
        callc_0arg_prologue
        ldi     TOSL, '-'
        call    forth_hold
        callc_0arg_epilogue
1:      rnext

; #> ( ud -- c-addr len ) finalize pictured numeric output string
; Drops the double-cell number on the stack, reverses the contents of the
; pictured numeric output buffer, and pushes its address/length
ext_opcode_numend:
        lds     ZL, forth_hld0          ; get p.n.o. buffer base address
        lds     ZH, forth_hld0+1        ; Z points to the beginning of the string
        st      DSP, ZL                 ; store string start on stack
        std     DSP+1, ZH
        rcall   .forth_finalize_numeric_output
        rnext

; void forth_finalize_numeric_output(void) - reverses the contents of the
; pictured numeric output buffer
.global forth_finalize_numeric_output
forth_finalize_numeric_output:
        lds     ZL, forth_hld0          ; get p.n.o. buffer base address
        lds     ZH, forth_hld0+1        ; Z points to the beginning of the string
.forth_finalize_numeric_output:
        movw    r20, X                  ; save X
        lds     TOSL, forth_hld         ; get length of string
        clr     TOSH
        movw    X, Z
        add     XL, TOSL
        adc     XH, TOSH                ; X points to the end of the string
1:      ld      TMPL, Z
        ld      TMPH, -X
        st      X, TMPL
        st      Z+, TMPH
        cp      ZL, XL
        cpc     ZH, XH
        brlo    1b
        movw    X, r20                  ; restore X
        ret

; HOLDS ( c-addr len -- ) add string to pictured numeric output string
ext_opcode_holds:
        callc_prologue
        nip                     ; nip address into r23:r22
        call    forth_holds
        popd                    ; drop remaining item on stack
        callc_epilogue

; >MARKER ( -- latest cp np np0 ) push dictionary snapshot
; to get the size of name space:
;   >MARKER - -ROT 2DROP .
ext_opcode_tomarker:
        pushd
        lds     TOSL, forth_latest
        lds     TOSH, forth_latest+1
        pushd
        lds     TOSL, forth_cp
        lds     TOSH, forth_cp+1
        pushd
        lds     TOSL, forth_np
        lds     TOSH, forth_np+1
        pushd
        lds     TOSL, forth_np0
        lds     TOSH, forth_np0+1
        rnext

; Extended dispatch table aligned to a 256-word (512-byte) boundary.
.align 9
.global vm_ext_dispatch_table
vm_ext_dispatch_table:
        create_dispatch_table   .ext_opcode_, 256
.align 0

; MARKER> ( latest cp np np0 -- ) restore dictionary snapshot
; just does the inverse of the other one
ext_opcode_markerfrom:
        sts     forth_np0, TOSL
        sts     forth_np0+1, TOSH
        popd
        sts     forth_np, TOSL
        sts     forth_np+1, TOSH
        popd
        sts     forth_cp, TOSL
        sts     forth_cp+1, TOSH
        popd
        sts     forth_latest, TOSL
        sts     forth_latest+1, TOSH
        popd
        rnext

; MARKER, ( latest cp np np0 -- ) compile dictionary snapshot
; into current dictionary entry
ext_opcode_markercomma:
        callc_prologue
        call    comma   ; store latest
        popd
        call    comma   ; store cp
        popd
        call    comma   ; store np
        popd
        call    comma   ; store np0
        popd
        rnext

; EMPTY ( -- ) clear entire dictionary (code space and name space)
ext_opcode_empty:
        callc_0arg_prologue
        call    dict_init
        callc_0arg_epilogue

; SHRED ( -- ) deletes name space, giving compiled code access
; to the entire Forth data space region.
; (you'll probably going to want to remember one or more xt's for words in
; code space, so you can actually get that compiled code running)
ext_opcode_shred:
        ldi     TMPL, lo8(forth_pmax)
        ldi     TMPH, hi8(forth_pmax)
        sts     forth_np0, TMPL
        sts     forth_np0+1, TMPH
        sts     forth_np, TMPL
        sts     forth_np+1, TMPH
        sts     forth_npmax, TMPL
        sts     forth_npmax+1, TMPH
        rnext

; SPACES ( n -- ) write n space characters to output device
ext_opcode_spaces:
        callc_prologue
        call    forth_spaces
        popd
        callc_epilogue

; KEY? ( -- flag ) return true if a character is available
ext_opcode_keyq:
        pushd
        callc_prologue
        ldi     TOSL, 1 ; nonblocking check
        lds     ZL, mio_getc
        lds     ZH, mio_getc+1
        icall
        callc_restore
        ; extend bool to forth flag
        clr     TOSH
        tst     TOSL
        brne    1f
        rnext
1:      movw    TOS, TRUE
        rnext

; ACCEPT ( addr +n1 -- +n2 ) receive a string of at most +n1 characters
; readline() takes args in this order:
; r25:r24 - buffer address
; r23:r22 - buffer length
ext_opcode_accept:
        movw    TMP, TOS        ; get buffer size into r23:r22
        popd                    ; get buffer address into r25:r24
        callc_prologue
        call    readline
        callc_restore            ; check if readline returned an error
        sbrc    TOSH, 7
        rjmp    1f
        rnext
1:      throw   FE_USER_INTERRUPT

; REFILL ( -- flag ) fill the input buffer from the current input source
ext_opcode_refill:
        pushd
        callc_prologue
        call    forth_refill
        clr     TOSH    ; convert bool to forth flag
        cpse    TOSL, ZERO
        movw    TOS, TRUE
        rnext

; EVALUATE ( i * x c-addr u -- j * x ) save input source, make the string
; the input source, and interpret. Restore input source afterward.
ext_opcode_evaluate:
        movw    r20, TOS        ; move string length to r21:r20
        nip                     ; nip address into r23:r22
        popd                    ; drop TOS
; don't push a return address if invoked from the text interpreter, i.e.
; make this a tail call
        call    .was_called_from_text_interpreter
        breq    1f
; not invoked from the text interpreter? push IP normally
        pushr
1:      jmp     forth_evaluate_str

; :NONAME ( -- xt ) create a new colon-word with a zero-length name, push its
; execution token, and enter compilation state.
ext_opcode_noname:
        pushd                   ; reserve space on the stack for the xt
        movw    TOS, ZERO       ; name is the empty string
        movw    TMP, ZERO
        ldi     r20, FL_COLON   ; it's a colon word
        callc_prologue
        call    namespace_create_nonamecheck
        ; update saved dsp to prevent spurious "control structure mismatch"
        sts     forth_saved_dsp, DSPL
        sts     forth_saved_dsp+1, DSPH
        rsp_to_z
        sts     forth_saved_rsp, ZL
        sts     forth_saved_rsp+1, ZH
        ; get execution token
        lds     TOSL, forth_latest
        lds     TOSH, forth_latest+1
        call    ramdict_lfa_to_xt
        callc_restore
        ; enter compile mode
        lds     ZL, forth_flags
        sbr     ZL, (1<<FF_STATE_BIT)
        sts     forth_flags, ZL
        rnext

; COMP' ( -- ct ) read a name from the input stream and push its compilation token
ext_opcode_comptick:
        pushd
        callc_prologue
        call    forth_parse_name
        tstw    TOS     ; check for empty string
        breq    1f
        call    find_ct
        ; 0 indicates no ct
        tstw    TOS
        breq    2f
        callc_epilogue
1:      throw   FE_ZERO_LENGTH_NAME
2:      throw   FE_UNDEFINED_WORD

; FIND ( c-addr -- c-addr 0 | xt 1 | xt -1 )
ext_opcode_find:
        ; convert counted string to addr+len
        movw    Z, TOS
        pushd
        callc_prologue
        ld      TOSL, Z+
        movw    TMP, Z
        clr     TOSH
        call    find_xt
        tstw    TOS
        breq    1f
; found? store xt to second-on-stack
        st      DSP, TOSL
        std     DSP+1, TOSH
; immediate or not?
        call    xt_immediate_flag
1:      callc_epilogue

; FIND-NAME ( addr len -- nt|0 ) return name token for word with name
ext_opcode_findname:
        nip
        callc_prologue
        call    find_name
        callc_epilogue

; FORGET-NAME ( nt -- ) delete word and all words defined after it
ext_opcode_forgetname:
        callc_prologue
        call    forget_name
        popd
        callc_epilogue

; >BODY ( xt -- addr ) obtain the body address of the word represented by xt
; Behavior is undefined if used on the xt of a non-child word.
ext_opcode_tobody:
        ; does the xt represent an lfa, or a code address?
        mov     TMPL, TOSH
        andi    TMPL, 0b11000000
        cpi     TMPL, 0b10000000        ; XT_NP0_LFA
        brne    .xt_is_code
        ; otherwise, convert xt to lfa
        andi    TOSH, 0b00111111        ; clear upper bits
        lds     TMPL, forth_np0
        lds     TMPH, forth_np0+1
        add     TOSL, TMPL              ; add np0 to offset
        adc     TOSH, TMPH
        callc_prologue
        call    ramdict_lfa_to_real_body
        callc_epilogue
.xt_is_code:
        ; xt is a code address. the bodies of all child words start with a
        ; 3-byte (DODOES) call to the parent semantics, followed by the body data.
        ; so adding 3 gives the address of the word's body data.
        adiw    TOS, 3
        rnext

; SP@ ( -- addr ) push address of stack pointer (before execution of this word)
ext_opcode_spfetch:
        movw    TMP, DSP
        pushd
        movw    TOS, TMP
        rnext

; SP! ( addr -- ) set stack pointer
ext_opcode_spstore:
        movw    DSP, TOS
        rnext

; DEPTH ( -- u ) number of values on the stack (before execution of this word)
ext_opcode_depth:
        ldi     TMPL, lo8(forth_sp0)
        ldi     TMPH, hi8(forth_sp0)
        sub     TMPL, DSPL
        sbc     TMPH, DSPH
        lsr     TMPH
        ror     TMPL
        pushd
        movw    TOS, TMP
        rnext

; UNUSED ( -- u ) number of free bytes in dictionary
ext_opcode_unused:
        pushd
        callc_prologue
        call    forth_available_memory
        callc_epilogue

; PARSE" ( -- addr u ) parse quote-delimited string from input stream
; building block for S" ." ABORT" and others
ext_opcode_parsequote:
        pushd
        ldi     TOSL, '"'
        jmp     opcode_parse

; PARSE\" ( -- addr u ) parse quote-delimited string from input stream and
; convert escape
ext_opcode_parseslashquote:
        pushd
        callc_prologue
        call     parse_escaped_string
        ; upon return: addr in r23:r22, len in r25:r24
        st      -DSP, TMPH
        st      -DSP, TMPL
        callc_epilogue

; WORD ( delim -- c-addr ) skip leading delimiters, parse string up to next
; occurrence of delimiter, copy to a transient region as a counted string, and
; push its address
ext_opcode_word:
        callc_0arg_prologue     ; makes a second copy of delim
        call    forth_consume_delim
        ; upon return: addr in r23:r22, len in r25:r24
        movw    TOS, TSAV
        call    forth_parse
        ; too long?
        cpi     TOSL, FORTH_HOLD_BUFFER_SIZE
        cpc     TOSH, ZEROH
        brsh    .too_big_for_tmpbuf
        lds     ZL, forth_hld0
        lds     ZH, forth_hld0+1
        rjmp    .place
.too_big_for_tmpbuf:
        callc_restore
        throw   FE_PARSED_STR_OVERFLOW

; PLACE ( addr1 len dst -- addr2 ) write to memory as a counted string
ext_opcode_place:
        movw    Z, TOS          ; save dest
        popd                    ; TOS is now count
        nip                     ; address in r23:r22
        callc_prologue          ; meh, need to do this
; dst=Z, src=r23:r22, len=r25:r24
.place:
        st      Z+, TOSL        ; store count
        ; use memcpy: dst=r25:r24, src=r23:r22, len=r21:r20
        movw    r20, TOS        ; length to r21:r20
        movw    TOS, Z          ; dest address to r25:r24
        call    memcpy
        sbiw    TOS, 1          ; return value points at the count
        callc_epilogue


; CMOVE ( c-addr1 c-addr2 u -- ) copy u bytes from c-addr1 to c-addr2
; Bytes are copied from lower addresses to higher addresses
ext_opcode_cmove:
        tstw    TOS
        breq    .nomove
; need 2 pointers... save IP so we can use X
; (TOS is byte count)
        movw    XSAV, IP
        ld      ZL, DSP+        ; Z is destination ptr
        ld      ZH, DSP+
        ld      XL, DSP+       ; X is source ptr
        ld      XH, DSP+
.cmove_fwd:
        ld      TMPL, X+
        st      Z+, TMPL
        sbiw    TOS, 1
        brne    .cmove_fwd
        movw    IP, XSAV ; don't forget to restore IP
        popd
        rnext
.nomove:
; drop args and return
        drop3
        rnext

; CMOVE> ( c-addr1 c-addr2 u -- ) copy u bytes from c-addr1 to c-addr2
; Bytes are copied from higher addresses to lower addresses
ext_opcode_cmoveup:
        tstw    TOS
        breq    .nomove
        movw    XSAV, IP
        ld      ZL, DSP+        ; Z is destination ptr
        ld      ZH, DSP+
        ld      XL, DSP+       ; X is source ptr
        ld      XH, DSP+
; advance source/dest pointers to the end of their respective regions
; (count is in TOS)
.cmove_rev:
        add     ZL, TOSL
        adc     ZH, TOSH
        add     XL, TOSL
        adc     XH, TOSH
1:      ld      TMPL, -X
        st      -Z, TMPL
        sbiw    TOS, 1
        brne    1b
        movw    IP, XSAV ; don't forget to restore IP
        popd
        rnext

; MOVE ( addr1 addr2 u -- ) copy u bytes from addr1 to addr2
ext_opcode_move:
        tstw    TOS
        breq    .nomove
        movw    XSAV, IP
        ld      ZL, DSP+        ; Z is destination ptr
        ld      ZH, DSP+
        ld      XL, DSP+       ; X is source ptr
        ld      XH, DSP+
; if src < dest, use CMOVE> otherwise use CMOVE
        cp      XL, ZL
        cpc     XH, ZH
        brlt    .cmove_rev
        rjmp    .cmove_fwd

; ( x1 x2 x3 x4 -- x2 x1 x3 x4 )
; x4 - TOS (r25:24) -> TOS (r25:r24)
; x3 - DSP+1:DSP    -> DSP+1:DSP
; x2 - DSP+3:DSP+2  -> DSP+5:DSP+4
; x1 - DSP+5:DSP+4  -> DSP+3:DSP+2
.swap_2_under:
        ldd     TMPL, DSP+2
        ldd     TMPH, DSP+3
        ldd     r20,  DSP+4
        ldd     r21,  DSP+5
        std     DSP+2, r20
        std     DSP+3, r21
        std     DSP+4, TMPL
        std     DSP+5, TMPH
        ret

; >NUMBER ( ud1 addr1 len1 -- ud2 addr2 len2 ) string to number
ext_opcode_tonumber:
; forth_to_number() args:
; r25:r24 - string length
; r23:r22 - string address
; r21:r20 - pointer to double-cell value
; returns:
; r25:r24 - number of chars unconverted
; r23:r22 - address of first unconverted char
; the double-cell value needs to be word-swapped! TODO not great
        rcall    .swap_2_under
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        movw    r20, DSP
        subi    r20, lo8(-2)
        sbci    r21, hi8(-2)
        callc_prologue
        call    forth_to_number
        ; write back to stack
        st      DSP, TMPL
        std     DSP+1, TMPH
        rcall   .swap_2_under
1:      callc_epilogue

; -TRAILING ( addr u1 -- addr u2 ) delete trailing spaces from string
ext_opcode_dashtrailing:
        ld      ZL, DSP         ; get address of string start
        ldd     ZH, DSP+1
        add     ZL, TOSL        ; add length to get address of string end
        adc     ZH, TOSH
        tstw    TOS             ; stop if string is empty
        rjmp    2f
1:      ld      TMPL, -Z        ; get character
        cpi     TMPL, ' '       ; is it a space?
        brne    3f              ; if so, end
        sbiw    TOS, 1          ; otherwise, delete character
2:      brne    1b
3:      rnext

; /STRING ( addr1 u1 n -- addr2 u2 ) delete/add n characters from beginning of
; string. No bounds-checking is performed.
ext_opcode_slashstring:
        movw    TMP, TOS        ; pop n
        popd
        sub     TOSL, TMPL      ; adjust length
        sbc     TOSH, TMPH
        ld      r20, DSP        ; adjust character address
        ldd     r21, DSP+1
        add     r20, TMPL
        adc     r21, TMPH
        st      DSP, r20
        std     DSP+1, r21
        rnext

; COMPARE ( addr1 len1 addr2 len2 -- n ) compare string 1 and string 2
; this is memcmp(addr1, addr2, len1) if len1==len2, otherwise
; memcmp(addr1, addr2, min(len1,len2)) and if it returns 0 then return -1 if
; len1<len2 or 1 if len1>len2.
; int memcmp(const void *s1, const void *s2, size_t len);
; r25:r24 - s1
; r23:r22 - s2
; r21:r20 - len
ext_opcode_compare:
        movw    TSAV, ZERO      ; prepare the constant 1
        inc     TSAVL
        ; move len2 to r21:r20
        movw    r20, TOS
        ; nip addr1, len1, and len2 into registers
        ld      r22, DSP+       ; nip addr2 into r23:r22
        ld      r23, DSP+
        ld      r18, DSP+       ; nip len1 into a temp register pair
        ld      r19, DSP+
        ld      r24, DSP+       ; nip addr1 into r25:r24
        ld      r25, DSP+
        ; are lengths equal?
        cp      r18, r20
        cpc     r19, r21
        brlt    .compare_s1_shorter
        brne    .compare_s2_shorter
        ; if lengths equal, return sign of memcmp() result
        callc_prologue
        call    memcmp
        callc_restore
        tstw    TOS
        breq    1f      ; if zero, just return zero
        brmi    .compare_return_minus1
.compare_return_plus1:
        ldi     TOSL, 1
        clr     TOSH
1:      rnext
.compare_return_minus1:
        movw    TOS, TRUE
        rnext
.compare_s1_shorter:
        movw    r20, r18        ; compare up to length of string1
        movw    TSAV, TRUE      ; return -1 if 2 strings are different
.compare_s2_shorter:
        callc_prologue
        call    memcmp
        callc_restore
        tstw    TOS             ; did memcmp return zero?
        breq    .compare_return_len_of_shorter  ; if so, compare lengths
        brmi    .compare_return_minus1
        rjmp    .compare_return_plus1
.compare_return_len_of_shorter:
        movw    TOS, TSAV
        rnext

; SEARCH ( addr1 len1 addr2 len2 -- addr3 len3 flag ) search string 1 for string 2
; this is just memmem(), but we need to get the args off the stack and into
; the right registers:
; void *memmem(const void *s1, size_t len1, const void *s2, size_t len2);
; r25:r24 - s1
; r23:r22 - len1
; r21:r20 - s2
; r19:r18 - len2
ext_opcode_search:
        movw    r18, TOS        ; len2 to r19:r18
        ld      r20, DSP+       ; nip addr2 to r21:r20
        ld      r21, DSP+
        ld      r22, DSP        ; len1 to r22:r23
        ldd     r23, DSP+1
        ldd     r24, DSP+2      ; addr1 to r25:r24
        ldd     r25, DSP+3
        callc_0arg_prologue     ; also copy addr1 to TSAV
        call    memmem
        callc_restore
        tstw    TOS
        breq    .search_notfound
; found
        std     DSP+2, TOSL     ; store pointer to match start
        std     DSP+3, TOSH
        sub     TOSL, TSAVL     ; calculate offset from start
        sbc     TOSH, TSAVH
        ld      TMPL, DSP
        ldd     TMPH, DSP+1
        sub     TMPL, TOSL      ; subtract offset from old length to get new length
        sbc     TMPH, TOSH
        st      DSP, TMPL       ; store number of chars left in string
        std     DSP+1, TMPH
        movw    TOS, TRUE       ; return true flag
.search_notfound:
        ; return with 0-flag in TOS and addr1/len1 unchanged
        rnext

; ENVIRONMENT? ( addr len -- false | i*x true ) environmental query
ext_opcode_environmentq:
        callc_prologue
        nip                     ; nip address into r23:r22
        call    environment_find
        tstw    TOS             ; if null returned, keep 0 on the stack
        breq    1f
        jmp     opcode_execute  ; execute the xt
1:      callc_epilogue

; SOURCE ( -- c-addr u ) push address of and number of chars in the input buffer
ext_opcode_source:
        pushd
        callc_prologue
        call    forth_inputbuf
        callc_restore
        pushd
        clr     TOSH
        lds     TOSL, forth_inputlen
        rnext


; CATCH ( i * x xt -- j * x 0 | i * x n )
ext_opcode_catch:
; drop xt from the stack
        movw    TMP, TOS        ; drop xt from stack
        popd
; make sure TOS register value is committed to RAM
        pushd
        adiw    DSP, 2
; push stack pointer value onto return stack
        push    DSPH
        push    DSPL
; push current input source
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
; don't push a return address if invoked from the text interpreter, i.e.
; make this a tail call
        call    .was_called_from_text_interpreter
        breq    1f
; not invoked from the text interpreter? push IP normally
        pushr
; push a new exception frame
1:      push_exception_frame .catch_exception_handler
; push 0x0001 so the exception handler is invoked even if the word
; returns without throwing
        push    ZEROH
        ldi     ZL, 1
        push    ZL
; now EXECUTE
        ser     r18             ; tell forth_execute_xt NOT to push a return addr
        jmp     forth_execute_xt

; DELAY ( u -- ) wait a specific number of frames (1/60 sec intervals)
ext_opcode_delay:
        tstw    TOS                     ; do nothing if 0 frames specified
        breq    2f
        lds     TMPL, screen_ptr+1      ; are we in tiny mode?
        tst     TMPL
        brmi    1f
        callc_prologue
        movw    TMP, TOS
        ldi     r24, lo8(fcon)
        ldi     r25, hi8(fcon)
        callc_prologue
        call    console_delay
        popd
        callc_epilogue
1:      sbis    new_frame, new_frame_bit
        rjmp    1b
        cbi     new_frame, new_frame_bit
        sbiw    TOS, 1
        brne    1b
2:      popd
        rnext

; (ABORT") ( flag c-addr len -- ) runtime of ABORT"
ext_opcode_abortq:
        ldd     TMPL, DSP+2
        ldd     TMPH, DSP+3
        or      TMPL, TMPH
        breq    .noabortq
        throw   FE_ABORTQ
.noabortq:
        drop3
        rnext

; TEXT ( -- ) switch to 40x25 text mode
ext_opcode_fortycol:
        callc_0arg_prologue
        call    forth_40x25_console
        callc_0arg_epilogue

; HTEXT ( -- ) switch to 80x25 text mode
ext_opcode_eightycol:
        callc_0arg_prologue
        call    forth_80x25_console
        callc_0arg_epilogue

; CTEXT ( -- ) switch to 40x25 color text mode
ext_opcode_colorfortycol:
        callc_0arg_prologue
        call    forth_40x25_color_console
        callc_0arg_epilogue

; +COLOR ( -- ) enable colorburst
ext_opcode_coloron:
        sbi     color_video, color_video_bit
        rnext

; -COLOR ( -- ) disable colorburst
ext_opcode_coloroff:
        cbi     color_video, color_video_bit
        rnext

; (.S) ( -- ) print stack dump
; read byte from input stream to determine format:
; < 0 - signed in current base
; > 0 - unsigned in current base
; = 0 - unsigned hexadecimal
ext_opcode_dumpstack:
; stack pointer in r23:r22
        movw    TMP, DSP
; format byte in r20
        ld      r20, IP+
        callc_prologue
; forth_dump_stack() preserves TOS
        call    forth_dump_stack
        callc_epilogue

; BYE ( -- ) exit Forth
ext_opcode_bye:
        jmp     app_quit

; TINY ( i*x -- | R: j*y -- ) clear data stack and enter "tiny" mode:
; - Name space is deleted
; - 16 byte return stack
; - 16 byte data stack
; - No support for exceptions, graphical console, numeric formatting, anything
;   that uses the dictionary or runtime
ext_opcode_tiny:
        ldi     DSPL, lo8(forth_tiny_sp0)
        ldi     DSPH, hi8(forth_tiny_sp0)
; set high bit of screen_ptr, indicating tiny mode
        lds     ZL, screen_ptr+1
        ori     ZL, 0b10000000
        sts     screen_ptr+1, ZL
        rnext

ext_opcode_exittiny:
; clear high bit of screen_ptr
        lds     ZL, screen_ptr+1
        andi    ZL, 0b01111111
        sts     screen_ptr+1, ZL
        jmp     forth_cold_start

; +INT ( -- ) enable interrupts (i.e. video and keyboard polling)
ext_opcode_sei:
        sei
        rnext

; -INT ( -- ) disable interrupts (i.e. video and keyboard polling)
; This *will* hang the system if control is returned to the text interpreter
; before interrupts are re-enabled, requiring a hard reset.
; Disabling interrupts frees up the CPU from drawing video, allowing it to be
; 100% utilized for Forth code.
ext_opcode_cli:
        cli
        rnext

; BEEP ( -- ) simple beep, square wave, 8 frames long
ext_opcode_beep:
        callc_0arg_prologue
        call    console_beep
        callc_0arg_epilogue

; PTONE! ( freq duration dutycycle -- ) pulse wave, with given duty cycle
; (0-255, 128=50%), frequency (0-863) and duration in frames (1-255). Execution
; continues immediately, without waiting for the sound to finish.
ext_opcode_ptone:
        movw    r20, TOS        ; get duty cycle into r21:r20
        nip                     ; get duration into r23:r22
        rjmp    1f
; TONE! ( freq duration -- ) square wave, with given frequency (0-863) and duration
; in frames (1-255). Execution continues immediately, without waiting for the
; sound to finish.
ext_opcode_tone:
        ldi     r20, 128        ; 50% duty cycle square wave
        movw    TMP, TOS        ; get duration into r23:r22
1:      popd                    ; get frequency into r25:r24
        callc_prologue
        call    tone
        popd
        callc_epilogue

; PTONE ( freq duration dutycycle -- ) pulse wave, with given duty cycle
; (0-255, 128=50%), frequency (0-863) and duration in frames (1-255). Pauses
; execution until the sound finishes.
ext_opcode_ptonesync:
        movw    r20, TOS        ; get duty cycle into r21:r20
        nip                     ; get duration into r23:r22
        rjmp    1f
; TONE ( freq duration -- ) square wave, with given frequency (0-863) and duration
; in frames (1-255). Pauses execution until the sound finishes.
ext_opcode_tonesync:
        ldi     r20, 128        ; 50% duty cycle square wave
        movw    TMP, TOS        ; get duration into r23:r22
1:      popd                    ; get frequency into r25:r24
        callc_prologue
        call    tone
        popd
        callc_restore
1:      sbic    tone_enabled, tone_enabled_bit  ; loop until the tone-enabled bit is cleared
        rjmp    1b
        rnext

; PNOTE! ( note duration dutycycle -- ) pulse wave, with given duty cycle
; (0-255, 128=50%), MIDI note number (0-127) and duration in frames (1-255).
; Execution continues immediately, without waiting for the sound to finish.
ext_opcode_pnote:
        movw    r20, TOS        ; get duty cycle into r21:r20
        nip                     ; get duration into r23:r22
        rjmp    1f
; NOTE! ( note duration -- ) square wave, with MIDI note number (0-127) and duration
; in frames (1-255). Execution continues immediately, without waiting for the
; sound to finish.
ext_opcode_note:
        ldi     r20, 128        ; 50% duty cycle square wave
        movw    TMP, TOS        ; get duration into r23:r22
1:      popd                    ; get note number into r25:r24
        callc_prologue
        call    note
        popd
        callc_epilogue

; PNOTE ( note duration dutycycle -- ) pulse wave, with given duty cycle
; (0-255, 128=50%), MIDI note number (0-127) and duration in frames (1-255).
; Pauses execution until the sound finishes.
ext_opcode_pnotesync:
        movw    r20, TOS        ; get duty cycle into r21:r20
        nip                     ; get duration into r23:r22
        rjmp    1f
; NOTE ( freq duration -- ) square wave, with MIDI note number (0-127) and duration
; in frames (1-255). Pauses execution until the sound finishes.
ext_opcode_notesync:
        ldi     r20, 128        ; 50% duty cycle square wave
        movw    TMP, TOS        ; get duration into r23:r22
1:      popd                    ; get frequency into r25:r24
        callc_prologue
        call    note
        popd
        callc_restore
1:      sbic    tone_enabled, tone_enabled_bit  ; loop until the tone-enabled bit is cleared
        rjmp    1b
        rnext

; +SOUND ( -- ) enable audio output
ext_opcode_soundon:
        callc_0arg_prologue
        call    audio_init
        callc_0arg_epilogue

; -SOUND ( -- ) disable audio output. BEEP, TONE, NOTE, etc. have no effect.
ext_opcode_soundoff:
        callc_0arg_prologue
        call    audio_off
        callc_0arg_epilogue

; LOAD ( i*x n -- j*x ) load numbered block
ext_opcode_load:
        tstw    TOS
        brmi    .invalidblk
        breq    .noload
        movw    TMP, TOS
        callc_prologue
        call    num_blocks
        callc_restore
        cp      TOSL, TMPL
        brlo    .invalidblk
        popd
; don't push a return address if invoked from the text interpreter, i.e.
; make this a tail call (same as with EVALUATE)
        call    .was_called_from_text_interpreter
        breq    1f
; not invoked from the text interpreter? push IP normally
        pushr
1:      jmp     forth_load_block
.invalidblk:
        popd
        throw   FE_INVALID_BLOCK_NUMBER
; 0 LOAD does nothing
.noload:
        popd
        rnext

; CLS ( -- ) clear screen, reset margins, and return cursor to home
; (Use PAGE or 12 EMIT to clear only the scrolling region.)
ext_opcode_cls:
        callc_0arg_prologue
        ldi     r24, lo8(fcon)
        ldi     r25, hi8(fcon)
        call    console_clrhome
        callc_0arg_epilogue

; FORM ( -- rows cols ) returns dimensions of console scroll region
ext_opcode_form:
        pushd
        clr     TOSH
        lds     TOSL, fcon_mbottom
        andi    TOSL, fcon_bitmask_mbottom
        lds     TMPL, fcon_mtop
        andi    TMPL, fcon_bitmask_mtop
        sub     TOSL, TMPL
        inc     TOSL
        pushd
        lds     TOSL, fcon_mright
        andi    TOSL, fcon_bitmask_mright
        lds     TMPL, fcon_mleft
        sub     TOSL, TMPL
        inc     TOSL
        rnext

; AT-XY ( col row -- ) set cursor position within scroll region
; console_gotoxy():
; r25:r24 console struct
; r23     x
; r22     y
ext_opcode_atxy:
        nip                     ; get x pos into r22
        mov     r20, r24        ; get y pos into r20
        ldi     r24, lo8(fcon)
        ldi     r25, hi8(fcon)
        callc_prologue
        call    console_gotoscrollxy
        callc_restore
        popd
        rnext

; CURSOR@ ( -- xy ) get absolute cursor position
ext_opcode_getcursor:
        pushd
        lds     TOSL, fcon_cx
        andi    TOSL, fcon_bitmask_cx
        lds     TOSH, fcon_cy
        andi    TOSL, fcon_bitmask_cy
        rnext

; CURSOR! ( xy -- ) set absolute cursor position
ext_opcode_setcursor:
        mov     r22, TOSL       ; get x pos into r22
        mov     r20, TOSH       ; get y pos into r20
        ldi     r24, lo8(fcon)
        ldi     r25, hi8(fcon)
        callc_prologue
        call    console_gotoxy
        callc_restore
        popd
        rnext

; CURSOR+! ( xy -- ) add xy to cursor position
ext_opcode_movecursor:
        mov     r22, TOSL       ; get x delta into r22
        mov     r20, TOSH       ; get y delta into r20
        ldi     r24, lo8(fcon)
        ldi     r25, hi8(fcon)
        callc_prologue
        call    console_movexy
        callc_restore
        popd
        rnext

; WINDOW ( xy wh -- ) set console scrolling region
ext_opcode_window:
        movw    r22, TOS        ; w to r22, h to r23
        ld      r20, DSP+       ; x to r20
        ld      r21, DSP+       ; y to r21
        ldi     TOSL, lo8(fcon)
        ldi     TOSH, hi8(fcon)
        callc_prologue
        call    console_setmarginrect
        callc_restore
        popd
        rnext

; XY>ADDR ( xy -- addr ) convert screen coordinate xy to pixel address in the
; screen buffer
ext_opcode_xytoaddr:
        movw    TMP, ZERO               ; mul will clobber the zero regs
        lds     r0, bytesperline        ; multiply y coord * bytes per line
        mul     r0, TOSH
        add     r0, TOSL                ; add x coord
        adc     r1, TMPL                ; will be zero
        lds     TOSL, screen_ptr        ; add screen offset
        lds     TOSH, screen_ptr+1
        add     TOSL, r0
        adc     TOSH, r1
        movw    ZERO, TMP               ; restore zero regs
        rnext

; TSETCC ( cc xy -- ) set character (LSB of NOS) and color (MSB of NOS)
; of text cell at xy
ext_opcode_tsetcc:
        nip                             ; get color into TMPH and char into TMPL
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
        st      Z, TMPL                 ; store char
        subi    ZL, lo8(-1000)          ; cell color is 1000 bytes ahead
        sbci    ZH, hi8(-1000)
        ldi     TMPL, hi8(bitspreadtable)       ; TODO how to do this with one subi?
        add     TMPH, TMPL
        st      Z, TMPH                 ; store color
        popd
        rnext

; TBOX ( xy wh -- ) draw box in text console using box-drawing characters
; (interior is not filled)
; if w or h is 0, nothing is drawn
; if w is 1, a vertical line is drawn
; if h is 1, a horizontal line is drawn
; the current reverse-video and color (for color console) settings are used
ext_opcode_tbox:
        movw    r22, TOS        ; w to r22, h to r23
        ld      r20, DSP+       ; x to r20
        ld      r21, DSP+       ; y to r21
        ldi     TOSL, lo8(fcon)
        ldi     TOSH, hi8(fcon)
        callc_prologue
        call    console_box
        callc_restore
        popd
        rnext

; TRECTC ( c xy wh -- ) set color of rectangular area of text buffer
; (color console only)
; no bounds checking is performed
ext_opcode_trectc:
; do nothing if width or height is zero
        mov     TMPL, TOSL
        or      TMPL, TOSH
        breq    .no_rect
        ld      TMPL, DSP       ; x coord
        ldd     TMPH, DSP+1     ; y coord; color is 25 rows ahead
        subi    TMPH, -25       ; add 25 to y
        ldd     r20, DSP+2      ; get color index
        ldi     r21, hi8(bitspreadtable) 
        add     r20, r21        ; get bitspread table offset
        rjmp    .rect

; TRECT ( c xy wh -- ) fill rectangular area of screen buffer with byte c
; no bounds checking is performed
ext_opcode_trect:
; do nothing if width or height is zero
        mov     TMPL, TOSL
        or      TMPL, TOSH
        breq    .no_rect
        ld      TMPL, DSP       ; x coord
        ldd     TMPH, DSP+1     ; y coord
        ldd     r20, DSP+2      ; get fill value
.rect:
        lds     r21, bytesperline      ; get screen width
        lds     ZL, screen_ptr  ; screen base address
        lds     ZH, screen_ptr+1
        add     ZL, TMPL        ; add x coord
        adc     ZH, ZEROH
        mul     r21, TMPH       ; multiply y by screen width to get byte offset
        add     ZL, r0          ; add y byte offset
        adc     ZH, r1
        clr     ZEROL           ; mul clobbers zero regs, restore them
        clr     ZEROH
        sub     r21, TOSL       ; subtract rect width from screen width, giving byteskip btwn lines
; loop
; TOSL - width
; TOSH - y counter
; TMPL - x counter
; r21  - number of bytes to skip to get to next row
; r20  - value to fill with
.yloop: mov     TMPL, TOSL
.xloop: st      Z+, r20
        dec     TMPL
        brne    .xloop
; end of x loop
        add     ZL, r21
        adc     ZH, ZEROL
        dec     TOSH
        brne    .yloop
.no_rect:
        drop3
        rnext

; TRECTCC ( cc xy wh -- ) set rectangular area of color console to character
; and color. no bounds checking is performed
ext_opcode_trectcc:
; do nothing if width or height is zero
        mov     TMPL, TOSL
        or      TMPL, TOSH
        breq    .no_rect
        ld      TMPL, DSP       ; x coord
        ldd     TMPH, DSP+1     ; y coord
        ldd     r20, DSP+2      ; get fill character
        lds     r21, bytesperline      ; get screen width
        lds     ZL, screen_ptr  ; screen base address
        lds     ZH, screen_ptr+1
        add     ZL, TMPL        ; add x coord
        adc     ZH, ZEROH
        mul     r21, TMPH       ; multiply y by screen width to get byte offset
        add     ZL, r0          ; add y byte offset
        adc     ZH, r1
        clr     ZEROL           ; mul clobbers zero regs, restore them
        clr     ZEROH
        sub     r21, TOSL       ; subtract rect width from screen width, giving byteskip btwn lines
        ldd     TMPH, DSP+3     ; get fill color
        ldi     TMPL, hi8(bitspreadtable)
        add     TMPH, TMPL      ; get bitspread table offset
; loop
; TOSL - width
; TOSH - y counter
; TMPL - x counter
; r21  - number of bytes to skip to get to next row
; r20  - character to fill with
; TMPH - color to fill with
.ylp2:  mov     TMPL, TOSL
.xlp2:  subi    ZL, lo8(-1000)
        sbci    ZH, hi8(-1000)
        st      Z, TMPH         ; set color
        subi    ZL, lo8(1000)
        sbci    ZH, hi8(1000)
        st      Z+, r20         ; set character
        dec     TMPL
        brne    .xlp2
; end of x loop
        add     ZL, r21
        adc     ZH, ZEROL
        dec     TOSH
        brne    .ylp2
        drop3
        rnext

; TSPRITE ( c-addr wh xy -- ) copy characters from c-addr to screen, organized
; as w by h, at point xy. the byte value 0 is interpreted as transparent.
; if h=0, it is considered the same as h=1. this allows this function to draw
; Forth strings (addr/len pairs) provided they are less than 256 chars.
; for color console, the sprite is drawn using the current color
; no bounds checking is performed
ext_opcode_tsprite:
        ld      TMPL, DSP       ; width
        tst     TMPL
        breq    .no_rect
        ldd     TMPH, DSP+1     ; height
        tst     TMPH            ; treat h=0 as h=1
        brne    1f
        ldi     TMPH, 1
1:      lds     r21, bytesperline      ; get screen width
        ldd     r18, DSP+2       ; srcptr lo
        ldd     r19, DSP+3       ; srcptr hi
        lds     ZL, screen_ptr  ; screen base address
        lds     ZH, screen_ptr+1
        add     ZL, TOSL        ; add x coord
        adc     ZH, ZEROH
        mul     r21, TOSH       ; multiply y by screen width to get byte offset
        add     ZL, r0          ; add y byte offset
        adc     ZH, r1
        movw    TSAV, Z         ; save a copy
        clr     ZEROL           ; mul clobbers zero regs, restore them
        clr     ZEROH
        sub     r21, TMPL       ; subtract rect width from screen width, giving byteskip btwn lines
        movw    XSAV, X
        movw    X, r18
        mov     TOSH, TMPH
; loop
; TMPL - width
; TMPH - height
; TOSH - y counter
; TOSL - x counter
; r21  - number of bytes to skip to get to next row
.ylps:  mov     TOSL, TMPL
.xlps:  ld      r20, X+         ; get byte from src
        cpse    r20, ZEROL      ; don't write if zero
        st      Z, r20          ; write to dst
        adiw    Z, 1            ; advance dst
        dec     TOSL
        brne    .xlps
; end of x loop
        add     ZL, r21
        adc     ZH, ZEROL
        dec     TOSH
        brne    .ylps
; end of y loop
; need to do color?
        lds     r20, fcon_color
        andi    r20, fcon_bitmask_color
        breq    2f
; reinitialize params
        movw    X, r18          ; src
        movw    Z, TSAV         ; dst
        subi    ZL, lo8(-1000)
        sbci    ZH, hi8(-1000)
        lds     r18, globalcolor
; color loop
; TMPL - width
; TMPH - y counter
; TOSL - x counter
.ylpsc: mov     TOSL, TMPL
.xlpsc: ld      r20, X+         ; get byte from src
        cpse    r20, ZEROL      ; don't write color if zero
        st      Z, r18          ; write color to dst
        adiw    Z, 1            ; advance dst
        dec     TOSL
        brne    .xlpsc
; end of color x loop
        add     ZL, r21
        adc     ZH, ZEROL
        dec     TMPH
        brne    .ylpsc
2:      movw    X, XSAV
.no_rect2:
        drop3
        rnext


; TSPRITECC ( c-addr wh xy -- ) copy characters and colors from c-addr to screen
; organized as w by h, at point xy. c-addr points to an array of (2*w*h) bytes,
; with the first w*h bytes specifying characters and the second w*h bytes
; specifying colors.
; the byte value 0 has two special interpretations:
; - if a character byte is 0, only the color is changed
; - if a color byte is 0, only the character is changed
; - if both are 0, the pixel is effectively transparent
; (Although zero indicates the color black, opaque black cells can still be
; drawn using the ASCII space character, and any of the colors 1-15).
; no bounds checking is performed
ext_opcode_tspritecc:
        ld      TMPL, DSP       ; width
        tst     TMPL
        breq    .no_rect2
        ldd     TMPH, DSP+1     ; height
        tst     TMPH            ; treat h=0 as h=1
        brne    1f
        ldi     TMPH, 1
1:      lds     r21, bytesperline      ; get screen width
        movw    XSAV, X
        ldd     XL, DSP+2       ; srcptr lo
        ldd     XH, DSP+3       ; srcptr hi
        lds     ZL, screen_ptr  ; screen base address
        lds     ZH, screen_ptr+1
        add     ZL, TOSL        ; add x coord
        adc     ZH, ZEROH
        mul     r21, TOSH       ; multiply y by screen width to get byte offset
        add     ZL, r0          ; add y byte offset
        adc     ZH, r1
        movw    TSAV, Z         ; save a copy
        clr     ZEROL           ; mul clobbers zero regs, restore them
        clr     ZEROH
        sub     r21, TMPL       ; subtract rect width from screen width, giving byteskip btwn lines
        mov     TOSH, TMPH
; loop
; TMPL - width
; TMPH - height
; TOSH - y counter
; TOSL - x counter
; r21  - number of bytes to skip to get to next row
.y3:    mov     TOSL, TMPL
.x3:    ld      r20, X+         ; get byte from src
        cpse    r20, ZEROL      ; don't write if zero
        st      Z, r20          ; write to dst
        adiw    Z, 1            ; advance dst
        dec     TOSL
        brne    .x3
; end of x loop
        add     ZL, r21
        adc     ZH, ZEROL
        dec     TOSH
        brne    .y3
; end of y loop--do color
        movw    Z, TSAV         ; dst
        subi    ZL, lo8(-1000)
        sbci    ZH, hi8(-1000)
        ldi     r18, hi8(bitspreadtable)
; color loop
; TMPL - width
; TMPH - y counter
; TOSL - x counter
.y4:    mov     TOSL, TMPL
.x4:    ld      r20, X+         ; get color from src
        add     r20, r18        ; convert color to bitspread table offset
        cpse    r20, r18        ; don't write color if zero
        st      Z, r20          ; write color to dst
        adiw    Z, 1            ; advance dst
        dec     TOSL
        brne    .x4
; end of color x loop
        add     ZL, r21
        adc     ZH, ZEROL
        dec     TMPH
        brne    .y4
        movw    X, XSAV
        drop3
        rnext

; GLYPH@ ( f-addr c -- d1 d2 ) get glyph c from font as 8 bytes on stack
ext_opcode_getglyph:
        mov     ZL, TOSL        ; character index
        ld      ZH, DSP+        ; get address msb
        ld      TMPL, DSP+      ; get rom/ram page
        sbrc    TMPL, 7         ; rom or ram?
        rjmp    .getglyph_ram
.getglyph_rom:
        ; save RAMPZ and set new RAMPZ
        in      r18, _SFR_IO_ADDR(RAMPZ)
        out     _SFR_IO_ADDR(RAMPZ), TMPL
        .rept 3
        ; row 0/2/4
        elpm    TOSL, Z
        inc     ZH
        ; row 1/3/5
        elpm    TOSH, Z
        inc     ZH
        pushd
        .endr
        ; row 6
        elpm    TOSL, Z
        inc     ZH
        ; row 7
        elpm    TOSH, Z
        ; restore RAMPZ
        out     _SFR_IO_ADDR(RAMPZ), r18
        rnext
.getglyph_ram:
        .rept 3
        ; row 0/2/4
        ld      TOSL, Z
        inc     ZH
        ; row 1/3/5
        ld      TOSH, Z
        inc     ZH
        pushd
        .endr
        ; row 6
        ld      TOSL, Z
        inc     ZH
        ; row 7
        ld      TOSH, Z
        rnext

; (FONT:) runtime of FONT:
; Throw an exception if cp is not 256-byte aligned.
; Store 0xFF00|(cp>>8) in name space.
; Allot 2048 bytes in code space and preload it with a copy of the default font.
ext_opcode_makefont:
        lds     TMPL, forth_cp
        tst     TMPL
        brne    .makefont_unaligned
        callc_0arg_prologue
        lds     TOSL, forth_cp+1        ; store msb in name space
        ldi     TOSH, 0xFF
        call    comma
        ldi     TOSL, lo8(2048) ; allot 2048 bytes for font
        ldi     TOSH, hi8(2048)
        call    codespace_allot
        tstw    TOS             ; abort if allot failed
        brne    .makefont_nospace
        movw    TOS, TMP        ; initialize with default font
        ldi     TMPL, lo8(amscii_font_8x8)
        ldi     TMPH, hi8(amscii_font_8x8)
        ldi     r20, lo8(2048)
        ldi     r21, hi8(2048)
        call    memcpy_P
        callc_0arg_epilogue
.makefont_unaligned:
        throw   FE_ALIGNMENT
.makefont_nospace:
        movw    r20, TOS
        throw_r21r20

; GMODE ( c -- ) set bitmap graphics mode 0-17
ext_opcode_gmode:
        callc_prologue
        call    forth_set_bitmap_mode
        popd
        callc_epilogue

; GSPLIT ( c -- ) set split bitmap/text mode 0-17
ext_opcode_gsplit:
        callc_prologue
        call    forth_set_split_screen_mode
        popd
        callc_epilogue

; LPAL! ( c -- ) set palette used for low-color (4-color) bitmap modes
; this actually just changes the bitspread table, but bitmap modes use
; tilemap_hi (not globalcolor)
ext_opcode_lpalette:
        cpi     TOSL, bitspreadtable_numpatterns
        cpc     TOSH, ZEROH
        brlo    1f
        ldi     TOSL, bitspreadtable_numpatterns-1
1:      ldi     TOSH, hi8(bitspreadtable)       ; TODO how to do this with one subi?
        add     TOSL, TOSH
        sts     tilemap_hi, TOSL
        popd
        rnext

; EEC@ ( e-addr -- c ) read byte from EEPROM
ext_opcode_eecfetch:
        ; wait for completion of previous write
1:      sbic    _SFR_IO_ADDR(EECR), EEPE
        rjmp    1b
        ; set address
        out     _SFR_IO_ADDR(EEARL), TOSL
        out     _SFR_IO_ADDR(EEARH), TOSH
        ; start eeprom read
        sbi     _SFR_IO_ADDR(EECR), EERE
        in      TOSL, _SFR_IO_ADDR(EEDR)
        clr     TOSH
        rnext

; EE@ ( e-addr -- n ) read word from EEPROM
ext_opcode_eefetch:
        ; wait for completion of previous write
1:      sbic    _SFR_IO_ADDR(EECR), EEPE
        rjmp    1b
        ; set address
        out     _SFR_IO_ADDR(EEARL), TOSL
        out     _SFR_IO_ADDR(EEARH), TOSH
        ; read low byte
        sbi     _SFR_IO_ADDR(EECR), EERE
        in      TMPL, _SFR_IO_ADDR(EEDR)
        ; advance address by 1 byte
        adiw    TOS, 1
        out     _SFR_IO_ADDR(EEARL), TOSL
        out     _SFR_IO_ADDR(EEARH), TOSH
        ; read high byte
        sbi     _SFR_IO_ADDR(EECR), EERE
        in      TOSH, _SFR_IO_ADDR(EEDR)
        mov     TOSL, TMPL
        rnext

; EEC! ( c e-addr -- ) write byte to EEPROM
ext_opcode_eecstore:
        ; wait for completion of previous write
1:      sbic    _SFR_IO_ADDR(EECR), EEPE
        rjmp    1b
        ; get value to write
        nip
        ; set address
        out     _SFR_IO_ADDR(EEARL), TOSL
        out     _SFR_IO_ADDR(EEARH), TOSH
        ; read first--don't do a write cycle if the value is the same
        sbi     _SFR_IO_ADDR(EECR), EERE
        in      TMPH, _SFR_IO_ADDR(EEDR)
        cp      TMPL, TMPH
        breq    2f
        ; set data byte
        out     _SFR_IO_ADDR(EEDR), TMPL
        ; atomically set EEMPE and then EEPE
        atomic_start
        sbi     _SFR_IO_ADDR(EECR), EEMPE
        sbi     _SFR_IO_ADDR(EECR), EEPE
        atomic_end
2:      popd
        rnext

; EE! ( n e-addr -- ) write word to EEPROM
ext_opcode_eestore:
        ; wait for completion of previous write
1:      sbic    _SFR_IO_ADDR(EECR), EEPE
        rjmp    1b
        ; get value to write
        nip
        ; set address
        out     _SFR_IO_ADDR(EEARL), TOSL
        out     _SFR_IO_ADDR(EEARH), TOSH
; low byte
        ; read first--don't do a write cycle if the value is the same
        sbi     _SFR_IO_ADDR(EECR), EERE
        in      r20, _SFR_IO_ADDR(EEDR)
        cp      TMPL, r20
        breq    3f
        ; set data byte
        out     _SFR_IO_ADDR(EEDR), TMPL
        ; atomically set EEMPE and then EEPE
        atomic_start
        sbi     _SFR_IO_ADDR(EECR), EEMPE
        sbi     _SFR_IO_ADDR(EECR), EEPE
        atomic_end
        ; wait for completion of write
2:      sbic    _SFR_IO_ADDR(EECR), EEPE
        rjmp    2b
; high byte
3:      adiw    TOS, 1
        out     _SFR_IO_ADDR(EEARL), TOSL
        out     _SFR_IO_ADDR(EEARH), TOSH
        ; read first--don't do a write cycle if the value is the same
        sbi     _SFR_IO_ADDR(EECR), EERE
        in      r20, _SFR_IO_ADDR(EEDR)
        cp      TMPH, r20
        breq    4f
        ; set data byte
        out     _SFR_IO_ADDR(EEDR), TMPH
        ; atomically set EEMPE and then EEPE
        atomic_start
        sbi     _SFR_IO_ADDR(EECR), EEMPE
        sbi     _SFR_IO_ADDR(EECR), EEPE
        atomic_end
4:      popd
        rnext

; EE>RAM ( e-addr-from addr-to u -- ) copy u bytes from eeprom to ram
ext_opcode_eetoram:
        tstw    TOS
        breq    .noeeread
        movw    r20, TOS        ; save count
        ld      ZL, DSP+        ; Z is destination ptr
        ld      ZH, DSP+
        ld      TOSL, DSP+      ; start address
        ld      TOSH, DSP+
        add     r20, TOSL       ; add to count, giving end address
        adc     r21, TOSH
; TOS has current eeprom address, r21:r20 has eeprom end address, Z has ram addr
        ; wait for completion of previous write
1:      sbic    _SFR_IO_ADDR(EECR), EEPE
        rjmp    1b
.eereadloop:
        ; set address
        out     _SFR_IO_ADDR(EEARL), TOSL
        out     _SFR_IO_ADDR(EEARH), TOSH
        ; start eeprom read
        sbi     _SFR_IO_ADDR(EECR), EERE
        in      TMPL, _SFR_IO_ADDR(EEDR)
        st      Z+, TMPL
        ; advance eeprom address and compare to end address
        adiw    TOS, 1
        cp      r20, TOSL
        cpc     r21, TOSH
        brne    .eereadloop
; done
        popd
        rnext
.noeeread:
.noeewrite:
        drop3
        rnext

; RAM>EE ( addr-from e-addr-to u -- ) copy u bytes from ram to eeprom
ext_opcode_ramtoee:
        tstw    TOS
        breq    .noeewrite
        movw    r20, TOS        ; get count
        ld      TOSL, DSP+      ; get eeprom dest address
        ld      TOSH, DSP+
        ld      ZL, DSP+        ; get source address
        ld      ZH, DSP+
        add     r20, TOSL       ; add to count giving eeprom end address
        adc     r21, TOSH
.eewriteloop:
        ; wait for completion of previous write
        sbic    _SFR_IO_ADDR(EECR), EEPE
        rjmp    .eewriteloop
        ; set address
        out     _SFR_IO_ADDR(EEARL), TOSL
        out     _SFR_IO_ADDR(EEARH), TOSH
        ; get byte from ram
        ld      TMPL, Z+
        ; read byte from eeprom, skip if identical
        sbi     _SFR_IO_ADDR(EECR), EERE
        in      TMPH, _SFR_IO_ADDR(EEDR)
        cp      TMPL, TMPH
        breq    1f
        ; set data byte and do write
        out     _SFR_IO_ADDR(EEDR), TMPL
        atomic_start
        sbi     _SFR_IO_ADDR(EECR), EEMPE
        sbi     _SFR_IO_ADDR(EECR), EEPE
        atomic_end
        ; advance eeprom address and compare to end address
1:      adiw    TOS, 1
        cp      r20, TOSL
        cpc     r21, TOSH
        brne    .eewriteloop
        popd
        rnext

; EEFILL ( e-addr u c -- ) fill u bytes of eeprom starting at e-addr with c
ext_opcode_eefill:
        mov     TMPL, TOSL      ; get character
        ld      ZL, DSP+        ; get count
        ld      ZH, DSP+
        tstw    Z
        breq    .noeefill
        ld      TOSL, DSP+      ; get eeprom address
        ld      TOSH, DSP+
        add     ZL, TOSL        ; add to count, giving eeprom end address in Z
        adc     ZH, TOSH
.eefillloop:
        ; wait for completion of previous write
        sbic    _SFR_IO_ADDR(EECR), EEPE
        rjmp    .eefillloop
        ; set address
        out     _SFR_IO_ADDR(EEARL), TOSL
        out     _SFR_IO_ADDR(EEARH), TOSH
        ; read previous byte, skip if identical
        sbi     _SFR_IO_ADDR(EECR), EERE
        in      TMPH, _SFR_IO_ADDR(EEDR)
        cp      TMPL, TMPH
        breq    1f
        ; set data byte and do write
        out     _SFR_IO_ADDR(EEDR), TMPL
        atomic_start
        sbi     _SFR_IO_ADDR(EECR), EEMPE
        sbi     _SFR_IO_ADDR(EECR), EEPE
        atomic_end
        ; advance eeprom address and compare to end address
1:      adiw    TOS, 1
        cp      ZL, TOSL
        cpc     ZH, TOSH
        brne    .eefillloop
        popd
        rnext
.noeefill:
.noeetype:
        drop2
        rnext

; EETYPE ( e-addr u -- ) print string from eeprom
ext_opcode_eetype:
        tstw    TOS
        breq    .noeetype
        movw    LINKL, TOS      ; save count in a register not clobbered by C
        ld      TOSL, DSP+      ; start address
        ld      TOSH, DSP+
        add     LINKL, TOSL     ; add to count, giving end address
        adc     LINKH, TOSH
; TOS has current eeprom address, r21:r20 has eeprom end address
        ; wait for completion of previous write
1:      sbic    _SFR_IO_ADDR(EECR), EEPE
        rjmp    1b
        callc_prologue
.eetypeloop:
        ; set address
        out     _SFR_IO_ADDR(EEARL), TOSL
        out     _SFR_IO_ADDR(EEARH), TOSH
        movw    TSAV, TOS
        ; start eeprom read
        sbi     _SFR_IO_ADDR(EECR), EERE
        in      TOSL, _SFR_IO_ADDR(EEDR)
        ; print character
        lds     ZL, mio_putc    ; TODO this could take 1 cycle if mio_putc was saved in a register pair
        lds     ZH, mio_putc+1
        icall
        movw    TOS, TSAV
        ; advance eeprom address and compare to end address
        adiw    TOS, 1
        cp      LINKL, TOSL
        cpc     LINKH, TOSH
        brne    .eetypeloop
; done
        callc_restore
        popd
        rnext

.macro spi_tx, reg
        out     _SFR_IO_ADDR(SPDR), \reg        ; transmit byte
1:      in      TMPL, _SFR_IO_ADDR(SPSR)        ; wait until complete
        sbrs    TMPL, SPIF
        rjmp    1b
        in      \reg, _SFR_IO_ADDR(SPDR)        ; get received byte
.endm

; SPI0 ( n -- n ) transmit/receive 2 bytes on SPI port 0, MSB first
ext_opcode_spi0:
        cbi     _SFR_IO_ADDR(SPI0_CS_PORT), SPI0_CS_PIN ; assert chip select
        spi_tx  TOSH
        spi_tx  TOSL
        sbi     _SFR_IO_ADDR(SPI0_CS_PORT), SPI0_CS_PIN ; deassert chip select
        rnext

; CSPI0 ( c -- c ) transmit/receive 1 byte on SPI port 0
ext_opcode_cspi0:
        cbi     _SFR_IO_ADDR(SPI0_CS_PORT), SPI0_CS_PIN ; assert chip select
        spi_tx  TOSL
        sbi     _SFR_IO_ADDR(SPI0_CS_PORT), SPI0_CS_PIN ; deassert chip select
        clr     TOSH
        rnext

; SPI1 ( n -- n ) transmit/receive 2 bytes on SPI port 1, MSB first
ext_opcode_spi1:
        cbi     _SFR_IO_ADDR(SPI1_CS_PORT), SPI1_CS_PIN ; assert chip select
        spi_tx  TOSH
        spi_tx  TOSL
        sbi     _SFR_IO_ADDR(SPI1_CS_PORT), SPI1_CS_PIN ; deassert chip select
        rnext

; CSPI1 ( c -- c ) transmit/receive 1 byte on SPI port 1
ext_opcode_cspi1:
        cbi     _SFR_IO_ADDR(SPI1_CS_PORT), SPI1_CS_PIN ; assert chip select
        spi_tx  TOSL
        sbi     _SFR_IO_ADDR(SPI1_CS_PORT), SPI1_CS_PIN ; deassert chip select
        clr     TOSH
        rnext

; SPI2 ( n -- n ) transmit/receive 2 bytes on SPI port 2 (player 1 controller), MSB first
ext_opcode_spi2:
        cbi     _SFR_IO_ADDR(SPI2_CS_PORT), SPI2_CS_PIN ; assert chip select
        spi_tx  TOSH
        spi_tx  TOSL
        sbi     _SFR_IO_ADDR(SPI2_CS_PORT), SPI2_CS_PIN ; deassert chip select
        rnext

; CSPI2 ( c -- c ) transmit/receive 1 byte on SPI port 2 (player 1 controller)
ext_opcode_cspi2:
        cbi     _SFR_IO_ADDR(SPI2_CS_PORT), SPI2_CS_PIN ; assert chip select
        spi_tx  TOSL
        sbi     _SFR_IO_ADDR(SPI2_CS_PORT), SPI2_CS_PIN ; deassert chip select
        clr     TOSH
        rnext

; SPI3 ( n -- n ) transmit/receive 2 bytes on SPI port 3 (player 2 controller), MSB first
ext_opcode_spi3:
        cbi     _SFR_IO_ADDR(SPI3_CS_PORT), SPI3_CS_PIN ; assert chip select
        spi_tx  TOSH
        spi_tx  TOSL
        sbi     _SFR_IO_ADDR(SPI3_CS_PORT), SPI3_CS_PIN ; deassert chip select
        rnext

; CSPI3 ( c -- c ) transmit/receive 1 byte on SPI port 3 (player 2 controller)
ext_opcode_cspi3:
        cbi     _SFR_IO_ADDR(SPI3_CS_PORT), SPI3_CS_PIN ; assert chip select
        spi_tx  TOSL
        sbi     _SFR_IO_ADDR(SPI3_CS_PORT), SPI3_CS_PIN ; deassert chip select
        clr     TOSH
        rnext

; XMSTAT ( -- c ) read external memory (SPI0) status register
ext_opcode_xmgetstatus:
        pushd
        callc_prologue
        call    spi_eeprom_status
        callc_epilogue

; XMSTAT! ( c -- ) set external memory (SPI0) status register
ext_opcode_xmsetstatus:
        callc_prologue
        call    spi_eeprom_set_status
        popd
        callc_epilogue

; XMC@ ( x-addr -- c ) fetch byte from external memory (SPI0)
ext_opcode_xmcfetch:
        callc_prologue
        call    spi_eeprom_read_byte
        clr     TOSH
        callc_epilogue

; XMC! ( c x-addr -- ) store byte to external memory (SPI0)
ext_opcode_xmcstore:
        nip             ; get value into r22
        callc_prologue
        call    spi_eeprom_write_byte
        popd
        callc_epilogue

; RAM>XM ( addr-from x-addr-to u -- ) copy u bytes from ram to external memory (SPI0)
; void spi_eeprom_write_block(const uint8_t *src, uint16_t dst, uint16_t n)
; r25:r24 - src
; r23:r22 - dst
; r21:r20 - n
ext_opcode_ramtoxm:
        movw    r20, TOS        ; move byte count
        nip                     ; get dst addr into r23:r22
        popd                    ; get src addr into r25:r24
        callc_prologue
        call    spi_eeprom_write_block
        popd
        callc_epilogue

; XM>RAM ( x-addr-from addr-to u -- ) copy u bytes from external memory (SPI0) to ram
; void spi_eeprom_read_block(uint8_t *dst, uint16_t src, uint16_t n)
; r25:r24 - dst
; r23:r22 - src
; r21:r20 - n
ext_opcode_xmtoram:
        movw    r20, TOS        ; move byte count
        popd                    ; get dest addr into r25:r24
        nip                     ; get src addr into r23:r22
        callc_prologue
        call    spi_eeprom_read_block
        popd
        callc_epilogue

.macro  daa reg
        mov     ZL, \reg
        rcall   decimal_adj_add
        mov     \reg, ZL
.endm

.macro  das reg
        mov     ZL, \reg
        rcall   decimal_adj_sub
        mov     \reg, ZL
.endm

; #BLOCKS ( -- u ) number of blocks available for storage (internal and external)
ext_opcode_numblocks:
        pushd
        callc_prologue
        call    num_blocks
        clr     TOSH
        callc_epilogue

; #XMBLOCKS ( -- u ) number of blocks available for storage in external memory
; returns 0 if no external memory device detected
ext_opcode_numxmblocks:
        pushd
        callc_prologue
        call    num_xmem_blocks
        clr     TOSH
        callc_epilogue


; COPY-BLOCKS ( block-src block-dst u -- ) copy u blocks to block-dst, starting at block-src
; r24 - dstblk
; r22 - srcblk
; r20 - u
ext_opcode_copyblocks:
        mov     r20, TOSL       ; get count into r20
        ld      TOSL, DSP+      ; get dest into r24
        ld      TOSH, DSP+
        ld      TMPL, DSP+      ; get src into r22
        ld      TMPH, DSP+
        callc_prologue
        call    copy_blocks
        popd
        callc_epilogue

; FILL-BLOCKS ( block u c -- ) fill u blocks with byte c, starting at block
; void fill_blocks(uint8_t startblk, uint8_t nblocks, char c)
; r24 - startblk
; r22 - nblocks
; r20 - c
ext_opcode_fillblocks:
        mov     r20, TOSL       ; get byte value
        nip                     ; get count
        popd                    ; get start block number
        callc_prologue
        call    fill_blocks
        popd
        callc_epilogue

; BCD+ ( u1 u2 -- u ) add cells u1 and u2 as 4-digit binary-coded-decimal numbers
ext_opcode_bcdplus:
        nip
        add     TOSL, TMPL
        daa     TOSL
        adc     TOSH, TMPH
        daa     TOSH
        rnext

; BCD- ( u1 u2 -- u ) subtract cells u1 and u2 as 4-digit binary-coded-decimal numbers
ext_opcode_bcdminus:
        nip
        sub     TMPL, TOSL
        das     TMPL
        sbc     TMPH, TOSH
        das     TMPH
        movw    TOS, TMP
        rnext

; DBCD+ ( ud1 ud2 -- ud ) add double-cells ud1 and ud2 as 8-digit binary-coded-decimal numbers
ext_opcode_dbcdplus:
        ld      TMPL, DSP+      ; byte 0 (LSB) of xd2
        ld      TMPH, DSP+      ; byte 1 of xd2
        ld      r20, DSP+       ; byte 2 of xd1
        ld      r21, DSP+       ; byte 3 (MSB) of xd1
        ld      r18, DSP        ; byte 0 (LSB) of xd1
        ldd     r19, DSP+1      ; byte 1 of xd1
.dbcdadd:
        add     TMPL, r18
        daa     TMPL
        adc     TMPH, r19
        daa     TMPH
        adc     TOSL, r20       ; upper word of sum is already in TOS
        daa     TOSL
        adc     TOSH, r21
        daa     TOSH
        st      DSP, TMPL       ; store lower word of sum back to stack
        std     DSP+1, TMPH
        rnext

; MBCD+ ( ud1 u2 -- ud ) add single-cell (4-digit) binary-coded-decimal-number to
; double-cell (8-digit) binary-coded-decimal number
ext_opcode_mbcdplus:
        movw    r18, TOS        ; get the single-cell value
        movw    r20, TOS        ; and zero-extend it
        clr     r20
        clr     r21
        popd
        ld      TMPL, DSP       ; now the double-cell value is in r25-r22
        ldd     TMPH, DSP+1
        rjmp    .dbcdadd

; DBCD- ( ud1 ud2 -- ud ) subtract double-cells du1 and ud2 as 8-digit binary-coded-decimal numbers
ext_opcode_dbcdminus:
        movw    r20, TOS        ; bytes 2 and 3 (MSB) of xd2
        ld      r18, DSP+       ; byte 0 (LSB) of xd2
        ld      r19, DSP+       ; byte 1 of xd2
        ld      TOSL, DSP+      ; byte 2 of xd1
        ld      TOSH, DSP+      ; byte 3 (MSB) of xd1
        ld      TMPL, DSP       ; byte 0 (LSB) of xd1
        ldd     TMPH, DSP+1     ; byte 1 of xd1
        sub     TMPL, r18
        das     TMPL
        sbc     TMPH, r19
        das     TMPH
        sbc     TOSL, r20       ; upper word of difference is already in TOS
        das     TOSL
        sbc     TOSH, r21
        das     TOSH
        st      DSP, TMPL       ; store lower word of difference back to stack
        std     DSP+1, TMPH
        rnext

; (BCD>ASCII!) ( u c-addr -- c-addr+4 ) store 4-byte ASCII representation of
; binary-coded-decimal number u starting at c-addr. (most significant digit to
; least significant digit, as it would be displayed)
; Leaves the address of the byte after the last byte written. (to facilitate
; multiprecision operations)
ext_opcode_bcdtoascii:
        movw    Z, TOS
        nip                     ; value in TMPH/TMPL
        mov     r20, TMPH       ; first digit (most significant)
        swap    r20
        andi    r20, 0x0F
        subi    r20, -'0'
        st      Z+, r20
        andi    TMPH, 0x0F      ; second digit
        subi    TMPH, -'0'
        st      Z+, TMPH
        mov     r20, TMPL       ; third digit
        swap    r20
        andi    r20, 0x0F
        subi    r20, -'0'
        st      Z+, r20
        andi    TMPL, 0x0F      ; fourth digit (least significant)
        subi    TMPL, -'0'
        st      Z+, TMPL
        movw    TOS, Z
        rnext

dispatch_stubs .ext_opcode_, 256
        throw   FE_UNSUPPORTED_OPERATION

; decimal adjust after addition
; adapted from http://imrannazar.com/Binary-Coded-Decimal-Addition-on-Atmel-AVR
; and modified to require only one temporary register (and T flag)
; input:    number in r30, H and C flags set
; output:   adjusted number in r30
;           carry-out in C flag
; clobbers: r31 and T flag
decimal_adj_add:
; move H flag to T flag so it's not clobbered by arithmetic
        in      r31, _SFR_IO_ADDR(SREG)
        bst     r31, SREG_H
; if old carry was set, or >= 0x99, add 0x60 and set carry-out
        brcs    .daa_carry_out
        cpi     r30, 0x9A
        brcc    .daa_carry_out
; if (half-carry set) or (lower nibble > 9), add 6
        brts    .adj_lo
        mov     r31, r30
        andi    r31, 0xF        ; isolate lower nibble
        cpi     r31, 10
        brlo    .noadj_lo
.adj_lo:
        subi    r30, -0x06
.noadj_lo:
        clc
        ret
.daa_carry_out:
; if (half-carry set) or (lower nibble > 9), add 6
        brts    .adj_lo2
        mov     r31, r30
        andi    r31, 0xF        ; isolate lower nibble
        cpi     r31, 10
        brlo    .noadj_lo2
.adj_lo2:
        subi    r30, -0x06
.noadj_lo2:
; add 0x60 and set carry
        subi    r30, -0x60
        sec
        ret


; decimal adjust after subtraction
; adapted from Atmel app note AVR204
; input:    number in r30, H and C flags set
; output:   adjusted number in r30
;           carry-out in C flag
; clobbers: r31
decimal_adj_sub:
        clr     r31             ; clear BCD carry
        brcc    .sub_0          ; if carry-in not clear
        ldi     r31, 1          ; set BCD carry
.sub_0: brhc    .sub_1          ; if half carry not clear
        subi    r30, 0x06       ;    LSD = LSD - 6
.sub_1: sbrs	r31, 0          ; if previous carry not set
        rjmp    .sub_2          ;     return
        subi    r30 ,0x60       ; subtract 6 from MSD
        ldi     r31, 1          ; set underflow carry
        brcc    .sub_2          ; if carry not clear
        ldi     r31, 1          ;    clear underflow carry
.sub_2:
        lsr     r31             ; copy BCD carry to C flag
        ret


.catch_exception_handler:
; always unnest the exception frame, even if we got here via a BREAK
        brts    1f
; get this exception frame, so we can unnest it
        lds     ZL, forth_exception_frame
        lds     ZH, forth_exception_frame+1
        z_to_rsp
        pop     ZL
        pop     ZH
; pop and restore the address of the previous frame
        sts     forth_exception_frame, ZL
        sts     forth_exception_frame+1, ZH
; pop exception handler (we're already here!)
        pop     ZL
        pop     ZH
; pop return address
1:      popr
; pop previous input source
        pop     ZH      ; forth_inputpos
        pop     ZL      ; forth_inputlen
        pop     r19     ; forth_inputsrc+1
        pop     r18     ; forth_inputsrc
; pop data stack pointer
        pop     TMPL
        pop     TMPH
; if there's an error code, restore previous input source and stack ptr
        brtc    1f
        movw    DSP, TMP
        sbiw    DSP, 2
        ld      TOSL, DSP
        ldd     TOSH, DSP+1
        sts     forth_inputsrc, r18
        sts     forth_inputsrc+1, r19
        sts     forth_inputlen, ZL
        sts     forth_inputpos, ZH
; transfer error code to stack and resume
        movw    TOS, r20
        rnext
; if no exception, push 0 on stack and resume
1:      pushd
        movw    TOS, ZERO
        rnext
