; "crumb" is an obscure term for a 2-bit quantity
.section .progmem.assets
.balign 256
.global crumbquadtable
.type crumbquadtable, @object
crumbquadtable:
.rept 64
.byte 0b00000000, 0b01010101, 0b10101010, 0b11111111
.endr

.global crumbmasktable
.type crumbmasktable, @object
crumbmasktable:
.rept 64
.byte 0b11000000, 0b00110000, 0b00001100, 0b00000011
.endr

.global bitmasktable
.type bitmasktable, @object
bitmasktable:
.rept 32
.byte 0b10000000, 0b01000000, 0b00100000, 0b00010000
.byte 0b00001000, 0b00000100, 0b00000010, 0b00000001
.endr


; fragments of horizontal lines, used in 2-bit-per-pixel bitmap modes
; The table is padded out because it's easier to combine 2 nibbles into a byte
; (2 cycles: SWAP+OR) than to combine two crumbs into a nibble (3 cycles: LSL+LSL+OR)
.global lhlin_fragments
.type lhlin_fragments, @object
lhlin_fragments:
; Start bit:   0
; End bit:     0   1          2           3
.byte 0b11000000, 0b11110000, 0b11111100, 0b11111111, 0,0,0,0,0,0,0,0,0,0,0,0
; Start bit:   1
; End bit:     0   1          2           3
.byte 0b11110000, 0b00110000, 0b00111100, 0b00111111, 0,0,0,0,0,0,0,0,0,0,0,0
; Start bit:   2
; End bit:     0   1          2           3
.byte 0b11111100, 0b00111100, 0b00001100, 0b00001111, 0,0,0,0,0,0,0,0,0,0,0,0
; Start bit:   3
; End bit:     0   1          2           3
.byte 0b11111111, 0b00111111, 0b00001111, 0b00000011, 0,0,0,0,0,0,0,0,0,0,0,0
.space 192

; fragments of vertical lines, used i1 2-bit-per-pixel bitmap modes
; The table is padded out because it's easier to combine 2 nibbles into a byte
; (2 cycles: SWAP+OR) than to combine two three-bit quantities into a byte
; (4 cycles: LSL+LSL+LSL+OR)
.global bhlin_fragments
.type bhlin_fragments, @object
bhlin_fragments:
; Start bit:   0
; End bit:     0   1          2           3           4           5           6           7
.byte 0b10000000, 0b11000000, 0b11100000, 0b11110000, 0b11111000, 0b11111100, 0b11111110, 0b11111111, 0,0,0,0,0,0,0,0
; Start bit:   1
; End bit:     0   1          2           3           4           5           6           7
.byte 0b11000000, 0b01000000, 0b01100000, 0b01110000, 0b01111000, 0b01111100, 0b01111110, 0b01111111, 0,0,0,0,0,0,0,0
; Start bit:   2
; End bit:     0   1          2           3           4           5           6           7
.byte 0b11100000, 0b01100000, 0b00100000, 0b00110000, 0b00111000, 0b00111100, 0b00111110, 0b00111111, 0,0,0,0,0,0,0,0
; Start bit:   3
; End bit:     0   1          2           3           4           5           6           7
.byte 0b11110000, 0b01110000, 0b00110000, 0b00010000, 0b00011000, 0b00011100, 0b00011110, 0b00011111, 0,0,0,0,0,0,0,0
; Start bit:   4
; End bit:     0   1          2           3           4           5           6           7
.byte 0b11111000, 0b01111000, 0b00111000, 0b00011000, 0b00001000, 0b00001100, 0b00001110, 0b00001111, 0,0,0,0,0,0,0,0
; Start bit:   5
; End bit:     0   1          2           3           4           5           6           7
.byte 0b11111100, 0b01111100, 0b00111100, 0b00011100, 0b00001100, 0b00000100, 0b00000110, 0b00000111, 0,0,0,0,0,0,0,0
; Start bit:   6
; End bit:     0   1          2           3           4           5           6           7
.byte 0b11111110, 0b01111110, 0b00111110, 0b00011110, 0b00001110, 0b00000110, 0b00000010, 0b00000011, 0,0,0,0,0,0,0,0
; Start bit:   7
; End bit:     0   1          2           3           4           5           6           7
.byte 0b11111111, 0b01111111, 0b00111111, 0b00011111, 0b00001111, 0b00000111, 0b00000011, 0b00000001, 0,0,0,0,0,0,0,0



