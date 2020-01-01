#ifndef APP_H_
#define APP_H_

/* include the common headers */
#include "video.h"
#include "audio.h"
#include "keys.h"
#include "colors.h"
#include "serial.h"
#include "spi.h"
#include "fastrand.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

struct app_def {
  void (*entrypoint)();
  uint16_t data_load_start;
  uint16_t data_load_start_xpage; /* only LSB is significant on atmega1284 but this member has to be 16 bits; if arithmetic is done on the address of an extern var (incl. downcasting) it is no longer a compile time constant */
  uint16_t data_end;
  uint16_t bss_end;
  char appname[];
} __attribute__((packed));

#define APP_MAIN_NAMED(appid,namestr,...) \
extern char __data_load_start_app_##appid; \
extern char __data_load_start_xpage_app_##appid; \
extern char __data_end_app_##appid; \
extern char __bss_end_app_##appid; \
noreturn void APP_MAIN_##appid(void); \
__attribute__((used,section(".app_index" __VA_ARGS__))) const struct app_def APP_DEF_##appid = { \
  .entrypoint = APP_MAIN_##appid, \
  .data_load_start = (uint16_t)(&__data_load_start_app_##appid), \
  .data_load_start_xpage = (uint16_t)(&__data_load_start_xpage_app_##appid), \
  .data_end = (uint16_t)&__data_end_app_##appid, \
  .bss_end = (uint16_t)&__bss_end_app_##appid, \
  .appname = namestr \
}; \
noreturn void APP_MAIN_##appid(void)

#define APP_MAIN(appid,...) APP_MAIN_NAMED(appid,#appid,__VA_ARGS__)


#define APP_ALIAS_NAMED(aliasof,aliasid,namestr,...) \
noreturn void APP_MAIN_##aliasid(void); \
__attribute__((used,section(".app_index" __VA_ARGS__))) const struct app_def APP_DEF_##aliasid = { \
  .entrypoint = APP_MAIN_##aliasid, \
  .data_load_start = (uint16_t)(&__data_load_start_app_##aliasof), \
  .data_load_start_xpage = (uint16_t)(&__data_load_start_xpage_app_##aliasof), \
  .data_end = (uint16_t)&__data_end_app_##aliasof, \
  .bss_end = (uint16_t)&__bss_end_app_##aliasof, \
  .appname = namestr \
}; \
noreturn void APP_MAIN_##aliasid(void)

#define APP_ALIAS(aliasof,aliasid,...) APP_ALIAS_NAMED(aliasof,aliasid,#aliasid,__VA_ARGS__)

/**
 * Passing this option as an additional trailing argument to one of the APP_()
 * macros excludes this app from the index. It can still be invoked by passing a
 * a pointer to the struct app_def to app_launch().
 */
#define APP_HIDDEN  ".hidden"

/**
 * Returns a pointer to the next app in the app index.
 * If the argument is NULL, returns a pointer to the first app in the index.
 * Returns NULL if the argument is the last app in the index.
 */
const struct app_def *app_index_next(const struct app_def *app);

/**
 * Returns a pointer to the n'th app (zero-indexed) in the index.
 * Returns NULL if the number is invalid.
 */
const struct app_def *app_index_nth(uint8_t n);

/**
 * Returns a pointer to the app's name (in progmem) as a null-terminated string.
 */
PGM_P app_name(const struct app_def *app);

/**
 * Returns the load address (in ROM) of the data section for this app.
 */
uint32_t app_data_load_start(const struct app_def *app);

/**
 * Returns the last address (in RAM) of the data section for this app.
 * This address is also the start of the app's bss section.
 */
uint16_t app_data_end(const struct app_def *app);

/**
 * Returns the last address (in RAM) of the bss section for this app.
 */
uint16_t app_bss_end(const struct app_def *app);

/**
 * Initializes and launches the given app.
 */
noreturn void app_launch(const struct app_def *app);

/**
 * Quits the current app and soft-resets the system.
 */
noreturn void app_quit(void);

#endif
