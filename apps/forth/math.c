#include "forth.h"
#include "minio.h"

#include <stdlib.h>

int16_t starslash(int16_t n3, int16_t n2, int16_t n1)
{
  int32_t d = ((int32_t)n1) * ((int32_t)n2);
  return d / n3;
}


typedef union {
  struct {
    cell low;
    cell mid;
    cell high;
  } __attribute__((packed));
  struct {
    dcell lowmid;
    cell high_;
  } __attribute__((packed));
  struct {
    cell low_;
    dcell midhigh;
  } __attribute__((packed));
} __attribute__((packed)) triple_cell;


/* this can probably be optimized further */
/* also it uses a ton of stack space */
uint32_t umstarslash(uint16_t n2, uint16_t n1, uint32_t d1)
{
  triple_cell i = {0};

  /* compute partial products */
  uint32_t p1 = (d1&0xFFFF)*n1;
  uint32_t p2 = (d1>>16)*n1;
  i.lowmid = p1;
  i.midhigh += p2;

  /* compute partial quotients */
  uint16_t q1 = i.midhigh / n2;
  uint16_t r1 = i.midhigh % n2;
  i.mid = r1;
  uint16_t q2 = i.lowmid / n2;
  return ((uint32_t)q2) | (((uint32_t)q1)<<16);
}


/* the spec says that the divisor (n2) must be positive */
int32_t mstarslash(uint16_t n2, int16_t n1, int32_t d1)
{
  /* record sign bits and take absolute values */
  _Bool d1sign = (d1 & 0x80000000) != 0;
  _Bool d2sign = (n1 & 0x8000) != 0;
  d1 = labs(d1);
  n1 = abs(n1);
  int32_t result = umstarslash(n2, n1, d1);
  if (d1sign ^ d2sign) { result = -result; }
  return result;
}


/* div_t has quot and rem in the opposite order of what forth wants */
/* so we use our own struct */
struct forth_div {
  int16_t rem;
  int16_t quot;
} __attribute__((packed));

struct forth_udiv {
  uint16_t rem;
  uint16_t quot;
} __attribute__((packed));

struct forth_div starslashmod(int16_t n3, int16_t n2, int16_t n1)
{
  int32_t d = ((int32_t)n1) * ((int32_t)n2);
  /* gcc is smart enough to use a single __divmodsi4 call for / and % */
  return (struct forth_div){ .quot = d/n3, .rem = d%n3 };
}


/* Integer division in C99 always rounds toward zero */
struct forth_div smslashrem(int16_t n1, int32_t d1)
{
  return (struct forth_div){ .quot = d1/n1, .rem = d1%n1 };
}


struct forth_div fmslashmod(int16_t denom, int32_t numer)
{
  int16_t quot = numer/denom;
  int16_t rem  = numer%denom;
  if (rem != 0 && ((rem < 0) != (denom < 0))) {
    --quot;
    rem += denom;
  }
  return (struct forth_div){ .quot = quot, .rem = rem };
}


struct forth_udiv umslashmod(uint16_t u1, uint32_t ud)
{
  return (struct forth_udiv){ .quot = ud/u1, .rem = ud%u1 };
}
