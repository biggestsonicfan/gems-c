#!/bin/bash
# Disassemble stf.elf's .text (PowerPC 750, the GameCube's Gekko less its
# paired singles) into one listing to grep by GC address.
# Usage: disasm.sh [out]  (default /dev/shm/gems/text1.s, ~7.6 MB, ~1 min)
set -e
D=$(cd "$(dirname "$0")" && pwd); ELF="$D/../../stf.elf"
OUT=${1:-/dev/shm/gems/text1.s}; mkdir -p "$(dirname "$OUT")"
gdb-multiarch -batch -ex 'set architecture powerpc:750' -ex 'x/246000i 0x80005660' "$ELF" > "$OUT" 2>&1
wc -l "$OUT"
