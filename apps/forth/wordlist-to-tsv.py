#!/usr/bin/env python

from collections import defaultdict
from pprint import pprint

WORDS = defaultdict(list)
WORDLISTS = set()

with open('words.txt', 'r') as f:
  for line in f:
    fields = line.split()
    word = fields[0]
    wordlist = fields[1]
    WORDLISTS.add(wordlist)
    WORDS[word].append(wordlist)

wlists = sorted(list(WORDLISTS))
words = sorted(WORDS.keys())

for word in words:
  print '{}\t'.format(word),
  for wl in wlists:
    flag = '\tY' if wl in WORDS[word] else '\tN'
    print flag,
  print
print wlists