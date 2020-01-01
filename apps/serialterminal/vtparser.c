#include "vtparser.h"

#ifdef __AVR__
#include <avr/pgmspace.h>
#else
#define PROGMEM
#define pgm_read_ptr(x) *(x)
#endif

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define SUPPORT_C1_CODES 0
#define LOGGING          0

#if !LOGGING
#define VT_LOG(...)
#define STATE_NAMES 0
#else
#include <stdio.h>
#define VT_LOG(f,...) printf(f "\n", ##__VA_ARGS__)
#define STATE_NAMES 1
#endif


#if STATE_NAMES
#define SET_STATE_NAME(n) .name = #n,
#define STATE_NAME(s) (s)->name
#else
#define SET_STATE_NAME(n)
#define STATE_NAME(s) NULL
#endif


typedef void (*vt_state_entry)(struct vt_parser *);
typedef _Bool (*vt_state_event)(struct vt_parser *, uint8_t c);
struct vt_parser_state {
#if STATE_NAMES
  const char *name;
#endif
  vt_state_entry entry; /* Called upon state entry */
  vt_state_event event; /* Called for each character received in this state */
};


#define VT_INVOKE(fn, ...) if (fn) { fn(__VA_ARGS__); }

/* --- Actions --- */
static void vt_parser_clear(struct vt_parser *vtp) {
  VT_LOG("    !clear");
  /* Initialize all parameters to default placeholders */
  memset(vtp->params, 0, sizeof(vtp->params));
  vtp->num_params = 0;
  vtp->intermediate_char = 0;
}

static void vt_parser_print(struct vt_parser *vtp, uint8_t c) {
  VT_LOG("    !print 0x%02x %c", c, (c >= 0x20 && c <= 0x7E) ? c : 0);
  VT_INVOKE(vtp->print, c);
}

static void vt_parser_execute_c0_c1(struct vt_parser *vtp, uint8_t c) {
  VT_LOG("    !execute 0x%02x", c);
  VT_INVOKE(vtp->execute_c0_c1, c);
}

static void vt_parser_collect(struct vt_parser *vtp, uint8_t c) {
  VT_LOG("    !collect 0x%02x %c", c, (c >= 0x20 && c <= 0x7E) ? c : 0);
  vtp->intermediate_char = c;
}

static void vt_parser_param(struct vt_parser *vtp, uint8_t c) {
  VT_LOG("    !param 0x%02x %c", c, (c >= 0x20 && c <= 0x7E) ? c : 0);

  if (vtp->num_params >= VT_PARSER_MAX_PARAMS) {
    return;
  /* Semicolon separates parameters */
  } else if (c == ';') {
    vtp->num_params++;
  /* Not a semicolon? It's a decimal digit */
  } else {
    /* Append digit to the current parameter */
    uint8_t digit = c - '0';
    vt_param param_val = vtp->params[vtp->num_params];
    param_val *= 10;
    param_val += digit;
    vtp->params[vtp->num_params] = param_val;
  }
}

static void vt_parser_esc_dispatch(struct vt_parser *vtp, uint8_t c) {
  VT_LOG("    !esc_dispatch 0x%02x %c", c, (c >= 0x20 && c <= 0x7E) ? c : 0);
  VT_INVOKE(vtp->execute_esc, c);
}

static void vt_parser_csi_dispatch(struct vt_parser *vtp, uint8_t c) {
  VT_LOG("    !csi_dispatch 0x%02x %c", c, (c >= 0x20 && c <= 0x7E) ? c : 0);
  /* Final character implies the end of the last parameter */
  if (vtp->num_params < VT_PARSER_MAX_PARAMS) {
    vtp->num_params++;
  }
  VT_INVOKE(vtp->execute_csi, c);
}

static _Bool vt_parser_change_state(struct vt_parser *vtp, const struct vt_parser_state *next)
{
  vtp->current = next;
  VT_LOG(">%s", STATE_NAME(next));
  vt_state_entry entry = pgm_read_ptr(&(next->entry));
  if (entry) { entry(vtp); }
  return 1;
}



/* --- States --- */

