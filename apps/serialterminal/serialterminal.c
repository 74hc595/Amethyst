/**
 * Mostly-VT100-and-ANSI-compatible serial terminal.
 * - 80 columns by 24 rows, monochrome
 * - 7-bit ASCII and DEC special graphics characters
 * - Only reverse video attribute supported
 * - Double-height/width characters not supported
 * - C1 control characters not supported
 * - VT52 compatibility mode not supported
 * - Passes vttest (20190710) suites 1, 2, 3, 5.3, 5.4, 5.9, 6.3, 6.4,
 *   9.7, 9.9, 10.1, and 11.5
 *
 * Also see "Minimum requirements for VT100 emulation":
 *   http://www.inwap.com/pdp10/ansicode.txt
 *
 * Another amazing and very complete resource: "Terminal Guide"
 *   https://terminalguide.netlify.com
 *
 * TODO: support some VT102 and VT220 features, like Insert Line (IL),
 * Delete Line (DL), Delete Character (DCH), Insert Mode (ILM), and Cursor
 * Visible (DECTCEM), as modern Linux systems will use these unless $TERM is
 * explicitly set to `vt100`.
 */

#include "app.h"

#include "console_macros.h"
#include "vtparser.h"
#include "dec_graphics_extra_8x8.h"
#include <stdlib.h>

struct terminal_option {
  uint8_t str_xpos;
  uint8_t num_values;
  const char * const *value_strs;
};

static const char str_baud_300[]    PROGMEM = "  300";
static const char str_baud_1200[]   PROGMEM = " 1200";
static const char str_baud_2400[]   PROGMEM = " 2400";
static const char str_baud_9600[]   PROGMEM = " 9600";
static const char str_baud_19200[]  PROGMEM = "19200";
static const char str_baud_38400[]  PROGMEM = "38400";
static const char str_baud_57600[]  PROGMEM = "57600";
static const char * const baud_rate_strs[SERIAL_BAUD_NUM_VALUES] PROGMEM = {
  str_baud_57600, str_baud_300, str_baud_1200, str_baud_2400, str_baud_9600, str_baud_19200, str_baud_38400
};

static const char str_databits_7[]  PROGMEM = "7";
static const char str_databits_8[]  PROGMEM = "8";
static const char * const databits_strs[SERIAL_DATA_BITS_NUM_VALUES] PROGMEM = {
  str_databits_8, str_databits_7
};

static const char str_parity_none[] PROGMEM = "N";
static const char str_parity_even[] PROGMEM = "E";
static const char str_parity_odd[]  PROGMEM = "O";
static const char * const parity_strs[SERIAL_PARITY_NUM_VALUES] PROGMEM = {
  str_parity_none, str_parity_even, str_parity_odd
};

static const char str_stopbits_1[]  PROGMEM = "1";
static const char str_stopbits_2[]  PROGMEM = "2";
static const char * const stopbits_strs[SERIAL_STOP_BITS_NUM_VALUES] PROGMEM = {
  str_stopbits_1, str_stopbits_2
};

/* does Enter send CR, LF, or CRLF? */
static const char str_enter_cr[]   PROGMEM = "CR  ";
static const char str_enter_lf[]   PROGMEM = "LF  ";
static const char str_enter_crlf[] PROGMEM = "CRLF";
static const char * const enter_strs[3] PROGMEM = {
  str_enter_cr, str_enter_lf, str_enter_crlf
};

/* does Backspace send DEL or BS? */
static const char str_bksp_del[]  PROGMEM = "DEL";
static const char str_bksp_bs[]   PROGMEM = "BS ";
static const char * const bksp_strs[2] PROGMEM = {
  str_bksp_del, str_bksp_bs
};

static const struct terminal_option terminal_options[] PROGMEM = {
  {  0, COUNT_OF(baud_rate_strs), baud_rate_strs },
  {  6, COUNT_OF(databits_strs),  databits_strs },
  {  7, COUNT_OF(parity_strs),    parity_strs },
  {  8, COUNT_OF(stopbits_strs),  stopbits_strs },
  { 10, COUNT_OF(enter_strs),     enter_strs },
  { 15, COUNT_OF(bksp_strs),      bksp_strs }
};

