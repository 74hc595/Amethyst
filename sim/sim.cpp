/**
 * Basic UART-only simulator for Amethyst, using simavr
 * (https://github.com/buserror/simavr)
 * Starts a picocom session hooked up to USART1 on the simulated AVR, allowing
 * interaction via serial. Works well with the `forth_serial` app.
 * Apps that use video will hang the simulator. That includes the launcher. So
 * if you're going to use the simulator, make sure STARTUP_APP isn't an app that
 * starts video generation.
 */
#include <sim_avr.h>
#include <sim_elf.h>
#include <sim_hex.h>
#include <sim_gdb.h>
#include <sim_io.h>
#include <avr_uart.h>
#include <avr_eeprom.h>

#include <cstdio>
#include <cstring>
#include <err.h>
#include <termios.h>
#include <unistd.h>

#include "uart_pty.h"

#define STDIO_UART_NUM  '1'

static avr_t *avr = nullptr;
uart_pty_t uart_pty;

static void dump_mem_bytes(uint16_t startaddr, uint16_t stackptr, unsigned nbytes)
{
  uint16_t addr = startaddr;
  for (unsigned i = 0; i < nbytes; i++, addr++) {
    if (addr == stackptr) {
      printf("vvvv ");
    } else {
      printf("     ");
    }
  }
  putc('\n', stdout);

  addr = startaddr;
  for (unsigned i = 0; i < nbytes; i++, addr++) {
    printf("%04x ", addr);
  }
  putc('\n', stdout);

  addr = startaddr;
  for (unsigned i = 0; i < nbytes; i++, addr++) {
    if (addr <= avr->ramend) {
      printf("  %02x ", avr->data[addr]);
    } else {
      printf("  .. ");
    }
  }
  putc('\n', stdout);
}


static void dump_avr_state() {
  char sreg_str[9] = "ITHSVNZC";
  uint16_t rsp = avr->data[R_SPL] | (avr->data[R_SPH]<<8);
  
  for (int bit = 0; bit < 8; bit++) {
    if (avr->sreg[bit] == 0) { sreg_str[7-bit] = '-'; }
  }

  puts("Registers:");
   for (int reg = 0; reg < 32; reg++) {
    printf("%sr%d=%02x  ", (reg <= 9) ? " " : "", reg, avr->data[reg]);
    if ((reg % 8) == 7) {
      printf("\n");
    }
  }
  printf("r25:24=%04x  X=%04x  Y=%04x  Z=%04x  PC=%04x         SREG=%s\n",
    avr->data[24] | (avr->data[25]<<8),
    avr->data[R_XL] | (avr->data[R_XH]<<8),
    avr->data[R_YL] | (avr->data[R_YH]<<8),
    avr->data[R_ZL] | (avr->data[R_ZH]<<8),
    avr->pc, sreg_str);
  printf("SP=%04x\n", rsp);
  puts("================================================================================\n\n");
}



static struct termios saved_tio;
void restore_tty(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &saved_tio);
}

int trace_handler(struct avr_t * avr, uint8_t v, void * param) {
  uint16_t rsp = avr->data[R_SPL] | (avr->data[R_SPH]<<8);
  uint16_t rs0 = avr->data[rsp+2] | (avr->data[rsp+1]<<8);
  uint16_t rs1 = avr->data[rsp+4] | (avr->data[rsp+3]<<8);
  uint16_t rs2 = avr->data[rsp+6] | (avr->data[rsp+5]<<8);
  uint16_t tmp = avr->data[22]    | (avr->data[23]<<8);
  uint16_t tos = avr->data[24]    | (avr->data[25]<<8);
  uint16_t ip  = avr->data[26]    | (avr->data[27]<<8);
  uint16_t dsp = avr->data[28]    | (avr->data[29]<<8);
  uint16_t ds0 = avr->data[dsp]   | (avr->data[dsp+1]<<8);
  uint16_t ds1 = avr->data[dsp+2] | (avr->data[dsp+3]<<8);
  uint16_t ds2 = avr->data[dsp+4] | (avr->data[dsp+5]<<8);
  printf("\r\n%c[PC=%04x IP=%04x TOS=%04x DSP=%04x(%04x %04x %04x) RSP=%04x(%04x %04x %04x)]\r\n",
    avr->data[30], avr->pc, ip, tos, dsp, ds0, ds1, ds2, rsp, rs0*2, rs1*2, rs2*2);
  return 0;
}


int main(int argc, char **argv)
{
  avr = avr_make_mcu_by_name("atmega1284");
  avr->frequency = 14318182;
  uint32_t dsize, start;
  const char * fname = (argc) ? argv[1] : "../colorvideo.hex";
  uint8_t *fw = read_ihex_file(fname, &dsize, &start);
  avr_init(avr);
  avr_loadcode(avr, fw, dsize, start);

  /* load eeprom */
  uint8_t eeprom[4096];
  memset(eeprom, ' ', sizeof(eeprom));
  FILE *f = fopen("eeprom.txt", "r");
  if (f) {
    fread(eeprom, 1, sizeof(eeprom), f);
  }
  avr_eeprom_desc_t d = {
    .ee = eeprom,
    .offset = 0,
    .size = sizeof(eeprom)
  };
  avr_ioctl(avr, AVR_IOCTL_EEPROM_SET, &d);

  // avr->log = LOG_DEBUG;
  avr_set_command_register(avr, 0x3C);
  avr_cmd_register(avr, 0, trace_handler, NULL);

  uart_pty_init(avr, &uart_pty);
  uart_pty_connect(&uart_pty, STDIO_UART_NUM);

  //avr->gdb_port = 1234;
  //avr_gdb_init(avr);

  /* Stack canary */
  for (int i = 0x0100; i <= 0x40FF; i++) {
    avr->data[i] = 0x77;
  }

  int state = cpu_Running;
  while ((state != cpu_Done) && (state != cpu_Sleeping)) {
    state = avr_run(avr);
  }
  if (state == cpu_Crashed) {
    puts("!!!!!!!! CRASHED");
  }
  dump_avr_state();
}
