#!/usr/bin/env python

import re
import sys
from os.path import basename, splitext
from PIL import Image
 
 # TODO: parse command line args
PATTERN_WIDTH = 8
PATTERN_HEIGHT = 8
PACK = False
Y_FLIP = 1
X_FLIP = 0
DEINTERLEAVE_16X8_HALVES = False

# reverse bits
def revb(bitpattern, nbits):
    out = 0
    for _ in xrange(0, nbits):
        out <<= 1
        bit = bitpattern & 1
        bitpattern >>= 1
        out |= bit
    return out

def to_row(img, sx, sy):
    n = 0
    for x in xrange(sx, sx+PATTERN_WIDTH):
        n >>= 1
        color = img.getpixel((x, sy))
        if color >= 128:
            n |= 0x80
    n = (n >> (8-PATTERN_WIDTH)) & 0xFF # align if width < 8
    return n
 
def row_to_c_code(row):
    return '  0b{:08b},'.format(revb(row,8))

def row_to_asm_code(row):
    return '.byte 0b{:08b}'.format(revb(row,8))

def to_c_code(pattern_num, rows):
    if pattern_num >= 32 and pattern_num < 127:
        comment = '  /* {} {} */\n'.format(pattern_num, chr(pattern_num))
    else:
        comment = '  /* {} */\n'.format(pattern_num)
    code_rows = [row_to_c_code(r) for r in rows]
    code_rows = reversed(code_rows) if Y_FLIP else code_rows
    return comment + '\n'.join(code_rows)
 
def to_asm_code(pattern_num, rows):
    if pattern_num >= 32 and pattern_num < 127:
        comment = '  ; {} {}\n'.format(pattern_num, chr(pattern_num))
    else:
        comment = '; {}\n'.format(pattern_num)
    return comment + '\n'.join(
            [row_to_asm_code(r) for r in rows])
 
if sys.argv[1] == '-a':
    del sys.argv[1]
    genfunc = to_asm_code
    row_genfunc = row_to_asm_code
    extension = '.S'
    header = lambda fontname: '.text\n.balign 256\n.global '+fontname+'\n.type '+fontname+', @object\n'+fontname+':\n'
    footer = '\n'
else:
    genfunc = to_c_code
    row_genfunc = row_to_c_code
    extension = '.c'
    header = lambda fontname: '#include <stdint.h>\nconst uint8_t '+fontname+'[] __attribute__((aligned(256))) PROGMEM = {\n'
    footer = '\n};\n'
 
infile = sys.argv[1]
outfile = sys.argv[2] if len(sys.argv) >= 3 else splitext(infile)[0]+extension
fontname = splitext(basename(outfile))[0]
 
# filename suffix can be used to specify pattern width/height
match = re.search(r"_(\d+)x(\d+)$", fontname)
if match:
    PATTERN_WIDTH, PATTERN_HEIGHT = int(match.group(1)), int(match.group(2))
 
if PATTERN_WIDTH < 1 or PATTERN_HEIGHT < 1:
    raise Exception('pattern width and height must be greater than 0')
if PATTERN_WIDTH > 8:
    raise Exception('pattern width cannot be greater than 8')
 
img = Image.open(infile)
if img.size[0] % PATTERN_WIDTH != 0:
    raise Exception('image width must be a multiple of {}'.format(PATTERN_WIDTH))
if img.size[1] % PATTERN_HEIGHT != 0:
    raise Exception('image height must be a multiple of {}'.format(PATTERN_HEIGHT))

pattern_strs = []
row_strs = ['']*PATTERN_HEIGHT

if DEINTERLEAVE_16X8_HALVES:
    alt_row_strs = ['']*PATTERN_HEIGHT
else:
    alt_row_strs = []


img = img.convert(mode='L')
pattern_num = 0
for py in xrange(0, img.size[1], PATTERN_HEIGHT):
    for px in xrange(0, img.size[0], PATTERN_WIDTH):
        rows = [to_row(img, px, py+n) for n in xrange(0, PATTERN_HEIGHT)]
        for r in xrange(0,PATTERN_HEIGHT):
            row = rows[r]
            row = revb(row,8) if X_FLIP else row
            row_str = row_genfunc(row)+'\n'
            if not DEINTERLEAVE_16X8_HALVES:
                row_strs[r] += row_str
            else:
                if px%(PATTERN_WIDTH*2) == PATTERN_WIDTH:
                    alt_row_strs[r] += row_str
                else:
                    row_strs[r] += row_str
        pattern_strs.append(genfunc(pattern_num, rows))
        pattern_num += 1
 

output = header(fontname) #'#include <stdint.h>\nconst uint8_t '+fontname+'[] __attribute__((aligned(8))) = {\n'
# output += '\n'.join(pattern_strs)
if not PACK:
    output += '\n'.join(reversed(row_strs))
    output += '\n'.join(reversed(alt_row_strs))
else:
    output += '\n'.join(pattern_strs)
output += footer
 
with open(outfile, 'w') as f:
    f.write(output)
