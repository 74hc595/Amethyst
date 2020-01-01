#!/usr/bin/env python
#
# Converts a regular NKI file (plain text strings, one per line) to a C header.
# Inserts line breaks as necessary to wrap to a screen less than 80 columns wide.

import sys
import textwrap

NKI_WRAP_WIDTH = 40
LONG_NKI_LINES_EXCLUDE = 2

C_ESCAPE_SEQUENCES = {
  0x07: '\\a',
  0x08: '\\b',
  0x09: '\\t',
  0x0A: '\\n',
  0x0B: '\\v',
  0x0C: '\\f',
  0x0D: '\\r',
  0x5C: '\\\\',
  0x22: '\\"',
  0x3F: '\\?',
}

# yucky but oh well
def escape_c_char(ch):
  asciival = ord(ch)
  s = C_ESCAPE_SEQUENCES.get(asciival)
  if s is None:
    s = '\\x{:02x}'.format(asciival) if (asciival < 32 or asciival >= 127) else ch
  return s

def c_string_literal_encode(s):
  return ''.join(escape_c_char(ch) for ch in s)

def encode_nki(num, text):
  text = text.strip()
  lines = textwrap.wrap(text, NKI_WRAP_WIDTH)
  text = '\n'.join(lines)
  return 'static const char nki_{}[] PROGMEM = "{}";'.format(num, c_string_literal_encode(text)), len(lines)

with open(sys.argv[1]) as f:
  num_nkis = 0
  max_lines = 0
  for line in f:
    text, nlines = encode_nki(num_nkis, line)
    if nlines > LONG_NKI_LINES_EXCLUDE:
      print >> sys.stderr, 'nki exceeds max lines, excluded: {}'.format(text)
      continue
    max_lines = max(max_lines, nlines)
    num_nkis += 1
    print text

  print '#define MAX_NKI_LINES {}'.format(max_lines)
  print '#define NUM_NKIS {}'.format(num_nkis)
  print '#define NKI_RAND_MASK {}'.format((1<<num_nkis.bit_length())-1)
  print 'static const char * const nki_table[] PROGMEM = {'
  for i in xrange(0,num_nkis):
    print '  nki_{},'.format(i)
  print '};'
