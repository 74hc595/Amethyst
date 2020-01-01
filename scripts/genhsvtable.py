#!/usr/bin/env python

from img2bin import load_colors_from_gimp_palette
import colorsys

rgb_colors = load_colors_from_gimp_palette('palettes/8bit.gpl')

ihsvrgb_colors = []

def color_distance(ihsvrgb, h2, s2, v2):
  _, h1, s1, v1, _, _, _ = ihsvrgb
  return (2*(h2-h1))**2 + (s2-s1)**2 + (v2-v1)**2

for i, c in enumerate(rgb_colors):
  r, g, b = c
  h, s, v = colorsys.rgb_to_hsv(r/255.,g/255.,b/255.)
  ihsvrgb_colors.append((i, h, s, v, r, g, b))


h_vals = [0., 1/15., 2/15., 3/15., 4/15., 5/15., 6/15., 7/15.,
          8/15., 9/15., 10/15., 11/15., 12/15., 13/15., 14/15., 1]
s_vals = [0.3, 0.6, 0.8, 1.0]
v_vals = [0.3, 0.6, 0.8, 1.0]

# print 'GIMP Palette'
# print 'Name: NTSC 8-bit HSV'
# print 'Columns: 8'
# for v in v_vals:
#   for s in s_vals:
#     for h in h_vals:
#       i, _, _, _, r, g, b = min(ihsvrgb_colors, key=lambda x: color_distance(x,h,s,v))
#       print '{} {} {}'.format(r,g,b)

print ".section .progmem.assets"
print ".balign 256"
print ".global hsvtable"
print ".type hsvtable, @object"
print "hsvtable:"
for v in v_vals:
  for s in s_vals:
    for h in h_vals:
      i, _, _, _, r, g, b = min(ihsvrgb_colors, key=lambda x: color_distance(x,h,s,v))
      print '  .byte {}'.format(i)
