/**
 * robotfindskitten
 */

#include "video.h"
#include "keys.h"
#include "colors.h"
#include "app.h"
#include "amscii.h"

#include <avr/pgmspace.h>
#include <string.h>
#include <stdlib.h>

#include "vanillaplusplus.nki.h"

/* our old friends */
#define STR_(x) #x
#define STR(x) STR_(x)
#define MIN(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define MAX(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

/* what kind of randoms do we want? */
/* TODO: avr-libc rand() pulls in multiplication and division routines */
/* replace wih LFSR? */
#define RAND()    rand()
#define SRAND(x)  srand(x)

/* want color, or 80-columns? */
#define COLOR   1

/* if 1, the item's character will be an ampersand iff the item is kitten  */
/* (for doing demos, etc) */
#define OBVIOUS_KITTEN_IS_OBVIOUS 0

/* screen size */
#if COLOR
#define NCOLS   40
#define MULTI_LINE_STATUS_MESSAGE 1
#else
#define NCOLS   80
#define MULTI_LINE_STATUS_MESSAGE 0
#endif
#define NROWS   25

#define NCHARS  (NROWS*NCOLS)
#if COLOR
#define SCRSIZ  NCHARS*2
#else
#define SCRSIZ  NCHARS
#endif

#if !MULTI_LINE_STATUS_MESSAGE
#undef MAX_NKI_LINES
#define MAX_NKI_LINES 1
#endif

/* playfield size and location */
#define PLAYFIELD_LEFT    1
#define PLAYFIELD_RIGHT   (NCOLS-2)
#define PLAYFIELD_TOP     (MAX_NKI_LINES+2)
#define PLAYFIELD_BOTTOM  (NROWS-2)

/* robot is a gray octothorpe by convention */
#define ROBOT_CHAR  '#'
#define ROBOT_COLOR TEXT_COLOR_WHITE

static const char PROGMEM version_str[] = "robotfindskitten 4.206900." STR(NUM_NKIS);

typedef union {
  struct {
    uint8_t chars[NROWS][NCOLS];
#if COLOR
    uint8_t colors[NROWS][NCOLS];
#endif
  };
  uint8_t bytes[SCRSIZ];
} screen;

static screen SCREEN;

struct item {
  uint8_t x;
  uint8_t y;
  uint8_t ch;
  uint8_t color;
  PGM_P description; /* null indicates kitten */
};

#define NITEMS  21
static struct item items[NITEMS+1];
#define robot  (items[NITEMS]) /* last item in list is robot */
#define kitten (items[0])      /* first item is kitten */

static _Bool seeded;

static struct keyreader kr = {0};


static void scr_clear(uint8_t color)
{
  memset(&SCREEN.chars, 0, NCHARS);
#if COLOR
  memset(&SCREEN.colors, color, NCHARS);
#else
  (void)color;
#endif
}


static void scr_puts_P(uint8_t x, uint8_t y, uint8_t color, PGM_P str)
{
  size_t len = strlen_P(str);
  memcpy_P(&SCREEN.chars[y][x], str, len);
#if COLOR
  memset(&SCREEN.colors[y][x], color, len);
#else
  (void)color;
#endif
}

static void scr_setchars_P(uint8_t x, uint8_t y, PGM_P str)
{
  size_t len = strlen_P(str);
  memcpy_P(&SCREEN.chars[y][x], str, len);
}

static void scr_setcolor(uint8_t x, uint8_t y, uint16_t len, uint8_t color)
{
#if COLOR
  memset(&SCREEN.colors[y][x], color, len);
#else
  (void)x; (void)y; (void)len; (void)color;
#endif
}

__attribute__((noinline)) static void scr_setchar(uint8_t x, uint8_t y, uint8_t ch, uint8_t color)
{
  SCREEN.chars[y][x] = ch;
#if COLOR
  SCREEN.colors[y][x] = color;
#else
  (void)color;
#endif
}

static void scr_box(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color)
{
  /* corners */
  scr_setchar(x1, y1, acBOXUL, color);
  scr_setchar(x2, y1, acBOXUR, color);
  scr_setchar(x1, y2, acBOXLL, color);
  scr_setchar(x2, y2, acBOXLR, color);
  /* top and bottom */
  for (uint8_t x = x1+1; x < x2; x++) {
    scr_setchar(x, y1, acBOXH, color);
    scr_setchar(x, y2, acBOXH, color);
  }
  /* left and right */
  for (uint8_t y = y1+1; y < y2; y++) {
    scr_setchar(x1, y, acBOXV, color);
    scr_setchar(x2, y, acBOXV, color);
  }
}


