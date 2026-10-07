#!/bin/bash
# Build i960dasm from MAME's own i960 disassembler, and prog.bin from the
# sfight program EPROMs. Usage: build.sh [out dir]  (default: this dir)
# Needs a MAME source tree ($MAME_SRC: the tree's src/ directory).
set -e
D=$(cd "$(dirname "$0")" && pwd); O=${1:-$D}; M=${MAME_SRC:?set MAME_SRC to the src directory of a MAME source tree}
mkdir -p "$O"
g++ -std=c++20 -O1 -I"$D" -I$M/emu -I$M/lib/util -I$M/osd -I$M/devices "$D/main.cpp" \
    $M/devices/cpu/i960/i960dis.cpp $M/lib/util/disasmintf.cpp $M/lib/util/strformat.cpp -o "$O/i960dasm"
python3 "$D/mkprog.py" "$O/prog.bin"
echo "built $O/i960dasm and $O/prog.bin"