enum {
  OPT_BAUDRATE = 0,
  OPT_DATABITS = 1,
  OPT_PARITY = 2,
  OPT_STOPBITS = 3,
  OPT_NEWLINE = 4,
  OPT_BKSP = 5,
  NUM_TERMINAL_OPTIONS = COUNT_OF(terminal_options)
};
static uint8_t config_fields[NUM_TERMINAL_OPTIONS] = {0};


/* Current character attributes */
struct terminal_attrs {
  union {
    struct {
      _Bool charset        :1;  /* G0 or G1 character set; SI sets to 1 and SO sets to 0 */
      _Bool auto_wrap      :1;  /* autowrap on/off */
      _Bool origin_mode    :1;  /* cursor origin absolute/relative */
      _Bool cursor_key_mode:1;  /* DECCKM: send ANSI or application sequences for cursor keys? */
    };
    uint8_t value;
  };
};


/* VT100 supports 2 character sets, G0 and G1 */
#define NUM_CHARSETS  2
#define G0            0
#define G1            1

/* Character set characteristics. Currently only 3 are supported: */
/* US ASCII, British, and DEC special graphics */
struct character_set {
  union {
    struct {
      _Bool graphic_chars:1;
      _Bool british_pound:1;
    };
    uint8_t value;
  };
};
#define CHARSET_US          0
#define CHARSET_GFX         1
#define CHARSET_UK          2


/* Terminal state saved and restored with 'ESC 7'/'ESC 8' */
struct saved_terminal_state {
  uint8_t cx__; /* msb is saved pending wrap flag */
  uint8_t cy__; /* msb is saved reverse video flag */
  struct terminal_attrs attrs;
  struct character_set charsets[NUM_CHARSETS];
};


MAKE_CONSOLE_80COL(stdcon);

static PAGE_ALIGNED uint8_t   recvbuf[256];
static PAGE_ALIGNED uint8_t   ram_font[2048];
static struct terminal_attrs  attrs;
static struct character_set   charsets[NUM_CHARSETS];
static uint8_t tab_stops[10]; /* one bit per column */

/* Values affected by DECSC (ESC 7) and DECRC (ESC 8) */
static struct saved_terminal_state saved_state;

static void execute_print(uint8_t c);
static void execute_c0(uint8_t c);
static void execute_esc(uint8_t c);
static void execute_csi(uint8_t c);
static struct vt_parser vtp = {
  .print         = execute_print,
  .execute_c0_c1 = execute_c0,
  .execute_esc   = execute_esc,
  .execute_csi   = execute_csi
};


/* Global reverse-video flag (set with DECSCNM) */
/* Set: black characters on white background */
/* Clear: white characters on white background (default) */
/* This is accomplished by inverting the bits in the screen font, without */
/* affecting the characters in the console screen buffer. */
static void set_global_reverse_video(_Bool flag) {
  _Bool is_black_on_white = (ram_font[0] == 0xFF); /* character 0 should be blank */
  if (flag == is_black_on_white) { return; }
  uint8_t *p = ram_font;
  uint16_t n = sizeof(ram_font);
  while (n--) { *p++ ^= 0xFF; }
}


static void save_term_state(void) {
  saved_state.cx__ = stdcon.cx__; /* also saves pending wrap flag */
  saved_state.cy__ = stdcon.cy__; /* also saves reverse video flag */
  saved_state.attrs = attrs;
  saved_state.charsets[G0].value = charsets[G0].value;
  saved_state.charsets[G1].value = charsets[G1].value;
}


static void restore_term_state(void) {
  stdcon.cx__ = saved_state.cx__; /* also restores pending wrap flag */
  stdcon.cy__ = saved_state.cy__; /* also restores reverse video flag */
  charsets[G0].value = saved_state.charsets[G0].value;
  charsets[G1].value = saved_state.charsets[G1].value;
  /* If called again before another save_term_state(), reset to defaults. */
  saved_state.attrs.value = 0;
}


static void reset_term(void) {
  memset(tab_stops, 1, sizeof(tab_stops)); /* set tab stop on every 8th column */
  attrs.value = 0;
  charsets[G0].value = CHARSET_US;
  charsets[G1].value = CHARSET_GFX;
  set_global_reverse_video(0);
  save_term_state();
}


static void gotoxy_respecting_origin_mode(int8_t x, int8_t y) {
  if (!attrs.origin_mode) { gotoxy(x,y); } else { gotoscrollxy(x,y); }
}

static void gotoy_respecting_origin_mode(int8_t y) {
  if (!attrs.origin_mode) { gotoy(y); } else { gotoscrolly(y); }
}

