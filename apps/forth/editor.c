/**
 * Screen editor.
 *
 * To minimize memory usage, this implementation does not use "block buffers."
 * Instead, data is loaded directly into the screen buffer, and parsed directly
 * from the screen buffer.
 */
#include "forth.h"
#include "minio.h"
#include "console.h"
#include "keys.h"
#include "video.h"

#define EDITOR_COLS       BLOCK_BYTES_PER_LINE
#define EDITOR_ROWS       LINES_PER_BLOCK
#define EDITOR_FIRST_COL  4
#define EDITOR_FIRST_ROW  2
#define EDITOR_LAST_ROW   ((EDITOR_FIRST_ROW)+(EDITOR_ROWS)-1)
#define EDITOR_LAST_COL   ((EDITOR_FIRST_COL)+(EDITOR_COLS)-1)

/* experimental: if 1, switch to a windowed split-screen "pane" when exiting */
/* editor */
#define EDITOR_CONSOLE_PANE 0

static void draw_header(const char * PROGMEM str, uint8_t blocknum, bool invert) {
  uint8_t scrwidth = 40;
  uint8_t headerlen = strlen_P(str)+1;
  uint8_t x = (scrwidth-headerlen)/2; /* centered */
  console_setrvs(&fcon, invert);
  console_fillrange(&fcon, 0, 0, scrwidth, ' ');
  CONSOLE_SAVECURSOR(&fcon);
  console_gotoxy(&fcon, x, 0);
  Psp(str); Pu8(blocknum);
  CONSOLE_RESTORECURSOR(&fcon);
  console_setrvs(&fcon, 0);
}


static void activate_console_pane(void) {
  draw_header(PSTR("Block "), forth_scr, false);


#if EDITOR_CONSOLE_PANE
  /* 80 column editor? switch to right side pane */
  if (fcon.eightycols) {
    console_setymargins(&fcon, 0,  console_maxrow(&fcon));
    console_setxmargins(&fcon, 40, console_maxcol(&fcon));
  } else {
    /* 40 column editor? switch to bottom pane */
    console_setymargins(&fcon, LINES_PER_BLOCK+3, console_maxrow(&fcon));
    console_setxmargins(&fcon, 0,                 console_maxcol(&fcon));
  }
#else
  console_resetmargins(&fcon);
#endif
  console_setscrollok(&fcon, true);
  console_clrscroll(&fcon);
}


static void load_block_to_screen_buf(uint8_t blocknum) {
  draw_header(PSTR("Editing block "), blocknum, true);
  uint8_t scrwidth = console_cols(&fcon);
  uint16_t src = 0;
  char *dst = screen_ptr + (EDITOR_FIRST_ROW*scrwidth) + EDITOR_FIRST_COL;
  for (uint8_t r = 0; r < EDITOR_ROWS; r++) {
    load_block_range(dst, blocknum, src, EDITOR_COLS);
    src += EDITOR_COLS;
    dst += scrwidth;
  }
}


static void save_screen_buf_to_block(uint8_t blocknum) {
  draw_header(PSTR("Saving block "), blocknum, true);
  uint8_t scrwidth = console_cols(&fcon);
  char *src = screen_ptr + (EDITOR_FIRST_ROW*scrwidth) + EDITOR_FIRST_COL;
  uint16_t dst = 0;
  for (uint8_t r = 0; r < EDITOR_ROWS; r++) {
    save_block_range(src, blocknum, dst, EDITOR_COLS);
    src += scrwidth;
    dst += EDITOR_COLS;
  }
  draw_header(PSTR("Editing block "), blocknum, true);
}


#define INV(c) ((c)|0x80)
static const char editor_legend1[] PROGMEM = {
  'B', 'l', 'o', 'c', 'k', ':', ' ',
  INV('^'), INV('S'), 'a', 'v', 'e', ' ',
  INV('^'), INV('E'), 'x', 'e', 'c', 'u', 't', 'e', ' ',
  INV('^'), INV('N'), 'e', 'x', 't', ' ',
  INV('^'), INV('P'), 'r', 'e', 'v', ' ', 0
};
static const char editor_legend2[] PROGMEM = {
  ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  INV('^'), INV('Q'), 'u', 'i', 't', '&', 'S', 'a', 'v', 'e', ' ', ' ',
  INV('^'), INV('R'), 'e', 'v', 'e', 'r', 't', 0,
};
static const char editor_legend3[] PROGMEM = {
  'L', 'i', 'n', 'e', ':', ' ', ' ',
  INV('^'), INV('I'), 'n', 's', ' ', ' ',
  INV('^'), INV('D'), 'e', 'l', ' ', ' ',
  INV('^'), INV('C'), 'l', 'e', 'a', 'r', ' ', ' ', ' ',
  INV('^'), INV('F'), 'i', 'r', 's', 't', 0
};


