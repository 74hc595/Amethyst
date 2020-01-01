/**
 * Simple but fast 16-bit pseudorandom number generator.
 * The default avr-libc implementation of rand() uses division/modulo, which
 * isn't quick, and requires bringing in parts of libgcc which otherwise might
 * not be needed.
 */
#include <stdint.h>

extern uint16_t seed;

/* From Leo Brodie's "Starting Forth." */
int fastrand(void) {
  seed = (seed * 31421) + 6927;
  return seed;
}

void sfastrand(unsigned s) {
  seed = s;
}
