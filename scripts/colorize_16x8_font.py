#!/usr/bin/env python

import sys
from os.path import basename, splitext
from PIL import Image

PATTERN_HEIGHT = 8
PATTERN_WIDTHS = [4, 8, 12, 16]
MAX_COL_WIDTH  = max(PATTERN_WIDTHS)

BIT_SPREAD_PATTERNS = [
    (3, ( 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15)),  # white
    (1, (15,  0, 15,  1, 15,  2, 15,  3, 15,  4, 15,  5, 15,  6, 15,  7)),  # gray
    (2, ( 0,  1,  2, 15,  3,  4,  5, 15,  6,  7,  8, 15,  9, 10, 11, 15)),  # pink
    (1, ( 0,  1, 15, 15,  2,  3, 15, 15,  4,  5, 15, 15,  6,  7, 15, 15)),  # red
    (2, ( 0,  1, 15,  2,  3,  4, 15,  5,  6,  7, 15,  8,  9, 10, 11, 15)),  # yellow
    (1, ( 0, 15,  15, 1,  2, 15, 15,  3,  4, 15, 15,  5,  6, 15, 15,  7)),  # green
    (2, ( 0, 15,  1,  2,  3, 15,  4,  5,  6, 15,  7,  8,  9, 15, 10, 11)),  # aqua
    (2, (15,  0,  1,  2, 15,  3,  4,  5, 15,  6,  7,  8, 15,  9, 10, 11)),  # light blue
    (1, (15, 15,  0,  1, 15, 15,  2,  3, 15, 15,  4,  5, 15, 15,  6,  7)),  # blue
    (1, (15,  0,  1, 15, 15,  2,  3, 15, 15,  4,  5, 15, 15,  6,  7, 15)),  # purple
    (0, (15,  0, 15, 15, 15,  1, 15, 15, 15,  2, 15, 15, 15,  3, 15, 15)),  # dark red
    (0, ( 0, 15, 15, 15,  1, 15, 15, 15,  2, 15, 15, 15,  3, 15, 15, 15)),  # dark orange
    (0, (15, 15, 15,  0, 15, 15, 15,  1, 15, 15, 15,  2, 15, 15, 15,  3)),  # dark green
    (0, (15, 15,  0, 15, 15, 15,  1, 15, 15, 15,  2, 15, 15, 15,  3, 15)),  # dark blue
    (0, (15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15)),
    (0, (15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15)),
]


def extract_subimage(img, x, row, width):
    y = row*PATTERN_HEIGHT
    return img.crop((x, y, x+width, y+PATTERN_HEIGHT))

def spread_bits(img, spreadpattern):
    out = Image.new('L', (MAX_COL_WIDTH,PATTERN_HEIGHT))
    for x in xrange(0,MAX_COL_WIDTH):
        dstcol = spreadpattern[x]
        col = img.crop((dstcol,0,dstcol+1,PATTERN_HEIGHT))
        out.paste(col,(x,0,x+1,PATTERN_HEIGHT))
    return out


infile = sys.argv[1]
outfile = splitext(infile)[0]+'_color.png'
img = Image.open(infile)
if img.size[0] % MAX_COL_WIDTH != 0:
    raise Exception('image width must be a multiple of {}'.format(MAX_COL_WIDTH))
if img.size[1] != PATTERN_HEIGHT*len(PATTERN_WIDTHS):
    raise Exception('image height must be {}'.format(PATTERN_HEIGHT*len(PATTERN_WIDTHS)))

# slice up input image
chars = []
for px in xrange(0, img.size[0], MAX_COL_WIDTH):
    src_images = map(lambda (r,w): extract_subimage(img,px,r,MAX_COL_WIDTH), enumerate(PATTERN_WIDTHS))
    chars.append(src_images)

# create output image
num_cols = len(chars)
num_rows = len(BIT_SPREAD_PATTERNS)
outimg = Image.new('L', (num_cols*MAX_COL_WIDTH, num_rows*PATTERN_HEIGHT), 0)

for col, char in enumerate(chars):
    for row, spr in enumerate(BIT_SPREAD_PATTERNS):
        outx = col*MAX_COL_WIDTH
        outy = row*PATTERN_HEIGHT
        src_img = char[spr[0]]
        spread_img = spread_bits(src_img, spr[1])
        outimg.paste(spread_img, (outx, outy, outx+src_img.size[0], outy+src_img.size[1]))

outimg.save(outfile)