static void title_screen(void)
{
  scr_clear(TEXT_COLOR_WHITE);
  scr_puts_P(0, 0, TEXT_COLOR_WHITE, version_str);
#if COLOR
  scr_puts_P(0, 2, TEXT_COLOR_WHITE, PSTR(
    "Original version by the illustrious     "
    "Leonard Richardson (C) 1997, 2000       "
    "Written originally for the Nerth Pork   "
    "robotfindskitten contest"));
  scr_puts_P(0, 7, TEXT_COLOR_WHITE, PSTR(
    "This AVR version by the less illustrious"
    "Matt Sarnoff (C) 2019"
    ));

  scr_puts_P(0, 9, TEXT_COLOR_LT_BLUE, PSTR(
    amBOXUL "\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13" amBOXUR " "
    "\x1c In this game, you are robot (#).    \x1c\x7f"
    "\x1c Your job is to find kitten.         \x1c\x7f"
    "\x1c This task is complicated by the     \x1c\x7f"
    "\x1c existence of various things which   \x1c\x7f"
    "\x1c are not kitten.                     \x1c\x7f"
    "\x1c Robot must touch items to determine \x1c\x7f"
    "\x1c if they are kitten or not.          \x1c\x7f"
    "\x1c Arrow keys move robot.              \x1c\x7f"
    "\x1c The game ends when robotfindskitten \x1c\x7f"
    "\x1c or the Esc key is pressed.          \x1c\x7f"
    amBOXLL "\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13\x13" amBOXLR amBLKFL
    " \x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f\x7f"
    ));
  scr_setcolor(31, 10, 1, ROBOT_COLOR);
  scr_setchars_P(8, 23, PSTR("Press any key to start."));
#else
  scr_setchars_P(0, 2, PSTR("Original version by the illustrious Leonard Richardson (C) 1997, 2000"));
  scr_setchars_P(0, 3, PSTR("This AVR version by the less illustrious Matt Sarnoff (C) 2019"));
  scr_setchars_P(0, 5, PSTR("In this game, you are robot (#). Your job is to find kitten. This task"));
  scr_setchars_P(0, 6, PSTR("is complicated by the existence of various things which are not kitten."));
  scr_setchars_P(0, 7, PSTR("Robot must touch items to determine if they are kitten or not."));
  scr_setchars_P(0, 8, PSTR("Arrow keys move robot."));
  scr_setchars_P(0, 9, PSTR("The game ends when robotfindskitten or the Esc key is pressed."));
  scr_setchars_P(0, 11, PSTR("Press any key to start."));
#endif

  uint8_t color = 0;
  while (frame_sync()) {
    if (get_key_ascii(&kr)) { break; }
#if COLOR
    scr_setcolor(0, 23, NCOLS, TEXT_COLOR_BASE+(color>>3));
#endif
    color++;
  }
  if (!seeded) {
    SRAND(color);
    seeded = 1;
  }
}


static struct item *item_at(uint8_t x, uint8_t y) {
  for (uint8_t i = 0; i < NITEMS+1; i++) {
    struct item *it = items+i;
    if (it->x == x && it->y == y) {
      return it;
    }
  }
  return NULL;
}


static void generate_random_position(uint8_t *retx, uint8_t *rety)
{
  while (1) {
    uint16_t n = RAND();
    uint8_t x = n & 0b111111;
    uint8_t y = (n >> 6) & 0b11111;
    /* generate new values if these are out of range */
    if (x > (PLAYFIELD_RIGHT-PLAYFIELD_LEFT) || y > (PLAYFIELD_BOTTOM-PLAYFIELD_TOP)) {
      continue;
    }
    x += PLAYFIELD_LEFT;
    y += PLAYFIELD_TOP;
    /* generate new position if there's already an item here */
    if (item_at(x, y)) {
      continue;
    }
    /* space is free, we're done */
    *retx = x;
    *rety = y;
    break;
  }
}


static uint8_t random_item_char(void)
{
  uint8_t n;
  do {
    n = RAND() & 127;
#if OBVIOUS_KITTEN_IS_OBVIOUS
    if (n == '&') { n = '\0'; }
#endif
  } while (n == '\0' || n == ' ' || n == ROBOT_CHAR);
  return n;
}


