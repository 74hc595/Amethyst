#include "app.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

/* Exposed by the linker */
extern const struct app_def __app_index_start;
extern const struct app_def __app_index_end;

const struct app_def *app_index_next(const struct app_def *app)
{
  if (!app) {
    return &__app_index_start;
  } else if (app >= &__app_index_end) {
    return NULL;
  } else {
    const char *app_byteptr = (const char *)app;
    /* advance past the fixed-length fields */
    app_byteptr += sizeof(struct app_def);
    /* advance past the name; add 1 for the null byte */
    app_byteptr += strlen_P(app_byteptr)+1;
    /* return NULL if there are no more apps */
    if (app_byteptr >= (const char *)&__app_index_end) {
      return NULL;
    } else {
      return (const struct app_def *)app_byteptr;
    }
  }
}


const struct app_def *app_index_nth(uint8_t n)
{
  const struct app_def *app = NULL;
  do {
    app = app_index_next(app);
  } while (n--);
  return app;
}


PGM_P app_name(const struct app_def *app) {
  return (PGM_P)&(app->appname);
}


uint32_t app_data_load_start(const struct app_def *app) {
  return pgm_read_dword(&(app->data_load_start));
}


uint16_t app_data_end(const struct app_def *app) {
  return pgm_read_word(&(app->data_end));
}


uint16_t app_bss_end(const struct app_def *app) {
  return pgm_read_word(&(app->bss_end));
}


/**
 * Soft-reset the system.
 * (assumes the startup code and/or bootloader will disable the wdt on reset)
 */
void app_quit(void) {
  cli();
  wdt_enable(WDTO_15MS);
  while (1) {}
}