#define VT_DEFINE_STATE(name_,entry_) \
  static _Bool name_##_event(struct vt_parser *vtp, uint8_t c); \
  static const struct vt_parser_state PROGMEM name_ = { SET_STATE_NAME(name_) .entry = entry_, .event = name_##_event }

/* Parser states */
VT_DEFINE_STATE(ground,              NULL);
VT_DEFINE_STATE(escape,              vt_parser_clear);
VT_DEFINE_STATE(escape_intermediate, NULL);
VT_DEFINE_STATE(csi_entry,           vt_parser_clear);
VT_DEFINE_STATE(csi_ignore,          NULL);
VT_DEFINE_STATE(csi_intermediate,    NULL);
VT_DEFINE_STATE(csi_param,           NULL);
VT_DEFINE_STATE(unsupported_string,  NULL); /* DCS, OSC, SOS, PM, APC sequences all ignored */
VT_DEFINE_STATE(dcs_ignore,          vt_parser_clear); /* unlike OSC/SOS/PM/APC, params need to be cleared at start of a DCS */



/* --- State event handlers --- */

static _Bool anywhere_event(struct vt_parser *vtp, uint8_t c)
{
  switch (c) {
    /* CAN and SUB cancel the current escape sequence */
    case 0x18:
    case 0x1A:
      vt_parser_execute_c0_c1(vtp, c);
      return vt_parser_change_state(vtp, &ground);
    /* ESC begins a new escape sequence */
    case 0x1B:
      return vt_parser_change_state(vtp, &escape);

#if SUPPORT_C1_CODES
    case 0x90: /* Device Control String - shorthand for ESC P */
      return vt_parser_change_state(vtp, &dcs_ignore);
    case 0x9B: /* Control Sequence Introducer - shorthand for ESC [*/
      return vt_parser_change_state(vtp, &csi_entry);
    case 0x9C: /* String Terminator - shorthand for ESC \ */
      return vt_parser_change_state(vtp, &ground);
    case 0x98: /* Start of String - shorthand for ESC X */
    case 0x9D: /* Operating System Command - shorthand for ESC ] */
    case 0x9E: /* Privacy Message - shorthand for ESC ^ */
    case 0x9F: /* Application Program Command - shorthand for ESC _ */
      return vt_parser_change_state(vtp, &unsupported_string);
#endif

    default:
#if SUPPORT_C1_CODES
      /* Other C1 codes cancel the current escape sequence */
      if (c >= 0x80 && c <= 0x9A) {
        vt_parser_execute_c0_c1(vtp, c);
        return vt_parser_change_state(vtp, &ground);
      }
#endif
      break;
  }
  return 0;
}


static _Bool ground_event(struct vt_parser *vtp, uint8_t c)
{
  // VT_LOG("  %s event 0x%02x %c", STATE_NAME(vtp->current), c, (c >= 0x20 && c <= 0x7E) ? c : 0);
  if (anywhere_event(vtp, c)) {
    return 0;
  } else if (c <= 0x1F) {
    vt_parser_execute_c0_c1(vtp, c);
  } else {
    vt_parser_print(vtp, c);
  }
  return 0;
}


static _Bool escape_event(struct vt_parser *vtp, uint8_t c)
{
  if (anywhere_event(vtp, c)) {
    return 0;
  } else switch (c) {
    case 0x50: /* Device Control String */
      return vt_parser_change_state(vtp, &dcs_ignore);
    case 0x5B: /* Control Sequence Introducer */
      return vt_parser_change_state(vtp, &csi_entry);
    case 0x58: /* Start of String */
    case 0x5D: /* Operating System Command */
    case 0x5E: /* Privacy Message */
    case 0x5F: /* Application Program Command */
      return vt_parser_change_state(vtp, &unsupported_string);
    case 0x7F: /* DEL - ignore */
      break;
    default:
      if (c >= 0x20 && c <= 0x2F) {
        /* start of intermediate string */
        vt_parser_collect(vtp, c);
        return vt_parser_change_state(vtp, &escape_intermediate);
      } else {
        /* escape sequence terminator; execute and return to ground state */
        vt_parser_esc_dispatch(vtp, c);
        return vt_parser_change_state(vtp, &ground);
      }
  }
  return 0;
}



static _Bool escape_intermediate_event(struct vt_parser *vtp, uint8_t c)
{
  if (anywhere_event(vtp, c)) {
    return 0;
  } else if (c <= 0x1F) {
    vt_parser_execute_c0_c1(vtp, c);
  } else if (c <= 0x2F) {
    vt_parser_collect(vtp, c);
  } else if (c <= 0x7E) {
    vt_parser_esc_dispatch(vtp, c);
    return vt_parser_change_state(vtp, &ground);
  }
  return 0;
}


static _Bool csi_entry_event(struct vt_parser *vtp, uint8_t c)
{
  if (anywhere_event(vtp, c)) {
    return 0;
  } else if (c <= 0x1F) {
    vt_parser_execute_c0_c1(vtp, c);
  } else if (c <= 0x2F) {
    vt_parser_collect(vtp, c);
    return vt_parser_change_state(vtp, &csi_intermediate);
  } else if (c == 0x3A) { /* colon is invalid in a CSI escape sequence */
    return vt_parser_change_state(vtp, &csi_ignore);
  } else if ((c >= 0x30 && c <= 0x39) || c == 0x3B) { /* digits and semicolon */
    vt_parser_param(vtp, c);
    return vt_parser_change_state(vtp, &csi_param);
  } else if (c >= 0x3C && c <= 0x3F) { /* < = > ? */
    vt_parser_collect(vtp, c);
    return vt_parser_change_state(vtp, &csi_param);
  } else if (c <= 0x7E) {
    /* CSI sequence terminator; execute and return to ground state */
    vt_parser_csi_dispatch(vtp, c);
    return vt_parser_change_state(vtp, &ground);
  }
  return 0;
}


static _Bool csi_ignore_event(struct vt_parser *vtp, uint8_t c)
{
  /* Discard the rest of the CSI sequence */
  if (anywhere_event(vtp, c)) {
    return 0;
  } else if (c <= 0x1F) {
    vt_parser_execute_c0_c1(vtp, c);
  } else if (c >= 0x40 && c <= 0x7E) {
    return vt_parser_change_state(vtp, &ground);
  }
  return 0;
}


static _Bool csi_intermediate_event(struct vt_parser *vtp, uint8_t c)
{
  if (anywhere_event(vtp, c)) {
    return 0;
  } else if (c <= 0x1F) {
    vt_parser_execute_c0_c1(vtp, c);
  } else if (c >= 0x20 && c <= 0x2F) {
    vt_parser_collect(vtp, c);
  } else if (c >= 0x30 && c <= 0x3F) { /* invalid in CSI escape sequence */
    return vt_parser_change_state(vtp, &csi_ignore);
  } else if (c >= 0x40 && c <= 0x7E) {
    /* CSI sequence terminator; execute and return to ground state */
    vt_parser_csi_dispatch(vtp, c);
    return vt_parser_change_state(vtp, &ground);
  }
  return 0;
}


static _Bool csi_param_event(struct vt_parser *vtp, uint8_t c)
{
  if (anywhere_event(vtp, c)) {
    return 0;
  } else if (c <= 0x1F) {
    vt_parser_execute_c0_c1(vtp, c);
  } else if (c >= 0x20 && c <= 0x2F) {
    vt_parser_collect(vtp, c);
    return vt_parser_change_state(vtp, &csi_intermediate);
  } else if ((c >= 0x30 && c <= 0x39) || c == 0x3B) { /* digits and semicolon */
    vt_parser_param(vtp, c);
  } else if (c <= 0x3F) { /* : < = > ? are invalid */
    return vt_parser_change_state(vtp, &csi_ignore);
  } else if (c >= 0x40 && c <= 0x7E) {
    /* CSI sequence terminator; execute and return to ground state */
    vt_parser_csi_dispatch(vtp, c);
    return vt_parser_change_state(vtp, &ground);
  }
  return 0;
}


/* Device Control Strings are ignored, but we need to treat this as a */
/* separate state so that the clear action can be performed upon entry. */
static _Bool dcs_ignore_event(struct vt_parser *vtp, uint8_t c) {
  return unsupported_string_event(vtp, c);
}

/* Ignore everything up to the String Terminator character. */
static _Bool unsupported_string_event(struct vt_parser *vtp, uint8_t c) {
  return anywhere_event(vtp, c);
}


/* --- Public API --- */
void vt_parser_init(struct vt_parser *vtp) {
  vt_parser_clear(vtp);
  vt_parser_change_state(vtp, &ground);
}


void vt_parser_process(struct vt_parser *vtp, uint8_t c) {
  const struct vt_parser_state *state = vtp->current;
  vt_state_event event = pgm_read_ptr(&(state->event));
  VT_LOG("  %s event 0x%02x %c", STATE_NAME(vtp->current), c, (c >= 0x20 && c <= 0x7E) ? c : 0);
  if (event) {
#if !SUPPORT_C1_CODES
    c &= 0x7F;
#endif
    event(vtp, c);
  }
}


vt_param vt_parser_get_param(struct vt_parser *vtp, uint8_t param_index, vt_param default_value) {
  if (param_index >= vtp->num_params) {
    return default_value;
  } else {
    /* If value is 0, return the supplied default. */
    vt_param value = vtp->params[param_index];
    return (value != 0) ? value : default_value;
  }
}
