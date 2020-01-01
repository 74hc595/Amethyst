#!/usr/bin/env python

import os
import sys
from PIL import Image

# Initialize palette
# These are the "updated" values from the "Apple II Video Display Theory"
# spreadsheet:
# https://docs.google.com/spreadsheets/d/1rKR6A_bVniSCtIP_rrv8QLWJdj4h6jEU1jJj0AebWwg/

PALETTE = [
      0,   0,   0, # Black
    167,  11,  64, # Red
     64,  28, 247, # Dark Blue
    230,  40, 255, # Purple
      0, 116,  64, # Dark Green
    128, 128, 128, # Gray 1
     25, 144, 255, # Medium Blue
    191, 156, 255, # Light Blue
     64,  99,   0, # Brown
    230, 111,   0, # Orange
    128, 128, 128, # Gray 2
    255, 139, 191, # Pink
     25, 215,   0, # Light Green
    191, 227,   8, # Yellow
     88, 244, 191, # Aqua
    255, 255, 255, # White
] + [0,0,0]*240
pal = Image.new("P", (1,1), 0)
pal.putpalette(PALETTE)

infile = sys.argv[1]
parts = os.path.splitext(infile)
previewfile = parts[0] + '-16color.png'
binfile = parts[0] + '.bin'

# Load image
image = Image.open(infile)
image = image.convert("RGB")
width, height = image.size
aspect = width / float(height)

# Crop/letterbox to 1.6:1 aspect ratio
target_width = 320
target_height = 200
target_aspect = target_width / float(target_height)
if aspect > target_aspect:
    new_width = int(target_aspect * height)
    offset = (width - new_width) / 2
    resize = (offset, 0, width - offset, height)
else:
    new_height = int(width / target_aspect)
    offset = (height - new_height) / 2
    resize = (0, offset, width, height - offset)

# Reduce horizontal resolution by half
# (effective chroma resolution is 160 pixels)
resized = image.crop(resize).resize((target_width/2, target_height), Image.ANTIALIAS)

# Quantize to color palette
quantized = resized.quantize(palette=pal)

# Save preview file: expand preview back to full width
preview = quantized.resize((target_width, target_height), Image.NEAREST)
preview.save(previewfile)

# Convert to binary
binary = bytearray()
w, h = quantized.size
for y in xrange(0,h):
    for x in xrange(0,w,2):
        p0 = quantized.getpixel((x,y))
        p1 = quantized.getpixel((x+1,y))
        b = ((p1 & 0xF) << 4) | (p0 & 0xF)
        binary.append(b)
with open(binfile, 'wb') as f:
    f.write(binary)

