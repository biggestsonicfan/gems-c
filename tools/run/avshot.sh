#!/bin/bash
# Look at what a run draws: start m2hle headless with an A/V port, wait WAIT
# seconds, record 3 s with m2-hle2's tools/av-record.py and keep frames 30, 90
# and 150 as PNGs.
# Usage: avshot.sh NAME AVPORT WAIT [m2hle args...]   e.g. avshot.sh i960 7182 95 --gems-i960
# Env: M2HLE_EXE, M2HLE_TREE (an m2-hle2 checkout, for tools/av-record.py),
#      OUT (default /dev/shm/gems-av), ROMS_DIR. Needs ffmpeg.
name=${1:?name} port=${2:?av port} wait=${3:?seconds}; shift 3
EXE=${M2HLE_EXE:?set M2HLE_EXE}; TREE=${M2HLE_TREE:?set M2HLE_TREE to an m2-hle2 checkout}
d=${OUT:-/dev/shm/gems-av}/$name; mkdir -p "$d"; cd "$d" || exit 1
"$EXE" --headless --rom "${ROMS_DIR:?set ROMS_DIR to the folder holding sfight.zip}/sfight.zip" --profile sfight --region japan \
  --run --av-port "$port" --av-mute --log "$d/m2hle.log" "$@" > out.txt 2>&1 &
pid=$!; trap 'kill $pid 2>/dev/null' EXIT
python3 -c "import time; time.sleep($wait)"
python3 "$TREE/tools/av-record.py" --port "$port" --seconds 3 "$d/clip.mp4" > rec.txt 2>&1
ffmpeg -loglevel error -y -i "$d/clip.mp4" -vf "select=eq(n\,30)+eq(n\,90)+eq(n\,150)" -vsync 0 "$d/f%d.png"
echo "frames: $d/f1.png f2.png f3.png"