void forth_40x25_console(void);
bool using_console(void);
cell edit_block(uint8_t blocknum)
{
  forth_scr = blocknum;
  if (forth_scr == 0 || forth_scr > num_blocks()) {
    forth_throw(FE_INVALID_BLOCK_NUMBER);
  } else if (!using_console()) {
    forth_throw(FE_UNSUPPORTED_OPERATION);
  }

  /* if in split-screen mode, need to exit */
  if (fcon.split) {
    forth_40x25_console();
  }
  forth_flags.editing = 1;
  console_clrhome(&fcon);

  /* draw border */
  console_box(&fcon, RECT(EDITOR_FIRST_COL-1, EDITOR_FIRST_ROW-1,
    EDITOR_COLS+2, EDITOR_ROWS+2));
  /* draw line numbers */
  for (uint8_t r = EDITOR_FIRST_ROW; r <= EDITOR_LAST_ROW; r++) {
    uint8_t linenum = r-EDITOR_FIRST_ROW;
    char onesdigit = (linenum < 10)  ? '0'+linenum : '0'+(linenum-10);
    char tensdigit = (linenum >= 10) ? '1' : ' ';
    console_xyaddch(&fcon, EDITOR_FIRST_COL-3, r, tensdigit);
    console_xyaddch(&fcon, EDITOR_FIRST_COL-2, r, onesdigit);
  }
  /* constrain cursor within the box */
  console_setscrollok(&fcon, false);
  console_setxmargins(&fcon, EDITOR_FIRST_COL, EDITOR_LAST_COL);
  console_setymargins(&fcon, EDITOR_FIRST_ROW, EDITOR_LAST_ROW);

  /* command legend */
  console_xyaddstr_P(&fcon, 0, 19, editor_legend1);
  console_xyaddstr_P(&fcon, 0, 20, editor_legend2);
  console_xyaddstr_P(&fcon, 0, 21, editor_legend3);

  load_block_to_screen_buf(forth_scr);

  while (1) {
    char key = console_getch(&fcon, 0);
    switch (key) {
      case 0:         break;
      case '\b':      console_backspace(&fcon); break;
      case KC_LEFT:   console_movex(&fcon, -1); break;
      case KC_RIGHT:  console_movex(&fcon, +1); break;
      case KC_DOWN:   console_movey(&fcon, +1); break;
      case KC_UP:     console_movey(&fcon, -1); break;
      case KC_HOME:   console_movesol(&fcon);   break;
      case KC_END:    console_gotox(&fcon, EDITOR_LAST_COL); break;
      case CTRL('f'):
        console_gohome(&fcon);
        break;
      case CTRL('i'):
        console_scrolldown_between(&fcon, console_cy(&fcon), EDITOR_LAST_ROW);
        console_movesol(&fcon);
        break;
      case CTRL('d'):
        console_scrollup_between(&fcon, console_cy(&fcon), EDITOR_LAST_ROW);
        console_movesol(&fcon);
        break;
      case CTRL('c'):
        console_clrline(&fcon);
        break;
      case CTRL('s'):
        save_screen_buf_to_block(forth_scr);
        break;
      case CTRL('n'):
        save_screen_buf_to_block(forth_scr);
        if (forth_scr == num_blocks()) { forth_scr=1; } else { forth_scr++; }
        goto loadblk;
      case CTRL('p'):
        save_screen_buf_to_block(forth_scr);
        if (forth_scr == 1) { forth_scr=num_blocks(); } else { forth_scr--; }
        goto loadblk;
      case CTRL('r'):
      loadblk:
        load_block_to_screen_buf(forth_scr);
        break;
      /* set the input source to this block, and QUIT */
      case CTRL('e'):
        save_screen_buf_to_block(forth_scr);
        goto exit_and_load;
      /* quit  */
      case CTRL('q'):
        save_screen_buf_to_block(forth_scr);
        goto exit_without_load;
      default:
        console_addch(&fcon, key);
        break;
    }
  }
exit_and_load:
  activate_console_pane();
  return forth_scr;
exit_without_load:
  activate_console_pane();
  return 0;
}
