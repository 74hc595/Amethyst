#include "app.h"
#include "console.h"

MAKE_CONSOLE_40COL(stdcon);

APP_MAIN(launcher,APP_HIDDEN)
{
  CONSOLE_MIO_INIT(stdcon);
  const struct app_def *app = NULL;
  uint8_t n = 0;
  while ((app = app_index_next(app))) {
    Pc('a'+n); Pc(':'); Pbl(); Psp(app_name(app)); Pnl();
    n++;
  }
  Pc('?');
  while (1) {
    uint8_t key = mio_getc(0);
    if (key >= 'a' && key < 'a'+n) {
      uint8_t app_num = key-'a';
      video_stop();
      app_launch(app_index_nth(app_num));
    }
  }
}