static void getcursorpos_respecting_origin_mode(int8_t *x, int8_t *y) {
  *x = getx();
  *y = gety() - ((attrs.origin_mode) ? topmargin() : 0);
}



static _Bool is_tab_stop(uint8_t x) {
  return bit_is_set(tab_stops[x>>3], x&7);
}

static void set_tab(uint8_t x) {
  if (x < getcols()) { set_bit(tab_stops[x>>3], x&7); }
}

static void clear_tab(uint8_t x) {
  if (x < getcols()) { clear_bit(tab_stops[x>>3], x&7); }
}

static void clear_all_tabs(void) {
  memset(tab_stops, 0, sizeof(tab_stops));
}

static void tab(void) {
  /* search forward until we find a column with a tab stop or hit the right edge */
  uint8_t x = getx();
  const uint8_t limit = maxcol();
  if (x == limit) { return; } /* don't clear the pending wrap flag if already at right edge */
  while (x < limit && !is_tab_stop(++x)) {}
  gotox(x); /* this implicitly clears the pending wrap flag */
}

static void backtab(void) {
  /* search backward until we find a column with a tab stop or hit the left edge */
  uint8_t x = getx();
  while (x && !is_tab_stop(--x)) {}
  gotox(x);
}




static void execute_print(uint8_t c)
{
  struct character_set charset = charsets[attrs.charset];
  if (charset.graphic_chars && (c >= '_' && c <= '~')) {
    c -= '_';
  }
  if (charset.british_pound && c == '#') {
    c = 30; /* replace # with pound sign from special graphics chars */
  }
  if (!attrs.auto_wrap && (getx()+1 >= getcols())) {
    setch(c); /* doesn't wrap */
  } else {
    addch_raw(c);
  }
}


static void execute_c0(uint8_t c)
{
    switch (c) {
      case '\a': /* BEL */
        console_beep();
        break;
      case '\b': /* BS */
        cback();
        break;
      case 0x0A: /* LF, VT, and FF all print a linefeed */
      case 0x0B:
      case 0x0C:
        linefeed();
        break;
      case 0x0D: /* CR */
        gotox(0);
        break;
      case 0x0E: /* SO: enable character set G1 (special graphics) */
        attrs.charset = G1;
        break;
      case 0x0F: /* SI: enable character set G0 */
        attrs.charset = G0;
        break;
      case '\t': /* HT: horizontal tab */
        tab();
        break;
    }
}


static void execute_esc_with_intermediate(uint8_t c)
{
  /* DECALN: print alignment pattern - ESC # 8 */
  if (vtp.intermediate_char == '#' && c == '8') {
    memset(stdcon_cells, 'E', getrows()*getcols());
  }
  /* SCS: select G0 character set - ESC ( */
  /*      select G1 character set - ESC ) */
  else if (vtp.intermediate_char == '(' || vtp.intermediate_char == ')') {
    uint8_t charset_idx = vtp.intermediate_char - '(';
    uint8_t charset;
    switch (c) {
      /* VT100 supports five values: A, B, 0, 1, 2 */
      case '0': case '2': charset = CHARSET_GFX; break;
      case '1': case 'B': charset = CHARSET_US;  break;
      case 'A':           charset = CHARSET_UK;  break;
      default:            return;
    }
    charsets[charset_idx].value = charset;
  }
}


static void execute_esc(uint8_t c)
{
  if (vtp.intermediate_char) {
    return execute_esc_with_intermediate(c);
  }

  switch (c) {
    case '7': /* DECSC: save cursor position and attributes */
      save_term_state();
      return;
    case '8': /* DECRC: restore cursor position and attributes */
      restore_term_state();
      return;
    case 'E': /* NEL: next line */
      movesol();
      __attribute__((fallthrough));
    case 'D': /* IND: index */
      if (gety() == botmargin()) {
        scrollup();
      } else {
        movey(1);
      }
      return;
    case 'H': /* HTS: horizontal tab set */
      set_tab(getx());
      break;
    case 'M': /* RI: reverse index */
      if (gety() == topmargin()) {
        scrolldown();
      } else {
        movey(-1);
      }
      return;
    case 'c': /* RIS: reset */
      clrhome();
      reset_term();
      return;
    default:
      break;
  }
}


