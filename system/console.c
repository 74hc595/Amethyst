#include "console.h"
#include "video.h"
#include "colors.h"
#include "audio.h"
#include <string.h>

/* What to use for box drawing characters? */
#define BOX_CHARS_AMSCII

#if defined(BOX_CHARS_AMSCII)
#include "amscii.h"
#define CHAR_BOXH   acBOXH
#define CHAR_BOXV   acBOXV
#define CHAR_BOXUL  acBOXUL
#define CHAR_BOXUR  acBOXUR
#define CHAR_BOXLL  acBOXLL
#define CHAR_BOXLR  acBOXLR
#elif defined(BOX_CHARS_CP437)
#define CHAR_BOXH   196
#define CHAR_BOXV   179
#define CHAR_BOXUL  218
#define CHAR_BOXUR  191
#define CHAR_BOXLL  192
#define CHAR_BOXLR  217
#elif defined(BOX_CHARS_DEC) /* VT100 special graphics characters in upper 32 code points */
#define CHAR_BOXH   18
#define CHAR_BOXV   25
#define CHAR_BOXUL  13
#define CHAR_BOXUR  12
#define CHAR_BOXLL  14
#define CHAR_BOXLR  11
#else /* plain ASCII */
#define CHAR_BOXH   '-'
#define CHAR_BOXV   '|'
#define CHAR_BOXUL  '+'
#define CHAR_BOXUR  '+'
#define CHAR_BOXLL  '+'
#define CHAR_BOXLR  '+'
#endif


#define CELLSTART(c)          (screen_ptr-(((c)->split)?200:0))
#define STATUSBARSIZE(c)      (((c)->statusbar)?1:0)
#define NCOLS(c)              (((c)->eightycols)?80:40)
#define NROWS(c)              ((((c)->split)?5:CONSOLE_ROWS)-STATUSBARSIZE(c))
#define NCELLS(c)             (NCOLS(c)*NROWS(c))
#define CELLOFFSET(c,col,row) (((row)*NCOLS(c))+(col))
#define CELLADDR(c,col,row)   (CELLSTART(c)+CELLOFFSET(c,col,row))
#define CELLROWADDR(c,row)    CELLADDR((c),0,(row))
#define CELLENDADDR(c)        (CELLSTART(c)+NCELLS(c))
#define CURSORADDR(c)         CELLADDR((c),(c)->cx,(c)->cy)
#define CURSORCOLORADDR(c)    CELLADDR((c),(c)->cx,((c)->cy)+CONSOLE_ROWS)
#define CURSOROFFSET(c)       CELLOFFSET((c),(c)->cx,(c)->cy)
#define SCROLLWIDTH(c)        (((c)->mright-(c)->mleft)+1)
#define SCROLLHEIGHT(c)       (((c)->mbottom-(c)->mtop)+1)
#define MIN_COL(c)            0
#define MIN_ROW(c)            0
#define MAX_COL(c)            ((NCOLS(c))-1)
#define MAX_ROW(c)            ((NROWS(c))-1)
#define BLANK_CHAR            32  /* ASCII space */
#define COLOROFFSET           ((CONSOLE_ROWS)*(CONSOLE_COLS))
#define STATUSBARADDR(c)      CELLADDR((c),0,NROWS(c))

#define SETCELL(c,addr,ch) ({ \
  __typeof__(addr) _a = addr; \
  *_a = ch ^ ((c->rvs) ? 0x80 : 0x00); \
  if ((c)->color) { _a[COLOROFFSET] = globalcolor; } \
})


#ifndef MIN
#define MIN(a,b) ({ auto _a=(a); auto _b=(b); (_a<_b)?_a:_b; })
#endif
#ifndef MAX
#define MAX(a,b) ({ auto _a=(a); auto _b=(b); (_a>_b)?_a:_b; })
#endif
#define CLAMP_LOWER MAX
#define CLAMP_UPPER MIN

/* doesn't do any integer type promotion */
#define CLAMP(x,lo,hi) ({ auto _x=(x); __typeof__(_x) _lo=(lo); __typeof__(_x) _hi=(hi); (_x>_hi)?_hi:((_x<_lo)?_lo:_x); })

