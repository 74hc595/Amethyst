#!/usr/bin/env python
#
# Convert an RGB image to a 1-, 2-, 4-, or 8-bit palette, and bit-pack the
# resulting indexed-color image into raw binary data.
#
# usage:
#   img2bin -p 8bit.gpl -s 320 200 img.png
# output:
#   img-8bit.png
#   img.bin

import argparse
import itertools
import os
from PIL import Image, ImageOps

def is_power_of_two(n):
  return n and (n & (n-1)) == 0

def tuple_half_difference(a, b):
  return (a[0]-b[0])/2, (a[1]-b[1])/2

# reverse bits
def revb(bitpattern, nbits):
    out = 0
    for _ in xrange(0, nbits):
        out <<= 1
        bit = bitpattern & 1
        bitpattern >>= 1
        out |= bit
    return out

SKIPPED_PREFIXES = set((
  'GIMP Palette',
  'Name:',
  'Columns:',
  '#'
))

def load_colors_from_gimp_palette(palfile):
  if isinstance(palfile,str):
    palfile = open(palfile, 'r')
  colors = []
  for line in palfile.readlines():
    line = line.rstrip()
    if not any(line.startswith(prefix) for prefix in SKIPPED_PREFIXES):
      colors.append(map(int, line.split()[:3]))
  return colors

def make_palette(colors):
  if len(colors) not in (2,4,16,256):
    raise Exception('Number of colors in palette file ({}) must be 2, 4, 16, or 256'.format(len(colors)))
  # PIL wants a flattened list of RGB values
  colors_flat = list(itertools.chain(*colors))
  # Also must have exactly 256 RGB triples (768 elements total)
  colors_flat.extend([0,0,0]*(256-len(colors)))
  pal = Image.new('P', (1,1), 0)
  pal.putpalette(colors_flat)
  return pal, len(colors)

def load_gimp_palette(palfile):
  return make_palette(load_colors_from_gimp_palette(palfile))

def image_to_binary(image, bpp, rev_bits=False, rev_pixels=False):
  bdata = bytearray()
  cols, rows = image.size
  for y in xrange(0,rows):
    tmpbyte = 0
    bitcount = 0
    for x in xrange(0,cols):
      if rev_pixels:
        tmpbyte >>= bpp
      else:
        tmpbyte <<= bpp
      bitcount += bpp
      px = image.getpixel((x,y))
      if rev_bits:
        px = revb(px, bpp)
      if rev_pixels:
        px <<= 8-bpp
      tmpbyte |= px
      if bitcount >= 8:
        bdata.append(tmpbyte)
        tmpbyte = 0
        bitcount = 0
  return bdata

def crop_to_aspect(self, aspect, divisor=1, alignx=0.5, aligny=0.5):
  if self.width / self.height > aspect / divisor:
    newwidth = int(self.height * (aspect / divisor))
    newheight = self.height
  else:
    newwidth = self.width
    newheight = int(self.width / (aspect / divisor))
  img = self.crop((alignx * (self.width - newwidth),
                     aligny * (self.height - newheight),
                     alignx * (self.width - newwidth) + newwidth,
                     aligny * (self.height - newheight) + newheight))
  return img


def image_file_to_bin(**kwargs):
  palette = kwargs['palette']
  image = Image.open(kwargs['infile'])
  orig_width, orig_height = image.size
  # reduce size
  outsize = kwargs['outsize']
  pxaspectratio = kwargs['pxaspectratio']
  revbits = kwargs['revbits']
  revpixels = kwargs['revpixels']
  if kwargs.get('aspectcrop') == True:
    image = crop_to_aspect(image, float(outsize[0])/float(outsize[1]))
  image.thumbnail(outsize, Image.ANTIALIAS)
  # pad image if necessary
  if image.size != outsize:
    padded_image = Image.new('RGB', outsize)
    padded_image.paste(image, tuple_half_difference(outsize, image.size))
    image = padded_image
  # shrink x resolution if pixel ratio is not 1:1
  if pxaspectratio != 1:
    image = image.resize((int(image.size[0]/pxaspectratio), image.size[1]), Image.NEAREST)
#  elif pxaspectratio < 1:
#    image = image.resize((image.size[0], int(image.size[1]/pxaspectratio)), Image.NEAREST)

  # quantize!
  quantized = image.quantize(palette=palette)
  # create preview file
  preview = quantized.resize(outsize, Image.NEAREST)
  # convert to binary data
  bits_per_pixel = kwargs['num_colors'].bit_length()-1
  bdata = image_to_binary(quantized, bits_per_pixel, revbits, revpixels)
  return bdata, preview


if __name__ == '__main__':
  parser = argparse.ArgumentParser()
  parser.add_argument('-p', metavar='PALETTE', type=argparse.FileType('r'), help='GIMP palette file; if unspecified, output 1-bit black/white')
  parser.add_argument('-s', metavar=('W','H'), nargs=2, type=int, help='Output image width and height', default=(320,200))
  parser.add_argument('-a', metavar='RATIO', type=float, help='Pixel aspect ratio', default=2)
  parser.add_argument('-r', action='store_true', help='Reverse bits in each pixel')
  parser.add_argument('-R', action='store_true', help='Reverse pixels in each byte')
  parser.add_argument('-v', action='store_true', help='Show preview of converted image')
  parser.add_argument('infile', type=argparse.FileType('r'), help='Input image file')
  args = parser.parse_args()
  if args.p is not None:
    palette, num_colors = load_gimp_palette(args.p)
    palette_name = os.path.splitext(os.path.basename(args.p.name))[0]
  else:
    palette, num_colors = make_palette([(0,0,0), (255,255,255)])
    palette_name = 'bw'
  bdata, preview = image_file_to_bin(
          palette=palette,
          num_colors=num_colors,
          infile=args.infile,
          outsize=args.s,
          pxaspectratio=args.a,
          revbits=args.r,
          revpixels=args.R)
  preview_filename = os.path.splitext(args.infile.name)[0] + '-' + palette_name + '.png'
  preview.save(preview_filename)
  out_filename = os.path.splitext(args.infile.name)[0] + '.bin'
  with open(out_filename, 'wb') as f:
    f.write(bdata)
  if args.v:
    preview.show()
