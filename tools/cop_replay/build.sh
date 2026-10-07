#!/bin/bash
# Build gems_cop_replay against an m2-hle2 checkout (its src/ and tests/cop_replay.c).
#   tools/cop_replay/build.sh <m2-hle2 checkout> [out]   (out: /dev/shm/gems-cop-replay/gems_cop_replay)
# GEMS_CFLAGS adds flags, e.g. -DGEMS_COP_NEAREST (the host's round-to-nearest).
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
M2=$(cd "${1:?m2-hle2 checkout}" && pwd)
OUT=${2:-/dev/shm/gems-cop-replay/gems_cop_replay}
GEN=$(dirname "$OUT")/gen
mkdir -p "$GEN"
python3 "$ROOT/gen_all.py" --cop-only "$GEN/gems_cop_all.h" > /dev/null
S=$M2/src
# -ffp-contract=off as m2-hle2 builds: no fused multiply-add anywhere
${CC:-gcc} -O2 -ffp-contract=off -fno-strict-aliasing -w $GEMS_CFLAGS \
    -I"$GEN" -I"$ROOT" -I"$M2/tests" -I"$S" -I"$S/board" -I"$S/core" -I"$S/net" -I"$S/ui" -I"$S/profiles" \
    -I"$M2/vendor/stb" "$HERE/gems_cop_replay.c" -o "$OUT" -lm
echo "$OUT"
