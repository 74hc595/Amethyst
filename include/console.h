#ifndef CONSOLE_H_
#define CONSOLE_H_

/* Constants for struct console offsets and sizes, */
/* allowing interoperability with assembler code. */
/* (Wish there was a way to generate these automatically, but the best I can*/
/* do is duplicate them and add _Static_assert()s to make sure they're right) */
#define offsetof_console_cx               0
#define offsetof_console_cy               1
#define offsetof_console_mleft            2
#define offsetof_console_mtop             3
#define offsetof_console_mright           4
#define offsetof_console_mbottom          5
#define offsetof_console_flags            6
#define offsetof_console_newline_pending  offsetof_console_cx
#define offsetof_console_rvs              offsetof_console_cy
#define offsetof_console_statusbar        offsetof_console_mtop
#define offsetof_console_slow             offsetof_console_mright
#define offsetof_console_split            offsetof_console_mbottom
#define offsetof_console_cursorphase      offsetof_console_flags
#define offsetof_console_color            offsetof_console_flags
#define offsetof_console_scrollok         offsetof_console_flags
#define offsetof_console_eightycols       offsetof_console_flags
#define sizeof_console                    16
#define bitmask_console_cx                0x7F
#define bitmask_console_newline_pending   0x80
#define bitmask_console_cy                0x7F
#define bitmask_console_rvs               0x80
#define bitmask_console_mtop              0x7F
#define bitmask_console_statusbar         0x80
#define bitmask_console_mright            0x7F
#define bitmask_console_slow              0x80
#define bitmask_console_mbottom           0x7F
#define bitmask_console_split             0x80
#define bitmask_console_cursorphase       0x1F
#define bitmask_console_color             0x20
#define bitmask_console_scrollok          0x40
#define bitmask_console_eightycols        0x80


#ifndef __ASSEMBLER__
#include "keys.h"
#include "videodefs.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdnoreturn.h>
#include <avr/pgmspace.h>

#define CONSOLE_COLS  40
#define CONSOLE_ROWS  25
#define DEFAULT_FONT  NULL


struct console {
  /* Cursor position */
  union {
    struct {
      uint8_t cx:7;
      uint8_t newline_pending:1;
    };
    uint8_t cx__;
  };
  union {
    struct {
      uint8_t cy:7;
      uint8_t rvs:1; /* if 1, print with inverse video */
    };
    uint8_t cy__;
  };

  /* Scrolling region margins (and other flags) */
  uint8_t mleft;
  union {
    struct {
      uint8_t mtop:7;
      uint8_t statusbar:1; /* if 1, reserve the last row of the console for a status bar */
    };
    uint8_t mtop__;
  };
  union {
    struct {
      uint8_t mright:7;
      uint8_t slow:1; /* if 1, pause and wait for keypress before scrolling up */
    };
    uint8_t mright__;
  };
  union {
    struct {
      uint8_t mbottom:7;
      uint8_t split:1; /* if 1, adjust for split-screen mode */
    };
    uint8_t mbottom__;
  };
  /* Flags/state */
  union {
    struct {
      unsigned cursorphase:5; /* if 0, don't show cursor */
      unsigned color:1;
      unsigned scrollok:1;
      unsigned eightycols:1;
    };
    uint8_t flags__;
  };
  struct keyreader kr;
} __attribute__((packed));

_Static_assert(offsetof(struct console, cx__) == offsetof_console_cx, "");
_Static_assert(offsetof(struct console, cx__) == offsetof_console_newline_pending, "");
_Static_assert(offsetof(struct console, cy__) == offsetof_console_cy, "");
_Static_assert(offsetof(struct console, cy__) == offsetof_console_rvs, "");
_Static_assert(offsetof(struct console, mleft) == offsetof_console_mleft, "");
_Static_assert(offsetof(struct console, mtop__) == offsetof_console_mtop, "");
_Static_assert(offsetof(struct console, mtop__) == offsetof_console_statusbar, "");
_Static_assert(offsetof(struct console, mright__) == offsetof_console_mright, "");
_Static_assert(offsetof(struct console, mright__) == offsetof_console_slow, "");
_Static_assert(offsetof(struct console, mbottom__) == offsetof_console_mbottom, "");
_Static_assert(offsetof(struct console, mbottom__) == offsetof_console_split, "");
_Static_assert(offsetof(struct console, flags__) == offsetof_console_cursorphase, "");
_Static_assert(offsetof(struct console, flags__) == offsetof_console_color, "");
_Static_assert(offsetof(struct console, flags__) == offsetof_console_scrollok, "");
_Static_assert(offsetof(struct console, flags__) == offsetof_console_eightycols, "");
_Static_assert(sizeof(struct console) == sizeof_console, "");


