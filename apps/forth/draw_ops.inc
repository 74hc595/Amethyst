; values for tilemap_hh:
; 000xxxx0 text (tilemap in ROM, lower 64KB)
; 000xxxx1 text (tilemap in ROM, upper 64KB)
; 001xxxxX 256 color bitmap
; 010xxxxX 16 color bitmap
; 011xxxxX 4 color bitmap
; 100xxxxX text (tilemap in RAM)
; 101xxxxX mono bitmap

.macro invoke_drawop n
        lds     ZL, tilemap_hh  ; dispatch based format
        ori     ZL, DRAW_OP_\n  ; or in the operation number
        ldi     ZH, pm_hi8(draw_op_table)
        ijmp
.endm

.equ DRAW_OP_xres,     0<<1
.equ DRAW_OP_clear,    1<<1
.equ DRAW_OP_getcolor, 2<<1
.equ DRAW_OP_setcolor, 3<<1
.equ DRAW_OP_plot,     4<<1
.equ DRAW_OP_pset,     5<<1
.equ DRAW_OP_vlin,     6<<1
.equ DRAW_OP_hlin,     7<<1
.equ DRAW_OP_rect,     8<<1
.equ DRAW_OP_line,     9<<1

.macro draw_op, fmtprefix, op
        rjmp    opcode_\fmtprefix\op
        rjmp    opcode_\fmtprefix\op
.endm

.macro draw_op_invalid
        rjmp    invalid_drawop
        rjmp    invalid_drawop
.endm

.macro draw_ops, fmtprefix
        draw_op \fmtprefix, xres        ; Draw operation 0: push screen width
        draw_op \fmtprefix, clear       ; Draw operation 1: clear screen
        draw_op \fmtprefix, getcolor    ; Draw operation 2: get current color
        draw_op \fmtprefix, setcolor    ; Draw operation 3: set current color
        draw_op \fmtprefix, plot_chk    ; Draw operation 4: set pixel to current color
        draw_op \fmtprefix, pset_chk    ; Draw operation 5: set color of pixel
        draw_op \fmtprefix, vlin_chk    ; Draw operation 6: vertical line
        draw_op \fmtprefix, hlin_chk    ; Draw operation 7: horizontal line
        draw_op \fmtprefix, rect_chk    ; Draw operation 8: filled rectangle
        draw_op \fmtprefix, line_chk    ; Draw operation 9: any-direction line
        .rept 6
        draw_op_invalid
        .endr
.endm

.align 9
draw_op_table:
        draw_ops t      ; 0b000xxxxX: text mode
        draw_ops h      ; 0b001xxxxX: 256 color bitmap mode
        draw_ops m      ; 0b010xxxxX: 16 color bitmap mode
        draw_ops l      ; 0b011xxxxX: 4 color bitmap mode
        draw_ops t      ; 0b100xxxxX: text mode
        draw_ops b      ; 0b101xxxxX: mono bitmap mode

opcode_tplot_chk:
opcode_thlin_chk:
opcode_tvlin_chk:
opcode_trect_chk:
opcode_tline_chk:
invalid_drawop:
        throw   FE_UNSUPPORTED_OPERATION
