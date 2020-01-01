#!/usr/bin/env python

from math import pi, cos

print ".section .progmem.assets"
print ".balign 256"
print ".global cosinetable"
print ".type cosinetable, @object"
print "cosinetable:"
for i in xrange(0,256):
    radians = (i/256.)*2*pi
    cosval = 32767*cos(radians)
    cosintval = int(round(cosval))
    print "  .byte lo8({:6})  ; i={:3}  theta={:.5f}  cos(theta)*32767={:.3f}".format(cosintval, i, radians, cosval)
for i in xrange(0,256):
    radians = (i/256.)*2*pi
    cosval = 32767*cos(radians)
    cosintval = int(round(cosval))
    print "  .byte hi8({:6})  ; i={:3}  theta={:.5f}  cos(theta)*32767={:.3f}".format(cosintval, i, radians, cosval)

