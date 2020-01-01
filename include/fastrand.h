/**
 * Simple but fast 16-bit pseudorandom number generator.
 * The default avr-libc implementation of rand() uses division/modulo, which
 * isn't quick, and requires bringing in parts of libgcc which otherwise might
 * not be needed.
 */
#ifndef FASTRAND_H_
#define FASTRAND_H_

int fastrand(void);
void sfastrand(unsigned s);

#endif
