/**
 * Convenience macros when only a single console is used.
 */

#ifndef CONSOLE_MACROS_H_
#define CONSOLE_MACROS_H_

#include "console.h"

#ifndef DEFAULT_CONSOLE
#define DEFAULT_CONSOLE stdcon
#endif

#define clrhome(...)        console_clrhome(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define clrscroll(...)      console_clrscroll(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define clrline(...)        console_clrline(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define fillrange(...)      console_fillrange(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define xfrmrange(...)      console_xfrmrange(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define gohome(...)         console_gohome(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define gotoxy(...)         console_gotoxy(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define gotox(...)          console_gotox(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define gotoy(...)          console_gotoy(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define gotoscrollxy(...)   console_gotoscrollxy(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define gotoscrollx(...)    console_gotoscrollx(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define gotoscrolly(...)    console_gotoscrolly(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define movexy(...)         console_movexy(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define movex(...)          console_movex(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define movey(...)          console_movey(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define movesol(...)        console_movesol(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define erase(...)          console_erase(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define eraseline(...)      console_eraseline(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define setymargins(...)    console_setymargins(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define setxmargins(...)    console_setxmargins(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define resetmargins(...)   console_resetmargins(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define scrollup(...)       console_scrollup(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define scrolldown(...)     console_scrolldown(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define lback(...)          console_lback(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define lfwd(...)           console_lfwd(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define linefeed(...)       console_linefeed(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define cback(...)          console_cback(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define cfwd(...)           console_cfwd(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define backspace(...)      console_backspace(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define setch(...)          console_setch(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define addch(...)          console_addch(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define addch_raw(...)      console_addch_raw(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define addstr(...)         console_addstr(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define addstr_P(...)       console_addstr_P(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define xyaddch(...)        console_xyaddch(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define xyaddstr(...)       console_xyaddstr(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define xyaddstr_P(...)     console_xyaddstr_P(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define getch(...)          console_getch(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define getx(...)           console_cx(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define gety(...)           console_cy(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define getcols(...)        console_cols(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define getrows(...)        console_rows(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define maxcol(...)         console_maxcol(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define maxrow(...)         console_maxrow(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define scrollwidth(...)    console_scrollwidth(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define scrollheight(...)   console_scrollheight(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define topmargin(...)      console_mtop(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define botmargin(...)      console_mbottom(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define leftmargin(...)     console_mleft(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define rightmargin(...)    console_mright(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define scrollok(...)       console_scrollok(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define setscrollok(...)    console_setscrollok(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define showcursor(...)     console_showcursor(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define setshowcursor(...)  console_setshowcursor(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#define setrvs(...)         console_setrvs(&DEFAULT_CONSOLE, ##__VA_ARGS__)
#endif