#define CLAMPTOSCREEN(c,x,y) \
  x = CLAMP_UPPER((x),MAX_COL(c)); \
  y = CLAMP_UPPER((y),MAX_ROW(c))

#define CLAMPTOSCREEN_SIGNED(c,x,y) \
  x = CLAMP((x),MIN_COL(c),MAX_COL(c)); \
  y = CLAMP((y),MIN_ROW(c),MAX_ROW(c))

static void console_slow_prompt(struct console *c);


/************** Cursor positioning **************/

/* Absolute positioning, does not respect margins */
void console_gotoxy(struct console *c, int8_t x, int8_t y) {
  CLAMPTOSCREEN_SIGNED(c,x,y);
  c->cx__ = x & 0x7F; /* clears pending wrap flag */
  c->cy = y;
}


/* Positioning relative to left margin, and restricted to scroll region */
void console_gotoscrollx(struct console *c, int8_t x) {
  x += c->mleft;
  c->cx__ = CLAMP(x, c->mleft, c->mright) & 0x7F; /* clears pending wrap flag */
}


/* Positioning relative to top margin */
void console_gotoscrolly(struct console *c, int8_t y) {
  y += c->mtop;
  c->cy = CLAMP(y, c->mtop,  c->mbottom);
}


/* Positioning relative to the left and top margins */
void console_gotoscrollxy(struct console *c, int8_t x, int8_t y) {
  console_gotoscrollx(c, x);
  console_gotoscrolly(c, y);
}


void console_gohome(struct console *c) {
  console_gotoxy(c, c->mleft, c->mtop);
}


void console_gotox(struct console *c, int8_t x) {
  console_gotoxy(c, x, c->cy);
}


void console_gotoy(struct console *c, int8_t y) {
  console_gotoxy(c, c->cx, y);
}


void console_movexy(struct console *c, int8_t dx, int8_t dy) {
  console_gotoxy(c, c->cx+dx, c->cy+dy);
}


void console_movex(struct console *c, int8_t dx) {
  int8_t x = CLAMP(c->cx+dx, c->mleft, c->mright);
  console_gotoxy(c, x, c->cy);
}


void console_movey(struct console *c, int8_t dy) {
  int8_t y = CLAMP(c->cy+dy, c->mtop, c->mbottom);
  console_gotoxy(c, c->cx, y);
}


void console_movesol(struct console *c) {
  console_gotoxy(c, c->mleft, c->cy);
}


void console_setymargins(struct console *c, uint8_t top, uint8_t bottom) {
  bottom = CLAMP_UPPER(bottom, MAX_ROW(c));
  if (top >= bottom) {
    top = MIN_ROW(c);
    bottom = MAX_ROW(c);
  }
  c->mtop = top;
  c->mbottom = bottom;
  console_gotoxy(c, c->mleft, top);
}


void console_setxmargins(struct console *c, uint8_t left, uint8_t right) {
  right = CLAMP_UPPER(right, MAX_COL(c));
  if (left >= right) {
    left = MIN_COL(c);
    right = MAX_COL(c);
  }
  c->mleft = left;
  c->mright = right;
  console_gotoxy(c, left, c->mtop);
}


void console_setmarginrect(struct console *c, rect r) {
  console_setxmargins(c, r.x, r.x+r.w-1);
  console_setymargins(c, r.y, r.y+r.h-1);
}


void console_resetmargins(struct console *c) {
  console_setxmargins(c, MIN_COL(c), MAX_COL(c));
  console_setymargins(c, MIN_ROW(c), MAX_ROW(c));
}



/************** Drawing **************/

void console_setch(struct console *c, char ch) {
  char *addr = CURSORADDR(c);
  SETCELL(c, addr, ch);
}


void console_fillrange(struct console *c, uint8_t x, uint8_t y, uint16_t n, char ch) {
  CLAMPTOSCREEN(c,x,y);
  auto addr = CELLADDR(c,x,y);
  auto endaddr = CLAMP_UPPER(addr+n, CELLENDADDR(c));
  while (addr != endaddr) { SETCELL(c, addr++, ch); }
}


void console_xfrmrange(struct console *c, uint8_t x, uint8_t y, uint16_t n, char (*xfrm)(char)) {
  CLAMPTOSCREEN(c,x,y);
  auto addr = CELLADDR(c,x,y);
  auto endaddr = CLAMP_UPPER(addr+n, CELLENDADDR(c));
  while (addr != endaddr) { SETCELL(c, addr, xfrm(*addr)); addr++; }
}