/* for SM and RM */
static void execute_csi_private(uint8_t c)
{
  if (vtp.intermediate_char != '?') {
    return;
  }
  if (c != 'h' && c != 'l') {
    return;
  }
  _Bool flag = (c == 'h');
  uint8_t param = vt_parser_get_param(&vtp,0,0);
  switch (param) {
    /* DECCKM: set cursor key mode (0=ANSI, 1=application) */
    case 1:
      attrs.cursor_key_mode = flag;
      break;
    /* DECCOLM: we can't actually change the screen width, */
    /* but we need to clear the screen and reset the margins */
    case 3:
      clrhome();
      break;
    /* DECSCNM: reverse display colors */
    case 5:
      set_global_reverse_video(flag);
      break;
    /* DECOM: set/reset origin mode. Home the cursor when set */
    case 6:
      attrs.origin_mode = flag;
      if (flag) { gotoxy_respecting_origin_mode(0, 0); }
      break;
    /* DECAWM: enable/disable autowrap at end of line */
    case 7:
      attrs.auto_wrap = flag;
      break;
  }
}


static void execute_csi(uint8_t c)
{
  if (vtp.intermediate_char) {
    return execute_csi_private(c);
  }

  switch (c) {
    case 'A': /* CUU: cursor up */
      movey(-vt_parser_get_param(&vtp,0,1));
      break;
    case 'B': /* CUD: cursor down */
      movey(vt_parser_get_param(&vtp,0,1));
      break;
    case 'C': /* CUF: cursor forward */
      movex(vt_parser_get_param(&vtp,0,1));
      break;
    case 'D': /* CUB: cursor back */
      movex(-vt_parser_get_param(&vtp,0,1));
      break;
    case 'E': /* CNL: cursor to next line */
      movey(vt_parser_get_param(&vtp,0,1));
      movesol();
      break;
    case 'F': /* CPL: cursor to previous line */
      movey(-vt_parser_get_param(&vtp,0,1));
      movesol();
      break;
    case 'G': case '`': /* HPA: horizontal position absolute */
      gotox(vt_parser_get_param(&vtp,0,1)-1); /* param is 1-indexed */
      break;
    case 'H': case 'f': { /* CUP: set cursor position */
      uint8_t y = vt_parser_get_param(&vtp,0,1);
      uint8_t x = vt_parser_get_param(&vtp,1,1);
      gotoxy_respecting_origin_mode(x-1, y-1); /* params are 1-indexed */
      break;
    }
    case 'I': { /* CHT: cursor forward tabulation */
      uint8_t ntabs = vt_parser_get_param(&vtp,0,1);
      while (ntabs--) { tab(); }
      break;
    }
    case 'J': /* ED: erase display */
      erase(vt_parser_get_param(&vtp,0,0));
      break;
    case 'K': /* EL: erase line */
      eraseline(vt_parser_get_param(&vtp,0,0));
      break;
    case 'S': { /* SU: scroll up */
      uint8_t nlines = vt_parser_get_param(&vtp,0,1);
      while (nlines--) { scrollup(); }
      break;
    }
    case 'T': { /* SD: scroll down */
      uint8_t nlines = vt_parser_get_param(&vtp,0,1);
      while (nlines--) { scrolldown(); }
      break;
    }
    case 'Z': { /* CBT: cursor backward tabulation */
      uint8_t ntabs = vt_parser_get_param(&vtp,0,1);
      while (ntabs--) { backtab(); }
      break;
    }
    case 'a': /* HPR: horizontal position relative */
      movex(vt_parser_get_param(&vtp,0,1));
      break;
    case 'c': /* identify */
      Psl("\x1B[?1;0c");
      break;
    case 'd': { /* VPA: vertical position absolute */
      uint8_t y = vt_parser_get_param(&vtp,0,1)-1; /* param is 1-indexed */
      gotoy_respecting_origin_mode(y);
      break;
    }
    case 'e': { /* VPR: vertical position relative */
      int8_t dy = vt_parser_get_param(&vtp,0,1)-1; /* param is 1-indexed */
      gotoy_respecting_origin_mode(gety()-topmargin()+dy);
      break;
    }
    case 'g': { /* TBC: tab clear */
      uint8_t cmd = vt_parser_get_param(&vtp,0,0);
      if (cmd == 0) {
        clear_tab(getx()); /* ESC [ 0 g - clear tab stop at current column */
      } else if (cmd == 3) {
        clear_all_tabs(); /* ESC [ 3 g - clear all tab stops */
      }
      break;
    }
    case 'm': /* SGR: set graphic rendition (Slope's Game Room?) */
      for (uint8_t i = 0; i < vtp.num_params; i++) {
        uint8_t attr = vt_parser_get_param(&vtp,i,0);
        /* Reverse video is the only attribute supported */
        if (attr == 0 || attr == 27) {
          setrvs(0);
        } else if (attr == 7) {
          setrvs(1);
        }
      }
      break;
    case 'n': { /* DSR: device status report */
      uint8_t param = vt_parser_get_param(&vtp,0,0);
      if (param == 5) {
        /* respond: terminal is OK */
        Psl("\x1B[0n");
      } else if (param == 6) {
        /* send cursor position */
        int8_t x=0, y=0; getcursorpos_respecting_origin_mode(&x, &y);
        Psl("\x1B["); Pu8(y+1); Pc(';'); Pu8(x+1); Pc('R');
      }
      break;
    }
    case 'r': { /* DECSTBM: set top and bottom margins */
      uint8_t top = vt_parser_get_param(&vtp,0,1);
      uint8_t bottom = vt_parser_get_param(&vtp,1,getrows());
      setymargins(top-1, bottom-1); /* params are 1-indexed */
      if (top < bottom) {
        gotoxy_respecting_origin_mode(0,0);
      }
      break;
    }
    default:
      break;
  }
}