#define MAKE_CONSOLE_MIO_HANDLERS_SC(name, storageclass) \
  storageclass void name##_mio_putc(char c)   { console_addch(&name,c); } \
  storageclass int  name##_mio_getc(char chk) { return console_getch(&name,chk); }

#define MAKE_CONSOLE_MIO_HANDLERS(name) \
  MAKE_CONSOLE_MIO_HANDLERS_SC(name, __attribute__((used)) static)

#define MAKE_CONSOLE_NO_CELLS(name,eightycol,colr) \
  static struct console name = { \
    .mright__ = (CONSOLE_COLS<<(eightycol))-1, \
    .mbottom__ = CONSOLE_ROWS-1, \
    .eightycols = eightycol, \
    .color = colr, \
    .scrollok = 1, \
    .cursorphase = 1 }; \
  MAKE_CONSOLE_MIO_HANDLERS(name)

#define MAKE_CONSOLE(name,eightycol,color) \
  MAKE_CONSOLE_NO_CELLS(name,eightycol,color) \
  static char name##_cells[CONSOLE_ROWS*(CONSOLE_COLS<<((eightycol)||(color)))] = {0}; \

#define MAKE_CONSOLE_40COL(name) MAKE_CONSOLE(name,0,0)
#define MAKE_CONSOLE_80COL(name) MAKE_CONSOLE(name,1,0)
#define MAKE_CONSOLE_COLOR(name) MAKE_CONSOLE(name,0,1)

#define MAKE_CONSOLE_40COL_NO_CELLS(name) MAKE_CONSOLE_NO_CELLS(name,0,0)
#define MAKE_CONSOLE_80COL_NO_CELLS(name) MAKE_CONSOLE_NO_CELLS(name,1,0)
#define MAKE_CONSOLE_COLOR_NO_CELLS(name) MAKE_CONSOLE_NO_CELLS(name,0,1)

#define CONSOLE_MIO_ACTIVATE(name) \
  mio_getc = name##_mio_getc; \
  mio_putc = name##_mio_putc;

