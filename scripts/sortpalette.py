#!/usr/bin/env python

import colorsys
import sys
from math import sqrt, pow
from collections import namedtuple
from operator import attrgetter
from img2bin import load_colors_from_gimp_palette

HSYI = namedtuple('HSYI', 'h s y index')

def write_palette_preview(colors, filename):
  from PIL import Image
  boxw, boxh = 16, 16
  cols = int(sqrt(len(colors)))
  preview = Image.new('RGB', (cols*boxw, cols*boxh))
  for n, rgb in enumerate(colors):
    box = Image.new('RGB', (boxw, boxh), tuple(rgb))
    preview.paste(box, ((n%cols)*boxw, (n/cols)*boxh))
  preview.save(filename)

hsv_colors = []
if __name__ == '__main__':
  colors = load_colors_from_gimp_palette(sys.argv[1])
  for n, rgb in enumerate(colors):
    normalized_rgb = tuple(c/255. for c in rgb)
    h, s, _ = colorsys.rgb_to_hsv(*normalized_rgb)
    y, _, _ = colorsys.rgb_to_yiq(*normalized_rgb)
    hsv_colors.append(HSYI(h,s,y,n))

  hsv_colors = sorted(hsv_colors, key=attrgetter('h'))

  for r in xrange(0,16):
    i, j = r*16, (r+1)*16
    hsv_colors[i:j] = sorted(hsv_colors[i:j], key=attrgetter('s'))
    hsv_colors[i:j] = sorted(hsv_colors[i:j], key=attrgetter('y'))

  sorted_colors = [[0,0,0]]*256
  sorted_color_indexes = [0]*256
  for i, hsvi in enumerate(hsv_colors):
    sorted_colors[i] = colors[hsvi.index]
    sorted_color_indexes[i] = hsvi.index

  print sorted_color_indexes
  write_palette_preview(sorted_colors, 'palprev.png')