static uint8_t random_item_color(void)
{
  uint8_t n;
  do {
    n = RAND() & 0xF;
  } while (n == 0);
  return TEXT_COLOR(n);
}


/* returns a pointer to a random NKI ("non-kitten item") string in ROM */
static PGM_P random_nki(void)
{
  uint16_t n;
  do {
    n = RAND() & NKI_RAND_MASK;
  } while (n >= NUM_NKIS);
  return pgm_read_ptr(&nki_table[n]);
}


/* has this NKI already been instantiated? */
static _Bool nki_exists(PGM_P desc)
{
  for (uint8_t i = 0; i < NITEMS; i++) {
    if (desc == items[i].description) {
      return 1;
    }
  }
  return 0;
}


/* keep picking NKIs until we find one that has not been instantiated */
static PGM_P random_unique_nki(void)
{
  PGM_P desc;
  do {
    desc = random_nki();
  } while (nki_exists(desc));
  return desc;
}


static void draw_all_items(void)
{
  for (uint8_t i = 0; i < NITEMS+1; i++) {
    scr_setchar(items[i].x, items[i].y, items[i].ch, items[i].color);
  }
}


static void clear_status(void)
{
  memset(&SCREEN.chars[1][0], ' ', NCOLS*MAX_NKI_LINES);
}


/* interprets newlines */
static void display_status_P(PGM_P msg)
{
  uint8_t y = 1;
  uint8_t x = 0;
  char c;
  while ((c = pgm_read_byte(msg++))) {
#if !MULTI_LINE_STATUS_MESSAGE
  if (c == '\n') { c = ' '; }
#endif
    if (c == '\n') {
      y++;
      x = 0;
    } else {
      SCREEN.chars[y][x] = c;
      x++;
    }
  }
}


static PGM_P sound_ptr;
static uint8_t sound_len;

static void play_sound_P(PGM_P snd)
{
  sound_ptr = snd;

}


static void update_sound(void)
{
  if (sound_len) {
    sound_len--;
    if (sound_len == 0) {
      OCR0A = 0; OCR0B = 0;
    }
    return;
  }

  if (!sound_ptr) {
    return;
  }

  uint8_t c;
  uint8_t sum = 0;
  uint8_t xor = 0;
  while (1) {
    c = pgm_read_byte(sound_ptr++);
    if (c == ' ' || c == '\n') {
      break;
    } else if (c == '\0') {
      sound_ptr = NULL;
      break;
    }
    sound_len++;
    sum += c;
    xor ^= c;
  }
  OCR0A = MAX(sum, xor);
  OCR0B = MIN(sum, xor);
}

/* returns NULL if robot moved or is up against a wall */
/* returns pointer to the booped item if an item was booped */
static struct item *robot_move(int8_t dx, int8_t dy)
{
  uint8_t new_x = robot.x+dx;
  uint8_t new_y = robot.y+dy;
  if (new_x < PLAYFIELD_LEFT || new_x > PLAYFIELD_RIGHT ||
      new_y < PLAYFIELD_TOP  || new_y > PLAYFIELD_BOTTOM) {
    return NULL;
  }
  /* item at new position? */
  struct item *occupied = item_at(new_x, new_y);
  if (occupied) {
    return occupied;
  }

  /* do a sound */
  play_sound_P(PSTR("{/"));

  /* erase and redraw robot */
  scr_setchar(robot.x, robot.y, ' ', TEXT_COLOR_WHITE);
  robot.x = new_x;
  robot.y = new_y;
  scr_setchar(robot.x, robot.y, robot.ch, robot.color);
  return NULL;
}


static _Bool boop_item(const struct item *it)
{
  /* if is kitten, game is over */
  if (it->description == NULL)  {
    return 1;
  }
  /* otherwise, draw description */
  else {
    clear_status();
    display_status_P(it->description);
    play_sound_P(it->description);
    return 0;
  }
}