#define CONSOLE_MIO_INIT(name) \
  CONSOLE_MIO_ACTIVATE(name) \
  console_init(&name, name##_cells, DEFAULT_FONT);

/* Waits for the start of a frame, and updates the console. */
void console_frame_sync_(struct console *c);
#define console_frame_sync(c) (({ console_frame_sync_(c); 1; }) || 1)

/* A callback-based approach, rather than "while (console_frame_sync(())" */
noreturn void console_main_loop(struct console *c, void (*idlefn)(void), void (*newframefn)(void));

/* Initializes the given console using the given font (which may be in ROM or
 * RAM), sets the appropriate video mode, and begins displaying the contents of
 * the cells array. Passing NULL uses the default 8x8 font in ROM.*/
void console_init(struct console *c, void *cells, TILEMAP_PTR font);

/* Clears the screen, returns the cursor to (0,0), and resets the margins
 * to the full size of the screen. */
void console_clrhome(struct console *c);

/* Clears the scrolling region, and returns the cursor to the upper left of
 * the scrolling region. */
void console_clrscroll(struct console *c);

/* Clears the current line and returns the cursor to the start of the line. */
void console_clrline(struct console *c);

/* erasemode = 0: erase from the cursor (inclusive) to the end of the screen.
 * erasemode = 1: erase from the start of the screen to the cursor (inclusive).
 * erasemode = 2: erase the entire screen.
 * The cursor does not move.
 * This call corresponds to the ANSI "Erase in Display" escape sequence.
 * Does not (currently) respect the left/right margins. */
void console_erase(struct console *c, uint8_t erasemode);

/* erasemode = 0: erase from the cursor (inclusive) to the end of the line.
 * erasemode = 1: erase from the start of the line to the cursor (inclusive).
 * erasemode = 2: erase the entire line.
 * The cursor does not move.
 * This call corresponds to the ANSI "Erase in Line" escape sequence.
 * Does not (currently) respect the left/right margins. */
void console_eraseline(struct console *c, uint8_t erasemode);

/* Fills n cells from the given position with a character.
 * The cursor does not move. */
void console_fillrange(struct console *c, uint8_t x, uint8_t y, uint16_t n, char ch);

/* Transforms the characters in the n cells from the given position by passing
 * them through a user-specified callback function. */
void console_xfrmrange(struct console *c, uint8_t x, uint8_t y, uint16_t n, char (*xfrm)(char));

/* Moves the cursor to the upper left corner of the scrolling region. */
void console_gohome(struct console *c);

/* Moves the cursor to the specified coordinates. 
 * These functions can be used to move the cursor outside of the margins. */
void console_gotoxy(struct console *c, int8_t x, int8_t y);
void console_gotox(struct console *c, int8_t x);
void console_gotoy(struct console *c, int8_t y);

/* Moves the cursor to the specified coordinates, relative to the upper left
 * of the scrolling region. These functions cannot be used to move the cursor
 * outside of the margins. */
void console_gotoscrollxy(struct console *c, int8_t x, int8_t y);
void console_gotoscrollx(struct console *c, int8_t x);
void console_gotoscrolly(struct console *c, int8_t y);

/* Moves the cursor by the specified x/y deltas.
 * This function can be used to move the cursor outside of the margins. */
void console_movexy(struct console *c, int8_t dx, int8_t dy);

/* Moves the cursor left/right by the specified number of columns.
 * The cursor does not move beyond the left/right margins. */
void console_movex(struct console *c, int8_t dx);

/* Moves the cursor up/down the specified number of lines.
 * The cursor does not move beyond the top/bottom margins. */
void console_movey(struct console *c, int8_t dy);

/* Moves the cursor to the start of the current line. */
void console_movesol(struct console *c);

/* Set the scrolling region's top and bottom margins. The cursor is moved to the
 * first column of the top margin. */
void console_setymargins(struct console *c, uint8_t top, uint8_t bottom);

/* Set the scrolling region's left and right margins. The cursor is moved to the
 * first column of the top margin. */
void console_setxmargins(struct console *c, uint8_t top, uint8_t bottom);

/* Resets the scrolling region's top margin to the top line of the screen and
 * its bottom margin to the bottom line of the screen. */
void console_resetmargins(struct console *c);

/* Scrolls the region between the top and bottom margins up one line.
 * A blank line is added at the bottom. The cursor is not moved. */
void console_scrollup(struct console *c);

/* Scrolls the region between the top and bottom rows (inclusive) up one line.
 * A blank line is added at the bottom. The cursor is not moved.
 * If top >= bottom, no scrolling is done, but the bottom line is cleared. */
void console_scrollup_between(struct console *c, int8_t top, int8_t bottom);

/* Scrolls the region between the top and bottom margins down one line.
 * A blank line is added at the top. The cursor is not moved. */
void console_scrolldown(struct console *c);

/* Scrolls the region between the top and bottom rows (inclusive) down one line.
 * A blank line is added at the top. The cursor is not moved.
 * If top <= bottom, no scrolling is done, but the top line is cleared. */
void console_scrolldown_between(struct console *c, int8_t top, int8_t bottom);

/* Moves the cursor to the end of the previous line, or to the first column
 * of the top margin if the top margin is exceeded. */
void console_lback(struct console *c);

/* Advances the cursor one line down and moves it to the start of the new line.
 * The screen is scrolled if the bottom margin is exceeded. */
void console_lfwd(struct console *c);

/* Advances the cursor one line down but does not return the cursor to the start
 * of the new line. The screen is scrolled if the bottom margin is exceeded. */
void console_linefeed(struct console *c);

/* Moves the cursor one character back, moving to the end of the previous line
 * if necessary. */
void console_cback(struct console *c);

/* Advances the cursor one character to the right, advancing to a new line
 * and/or scrolling the screen if necessary. */
void console_cfwd(struct console *c);

/* Moves the cursor one character back (moving to the end of the previous line
 * if necessary) then replaces the character under the cursor with a blank.
 * Correctly handles the special condition at the end of a line. */
void console_backspace(struct console *c);

/* Overwrites the character at the cursor position without moving it. */
void console_setch(struct console *c, char ch);

/* Prints a character at the cursor position and advances the cursor.
 * Carriage returns and newlines are interpreted. */
void console_addch(struct console *c, char ch);

/* Prints a character at the cursor position and advances the cursor.
 * Carriage returns and newlines are not interpreted. */
void console_addch_raw(struct console *c, char ch);

/* Prints a null-terminated string at the cursor position and advances the
 * cursor. The screen will be scrolled if necessary. */
void console_addstr(struct console *c, const char *str);

/* Prints a null-terminated string from program memory at the cursor position
 * and advances the cursor. The screen will be scrolled if necessary. */
void console_addstr_P(struct console *c, PGM_P str);

/* Prints a character at the specified position. The cursor is not advanced.
 * Carriage returns and newlines are not interpreted. */
void console_xyaddch(struct console *c, uint8_t x, uint8_t y, char ch);

/* Prints a null-terminated string at the given position. The string is clipped
 * at the end of the line, special characters are not interpreted, and the
 * cursor is not advanced. */
void console_xyaddstr(struct console *c, uint8_t x, uint8_t y, const char *str);

/* Prints a null-terminated string at the given position. The cursor is not
 * advanced and the screen is not scrolled. */
void console_xyaddstr_P(struct console *c, uint8_t x, uint8_t y, PGM_P str);

int console_getch(struct console *c, bool chk);

/* Returns the x coordinate of the cursor. */
uint8_t console_cx(struct console *c);

/* Returns the y coordinate of the cursor. */
uint8_t console_cy(struct console *c);

/* Returns the screen width in columns. */
uint8_t console_cols(struct console *c);

/* Returns the screen height in rows. */
uint8_t console_rows(struct console *c);

/* Returns the number of the screen's rightmost column. */
uint8_t console_maxcol(struct console *c);

/* Returns the number of the screen's bottom-most row. */
uint8_t console_maxrow(struct console *c);

/* Returns the width of the scrolling region in columns. */
uint8_t console_scrollwidth(struct console *c);

/* Returns the height of the scrolling region in rows. */
uint8_t console_scrollheight(struct console *c);

/* Returns the line number of the scrolling region's top margin. */
uint8_t console_mtop(struct console *c);

/* Returns the line number of the scrolling region's bottom margin. */
uint8_t console_mbottom(struct console *c);

/* Returns the column number of the scrolling region's left margin. */
uint8_t console_mleft(struct console *c);

/* Returns the column number of the scrolling region's right margin. */
uint8_t console_mright(struct console *c);

/* Returns true if moving the cursor off the end of the scrolling region */
/* (as the result of a newline or insertion of a character) causes the */
/* scrolling region to scroll up one line. */
bool console_scrollok(struct console *c);

/* Sets whether moving the cursor off the end of the scrolling region causes */
/* it to scroll up one line. */
void console_setscrollok(struct console *c, bool value);

/* Returns true if the visible cursor is enabled. */
bool console_showcursor(struct console *c);

/* Sets whether the cursor is visible. */
void console_setshowcursor(struct console *c, bool value);

/* Enables/disables reverse video for subsequent printed characters. */
#define console_setrvs(c,v)   (c)->rvs = !!(v)

/* Draws a horizontal line using the box-drawing characters. */
/* Nothing is drawn if w=0. */
void console_hline(struct console *c, uint8_t x, uint8_t y, uint8_t w);

/* Draws a vertical line using the box-drawing characters. */
/* Nothing is drawn if h=0. */
void console_vline(struct console *c, uint8_t x, uint8_t y, uint8_t h);

/* Emit a beep. Suitable for the ASCII BEL character. */
void console_beep(void);

/* Emit a key click sound. */
void console_keyclick(uint8_t key);


typedef union {
  struct {
    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
  } __attribute__((packed));
  uint32_t full;
} rect;
#define RECT(x,y,w,h) (rect){.full=((x)&0xFF)|(((uint32_t)((y)&0xFF))<<8)|((uint32_t)(((w)&0xFF))<<16)|(((uint32_t)((h)&0xFF))<<24)}

/* Draws a box using the box-drawing characters. */
/* The center of the box is not filled. */
/* Nothing is drawn if r.w=0 or r.h=0. */
/* If r.w=1, a vertical line is drawn. */
/* If r.h=1, a horizontal line is drawn. */
void console_box(struct console *c, rect r);

/* Sets the scrolling region to the rectangle with the given upper-left */
/* position and size. The cursor is moved to the upper left. */
void console_setmarginrect(struct console *c, rect r);

#define CONSOLE_SAVECURSOR(c) uint8_t __sx = console_cx(c), __sy = console_cy(c);
#define CONSOLE_RESTORECURSOR(c) console_gotoxy(c, __sx, __sy);


/* Low-level cursor drawing functions */
/* You don't need to call these if you're using console_frame_sync() */
void _console_cursor_draw(struct console *c);
void _console_cursor_erase(struct console *c);

/* Status bar. `statusbar` bit must be 1 */
void console_statusbar_fill(struct console *c, char ch);
void console_statusbar_xaddstr_P(struct console *c, uint8_t x, PGM_P str);
void console_statusbar_xfrmrange(struct console *c, uint8_t x, uint16_t n, char (*xfrm)(char));
#endif /* !__ASSEMBLER__ */

#endif