void console_hline(struct console *c, uint8_t x, uint8_t y, uint8_t w) {
  for (char *addr = CELLADDR(c,x,y); w; w--, addr++) {
    SETCELL(c, addr, CHAR_BOXH);
  }
}


void console_vline(struct console *c, uint8_t x, uint8_t y, uint8_t h) {
  uint8_t cols = NCOLS(c);
  for (char *addr = CELLADDR(c,x,y); h; h--, addr += cols) {
    SETCELL(c, addr, CHAR_BOXV);
  }
}


void console_box(struct console *c, rect r) {
  if (r.w <= 1) {
    return console_vline(c, r.x, r.y, r.h);
  } else if (r.h <= 1) {
    return console_hline(c, r.x, r.y, r.w);
  }
  uint8_t w = r.w;
  r.h--;
  char *p1 = CELLADDR(c, r.x, r.y);
  char *p2 = p1+(r.h * NCOLS(c));
  /* top and bottom left corners */
  SETCELL(c, p1++, CHAR_BOXUL);
  SETCELL(c, p2++, CHAR_BOXLL);
  /* top and bottom borders */
  while (--r.w != 1) {
    SETCELL(c, p1++, CHAR_BOXH);
    SETCELL(c, p2++, CHAR_BOXH);
  }
  /* top and bottom right corners */
  SETCELL(c, p1++, CHAR_BOXUR);
  SETCELL(c, p2, CHAR_BOXLR);
  /* left and right borders */
  uint8_t rowskip = NCOLS(c)-w;
  p1 += rowskip;
  while (--r.h) {
    SETCELL(c, p1, CHAR_BOXV);
    p1 += w-1;
    SETCELL(c, p1, CHAR_BOXV);
    p1 += rowskip+1;
  }
}



/************** Block operations (filling/clearing/scrolling) **************/

static void _fill(struct console *c, char *dst, char val, size_t len) {
  memset(dst, val ^ ((c->rvs) ? 0x80: 0x00), len);
  if (c->color) { memset(dst+COLOROFFSET, globalcolor, len); }
}


static void _move(struct console *c, char *dst, char *src, size_t len) {
  memmove(dst, src, len);
  if (c->color) { memmove(dst+COLOROFFSET, src+COLOROFFSET, len); }
}


static void _fillscr(struct console *c, char ch) {
  if (c->mleft == MIN_COL(c) && c->mright == MAX_COL(c)) {
    /* No left/right margins? Use one big memset. */
    _fill(c, CELLROWADDR(c,c->mtop), ch, SCROLLHEIGHT(c)*NCOLS(c));
  } else {
    /* Otherwise, need to do one memset per row. */
    for (int8_t r = c->mtop; r <= c->mbottom; r++) {
      _fill(c, CELLADDR(c,c->mleft,r), ch, SCROLLWIDTH(c));
    }
  }
}


/* reset cursor and clear scroll region (nonstandard) */
static void _formfeed(struct console *c) {
  c->cx__ = c->mleft; c->cy = c->mtop;
  _fillscr(c, BLANK_CHAR);
}


static void _clrline(struct console *c, int8_t y) {
  _fill(c, CELLADDR(c,c->mleft,y), BLANK_CHAR, SCROLLWIDTH(c));
}


void console_scrollup_between(struct console *c, int8_t top, int8_t bottom) {
  // _frame_sync(); /* wait for vsync to prevent tearing */
  if (c->mleft == MIN_COL(c) && c->mright == MAX_COL(c)) {
    /* if no left/right margins, we can do one big memmove */
    _move(c, CELLROWADDR(c,top), CELLROWADDR(c,top+1), (bottom-top)*NCOLS(c));
  } else {
    /* otherwise, need to go one row at a time */
    for (int8_t y = top; y < bottom; y++) {
      _move(c, CELLADDR(c,c->mleft,y), CELLADDR(c,c->mleft,y+1), SCROLLWIDTH(c));
    }
  }
  _clrline(c, bottom);
}


void console_scrollup(struct console *c) {
  console_scrollup_between(c, c->mtop, c->mbottom);
}