static void terminal_init(void)
{
  /* Copy ROM font to RAM (we need to remap the box-drawing characters) */
  memcpy_P(ram_font, amscii_font_8x8, sizeof(ram_font));
  /* Load DEC special graphics characters into 0-31. */
  for (uint8_t i = 0; i < COUNT_OF(amscii_to_dec_gfx_table); i++) {
    offsetpair p = { .w = pgm_read_word(&amscii_to_dec_gfx_table[i]) };
    load_char_pattern(ram_font+p.dst, amscii_font_8x8+p.src, 0x00);
    load_char_pattern(ram_font+128+p.dst, amscii_font_8x8+p.src, 0xFF);
  }
  for (uint8_t i = 0; i < COUNT_OF(extra_dec_gfx_table); i++) {
    offsetpair p = { .w = pgm_read_word(&extra_dec_gfx_table[i]) };
    load_char_pattern_packed(ram_font+p.dst, dec_graphics_extra_8x8+(8*p.src), 0x00);
    load_char_pattern_packed(ram_font+128+p.dst, dec_graphics_extra_8x8+(8*p.src), 0xFF);
  }

  stdcon.statusbar = 1; /* don't use the 25th row */
  console_init(&stdcon, stdcon_cells, ram_font);

  reset_term();
  vt_parser_init(&vtp);

  /* Use the mio putc stream to write to the serial port. */
  /* (getc is disabled, because we handle receieved characters asynchronously) */
  SERIAL_MIO_SETUP_WRONLY();
}


static void wraparound_increment(uint8_t *n, uint8_t limit) {
  uint8_t val = *n;
  if (++val >= limit) { val = 0; }
  *n = val;
}

static void wraparound_decrement(uint8_t *n, uint8_t limit) {
  uint8_t val = *n;
  if (val) { val--; } else { val = limit-1; }
  *n = val;
}


#if 0
static void stress_test_ui(void)
{
  uint32_t rcvd_bytes = 1;
  uint32_t faults = 0;

  console_statusbar_xaddstr_P(&stdcon, 20, PSTR("stress test      bytes: __________      faults: __________  "));

  /* wait until we get our first byte */
  while (!serial_bytes_available()) {}
  uint8_t expected_next_char = serial_get_byte()+1;

  /* loop indefinitely, each received byte must be one greater than the previous */
  while (frame_sync()) {
    ultoa(faults, stdcon_cells+(80*24 + 20 + 48), 10);
    ultoa(rcvd_bytes, stdcon_cells+(80*24 + 20 + 24), 10);
    while (serial_bytes_available()) {
      uint8_t new_char = serial_get_byte();
      rcvd_bytes++;
      if (new_char != expected_next_char) { faults++; }
      expected_next_char = new_char+1;
      addch_raw(new_char);
    }
  }
}
#endif


