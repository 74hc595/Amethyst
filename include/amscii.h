/**
 * Defines for the AMSCII (Amethyst ASCII) graphic characters assigned to
 * ASCII code points 0-31, 127-160, and 255.
 * Upper 128 code points are inverse-video copies of the lower 128
 */
#ifndef AMSCII_CHARS_H_
#define AMSCII_CHARS_H_

/* Characters as string literals, for interpolating into other string literals */
#define amBLANK     "\x00"  /* blank (U+0020) */
#define amBLKUL     "\x01"  /* block, upper-left (U+2598) */
#define amBLKUR     "\x02"  /* block, upper-right (U+259D) */
#define amBLKULUR   "\x03"  /* block, upper-left and upper-right (U+2580) */
#define amBLKLL     "\x04"  /* block, lower-left (U+2596) */
#define amBLKULLL   "\x05"  /* block, upper-left and lower-left (U+258C) */
#define amBLKURLL   "\x06"  /* block, upper-right and lower-left (U+259E) */
#define amBLKULURLL "\x07"  /* block, upper-left, upper-right, lower-left (U+259B) */
#define amHEART     "\x08"  /* filled heart (U+2665) */
#define amDOT       "\x09"  /* middle dot (U+00B7) */
#define amBULLET    "\x0a"  /* filled circle (U+2022) */
#define amCIRCLE    "\x0b"  /* outlined circle (U+25CB) */
#define amTRIUL     "\x0c"  /* triangle, upper-left half filled (U+25E4) */
#define amTRIUR     "\x0d"  /* triangle, upper-right half filled (U+25E5) */
#define amDIAGURLL  "\x0e"  /* diagonal line, upper-right to lower-left (U+2571) */
#define amDIAGULLR  "\x0f"  /* diagonal line, upper-left to lower-right (U+2572) */
#define amDIAGX     "\x10"  /* diagonal lines, crossed (U+2573) */
#define amARROWR    "\x11"  /* arrow, pointing right (U+2192) */
#define amARROWL    "\x12"  /* arrow, pointing left (U+2190) */
#define amBOXH      "\x13"  /* box, horizontal line (U+2500) */
#define amARROWD    "\x14"  /* arrow, pointing down (U+2193) */
#define amBOXUL     "\x15"  /* box, upper-left corner (U+250C) */
#define amBOXUR     "\x16"  /* box, upper-right corner (U+2510) */
#define amBOXTD     "\x17"  /* box, T pointing down (U+252C) */
#define amARROWU    "\x18"  /* arrow, pointing up (U+2191) */
#define amBOXLL     "\x19"  /* box, lower-left corner (U+2514) */
#define amBOXLR     "\x1a"  /* box, lower-right corner (U+2518) */
#define amBOXTU     "\x1b"  /* box, T pointing up (U+2534) */
#define amBOXV      "\x1c"  /* box, vertical line (U+2502) */
#define amBOXTR     "\x1d"  /* box, T pointing right (U+251C) */
#define amBOXTL     "\x1e"  /* box, T pointing left (U+2524) */
#define amBOXHV     "\x1f"  /* box, horizontal and vertical lines crossed (U+253C) */
#define amBLKFL     "\x7f"  /* full block, light shade (U+2591) */
#define amBLKFM     "\x80"  /* full block, medium shade (U+2592) */
#define amBLKURLLLR "\x81"  /* block, upper-left, lower-left, lower-right, (U+259F) */
#define amBLKULLLLR "\x82"  /* block, upper-left, lower-left, lower-right (U+2599) */
#define amBLKLLLR   "\x83"  /* block, lower-left and lower-right (U+2584) */
#define amBLKULURLR "\x84"  /* block, upper-left, upper-right, lower-right (U+259C) */
#define amBLKURLR   "\x85"  /* block, upper-right and lower-right (U+2590) */
#define amBLKULLR   "\x86"  /* block, upper-left and lower-right (U+259A) */
#define amBLKLR     "\x87"  /* block, lower-right (U+2597) */
#define amIBULLET   "\x8a"  /* inverse bullet (U+25D8) */
#define amICIRCLE   "\x8b"  /* inverse oultlined circle (U+25D9) */
#define amTRILR     "\x8c"  /* triangle, lower-right filled (U+25E2) */
#define amTRILL     "\x8d"  /* triangle, lower-left filled (U+25E3) */
#define amDBOXH     "\x93"  /* double-line box, horizontal line (U+2550) */
#define amDBOXUL    "\x95"  /* double-line box, upper-left corner (U+2554) */
#define amDBOXUR    "\x96"  /* double-line box, upper-right corner (U+2557) */
#define amDBOXTD    "\x97"  /* double-line box, T pointing down (U+2566) */
#define amDBOXLL    "\x99"  /* double-line box, lower-left corner (U+255A) */
#define amDBOXLR    "\x9a"  /* double-line box, lower-right corner (U+255D) */
#define amDBOXTU    "\x9b"  /* double-line box, T pointing up (U+2569) */
#define amDBOXV     "\x9c"  /* double-line box, vertical line (U+2551) */
#define amDBOXTR    "\x9d"  /* double-line box, T pointing right (U+2560) */
#define amDBOXTL    "\x9e"  /* double-line box, T pointing left (U+2563) */
#define amDBOXHV    "\x9f"  /* double-line box, horizontal and vertical lines crossed (U+256C) */
#define amBLKF      "\xa0"  /* full block (U+2588) */
#define amBLKFD     "\xff"  /* full block, dark shade (U+2593) */

