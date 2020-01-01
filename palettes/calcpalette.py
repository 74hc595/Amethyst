#!/usr/bin/env python
from math import sqrt
import itertools

# python doesn't have popcount but stackoverflow says this is the fastest way
def popcount(bitpattern, nbits):
    return bin(bitpattern & ((1<<nbits)-1)).count('1')

# rotate right by specified number of bits
def ror(bitpattern, places, nbits):
    for _ in xrange(0, places):
        bit = bitpattern & 1
        bitpattern >>= 1
        if bit:
            bitpattern |= 1 << (nbits-1)
    return bitpattern

# reverse bits
def revb(bitpattern, nbits):
    out = 0
    for _ in xrange(0, nbits):
        out <<= 1
        bit = bitpattern & 1
        bitpattern >>= 1
        out |= bit
    return out

def bit_weight_sum_signed(bitpattern, nbits):
    weight = 0
    for b in xrange(0,nbits/2):
        if bitpattern & (1<<b):
            weight += 1
    for b in xrange(nbits/2,nbits):
        if bitpattern & (1<<b):
            weight -= 1
    return weight

def bit(n,bit):
    return 1 if (n & (1<<bit)) != 0 else 0

def tuple_lerp(t1, t2, weight):
    inv_weight = 1.0 - weight
    return [a*weight + b*inv_weight for a, b in zip(t1, t2)]

def to_naive_yiq(bitpattern, nbits, rotate=0):
    y = popcount(bitpattern, nbits) / float(nbits)
    if nbits == 2:
        bitpattern = bit(bitpattern,0) + bit(bitpattern,0)*2 + bit(bitpattern,1)*4 + bit(bitpattern,1)*8
        bitpattern = ror(bitpattern,rotate,4)
        nbits = 4
    if nbits == 4:
        bitpattern = ror(bitpattern, rotate, 4)
        i = bit(bitpattern,0) + bit(bitpattern,3) - bit(bitpattern,1) - bit(bitpattern,2)
        q = bit(bitpattern,0) + bit(bitpattern,1) - bit(bitpattern,2) - bit(bitpattern,3)
    else:
        raise Exception('TODO, not supported')

    # i = -bit_weight_sum_signed(ror(bitpattern, nbits/4, nbits), nbits)
    # q = bit_weight_sum_signed(bitpattern, nbits)
    i /= float(nbits)
    q /= float(nbits)
    return y, i, q

# def to_actual_iq(i, q):
#     i_actual = hypot(i,q) * cos(atan2(q,i) - pi/15)
#     q_actual = hypot(i,q) * sin(atan2(q,i) - pi/15)
#     return i_actual, q_actual

def yiq_to_rgb(y, i, q):
    r = max(0, min(255, round((y + 0.956*i + 0.621*q)*255)))
    g = max(0, min(255, round((y - 0.272*i - 0.647*q)*255)))
    b = max(0, min(255, round((y - 1.105*i + 1.702*q)*255)))
    return int(r), int(g), int(b)

def generate_yiq_palette(nbits, rotate=0):
    palette = []
    for c in xrange(0, 1<<nbits):
        y, i, q = to_naive_yiq(revb(c,nbits), nbits, rotate)
        palette.append((y,i,q))
    return palette

def write_gimp_palette(yiq_palette, filename, palette_name=''):
    with open(filename, 'w') as f:
        f.write('GIMP Palette\n')
        f.write('Name: {}\n'.format(palette_name))
        f.write('Columns: 8\n')
        for y, i, q in yiq_palette:
            r, g, b = yiq_to_rgb(y, i, q)
            f.write('{} {} {}\n'.format(r, g, b))

def write_palette_preview(yiq_palette, filename):
    from PIL import Image
    boxw, boxh = 16, 16
    cols = int(sqrt(len(yiq_palette)))
    preview = Image.new('RGB', (cols*boxw, cols*boxh))
    for n, yiq in enumerate(yiq_palette):
        rgb = yiq_to_rgb(*yiq)
        box = Image.new('RGB', (boxw, boxh), rgb)
        preview.paste(box, ((n%cols)*boxw, (n/cols)*boxh))
    preview.save(filename)

def do_palette(bits, rotate, filename, palette_name):
    yiq_pal = generate_yiq_palette(bits, rotate)
    write_gimp_palette(yiq_pal, filename+'.gpl', palette_name)
    write_palette_preview(yiq_pal, filename+'-preview.png')
    return yiq_pal

def blend_palette(yiq_palette, weight, filename, palette_name):
    blended = []
    for c2 in yiq_palette:
        for c1 in yiq_palette:
            blended.append(tuple_lerp(c1, c2, weight))
    write_gimp_palette(blended, filename+'.gpl', palette_name)
    write_palette_preview(blended, filename+'-preview.png')
    return blended

twobit_gm  = do_palette(2, 0, '2bit-gm', 'NTSC 2-bit green/magenta')
twobit_bo  = do_palette(2, 1, '2bit-bo', 'NTSC 2-bit blue/orange')
fourbit    = do_palette(4, 1, '4bit', 'NTSC 4-bit')
fourbit_gm = blend_palette(twobit_gm, 0.75, '4bit-gm', 'NTSC 4-bit green/magenta')
fourbit_bo = blend_palette(twobit_bo, 0.75, '4bit-bo', 'NTSC 4-bit blue/orange')
eightbit   = blend_palette(fourbit, 0.75, '8bit', 'NTSC 8-bit blended')
