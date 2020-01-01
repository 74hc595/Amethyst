#!/usr/bin/env python

def popcount(bitpattern, nbits):
    return bin(bitpattern & ((1<<nbits)-1)).count('1')


def four_bit_pattern():
  data = bytearray()
  width = 80
  for y in xrange(0, 200):
    for x in xrange(0, width):
      xblock = x / 20
      yblock = y / 50
      block = yblock*4 + xblock
      b = 0x11 * block
      data.append(b)
  return data

def bit(n,bit):
    return 1 if (n & (1<<bit)) != 0 else 0

def revb(bitpattern, nbits):
    out = 0
    for _ in xrange(0, nbits):
        out <<= 1
        bit = bitpattern & 1
        bitpattern >>= 1
        out |= bit
    return out

lookuptable = sorted(xrange(0,256), key=lambda x: popcount(x,8))


remap_table = [
  0x0, 0x1, 0x2, 0x4,
  0x8, 0x3, 0x6, 0xC,
  0x9, 0x5, 0xA, 0xE,
  0xD, 0xB, 0x7, 0xF,
]

def eight_bit_pattern():
  data = bytearray()
  width = 160
  for y in xrange(0, 192):
    for x in xrange(0, width):
      xblock = x / 10
      yblock = y / 12
      # lo = xblock << 4
      # lo_black = 0
      # lo_gray = 0b0101 << 4
      # lo_white = 0b1111 << 4
      # hi = xblock
      # hi_black = 0
      # hi_gray = 0b0101
      # hi_white = 0b1111
      # if yblock < 2:
      #   c = hi_black + lo_gray
      # elif yblock < 4:
      #   c = hi_black + lo_white
      # elif yblock < 6:
      #   c = hi_gray + lo_black
      # elif yblock < 8:
      #   c = hi_gray + lo_gray
      # elif yblock < 10:
      #   c = hi_gray + lo_white
      # elif yblock < 12:
      #   c = hi_white + lo_black
      # elif yblock < 14:
      #   c = hi_white + lo_gray
      # else:
      #   c = hi_white + lo_white
      c = yblock | (xblock << 4)
      # c = bit(c,0)*1 + bit(c,0)*2 + bit(c,1)*4 + bit(c,1)*8 + bit(c,2)*16 + bit(c,2)*32 + bit(c,3)*64 + bit(c,3)*128
      #c = revb(c,8)
      data.append(c)
  return data

with open('4bit.bin', 'wb') as f:
  f.write(four_bit_pattern())

with open('8bit.bin', 'wb') as f:
  f.write(eight_bit_pattern())