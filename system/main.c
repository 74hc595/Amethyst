#include "app.h"
#include <avr/wdt.h>

#ifdef STARTUP_APP
#define STARTUP_APP_DEF PASTE_(APP_DEF_,STARTUP_APP)
#else
#error No STARTUP_APP defined
#endif

extern const struct app_def STARTUP_APP_DEF;

/* for simavr */
#include "avr_mcu_section.h"
AVR_MCU(F_CPU, "atmega1284");

/* Disable watchdog timer early in startup, so soft-resets don't lock the AVR */
/* (see avr-libc FAQ) */
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
void wdt_init(void) {
  MCUSR = 0;
  wdt_disable();
}


int main()
{
  /* disable unused peripherals */
  PRR0 = _BV(PRTWI)|_BV(PRADC);
  ACSR = _BV(ACD);
  video_init();
  audio_init();
  spi_init();
  app_launch(&STARTUP_APP_DEF);
}