void console_scrolldown(struct console *c) {
  console_scrolldown_between(c, c->mtop, c->mbottom);
}


void console_scrolldown_between(struct console *c, int8_t top, int8_t bottom) {
  // _frame_sync(); /* wait for vsync to prevent tearing */
  if (c->mleft == MIN_COL(c) && c->mright == MAX_COL(c)) {
    /* if no left/right margins, we can do one big memmove */
    _move(c, CELLROWADDR(c,top+1), CELLROWADDR(c,top), (bottom-top)*NCOLS(c));
  } else {
    /* otherwise, need to go one row at a time */
    for (int8_t y = bottom; y > top; y--) {
      _move(c, CELLADDR(c,c->mleft,y), CELLADDR(c,c->mleft,y-1), SCROLLWIDTH(c));
    }
  }
  _clrline(c, top);
}


void console_clrhome(struct console *c) {
  c->mtop     = MIN_ROW(c);
  c->mbottom  = MAX_ROW(c);
  c->mleft    = MIN_COL(c);
  c->mright   = MAX_COL(c);
  c->rvs      = 0; /* reverse video off */
  _formfeed(c);
}


void console_clrscroll(struct console *c) {
  c->cx__ = c->mleft;
  c->cy = c->mtop;
  _formfeed(c);
}


void console_clrline(struct console *c) {
  _clrline(c, c->cy);
}


void console_erase(struct console *c, uint8_t erasemode) {
  switch (erasemode) {
    case 0: /* erase from cursor to end of screen */
      _fill(c, CURSORADDR(c), BLANK_CHAR, NCELLS(c)-CURSOROFFSET(c));
      break;
    case 1: /* erase from beginning of screen to cursor */
      _fill(c, CELLSTART(c), BLANK_CHAR, CURSOROFFSET(c)+1);
      break;
    case 2: /* erase entire screen */
      _fill(c, CELLSTART(c), BLANK_CHAR, NCELLS(c));
      break;
  }
}


void console_eraseline(struct console *c, uint8_t erasemode) {
  switch (erasemode) {
    case 0: /* erase from cursor to end of line */
      _fill(c, CURSORADDR(c), BLANK_CHAR, NCOLS(c)-c->cx);
      break;
    case 1: /* erase from beginning of line to cursor */
      _fill(c, CELLROWADDR(c,c->cy), BLANK_CHAR, c->cx+1);
      break;
    case 2: /* erase entire line */
      _fill(c, CELLROWADDR(c,c->cy), BLANK_CHAR, NCOLS(c));
      break;
  }
}



/************** Cursor motions **************/

void console_linefeed(struct console *c) {
  if (++c->cy > c->mbottom) {
    c->cy = c->mbottom;
    if (c->scrollok) {
      if (c->slow) { console_slow_prompt(c); }
      console_scrollup_between(c, c->mtop, c->mbottom);
    }
  }
}


void console_lback(struct console *c) {
  c->cx__ = c->mright; /* clear pending wrap flag */
  if (c->cy <= c->mtop) {
    c->cx = c->mleft;
    c->cy = c->mtop;
  } else {
    c->cy--;
  }
}


void console_lfwd(struct console *c) {
  c->cx__ = c->mleft; /* clear pending wrap flag */
  console_linefeed(c);
}


void console_cfwd(struct console *c)
{
  /* Don't immediately wrap/scroll after writing a character to the last */
  /* column. Instead, wait until the next character is received before */
  /* outputting the newline. This is what the VT100 and compatible terminals */
  /* and terminal emulators do. */
  /* https://stackoverflow.com/questions/31360385/an-obscure-one-documented-vt100-soft-wrap-escape-sequence */
  if (c->cx == c->mright) {
    c->newline_pending = 1;
  } else if (++c->cx > c->mright) {
    console_lfwd(c);
  }
}


void console_cback(struct console *c) {
  c->newline_pending = 0;
  if (c->cx == c->mleft) {
    console_lback(c);
  } else {
    c->cx--;
  }
}


void console_backspace(struct console *c) {
  if (!c->newline_pending) { console_cback(c); }
  c->newline_pending = 0;
  console_setch(c, BLANK_CHAR);
}



/************** Printing **************/

