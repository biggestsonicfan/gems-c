#!/usr/bin/env python3
"""Interleave sfight's two program EPROMs (16 bits each) into the i960's
little-endian image, ROM address 0 at offset 0. Usage: mkprog.py out.bin"""
import os, sys, zipfile
if 'ROMS_DIR' not in os.environ:
    sys.exit('mkprog.py: set ROMS_DIR to the folder holding sfight.zip')
z = zipfile.ZipFile(os.path.join(os.environ['ROMS_DIR'], 'sfight.zip'))
a, b = z.read('epr-19001.15'), z.read('epr-19002.16')
out = bytearray()
for i in range(0, len(a), 2):
    out += a[i:i + 2] + b[i:i + 2]
open(sys.argv[1], 'wb').write(out)
