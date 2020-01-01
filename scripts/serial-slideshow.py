#!/usr/bin/env python

import serial
import time
import glob
import os

from img2bin import load_gimp_palette, image_file_to_bin

DIR = '/Users/matt/slideshow/'
images = []
images.extend(glob.glob(DIR+'*.jpg'))
images.extend(glob.glob(DIR+'*.jpeg'))
images.extend(glob.glob(DIR+'*.png'))
images.extend(glob.glob(DIR+'*.gif'))
images = sorted(images)
print len(images)

BAUD = 57600

palette, num_colors = load_gimp_palette('palettes/8bit.gpl')

def tobin(filename):
    aspectcrop = not os.path.splitext(filename)[0].endswith('-nocrop')
    image, _ = image_file_to_bin(
        palette=palette,
        num_colors=num_colors,
        infile=filename,
        outsize=(160,100),
        pxaspectratio=1,
        aspectcrop=aspectcrop,
        revbits=False,
        revpixels=False)
    return image


with serial.Serial('/dev/cu.usbserial-DN05KLWU', BAUD) as port:
    n = 0
    raw_input('press enter when ready:')
    while True:
        bdata = tobin(images[n])
        port.write(bdata)
        n += 1
        time.sleep(1.5)