static void config_ui(void)
{
  uint8_t field = 0;
  uint8_t field_maxval = 0;

  console_statusbar_fill(&stdcon, ' ');
  console_statusbar_xaddstr_P(&stdcon, 24, PSTR("up/down: change    left/right: advance    ENTER: connect"));

  _Bool prev_rvs = stdcon.rvs;
  while (frame_sync()) {
    switch (get_key_ascii(&stdcon.kr)) {
      case KC_LEFT:  wraparound_decrement(&field, NUM_TERMINAL_OPTIONS); break;
      case KC_RIGHT: wraparound_increment(&field, NUM_TERMINAL_OPTIONS); break;
      case KC_UP:    wraparound_increment(&config_fields[field], field_maxval); break;
      case KC_DOWN:  wraparound_decrement(&config_fields[field], field_maxval); break;
      case KC_ENTER:
      case KC_F1:    field = NUM_TERMINAL_OPTIONS; break; /* signal to end loop after next redraw */
    }
    /* draw status bar fields */
    for (uint8_t i = 0; i < NUM_TERMINAL_OPTIONS; i++) {
      struct terminal_option opt; memcpy_P(&opt, &terminal_options[i], sizeof(opt));
      if (i == field) { field_maxval = opt.num_values; }
      const char *valuestr = pgm_read_ptr(opt.value_strs+config_fields[i]);
      console_setrvs(&stdcon, (i == field));
      console_statusbar_xaddstr_P(&stdcon, opt.str_xpos, valuestr);
    }

    /* on exit, remove the cursor highlight */
    if (field >= NUM_TERMINAL_OPTIONS) { break; }
  }

  console_statusbar_fill(&stdcon, ' ');
  console_setrvs(&stdcon, prev_rvs);
}


static void config_apply(void)
{
  struct serial_port serialconfig = serial_port_with_options(
    config_fields[OPT_BAUDRATE],
    config_fields[OPT_PARITY],
    config_fields[OPT_DATABITS],
    config_fields[OPT_STOPBITS]
  );
  serial_init(serialconfig);
  serial_async(recvbuf);
}


static void idle(void) {
  /* Continuously process bytes as they are received */
  while (serial_bytes_available()) {
    uint8_t c = serial_get_byte();
    /* NUL and DEL are ignored outright */
    if (c == 0x00 || c == 0x7F) { continue; }
    /* Erase the cursor, because the screen contents may change arbitrarily */
    _console_cursor_erase(&stdcon);
    vt_parser_process(&vtp, c);
    stdcon.cursorphase = 1;
    _console_cursor_draw(&stdcon);
  }
}


static void send_delete(void) {
  Pc((config_fields[OPT_BKSP]) ? '\b' : '\x7F');
}

static void send_newline(void) {
  uint8_t bits = config_fields[OPT_NEWLINE]+1;
  if (bits & 1) { Pc(0x0D); }
  if (bits & 2) { Pc(0x0A); }
}

static void send_cursor_key(char c) {
  Pc('\x1B');
  Pc((attrs.cursor_key_mode) ? 'O' : '[');
  Pc(c);
}


static void newframe(void)
{
  uint8_t key = get_key_ascii(&stdcon.kr);
  if (key != 0) {
    switch (key) {
      case KC_BKSP:  send_delete(); break;
      case KC_ENTER: send_newline(); break;
      case KC_LEFT:  send_cursor_key('D'); break;
      case KC_RIGHT: send_cursor_key('C'); break;
      case KC_UP:    send_cursor_key('A'); break;
      case KC_DOWN:  send_cursor_key('B'); break;
      case KC_HOME:  send_cursor_key('H'); break;
      case KC_END:   send_cursor_key('F'); break;
      case KC_PGUP:  Psl("\x1B[5~"); break;
      case KC_PGDN:  Psl("\x1B[6~"); break;
      case KC_F1:    config_ui(); config_apply(); break;
      /* if high bit is 1, Ctrl is held down; translate to control character */
      default:       Pc((key&0x80) ? (key&31) : key); break;
    }
  }
}


noreturn static void terminal_run(_Bool use_defaults) {
  terminal_init();
  audio_init(); /* for audible bell */
  video_start();
  if (!use_defaults) { config_ui(); }
  config_apply();
  console_main_loop(&stdcon, idle, newframe);
}


APP_MAIN(serialterminal) {
  terminal_run(0);
}


APP_ALIAS(serialterminal,serialterminal_default) {
  terminal_run(1);
}
