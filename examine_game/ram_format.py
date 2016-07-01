#!/usr/bin/env python3

import sys
import binascii

rams = []

for fn in sys.argv[1:]:
    f = open(fn, 'rb')
    rams.append(list(binascii.hexlify(f.read()).decode('ascii')))
    f.close()

for j in range(0, len(rams[0]), 2):
    a = True
    for i in range(1, len(rams)):
        if rams[i][j] != rams[i-1][j] or rams[i][j+1] != rams[i-1][j+1]:
            a = False
    if a:
        for i in range(len(rams)):
            rams[i][j] = '.'
            rams[i][j+1] = '.'
                


n = 64
for i in range(0, len(rams[0]), n):
    for r in rams:
        s = ''.join(r[i:i+n])
        print(' '.join(s[j:j+4] for j in range(0, len(s), 4)))
    print()