static void game_init(void)
{
  /* clear out state from previous game */
  memset(items, 0, sizeof(items));

  /* place all items, including robot, at random positions */
  for (uint8_t i = 0; i < NITEMS+1; i++) {
    uint8_t x, y;
    generate_random_position(&x, &y);
    items[i].x = x;
    items[i].y = y;
  }

  /* set character and color of kitten and NKIs */
  /* item 0 will be kitten and will have a null description */
  /* other items are NKIs and get a random description */
  for (uint8_t i = 0; i < NITEMS; i++) {
    items[i].ch = random_item_char();
    items[i].color = random_item_color();
    items[i].description = (i) ? random_unique_nki() : NULL;
  }

#if OBVIOUS_KITTEN_IS_OBVIOUS
  kitten.ch = '&';
#endif

  /* robot is a gray # character */
  robot.ch = ROBOT_CHAR;
  robot.color = ROBOT_COLOR;
}


static void wait_frames(uint16_t nframes)
{
  while (--nframes) { (void)frame_sync(); }
}


static void found_kitten(uint8_t kitten_char, uint8_t kitten_color)
{
  OCR0A = 0; OCR0B = 0; /* sound off */
  /* clear status and gray out playfield */
  clear_status();
  scr_setcolor(0, MAX_NKI_LINES+1, NCOLS*(NROWS-(MAX_NKI_LINES+1)), TEXT_COLOR_GRAY);
  /* animate */
  for (int8_t distance = 3; distance >= 0; distance--) {
    scr_setchar((NCOLS/2)-distance, MAX_NKI_LINES, ROBOT_CHAR, ROBOT_COLOR);
    scr_setchar((NCOLS/2)+distance+1, MAX_NKI_LINES, kitten_char, kitten_color);
    wait_frames(60);
    scr_setchar((NCOLS/2)-distance, MAX_NKI_LINES, ' ', ROBOT_COLOR);
    scr_setchar((NCOLS/2)+distance+1, MAX_NKI_LINES, ' ', kitten_color);
  }
  scr_setchar((NCOLS/2), MAX_NKI_LINES-1, acHEART, TEXT_COLOR_PINK);
  scr_setchar((NCOLS/2)+1, MAX_NKI_LINES-1, acHEART, TEXT_COLOR_PINK);
  scr_setchar((NCOLS/2), MAX_NKI_LINES, ROBOT_CHAR, ROBOT_COLOR);
  scr_setchar((NCOLS/2)+1, MAX_NKI_LINES, kitten_char, kitten_color);
  wait_frames(60);
#if MULTI_LINE_STATUS_MESSAGE
  scr_puts_P(0, MAX_NKI_LINES-1, TEXT_COLOR_WHITE, PSTR("You found kitten!"));
  scr_puts_P(0, MAX_NKI_LINES, TEXT_COLOR_WHITE, PSTR("Way to go, robot!"));
#else
  scr_puts_P(0, MAX_NKI_LINES, TEXT_COLOR_WHITE, PSTR("You found kitten! Way to go, robot!"));
#endif
  scr_puts_P(0, NROWS-1, TEXT_COLOR_WHITE, PSTR("Press any key to return to title screen."));
  wait_for_key(&kr);
}


static void game(void)
{
  scr_clear(TEXT_COLOR_WHITE);
  scr_puts_P(0, 0, TEXT_COLOR_WHITE, version_str);
  scr_box(PLAYFIELD_LEFT-1, PLAYFIELD_TOP-1, PLAYFIELD_RIGHT+1, PLAYFIELD_BOTTOM+1, TEXT_COLOR_WHITE);
  draw_all_items();

  while (frame_sync()) {
    update_sound();
    struct item *booped_item = NULL;
    switch (get_key_ascii(&kr)) {
      case KC_UP:    case 'k': booped_item = robot_move(0, -1); break;
      case KC_DOWN:  case 'j': booped_item = robot_move(0, +1); break;
      case KC_LEFT:  case 'h': booped_item = robot_move(-1, 0); break;
      case KC_RIGHT: case 'l': booped_item = robot_move(+1, 0); break;
      case KC_ESC:   return;
    }
    if (booped_item) {
      _Bool is_kitten = boop_item(booped_item);
      if (is_kitten) {
        found_kitten(booped_item->ch, booped_item->color); /* do the animation */
        return; /* return to title screen */
      }
    }
  }
}


APP_MAIN(rfk)
{
#if COLOR
  init_40x25_color_text_mode(SCREEN.bytes, NULL);
#else
  init_80x25_mono_text_mode(SCREEN.bytes, NULL);
  globalcolor = TEXT_COLOR_WHITE;
#endif

  video_start();

  while (1) {
    title_screen();
    game_init();
    game();
  }
}