void console_addch_raw(struct console *c, char ch) {
  /* If the last character printed exceeded the right boundary,
   * we have to go to a new line. */
  if (c->newline_pending) {
    console_lfwd(c);
  }
  console_setch(c, ch);
  console_cfwd(c);
}


void console_addch(struct console *c, char ch) {
  if (ch == '\r') {
    c->cx__ = c->mleft; /* clear pending wrap flag */
  } else if (ch == '\n') {
    console_lfwd(c);
  } else if (ch == '\b') {
    console_backspace(c);
  } else if (ch == '\f') {
    _formfeed(c);
  } else {
    console_addch_raw(c, ch);
  }
}


void console_addstr(struct console *c, const char *str) {
  char ch;
  while ((ch = *str++)) {
    console_addch(c, ch);
  }
}


void console_addstr_P(struct console *c, PGM_P str) {
  char ch;
  while ((ch = pgm_read_byte(str++))) {
    console_addch(c, ch);
  }
}


/* Does not respect margins */
void console_xyaddch(struct console *c, uint8_t x, uint8_t y, char ch) {
  if (x > MAX_COL(c)) { return; }
  if (y > MAX_ROW(c)) { return; }
  char *addr = CELLADDR(c,x,y);
  SETCELL(c, addr, ch);
}


/* Does not respect margins */
void console_xyaddstr(struct console *c, uint8_t x, uint8_t y, const char *str) {
  if (x > MAX_COL(c)) { return; }
  if (y > MAX_ROW(c)) { return; }
  auto dst = CELLADDR(c,x,y);
  char ch;
  while ((ch = *str++) && (x++ < NCOLS(c))) {
    SETCELL(c, dst++, ch);
  }
}


static void _xyaddstr_P(struct console *c, uint8_t x, uint8_t y, PGM_P str) {
  auto dst = CELLADDR(c,x,y);
  char ch;
  while ((ch = pgm_read_byte(str++)) && (x++ < NCOLS(c))) {
    SETCELL(c, dst++, ch);
  }
}


/* Does not respect margins */
void console_xyaddstr_P(struct console *c, uint8_t x, uint8_t y, PGM_P str) {
  if (x > MAX_COL(c)) { return; }
  if (y > MAX_ROW(c)) { return; }
  _xyaddstr_P(c, x, y, str);
}



/************** Properties **************/

uint8_t console_cx(struct console *c)            { return c->cx; }
uint8_t console_cy(struct console *c)            { return c->cy; }
uint8_t console_cols(struct console *c)          { return NCOLS(c); }
uint8_t console_rows(struct console *c)          { return NROWS(c); }
uint8_t console_mtop(struct console *c)          { return c->mtop; }
uint8_t console_mbottom(struct console *c)       { return c->mbottom; }
uint8_t console_mleft(struct console *c)         { return c->mleft; }
uint8_t console_mright(struct console *c)        { return c->mright; }
uint8_t console_scrollwidth(struct console *c)   { return SCROLLWIDTH(c); }
uint8_t console_scrollheight(struct console *c)  { return SCROLLHEIGHT(c); }
uint8_t console_maxcol(struct console *c)        { return MAX_COL(c); }
uint8_t console_maxrow(struct console *c)        { return MAX_ROW(c); }
bool console_scrollok(struct console *c)         { return c->scrollok; }
bool console_showcursor(struct console *c)       { return c->cursorphase != 0; }

void console_setscrollok(struct console *c, bool value)   { c->scrollok = value; }
void console_setshowcursor(struct console *c, bool value) { c->cursorphase = !!value; }



/************** Video sync and init **************/

void console_frame_sync_(struct console *c) {
  if (c->cursorphase == 0) {
    _frame_sync(); /* block until the next frame has been drawn */
  } else {
    // uint8_t highlight = (c->cursorphase++ & 0b10000) ? 0x00 : 0x80;
    c->cursorphase++;
    if (c->cursorphase == 0) { c->cursorphase = 1; }
    /* don't need to draw cursor for the second half of the blink period */
    if (c->cursorphase & 0b10000) {
      _frame_sync();
      return;
    }
    char *cursoraddr = CURSORADDR(c);

    /* draw cursor by inverting high bit (inverse video) */
    /* in color mode, draw the highlighted character using the current color */
    /* (instead of that character's color), indicating the color of the next */
    /* character to be typed. */
    if (!c->color) {
      *cursoraddr ^= 0x80;
      /* block until the next frame has been drawn */
      _frame_sync();
      /* restore the character at the cursor position to its original value */
      *cursoraddr ^= 0x80;
    } else {
      /* same as above, but draw the cursor using the current color */
      char *cursorcolor = CURSORCOLORADDR(c);
      char prevcolor = *cursorcolor;
      *cursorcolor = globalcolor;
      *cursoraddr ^= 0x80;
      _frame_sync();
      *cursoraddr ^= 0x80;
      *cursorcolor = prevcolor;
    }
  }
}


