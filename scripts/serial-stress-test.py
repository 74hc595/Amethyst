#!/usr/bin/env python

import serial
import time

BAUD = 57600

with serial.Serial('/dev/cu.usbserial-DN05KLWU', BAUD) as port:
  time.sleep(5.0)
  n = 0
  while True:
    port.write(bytearray([n & 0xFF]))
    n += 1

