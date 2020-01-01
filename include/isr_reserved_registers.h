/**
 * This header must be included by *ALL* translation units to tell the compiler
 * to leave these registers alone. It is recommended that this is done on the
 * command line (using an `-include` argument) instead of explicit include
 * statements in each file (which could be easily forgotten, leading to
 * potential register clobbering and very strange behavior or crashes)
 */
#ifndef ISR_RESERVED_REGISTERS_H_
#define ISR_RESERVED_REGISTERS_H_

#ifdef __ASSEMBLER__

/**
 * Reserve as many registers as we can for use in interrupt routines.
 * avr-libc documentation indicates that r2 through r7 are safe to be used for
 * this purpose. Unfortunately, some libc and libm functions use these and need
 * to be avoided. (see the file `banned_stdlib_functions`)
 */

/* Scratch register used in video interrupt routines. */
#define isr_tmp           r2
/* Line count. Decrements by 1 each line. 0 indicates the last line. */
#define linenum           r3
/* Pointer to video data. Persisted across lines. */
#define vptr_lo           r4
#define vptr_hi           r5
/* Used for saving ZL/ZH registers in video interrupt routines. */
/* movw is 4x as fast as pushing/popping a register pair. */
#define zsav_lo           r6
#define zsav_hi           r7
/* Address of the handler to invoke for each new line and frame. */
/* Using I/O registers instead of SRAM saves 2 cycles per line. */
#define linehandler_lo    _SFR_IO_ADDR(GPIOR1)
#define linehandler_hi    _SFR_IO_ADDR(GPIOR2)
/* Set bit 1 of DDRB after the current frame has finished drawing. */
/* The value of this bit is ignored when the CKOUT fuse is programmed, */
/* so we can use it as one extra bit of storage in the I/O address space. */
#define new_frame         _SFR_IO_ADDR(DDRB)
/* Bit 1 of PORTB enables the colorburst. */
/* The value of this bit is ignored when the CKOUT fuse is programmed, */
/* so we can use it as one extra bit of storage in the I/O address space. */
#define color_video       _SFR_IO_ADDR(PORTB)
/* Bit 2 of DDRD indicates if asynchronous USART RX is enabled. */
/* The value of this bit is ignored when the USART1 receiver is enabled, */
/* so we can use it as one extra bit of storage in the I/O address space. */
#define usart_async       _SFR_IO_ADDR(DDRD)
/* Bit 4 of PORTB indicates if the "simple tone generator" is enabled. */
/* The simple tone generator can output a pulse wave with a given frequency, */
/* duty cycle, and duration. If enabled, the simple tone generator updates */
/* once per frame. */
#define tone_enabled      _SFR_IO_ADDR(PORTB)

#else  /* !ASSEMBLER */

register unsigned char isr_tmp   asm("r2");
register unsigned char linenum   asm("r3");
register unsigned char vptr_lo   asm("r4");
register unsigned char vptr_hi   asm("r5");
register unsigned char zsav_lo   asm("r6");
register unsigned char zsav_hi   asm("r7");
#define new_frame   DDRB
#define color_video PORTB
#define usart_async DDRD
#define tone_enabled PORTB
#define linehandler_lo  GPIOR1
#define linehandler_hi  GPIOR2
#endif

/* Reuse the pin-change interrupt registers, since it doesn't make sense to */
/* use these ports for interrupts */
#define tone_duration   PCMSK0

#define vptr  vptr_lo
#define zsav  zsav_lo

#define new_frame_bit   1
#define color_video_bit 1
#define usart_async_bit 2
#define tone_enabled_bit 4

#endif
