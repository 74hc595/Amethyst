#ifndef ERROR_CODES_H_
#define ERROR_CODES_H_

/**
 * Most of the standard ANS Forth exception codes apply to this implementation,
 * some do not. Some irrelevant codes are repurposed. Some standard codes are
 * unassigned and/or to be implemented.
 */
#define FORTH_ERROR_CODES \
X(FE_NO_ERROR,                0, "") \
X(FE_ABORT,                  -1, "ABORT") \
X(FE_ABORTQ,                 -2, "ABORT\"") \
X(FE_DSTACK_OVERFLOW,        -3, "stack overflow") \
X(FE_DSTACK_UNDERFLOW,       -4, "stack underflow") \
X(FE_RSTACK_OVERFLOW,        -5, "return stack overflow") \
X(FE_RSTACK_UNDERFLOW,       -6, "return stack underflow") \
X(FE_BRANCH_OUT_OF_RANGE,    -7, "branch out of range") /* ANS: "do-loops nested too deeply during execution" */ \
X(FE_NAMESPACE_OVERFLOW,     -8, "name space overflow") \
X(FE_CODESPACE_OVERFLOW,     -9, "code space overflow") /* ANS: "invalid memory address" */ \
X(FE_DIVIDE_BY_ZERO,        -10, "division by zero") \
X(FE_DICT_ENTRY_TOO_LONG,   -11, "dictionary entry too long") /* ANS: "result out of range" */ \
X(FE_ARG_TYPE_MISMATCH,     -12, "argument type mismatch") \
X(FE_UNDEFINED_WORD,        -13, "undefined word") \
X(FE_INTEPRET_C_ONLY_WORD,  -14, "interpreting a compile-only word") \
X(FE_INVALID_FORGET,        -15, "invalid FORGET") \
X(FE_ZERO_LENGTH_NAME,      -16, "attempt to use zero-length string as name") \
X(FE_NUMERIC_STR_OVERFLOW,  -17, "pictured numeric output string overflow") \
X(FE_PARSED_STR_OVERFLOW,   -18, "parsed string overflow") \
X(FE_NAME_TOO_LONG,         -19, "definition name too long") \
X(FE_READ_ONLY,             -20, "write to a read-only location") \
X(FE_UNSUPPORTED_OPERATION, -21, "unsupported operation") \
X(FE_CTRL_STRUCT_MISMATCH,  -22, "control structure mismatch") \
X(FE_ALIGNMENT,             -23, "address alignment exception") \
XUNUSED(-24) /* ANS: "invalid numeric argument" */ \
X(FE_RSTACK_IMBALANCE,      -25, "return stack imbalance") \
X(FE_COMPILE_I_ONLY_WORD,   -26, "compiling an interpret-only word") /* ANS: "loop parameters unavailable" */ \
XUNUSED(-27) /* ANS: "invalid recursion" */ \
X(FE_USER_INTERRUPT,        -28, "user interrupt") \
XUNUSED(-29) /* ANS: "compiler nesting" */ \
XUNUSED(-30) /* ANS: "obsolescent feature" */ \
X(FE_INVALID_TOBODY,        -31, ">BODY used on non-CREATEd definition") \
X(FE_INVALID_NAME_ARG,      -32, "invalid name argument") /* e.g. using TO on a non-value */ \
XUNUSED(-33) /* ANS: "block read exception" */ \
XUNUSED(-34) /* ANS: "block write exception" */ \
X(FE_INVALID_BLOCK_NUMBER,  -35, "invalid block number") \
XUNUSED(-36) /* ANS: "invalid file position" */ \
XUNUSED(-37) /* ANS: "file I/O exception" */ \
XUNUSED(-38) /* ANS: "non-existent file" */ \
XUNUSED(-39) /* ANS: "unexpected end of file" */ \
XUNUSED(-40) /* ANS: "invalid BASE for floating point conversion" */ \
X(FE_LOSS_OF_PRECISION,     -41, "loss of precision") \
XUNUSED(-42) /* ANS: "floating-point divide by zero" */ \
XUNUSED(-43) /* ANS: "floating-point result out of range" */ \
XUNUSED(-44) /* ANS: "floating-point stack overflow" */ \
XUNUSED(-45) /* ANS: "floating-point stack underflow" */ \
XUNUSED(-46) /* ANS: "floating-point invalid argument" */ \
XUNUSED(-47) /* ANS: "compilation word list deleted" */ \
X(FE_INVALID_POSTPONE,      -48, "invalid POSTPONE") /* ANS: "invalid POSTPONE" */ \
XUNUSED(-49) /* AND: "search-order overflow" */ \
XUNUSED(-50) /* AND: "search-order underflow" */ \
XUNUSED(-51) /* AND: "compilation word list changed" */ \
XUNUSED(-52) /* AND: "control-flow stack overflow" */ \
XUNUSED(-53) /* AND: "exception stack overflow" */ \
XUNUSED(-54) /* AND: "floating-point underflow" */ \
XUNUSED(-55) /* AND: "floating-point unidentified fault" */ \
X(FE_QUIT,                  -56, "QUIT")/* AND: "QUIT" */ \

#undef X
#define XUNUSED(num)

#ifndef __ASSEMBLER__
#define X(ident,num,str) ident = num,
#include <stdint.h>
typedef int16_t forth_err_code;
enum {
FORTH_ERROR_CODES
};
#else /* __ASSEMBLER__ */
#define X(ident,num,str) .equ ident, (num) $
FORTH_ERROR_CODES
#endif

#endif