/* Characters as numeric values */
#define STRTOCHR(s) ((s)[0])
#define acBLANK     STRTOCHR(amBLANK)
#define acBLKUL     STRTOCHR(amBLKUL)
#define acBLKUR     STRTOCHR(amBLKUR)
#define acBLKULUR   STRTOCHR(amBLKULUR)
#define acBLKLL     STRTOCHR(amBLKLL)
#define acBLKULLL   STRTOCHR(amBLKULLL)
#define acBLKURLL   STRTOCHR(amBLKURLL)
#define acBLKULURLL STRTOCHR(amBLKULURLL)
#define acHEART     STRTOCHR(amHEART)
#define acDOT       STRTOCHR(amDOT)
#define acBULLET    STRTOCHR(amBULLET)
#define acCIRCLE    STRTOCHR(amCIRCLE)
#define acTRIUL     STRTOCHR(amTRIUL)
#define acTRIUR     STRTOCHR(amTRIUR)
#define acDIAGURLL  STRTOCHR(amDIAGURLL)
#define acDIAGULLR  STRTOCHR(amDIAGULLR)
#define acDIAGX     STRTOCHR(amDIAGX)
#define acARROWR    STRTOCHR(amARROWR)
#define acARROWL    STRTOCHR(amARROWL)
#define acBOXH      STRTOCHR(amBOXH)
#define acARROWD    STRTOCHR(amARROWD)
#define acBOXUL     STRTOCHR(amBOXUL)
#define acBOXUR     STRTOCHR(amBOXUR)
#define acBOXTD     STRTOCHR(amBOXTD)
#define acARROWU    STRTOCHR(amARROWU)
#define acBOXLL     STRTOCHR(amBOXLL)
#define acBOXLR     STRTOCHR(amBOXLR)
#define acBOXTU     STRTOCHR(amBOXTU)
#define acBOXV      STRTOCHR(amBOXV)
#define acBOXTR     STRTOCHR(amBOXTR)
#define acBOXTL     STRTOCHR(amBOXTL)
#define acBOXHV     STRTOCHR(amBOXHV)
#define acBLKFL     STRTOCHR(amBLKFL)
#define acBLKFM     STRTOCHR(amBLKFM)
#define acBLKURLLLR STRTOCHR(amBLKURLLLR)
#define acBLKULLLLR STRTOCHR(amBLKULLLLR)
#define acBLKLLLR   STRTOCHR(amBLKLLLR)
#define acBLKULURLR STRTOCHR(amBLKULURLR)
#define acBLKURLR   STRTOCHR(amBLKURLR)
#define acBLKULLR   STRTOCHR(amBLKULLR)
#define acBLKLR     STRTOCHR(amBLKLR)
#define acIBULLET   STRTOCHR(amIBULLET)
#define acICIRCLE   STRTOCHR(amICIRCLE)
#define acTRILR     STRTOCHR(amTRILR)
#define acTRILL     STRTOCHR(amTRILL)
#define acDBOXH     STRTOCHR(amDBOXH)
#define acDBOXUL    STRTOCHR(amDBOXUL)
#define acDBOXUR    STRTOCHR(amDBOXUR)
#define acDBOXTD    STRTOCHR(amDBOXTD)
#define acDBOXLL    STRTOCHR(amDBOXLL)
#define acDBOXLR    STRTOCHR(amDBOXLR)
#define acDBOXTU    STRTOCHR(amDBOXTU)
#define acDBOXV     STRTOCHR(amDBOXV)
#define acDBOXTR    STRTOCHR(amDBOXTR)
#define acDBOXTL    STRTOCHR(amDBOXTL)
#define acDBOXHV    STRTOCHR(amDBOXHV)
#define acBLKF      STRTOCHR(amBLKF)
#define acBLKFD     STRTOCHR(amBLKFD)
#endif
