#include "forth.h"
#include "defs.h"
#include <stddef.h>

#undef X
#define X(ident,num,str) static const char errstr_##ident[] PROGMEM = str;
FORTH_ERROR_CODES

#undef X
#define X(ident,num,str) [-ident] = errstr_##ident,

static const char * const forth_error_strs[57] PROGMEM = {
FORTH_ERROR_CODES
};


PGM_P forth_error_str(forth_err_code err) {
  if (err > 0 || err <= -(signed)(COUNT_OF(forth_error_strs))) {
    return NULL;
  } else {
    uint8_t idx = -err;
    return pgm_read_ptr(forth_error_strs+idx);
  }
}

