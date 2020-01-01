#!/usr/bin/env python
#
# freq = Fcpu / (P * (2*n)), where P = 1, 8, 64, 256, or 1024
#    n = Fcpu / (2*freq*P)

Fcpu = 14318182.

PRESCALERS = [8, 64, 256, 1024]
NOTE_NAMES = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']

# see datasheet, Table 16-10
PRESCALER_BIT_PATTERNS = {
  0:    '0',
  1:    '_BV(CS00)',
  8:    '_BV(CS01)',
  64:   '_BV(CS01)|_BV(CS00)',
  256:  '_BV(CS02)',
  1024: '_BV(CS02)|_BV(CS00)'
}

def note_to_freq(n):
  return (2**((n-69)/12.)) * 440.

def freq_to_timer_params(freq):
  for P in PRESCALERS:
    n = Fcpu / (2*freq*P)
    if n >= 1.5 and n <= 255.5:
      return (P, int(round(n)))
  return (0,0)

def timer_params_to_freq(P, n):
  if P==0 or n==0:
    return 0
  return Fcpu / (P * (2.*n))

def percent_error(exact, approx):
  return 100.*(abs(approx-exact)/exact)

def note_name(n):
  octave = -2 + (n/12)
  octavestr = str(octave)
  name = NOTE_NAMES[n%12]
  return name+octavestr

def tone_to_timer_params(period):
  if period < 256:
    return 8, period
  elif period < 480:
    return 64, period-224
  elif period < 672:
    return 256, period-416
  else:
    return 1024, period-608

for notenum in xrange(0,128):
  exact_freq = note_to_freq(notenum)
  P, n = freq_to_timer_params(exact_freq)
  approx_freq = timer_params_to_freq(P, n)
  if P == 0 or n == 0:
    print '  {{ .tccrb=_BV(WGM02), .ocra=0 }}, /* {} ({}) */'.format(notenum, note_name(notenum))
  else:
    print '  {{ .tccrb=_BV(WGM02)|{}, .ocra={} }}, /* {} ({}), approx {:.3f} Hz ({:.2f}% error) */'.format(
      PRESCALER_BIT_PATTERNS[P], n, notenum, note_name(notenum),
      approx_freq, percent_error(exact=exact_freq, approx=approx_freq))

# for t in xrange(0,864):
#   P, n = tone_to_timer_params(t)
#   print '{},{}'.format(t, timer_params_to_freq(P, n))

# for P in PRESCALERS:
#   for n in xrange(0,256):
#     freq = timer_params_to_freq(P,n)
#     print '{:10.3f}'.format(freq), '\t', P, '\t', n
