#!/bin/bash
# Hold attract's replay fight against MAME frame by frame (m2-hle2's
# tools/match-replay.mjs). The first run with --mame takes the MAME reference.
# Usage: match-replay.sh OUTDIR [--mame]
# Env: M2HLE_EXE, M2HLE_TREE, MAME_EXE (a MAME with the model2 driver; needed for --mame).
OUT=${1:?out dir}; shift
TREE=${M2HLE_TREE:?set M2HLE_TREE}
export M2_EXE=${M2HLE_EXE:?set M2HLE_EXE} M2_NOCLIP=${M2_NOCLIP:-$TREE/vendor/noclip}
case " $* " in *" --mame "*) : "${MAME_EXE:?set MAME_EXE to a MAME with the model2 driver}";; esac
[ -n "${MAME_EXE:-}" ] && export MAME_EXE
mkdir -p "$OUT"; cd "$OUT" || exit 1
node "$TREE/tools/match-replay.mjs" --out "$OUT" "$@"
