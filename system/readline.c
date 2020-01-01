#include "readline.h"
#include <avr/pgmspace.h>

int readline(char *inbuf, int bufsize)
{
  int len = 0;
  while (1) {
    char c = mio_waitc();
    switch (c) {
      /* Newline finishes */
      case '\r':
      case '\n':
        inbuf[len] = '\0';
        return len;
      /* Esc aborts */
      case '\e':
        return -1;
      /* Form feed (Ctrl-L) clears screen and aborts */
      case '\f':
        Pc(c);
        return -2;
      /* Backspace and DEL delete a character */
      case '\b':
      case '\x7f':
        if (len) {
          Psl("\b \b");
          len--;
          inbuf[len] = 0;
        }
        break;
      /* Otherwise, add character to the input buffer and print it */
      default:
        if (len < bufsize) {
          inbuf[len++] = c;
          Pc(c);
        }
    }
  }
}
