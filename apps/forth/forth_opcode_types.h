#ifndef FORTH_OPCODE_TYPES_H_
#define FORTH_OPCODE_TYPES_H_

#define OP_TYPE_TO_FLAGS(n) ((n)&15)
#define OP_FLAGS_TYPE(fl)   ((fl)&15)
#define OP_2U   0 /* Two byte instruction with unsigned 8-bit argument */
#define OP_2X   1 /* Two byte instruction with unsigned 8-bit argument (hex) */
#define OP_2I   2 /* Two byte extended-opcode instruction */
#define OP_2F   3 /* 8-bit argument, forward relative branch */
#define OP_2B   4 /* 8-bit argument, backward relative branch */
#define OP_2C   5 /* Two byte CALL instruction */
#define OP_3S   6 /* Three byte instruction with signed 16-bit argument */
#define OP_3X   7 /* Three byte instruction with 16-bit address argument */
#define OP_STR  8 /* Inline string instruction */
#define OP_1    9 /* One byte instruction with no argument */

/* Visibility flags */
#define OP_VISIBLE      0x1
#define OP_COMPILEONLY  0x2
#define OP_USES_RSTACK  0x4

#endif
