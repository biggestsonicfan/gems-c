# Sonic Gems Collection C for m2-hle2

Sega's C from Sonic Gems Collection (GameCube, `stf.elf`), cleaned up so it
builds inside [m2-hle2](https://github.com/biggestsonicfan/m2-hle2) against
`src/core/gems.h`.

The aim is C hooks for the emulator that are as close to the board as we can
make them, not a copy of Gems: where the board does something else, follow the
board. A faithful record of Gems' C is
[gems-decomp](https://github.com/biggestsonicfan/gems-decomp) (`decomp/`).

m2-hle2 itself has only the framework: `gems.h`, the CMake option and the
command-line flags. A build without this repository still accepts the flags,
which then log that they do nothing.

## What Gems does, and what this is

The Gems port runs the arcade i960 program under an interpreter in the
PowerPC executable. Two parts of it are native C instead:

- **i960 traps.** Gems patches heavy i960 functions with a trap word
  (`traps.txt`, 64 entries: trap number, i960 address, name; the 47 named
  ones, 0x65-0x93, are converted and in `gems_traps[]`; the other 17 have no
  name in Gems' table and are not converted). The trap handler is at GC
  `0x800262D8`. It calls an entry function with `1`, which returns R:
  - R != 0 resumes the i960 at site + R/2 (Gems decodes 8 bytes per i960 word).
  - R == 0 means the C set the IP itself, with `i960_ret()` or a branch.

  Examples: `calc_unit_mat`, `get_frame_dat`, `coli_cont_cop`, `osage_dsp`. The
  entry functions call further i960 functions that Gems also ported to C. That
  makes 94 functions in `fn/`.
- **The coprocessor.** The SHARC firmware is replaced by one C function per COP
  command, under the firmware's own `Fn_*` names (`sharc/`: the commands, plus
  helpers and state).

m2-hle2 runs that C in place of its own code:

| flag | does |
|---|---|
| `--gems-i960` | a hook at every trap site runs `gfn_<name>(1)` instead of the i960 |
| `--gems-cop` | COP commands go to `gcop_NN` instead of `sharc_exec` (Gems handlers with an empty body are left to ours, so the i960 still gets its replies) |
| `--gems-verify` | each trapped function both ways (see "Verify") |

Our own `set_obj` hook (COP 0x78) stays. Gems' `set_obj` is converted too and
kept beside it, for reference.

Neither flag is the board:
- A trapped function is charged as one i960 instruction, so the timers (and
  `rand` at `0x66B0`, which reads them) see less time.
- Gems' COP keeps the GC's algorithms for division, square roots, atan2 and
  sin/cos, where the SHARC firmware has its own (see "The COP's arithmetic").
  Its arithmetic is the board's: no fused multiply-add, round toward zero.

A netplay session turns both flags off (`g_hle_extra_session_off`). Both apply
only to the sfight set (both STF profiles).

## Layout

| path | what |
|---|---|
| `fn/<name>.h` | one i960 function: `static uint32_t gfn_<name>(int entry)`. `entry` 1 = called from the trap (pops the i960 frame itself), 0 = called from other C |
| `fn_protos.h` | prototypes of every `gfn_*`, so files can call each other in any order |
| `sharc/NN_<name>.h` | one COP command: `static void gcop_NN(void)`, NN = lowercase hex opcode |
| `sharc/helpers.h` | the firmware helpers Gems calls: `gch_<GC address>` |
| `sharc/fpenv.h` | the SHARC's float mode (round toward zero, flush to zero) around each handler; see "The COP's arithmetic" |
| `sharc/state.h` | Gems' COP globals (r13 sdata), the 64K-entry sine table, `gcop_reset()` |
| `traps.txt` | the trap table, read by `gen_all.py` |
| `gen_all.py` | writes `gems_all.h`: includes plus `gems_traps[]` and `gems_cop_ops[]` (each handler wrapped in `gcop_board_run`; `--cop-only` leaves out `fn/` and the traps). Only functions that are defined go in the tables, so a half-converted tree builds. Argument and reply counts come from `decomp/SHARC/INDEX.md` |
| `gems_all.h` | written by `gen_all.py`; not tracked (`.gitignore`) |
| `gems_impl.h` | what `gems.h` includes (`fn_protos.h` + `gems_all.h`) |
| `check.sh` | syntax-check everything against an m2-hle2 tree (makes its own `gems_all.h` in a temp dir) |
| `tools/` | everything used to make and check the conversion (below) |
| `decomp/` | the submodule [gems-decomp](https://github.com/biggestsonicfan/gems-decomp): Ghidra's raw decompiles, the source material |

The source material is Ghidra's raw decompiles in `decomp/FN` and
`decomp/SHARC`, with their `INDEX.md` files (GC address, i960 address, argument
and reply counts). `gen_all.py` needs `decomp/SHARC/INDEX.md`, so check the
submodule out: clone with `--recursive`, or run
`git submodule update --init decomp`. `stf.elf`, the GameCube executable, is in
neither repository; the tools below that read it need your own copy.

## Building m2-hle2 with it

You need an m2-hle2 checkout that builds on its own first (see its README), and
Python 3: the build runs `gen_all.py` to write `gems_all.h` here.

**As m2-hle2's submodule.** m2-hle2 carries this repository as the submodule
`vendor/gems-c`, and this one carries `decomp/`. In an m2-hle2 checkout:

```bash
git submodule update --init --recursive vendor/gems-c
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

CMake's `M2HLE_GEMS_DIR` (and the Dreamcast Makefile's `GEMS=`) then default to
`vendor/gems-c`.

**From a copy anywhere else**, name this directory:

```bash
cmake -S <m2-hle2> -B <build> -DCMAKE_BUILD_TYPE=Release \
      -DM2HLE_GEMS_DIR=<path to this repository>
cmake --build <build>
```

CMake runs `gen_all.py` when it configures, and again when a file in `fn/` or
`sharc/`, `traps.txt` or `decomp/SHARC/INDEX.md` changes. The option fails
configure if `gems_impl.h` is missing or `gen_all.py` fails. Run
`python3 gen_all.py` yourself only when building some other way.

Then run m2-hle2 with `--gems-i960`, `--gems-cop` or both on the `sfight` ROM
set (your own dump; not in either repository).

The runtime API the C is written against is all in `gems.h`:
- Registers: `GEMS_R(n)`, `GEMS_G(n)`, `GEMS_AC`.
- Memory: `gems_ld32/16/16s/8/8s`, `gems_st*`, `gems_ldn/stn`, `gems_ldf/stf`.
- COP from the i960 side: `gems_cop_w/wf/wn`, `gems_cop_r/rf/rn`.
- COP state: `gems_in_f/w`, `gems_out_f/w`, `gems_dm`/`gems_dmf` (DM from
  0x30000), `gems_bram_rd/wr/rdf/wrf`.
- Control flow: `gems_i960_ret`, `gems_branch`.
- A profile's hook inside a trapped function: `gems_inner(0x...)` gives the
  active profile's hook on that instruction or NULL, and `gems_inner_call`
  runs it. The C puts the registers the instruction would hold into
  `GEMS_R`/`GEMS_G` first and takes back what the hook wrote. `gen_all.py`
  lists every `gems_inner(0x...)` in `fn/*.h` as `GEMS_INNER_SITES`, and
  `gems.h` then lets that profile use the trap. `get_frame_dat` does this for
  sfight_console's head tilt (0x30608, the fade-out loop's `mulr`); before, the
  Console profile left the whole function to the i960. An m2-hle2 from before
  the call (no `GEMS_INNER`) gets a stub from `gems_impl.h` and keeps its old
  rule.

`tools/BRIEF.md` is the conversion brief: every Ghidra-to-runtime mapping rule,
the float rules, and what "exact" means.

## Checking

```bash
MINIZ_GEN=<m2-hle2 build>/miniz-gen ./check.sh <m2-hle2 checkout>
```

It compiles `gems.h` with everything here (gcc `-fsyntax-only -Wall`). It
needs `miniz-gen` from a configured m2-hle2 build tree: `$MINIZ_GEN`, default
`<m2-hle2 checkout>/build/miniz-gen`.

### Complexity (lizard)

Run [lizard](https://github.com/terryyin/lizard) (`pip install lizard`) after
changing any C here:

```bash
lizard -l c -m -w fn sharc gems_impl.h      # functions over the limits
lizard -l c -m fn sharc gems_impl.h | tail -3   # totals
```

`-m` counts a whole `switch` once, as m2-hle2's `tools/spaghetti.py` does, and
`-w` prints only functions over lizard's defaults (15 branches, 1000 lines,
100 parameters). The code follows Sega's own functions one for one, so some of
those are as branchy as the originals (19 warnings of 396 functions when this
was written, `gch_800ad098` at 42 the worst). A change should not add a warning
or raise a function's count unless the original it follows has the branches;
split a helper out rather than grow one, and say in the commit if a count went up.

### Verify: the C against the i960, call by call

`--gems-verify` (implies `--gems-i960`) does this at every trap:
1. Snapshot the board: CPU, main RAM, RAM2, bufferram, tiles, palette, backup
   RAM, COP state, and the display-list pointers.
2. Run the C.
3. Snapshot what the C left.
4. Restore the first snapshot and step the i960 from the same state to where
   the C resumed.
5. Diff the two results.

The board carries on from the i960's run, so a verify run plays exactly as the
ROM does, and the report covers a long run.

What it counts and skips:
- Words that agree to about 1e-4 relative are counted as "float-close": PowerPC
  FMA against the i960's rounding.
- The i960's dead frames above SP (callee locals the C keeps on the host) are
  skipped.
- A run that differs only in registers is counted separately.

When it checks:
- By default it checks every call of a trap for the first 256 calls, then one
  in 32.
- `M2HLE_GEMS_VERIFY_ALL=1` checks every call. This is slow: a whole-board
  snapshot per call.

The report goes to the log at exit (`gems-verify:` lines), one row per trap:
calls / exact / float-close / regs-only / differ / lost. The first four
differences of each trap are logged with the first differing address.

### Run scripts (`tools/run/`)

All of them take `M2HLE_EXE` (an m2hle built with this directory) and
`ROMS_DIR` (the folder holding your `sfight.zip`), and run with
`--profile sfight --region japan` (the MAME-comparable profile). Each runs from
its own scratch dir, never from the ROM folder (m2hle writes beside its working
directory), and kills its m2hle on exit. Scratch output defaults to
`/dev/shm/...`; each script's header names the variable that moves it.

- `runverify.sh SECONDS [args]` runs headless with the MCP bridge for SECONDS,
  then sends `get_status` and `quit`, so the verify report is written. Example:
  `M2HLE_GEMS_VERIFY_ALL=1 RUN=/dev/shm/rv tools/run/runverify.sh 450 --gems-verify`.
  450 s covers attract's intro, the replay fight and the ranking.
- `avshot.sh NAME AVPORT WAIT [args]` shows what a run draws. It waits WAIT
  seconds, records 3 s through `--av-port` with m2-hle2's
  `tools/av-record.py` (`M2HLE_TREE`), and keeps 3 PNGs. Example:
  `tools/run/avshot.sh i960 7182 95 --gems-i960`. At 95 s, attract is past the
  replay fight, on the ranking screen.
- `match-replay.sh OUTDIR [--mame]` holds attract's replay fight against MAME
  frame by frame (m2-hle2's `tools/match-replay.mjs`). The first run with
  `--mame` takes the reference with MAME (`MAME_EXE`: a MAME with the
  model2 driver).
  This is the check for anything that changes fight state: the i960 core, a
  COP handler, or a Gems function in the fight.

## Tools for reading the code (`tools/`)

### i960 side (STF's program ROM)

- **`tools/i960dasm/`**: a command-line wrapper around MAME's own i960
  disassembler (`i960dis.cpp`).
  - `build.sh [outdir]` compiles it against a MAME source tree (`$MAME_SRC`,
    the tree's `src/` directory). `mkprog.py` builds `prog.bin`, the i960's
    little-endian program image, from sfight's two program EPROMs
    (`$ROMS_DIR/sfight.zip`).
  - Run: `i960dasm prog.bin 0xSTART 0xEND`.
  - Use it for the exact instruction a C load or store stands for, e.g. `ldos`
    (zero-extend) against `ldis` (sign-extend), or which register is left
    behind.
- **[stf-tools](https://github.com/biggestsonicfan/stf-tools)' `i960dis.mjs`**: a
  small i960 disassembler in JS that labels the stage-object routines from the
  stage table. Run: `node i960dis.mjs <hex addr> [hex len] [sfight.zip]`. It
  covers only the forms the game uses, so use MAME's for anything unusual.
- **An IDA (or Ghidra) database of STF's i960 program**, if you have one, for
  i960 names, struct offsets (rob +N) and the callers of a trapped function.
  The conversion used one with Sega's names, which is where the names in
  `traps.txt` and `fn/` come from.

### PowerPC side (stf.elf)

The tools here look for `stf.elf` at the root of this repository; copy yours
there. It is not tracked.

- **`tools/ppc/disasm.sh [out]`** disassembles stf.elf's `.text` (PowerPC 750)
  with `gdb-multiarch` into one ~7.6 MB listing (default
  `/dev/shm/gems/text1.s`), to grep by GC address. Read it when Ghidra's C is
  ambiguous:
  - fused multiply-adds (`fmadds` / `fmsubs` / `fnmadds` / `fnmsubs`, written
    as a separate multiply and add: the board has no FMA)
  - single against double rounding
  - signedness of a load (`lha` against `lhz`)
  - blocks Ghidra dropped as unreachable
- **`tools/annotate/annotate.py`** (with `elf.py`, a minimal big-endian ELF
  reader) resolves Ghidra's `*(type *)(unaff_r2/r13 + off)` sdata and sdata2
  reads to their values, as float literals or `GCMEM_*(addr /*=value*/)`. Run
  it in a scratch dir: it reads `decomp/FN`, `decomp/SHARC` and
  `decomp/SHARC/helpers` and writes `ann/FN`, `ann/SHARC` and `ann/SHARC/helpers`
  in the cwd. Those annotated files were the conversion's input. r2 =
  `0x801F0520`, r13 = `0x801EF7E0`.
- **Ghidra with the PS2 release.** The PS2 Sonic Gems Collection's STF
  executable is the same C compiled for MIPS: a second reading where the GC
  decompile is unclear.

### The firmware itself

- **[stf-sharc](https://github.com/biggestsonicfan/stf-sharc)** has the real SHARC firmware:
  - `cpres1.asm`, the COP: 136 commands, reassembles bit-for-bit, with Sega's
    labels.
  - `cpres2.asm`, the GEO feed.

  It is the reference for what a COP command should do on the board. Gems' C
  is the reference only for what Gems did.
- m2-hle2's own port of the same commands is in `src/board/sharc_exec.h`,
  `sharc_coli.h` and `sharc_zanzou.h`.

## Things learned the hard way

- **NaN compares.**
  - The SHARC's `comp` on a NaN sets only AI. So `if gt` / `if ge` are taken
    and `if lt` / `if le` are not.
  - The i960's `cmpr` on a NaN sets CC 000, so `bge` and `ble` do not branch
    (MAME `compute_fcomp`, `cmp_d`).
  - Gems' C gets both right. m2-hle2 had both wrong, and the comparison found
    it: attract's replay fight split from MAME at +510, and the `--gems-i960`
    run stopped on "max poly / err poly". Fixed in m2-hle2
    [PR #197](https://github.com/biggestsonicfan/m2-hle2/pull/197).
  - Write a firmware `comp(a,b); if gt` as `!(a <= b)`, never `a > b`.
- **The condition code is state.** A trapped function's
  `*(int*)(cpu+0x114) = n` is the CC the i960 code after the call may read.
  Keep it.
- **Keep R exactly.** A native call (entry 0) must not pop the i960 frame:
  keep the `if (entry) gems_i960_ret(); return 0;` shape.
- **Where Gems leaves the board differently from the i960, the conversion
  follows the i960.** For example, Gems skips the clearing of the enemy-command
  history at `0x50F600` (`select_enemy_command`), and the conversion writes
  it. This covers memory, registers and the condition code.
- **GC-only renderer calls are replaced.** The GX FIFO and tables
  (`FUN_8002f884`, `FUN_80056868`, ...) are replaced by what the i960 code
  writes at that point, so the board sees the ROM's display list.
- **Where the flags are wired in:** `src/main.c` parses them, and
  `gems_apply()` runs after the profile installs.

## The PS2 build (2026-10-07)

Gems' PS2 build compiles the same C for the Emotion Engine. Compared with
the GC build, handler by handler and for every FN trap:

- It is better in two places, and the conversion now follows it:
  - `Fn_outside_ball` (72) adds the push-out (dx, dz) and counts a NaN as
    outside. The GC handler drops both args.
  - `Fn_area_table_gen` (3a) turns a cell below 0 into an index signed, so it
    wraps (`& 63`). The GC handler makes it 0.
- Everywhere else it is the same as the GC or worse (no zero guard in rsqrt,
  `tri_shin` replies NaN, whole-word selector compares). The FN traps are
  the same in both.
- The EE rounds toward zero and has no FMA, like the board's SHARC
  (`mode1` TRUNCATE + RND32). The conversion now does the same (next
  section).

## The COP's arithmetic (Pinboard #548)

The board's SHARC firmware sets MODE1 = 0x18000 at boot (cpres1.asm): every
multiply and every add is rounded on its own, toward zero, to a 32-bit single.
It has no fused multiply-add, and denormals are flushed to zero. The handlers
here now compute the same way:

- **No FMA.** Each of the GC's `fmadds` / `fmsubs` / `fnmadds` / `fnmsubs` is
  a multiply and an add (`a * b + c`). m2-hle2 builds with
  `-ffp-contract=off`, so the compiler fuses none of them back.
- **Round toward zero, flush to zero**, set around each handler by
  `sharc/fpenv.h`. `gen_all.py` wraps every table entry, the reset, and the
  zanzou feed in `gcop_board_run`. It saves the host's mode, sets the board's,
  calls through a volatile pointer, and puts the host's back, so no other float
  code in m2-hle2 runs in it. On x86-64 that is MXCSR (RC, FTZ, DAZ). On
  AArch64 it is FPCR, and on SH-4 FPSCR. Elsewhere it falls back to
  `fesetround`. WebAssembly has round-to-nearest only and runs as before. The
  cost did not show: a replay of 676,781 commands takes the same 0.05-0.07 s
  either way.
- `-DGEMS_COP_NEAREST` keeps the host's round-to-nearest, for comparisons.
- The GC's fdlibm (`tan`, `asin`) is double precision written for
  round-to-nearest. It runs under `gcop_fp_nearest`.
- **The firmware's order where a sum has three or more terms.** The GC sums the
  products first, while `Fn_trans`, `Fn_point_trans` and `Fn_osage`'s segment
  ends add onto T a term at a time (`_L20182`, `_L20173`). Osage's sphere adds
  (z² + y²) + x² (`_L20873`). The other sums were checked against cpres1.asm
  and already agree.

What stays the GC's are its algorithms. The board divides with a `recips` seed
and three Newton steps (`_L205D0`), and has its own sqrt and 1/sqrt. Its
atan2 is ADI's (`_L202D1`), and its sin and cos come from the COP data ROM's
tables. m2-hle2's `sharc_fw_div`, `sharc_fw_sqrt`, `sharc_fw_rsqrt`,
`sharc_fw_atan2` and `sharc_sincos` model them. These are what the table
below still misses.

### Checking it: `tools/cop_replay/`

`capture.lua` records the COP firmware's side of its FIFOs off MAME, from
power-on to 600 frames into attract's replay fight. With `CAP_SNAP=1` it also
records the current matrix before every command. `build.sh <m2-hle2> [out]`
builds `gems_cop_replay`: m2-hle2's `tests/cop_replay.c` with the handlers
here in place of `sharc_exec` (op 0x78 and anything Gems leaves empty still
go to ours).

```bash
cd <scratch>   # not the ROM folder
CAP_OUT=$PWD/cap CAP_SNAP=1 mame sfight -rompath $ROMS_DIR -nodrc -video none -sound none \
    -nothrottle -skip_gameinfo -seconds_to_run 299 -cfg_directory cfg -nvram_directory nv \
    -autoboot_script <this repo>/tools/cop_replay/capture.lua
tools/cop_replay/build.sh <m2-hle2> /dev/shm/gcr/gems_cop_replay
COPRO_ROM=<the interleaved mpr-19015/19016> RESYNC=1 STATE_EXACT=1 \
    /dev/shm/gcr/gems_cop_replay <scratch>/cap 0
```

`RESYNC` puts the board's matrix in before each command, so one command's
difference does not run into the next. `STATE_EXACT` counts a matrix that
differs in any bit as a bad state.

Stock MAME's SHARC ignores TRUNCATE: its DRC rounds to nearest, and so does
its interpreter. So "the board" below is a MAME whose SHARC rounds toward zero
in both. That was a throwaway build, made from the fork's branch
`idea-548-sharc-rz-exp`. The table gives reply words bit for bit, of 748,805,
over one capture each (2026-10-07):

| handlers | vs MAME, toward zero (the board) | vs stock MAME (nearest) |
|---|---|---|
| main before #548 (GC: FMA, nearest) | 82.37% | 90.38% |
| no FMA, board order, nearest | – | 93.66% |
| no FMA, board order, toward zero (now) | **92.63%** | – |
| m2-hle2's `sharc_exec` | 69.24% | 84.70% |

Against the board, these are now exact for every word: `Fn_div`, `Fn_sub`,
`Fn_mul3`, `Fn_get_inner_2d`, `Fn_glo_to_loc`, `Fn_point_trans`,
`Fn_area_coli`, `Fn_mul_matrix`, `Fn_mul_unit_mat` and
`Fn_mul_matrix_inner`. The rest is the algorithms above:
- sin/cos: `sin`, `cos`, `tan`, the rotations, `rot_2d`, `get_loc_pos`, `calc_unit_hara`
- sqrt: `sqr`, the lengths, `regular_vector`, and `osage` (its points carry over from frame to frame)
- atan2: `get_2d_dir`, `sm_ang_f`/`_r`
- the divide: `fcurve_lin`/`_spl`, `inv_matrix`

## Status (2026-10-04)

Measured with `M2HLE_GEMS_VERIFY_ALL=1 tools/run/runverify.sh 450 --gems-verify`
over 6552 frames of attract: the intro, the replay fight and the ranking.

- 40 of the 47 converted traps ran in that stretch.
- 635,759 calls were checked. Every call left the board exactly as the i960
  did, except 14 `osage_dsp` calls that are float-close (PowerPC FMA).
- No call differed and none was lost.
- `--gems-i960` alone plays attract through the fight to the ranking screen.

Seven traps did not run in attract: `set_coli_ball_data`, `dented_cnt`,
`scale_parts_cnt`, `copy_option_data`, `set_obj_fifo`, `fill_pattern_s` and
`send_ram_coli_data`. A played game (select, fight, continue) is the way to
reach them. `--gems-cop` has no verify mode of its own; its check is
`match-replay.sh` and a look with `avshot.sh`.

## Host maths (`GEMS_HOST_MATH`, the Dreamcast disc)

A host that defines `GEMS_HOST_MATH` with `GEMS_HOST_SIN`, `GEMS_HOST_COS`
(16-bit binary angle) and `GEMS_HOST_SQRTF` replaces the sine table, the
double-precision Newton square root and the `fctiwz` angle word with its own
(`sharc/state.h`, `sin_table.h`, `helpers.h`). The Dreamcast game build
(`dreamcast/dc_math.h`, `HOST_MATH=1`) maps them to the SH-4's FSCA and FSQRT:
the 256 KB table drops out of the binary. It is not the Broadway's arithmetic,
so the board's numbers move; nothing else defines it, and the desktop is
unchanged.