void _console_cursor_draw(struct console *c) {
  /* Only draw if cursor should be showing */
  if (c->cursorphase == 0 || (c->cursorphase & 0b10000)) { return; }
  *CURSORADDR(c) ^= 0x80;
}


void _console_cursor_erase(struct console *c) {
  /* Only un-invert the character under the cursor if it should be showing */
  if (c->cursorphase == 0 || (c->cursorphase & 0b10000)) { return; }
  *CURSORADDR(c) ^= 0x80;
}


void console_main_loop(struct console *c, void (*idlefn)(void), void (*newframefn)(void))
{
  _console_cursor_draw(c);
  while (1) {
    idlefn();
    if (bit_is_set(DDRB,1)) {
      DDRB &= ~_BV(1);
      _console_cursor_erase(c);/* update cursor tick */
      if (c->cursorphase != 0) {
        c->cursorphase++;
        if (c->cursorphase == 0) { c->cursorphase = 1; }
      }
      newframefn();
      _console_cursor_draw(c);
    }
  }
}


void console_delay(struct console *c, uint16_t nframes) {
  while (nframes-- && console_frame_sync(c) && (get_key_ascii(&c->kr) != 3)) {}
}


void console_init(struct console *c, void *cells, TILEMAP_PTR font)
{
  if (c->color) {
    init_40x25_color_text_mode(cells, font);
    globalcolor = TEXT_COLOR_WHITE;
  } else if (c->eightycols) {
    init_80x25_mono_text_mode(cells, font);
  } else {
    init_40x25_mono_text_mode(cells, font);
  }
  console_clrhome(c);
  video_start();
}



/************** Keyboard **************/

int console_getch(struct console *c, bool chk) {
  if (chk) {
    return key_down_pending(&c->kr);
  } else {
    while (console_frame_sync(c)) {
      char key = get_key_ascii(&c->kr);
      if (key) {
        console_keyclick(key);
        /* every keypress resets the cursor blink phase */
        /* (if cursor is visible) */
        if (c->cursorphase) { c->cursorphase = 1; }
        return key;
      }
    }
  }
}


static void console_slow_prompt(struct console *c) {
  uint8_t k = console_getch(c, 0);
  if (k == ' ') { _formfeed(c); }
  else if (k == '\e') { c->slow = 0; }
}


void console_statusbar_fill(struct console *c, char ch) {
  if (!c->statusbar) { return; }
  memset(STATUSBARADDR(c), ch, NCOLS(c));
}


void console_statusbar_xaddstr_P(struct console *c, uint8_t x, PGM_P str) {
  if (!c->statusbar) { return; }
  if (x > MAX_COL(c)) { return; }
  _xyaddstr_P(c, x, NROWS(c), str);
}


void console_statusbar_xfrmrange(struct console *c, uint8_t x, uint16_t n, char (*xfrm)(char)) {
  auto addr = STATUSBARADDR(c);
  while (n) { SETCELL(c, addr, xfrm(*addr)); addr++; n--; }
}



/* alert beep parameters */
#define BEEP_FREQ     364       /* approx. 800 Hz */
#define BEEP_DURATION 6

void console_beep(void) {
  tone(BEEP_FREQ, BEEP_DURATION, SQUARE_WAVE);
}


void console_keyclick(uint8_t key) {
  uint16_t beepfreq;
  if (key == ' ' || key == '\b') {
    beepfreq = 630; /* C3 */
  } else if (key == '\n') {
    beepfreq = 438; /* C5 */
  } else {
    beepfreq = 523; /* middle C */
  }
  tone(beepfreq, 2, SQUARE_WAVE);
}
