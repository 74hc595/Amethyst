#ifndef COLORS_H_
#define COLORS_H_

#include <stdint.h>

#define NUM_TEXT_COLORS 32
extern const uint8_t bitspreadtable[256*NUM_TEXT_COLORS];
extern const uint8_t lowernibbledoubletable[256];
extern const uint8_t uppernibbledoubletable[256];

/**
 * For 40x25 color text modes, the color byte for a character cell is actually
 * the upper 8 bits of the bitspread table to use when expanding the 8-pixel-wide
 * character pattern to 16 pixels during display. Different bitspread tables
 * produce different colors.
 */

//!!! TODO this is suboptimal and gcc can't always optimize this to a constant expression.
//!!! If I can put the bitspread table at a fixed location in ROM then this would be better,
//!!! but that requires messing with linker stuff
#define TEXT_COLOR(c)                 ((uint8_t)((((uint16_t)(&bitspreadtable[256*c]))>>8)&0xFF))
#define TEXT_COLOR_BASE     TEXT_COLOR(0)
#define TEXT_COLOR_BLACK    TEXT_COLOR(0)
#define TEXT_COLOR_DK_GREEN TEXT_COLOR(1)
#define TEXT_COLOR_DK_BLUE  TEXT_COLOR(2)
#define TEXT_COLOR_BLUE     TEXT_COLOR(3)
#define TEXT_COLOR_RED      TEXT_COLOR(4)
#define TEXT_COLOR_GRAY     TEXT_COLOR(5)
#define TEXT_COLOR_PURPLE   TEXT_COLOR(6)
#define TEXT_COLOR_LT_BLUE  TEXT_COLOR(7)
#define TEXT_COLOR_BROWN    TEXT_COLOR(8)
#define TEXT_COLOR_GREEN    TEXT_COLOR(9)
#define TEXT_COLOR_GREY     TEXT_COLOR(10) /* same as 5 but different artifacting */
#define TEXT_COLOR_AQUA     TEXT_COLOR(11)
#define TEXT_COLOR_ORANGE   TEXT_COLOR(12)
#define TEXT_COLOR_YELLOW   TEXT_COLOR(13)
#define TEXT_COLOR_PINK     TEXT_COLOR(14)
#define TEXT_COLOR_WHITE    TEXT_COLOR(15)

#define TEXT_COLOR_BLUE_ON_DK_GREEN   TEXT_COLOR(16)
#define TEXT_COLOR_AQUA_ON_DK_GREEN   TEXT_COLOR(17)
#define TEXT_COLOR_YELLOW_ON_DK_GREEN TEXT_COLOR(18)
#define TEXT_COLOR_WHITE_ON_DK_GREEN  TEXT_COLOR(19)
#define TEXT_COLOR_BLUE_ON_DK_BLUE    TEXT_COLOR(20)
#define TEXT_COLOR_AQUA_ON_DK_BLUE    TEXT_COLOR(21)
#define TEXT_COLOR_PINK_ON_DK_BLUE    TEXT_COLOR(22)
#define TEXT_COLOR_WHITE_ON_DK_BLUE   TEXT_COLOR(23)
#define TEXT_COLOR_BLUE_ON_RED        TEXT_COLOR(24)
#define TEXT_COLOR_YELLOW_ON_RED      TEXT_COLOR(25)
#define TEXT_COLOR_PINK_ON_RED        TEXT_COLOR(26)
#define TEXT_COLOR_WHITE_ON_RED       TEXT_COLOR(27)
#define TEXT_COLOR_AQUA_ON_BROWN      TEXT_COLOR(28)
#define TEXT_COLOR_YELLOW_ON_BROWN    TEXT_COLOR(29)
#define TEXT_COLOR_PINK_ON_BROWN      TEXT_COLOR(30)
#define TEXT_COLOR_WHITE_ON_BROWN     TEXT_COLOR(31)
#define TEXT_COLOR_WHITE_ON_BLUE      TEXT_COLOR(32)
#define TEXT_COLOR_WHITE_ON_GRAY      TEXT_COLOR(33)
#define TEXT_COLOR_WHITE_ON_PURPLE    TEXT_COLOR(34)
#define TEXT_COLOR_WHITE_ON_GREEN     TEXT_COLOR(35)
#define TEXT_COLOR_WHITE_ON_GREY      TEXT_COLOR(36)  /* same as 33 but different artifacting */
#define TEXT_COLOR_WHITE_ON_ORANGE    TEXT_COLOR(37)


#define HORIZ_RES_HALF                ((uint8_t)((((uint16_t)(uppernibbledoubletable))>>8)&0xFF))

#endif
