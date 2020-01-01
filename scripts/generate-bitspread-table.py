#!/usr/bin/env python

O = 7 # indicates bit will always be zero
I = 6 # indicates bit will always be one
BIT_SPREAD_PATTERNS = [
  (O, O, O, O, O, O, O, O), # 0  black
  (O, O, O, 2, O, O, O, 0), # 1  dark green
  (O, O, 2, O, O, O, 0, O), # 2  dark blue
  (O, O, 3, 2, O, O, 1, 0), # 3  blue
  (O, 2, O, O, O, 0, O, O), # 4  red
  (O, 3, O, 2, O, 1, O, 0), # 5  gray
  (O, 3, 2, O, O, 1, 0, O), # 6  purple
  (O, 3, 2, 2, O, 1, 0, 0), # 7  light blue
  (2, O, O, O, 0, O, O, O), # 8  brown
  (3, O, O, 2, 1, O, O, 0), # 9  green
  (3, O, 2, O, 1, O, 0, O), # 10 grey
  (3, O, 3, 2, 1, O, 1, 0), # 11 aqua
  (3, 2, O, O, 1, 0, O, O), # 12 orange
  (3, 2, O, 2, 1, 0, O, 0), # 13 yellow
  (3, 2, 2, O, 1, 0, 0, O), # 14 pink
  (3, 3, 2, 2, 1, 1, 0, 0), # 15 white
  (O, 3, 2, I, O, 1, 0, I), # 16 blue on dark green
  (3, O, 2, I, 1, O, 0, I), # 17 aqua on dark green
  (3, 2, O, I, 1, 0, O, I), # 18 yellow on dark green
  (3, 2, 2, I, 1, 0, 0, I), # 19 white on dark green
  (O, 3, I, 2, O, 1, I, 0), # 20 blue on dark blue
  (3, O, I, 2, 1, O, I, 0), # 21 aqua on dark blue
  (3, 2, I, O, 1, 0, I, O), # 22 pink on dark blue
  (3, 2, I, 2, 1, 0, I, 0), # 23 white on dark blue
  (O, I, 3, 2, O, I, 1, 0), # 24 blue on red
  (3, I, O, 2, 1, I, O, 0), # 25 yellow on red
  (3, I, 2, O, 1, I, 0, O), # 26 pink on red
  (3, I, 3, 2, 1, I, 1, 0), # 27 white on red
  (I, O, 3, 2, I, O, 1, 0), # 28 aqua on brown
  (I, 3, O, 2, I, 1, O, 0), # 29 yellow on brown
  (I, 3, 2, O, I, 1, 0, O), # 30 pink on brown
  (I, 3, 2, 2, I, 1, 0, 0), # 31 white on brown
  (3, 2, I, I, 1, 0, I, I), # 32 white on blue
  (I, 3, I, 2, I, 1, I, 0), # 33 white on gray
  (3, I, I, 2, 1, I, I, 0), # 34 white on purple
  (I, 3, 2, I, I, 1, 0, I), # 35 white on green
  (3, I, 2, I, 1, I, 0, I), # 36 white on grey
  (I, I, 3, 2, I, I, 1, 0), # 37 white on orange
]



def set_bit(n, value, bitnum):
  return n | (value<<bitnum)

def get_bit(n, bit):
  return 1 if n&(1<<bit) else 0

def spread_nibble(nibble, pattern):
  byte = 0
  for i in range(7,-1,-1):
    idx = pattern[7-i]
    byte = set_bit(byte, get_bit(nibble|64, idx), i)
  return byte

def nibbleswap(n):
  return ((n&0xF)<<4) | ((n>>4)&0xF)

def generate_table_for_bitspread_pattern(pattern, invert=False):
  lines = []
  for i in xrange(0,256):
    j = nibbleswap(i)
    nibble = (j if not invert else j^15) & 15
    byte = spread_nibble(nibble, pattern)
    lines.append('.byte 0b{:08b}'.format(byte))
  lines.append('')
  return '\n'.join(lines)


print '.text'
print '.balign 256'
print '.global bitspreadtable'
print '.type bitspreadtable, @object'
print 'bitspreadtable:'
for pattern in BIT_SPREAD_PATTERNS:
  print generate_table_for_bitspread_pattern(pattern)
print '.global bitspreadtable_numpatterns'
print '.equ bitspreadtable_numpatterns, {}'.format(len(BIT_SPREAD_PATTERNS))