# Converting Sonic Gems Collection's decompiled C for m2-hle2

Paths are relative to this repository; `<scratch>` is any scratch directory.

Sonic Gems Collection (GameCube) runs Sonic the Fighters' i960 program under an
interpreter, but traps ~45 heavy i960 functions into native C, and replaces the
coprocessor's SHARC firmware with one C function per COP command. We have
Ghidra's decompiles of that C. Your job: turn a batch of them into clean,
structured, compilable C that runs inside m2-hle2 and does exactly what the
i960 code (FN) or the SHARC firmware as Gems implemented it (SHARC) does.

## Inputs
- Annotated decompiles (`tools/annotate/annotate.py`, run in `<scratch>`): `<scratch>/ann/FN/*.c`, `.../ann/SHARC/NN_name.c`,
  `.../ann/SHARC/helpers/FUN_xxxxxxxx.c` (sdata/sdata2 reads already resolved to `GCMEM_*(addr /*=value*/)` or float literals).
- Indexes: `decomp/FN/INDEX.md`, `decomp/SHARC/INDEX.md`.
- The m2-hle2 runtime you write against: `src/core/gems.h` in an m2-hle2 checkout (read it first).
- i960 disassembler: `tools/i960dasm/` (`i960dasm prog.bin 0xSTART 0xEND`)
  (STF's program ROM; the FN INDEX "i960" column is the i960 address; the trap table is `traps.txt`).
- PowerPC disassembly of stf.elf: `tools/ppc/disasm.sh` (grep the GC address) when Ghidra's output is ambiguous
  (float vs double rounding, signedness, unreachable blocks Ghidra removed).
- [stf-sharc](https://github.com/biggestsonicfan/stf-sharc) (if you have it): the real SHARC firmware sources, for names/intent only — port Gems' C, not the firmware.
- m2-hle2's own `src/board/sharc_exec.h`, `sharc_coli.h`, `sharc_zanzou.h`: our existing port of the same commands, a useful cross-check of intent.

## Conventions
### FN (i960 functions) — file `fn/<i960 name>.h`
- Entry: `static uint32_t gfn_<name>(int entry)` (see `fn_protos.h` for every name and the few exceptions; keep those signatures exactly).
  Trap entry functions (all in `traps.txt`) MUST be `uint32_t gfn_X(int entry)` even where Ghidra said void.
- Gems' `i960_cpu()` returns its register file: offset 0x00-0x3C = r0..r15, 0x40-0x7C = g0..g15 (g15 = fp), 0x114 = AC.
  `g4`, `r3` as bare names in the decompile are those registers. Write `GEMS_G(4)`, `GEMS_R(3)`, `GEMS_AC`.
  `*(int*)(cpu+0x114) = 4` is the condition code (4 less, 2 equal, 1 greater) — keep it, i960 code after the call may read it.
- Memory: `i960_ld32(&x, a)` -> `x = gems_ld32(a)`; `ld16/ld8`: decide zero- vs sign-extension from the i960 instruction
  (`ldos/ldob` zero, `ldis/ldib` sign; check the disassembly) -> `gems_ld16/ld16s/ld8/ld8s`. `i960_st*(a, &x)` -> `gems_st*(a, x)`.
  `i960_ldl/ldt/ldq(dst, a)` -> `gems_ldn(dst, a, 2/3/4)` (dst may be `&GEMS_G(n)`); `stl/stt/stq` -> `gems_stn`.
  `ram_rd32/rd16/rd16s/rd8`, `ram_wr32/wr16` (main RAM, i960 address) -> `gems_ld32/...`, `gems_st32/...`. Floats: `gems_ldf/gems_stf`, `gems_u2f/gems_f2u`.
- COP: `cop_write(x)` -> `gems_cop_w(x)` (float: `gems_cop_wf`), `cop_read()` -> `gems_cop_r()`/`gems_cop_rf()`,
  `cop_writeN_from(p)` -> `gems_cop_wn(p, N)`, `cop_readN_to(p)` -> `gems_cop_rn(p, N)`.
- Control: `i960_ret()` -> `gems_i960_ret()`. Keep the `if (entry) gems_i960_ret(); return 0;` shape: a native call
  (entry 0) must not pop the i960 frame. A nonzero return R resumes the i960 at trap site + R/2 — keep R exactly.
  `FUN_8002d34c(x)` -> `gems_branch(x/2)` (Gems passes a doubled IP). Calls to other `gc_X(0)` -> `gfn_X(0)`.
- Gems drops writes to the debug history at 0x50F600-0x50F61F; fine to keep that as Gems did.
- GC-only natives (`FUN_8002f884`, `FUN_8002f898`, `FUN_8002f978`, `FUN_8002fa28`, `FUN_80037840`, `FUN_80056868`,
  `FUN_80056a28`, `FUN_8005ec30`, `FUN_8005ee84`, `GCMEM_uint(0x801e7cc8)`, `PTR_FUN_80150258`) are the GameCube
  renderer / GX FIFO / tables. Where one appears, write what the **i960 code** does at that point instead
  (read the i960 disassembly of the same function), so the board sees the same writes as the ROM.
### SHARC (COP commands) — file `sharc/NN_<name>.h`, NN = lowercase hex opcode
- Handler: `static void gcop_NN(void)` (e.g. `gcop_0b`). Args: `cop_in_float()` -> `gems_in_f()`, `cop_in_word()` -> `gems_in_w()`.
  Replies: `cop_out_float(f)` -> `gems_out_f(f)`, `cop_out_word(w)` -> `gems_out_w(w)`, `cop_out_u16(v)` -> `gems_out_w((uint16_t)v)`
  (check the PPC code for whether it zero- or sign-extends).
- Gems' COP state pointer `GCMEM_int(0x801e7c30)` is an image of SHARC data memory from DM 0x30000:
  byte offset `o` from it is DM word `0x30000 + o/4`. Use `gems_dm(0x30000 + o/4)` / `gems_dmf(...)` (uint32_t* / float*).
  `state - 0xC0000 + addr*4` is DM address `addr`. +0xCFC is the current-matrix DM index (starts 0x5A0, push adds 0xC), +0xCF0 the stack depth.
- `bram_rd32(i)/bram_wr32(i,v)/bram_rd_float/bram_wr_float` take WORD indexes into bufferram: `gems_bram_rd/wr/rdf/wrf`.
- Other sdata globals (`GCMEM_*(0x801e7c34..)`), `DAT_800f9f20` (a 64K-entry float sine table), `DAT_801e7a18`: owned by
  `sharc/state.h` (helpers agent). Each becomes a named static in state.h; `gcop_reset()` (in state.h) puts them at their power-on values.
- Helpers `FUN_8001xxxx` -> `gch_8001xxxx` with the signatures in `sharc/helpers.h` (helpers agent owns it).
- Drop Ghidra artifacts: `unaff_GQR0`/`ldexpf(...)` blocks around paired-single loads are quantisation no-ops (GQR0 = 0); `cop_stub_NN` is an empty handler.
- **Arithmetic is the board's**: single precision (`fadds/fmuls/...`), and the GC's fused multiply-adds
  (`fmadds/fmsubs/fnmadds/fnmsubs`) written as a separate multiply and add (`a * b + c`), never `fmaf`: the SHARC has
  no FMA. The handlers run under round toward zero (`sharc/fpenv.h`). Where a sum of three or more terms has
  a firmware counterpart in cpres1.asm, write it in the firmware's order.
  Where Ghidra shows `(double)` round trips, check the PPC: single ops round to float each step. Don't "simplify" float expressions.

## Unspaghetti, but exact
- Turn gotos/`do{}while` soup into structured loops and ifs; name locals for what they hold (use the i960 names from the
  INDEX/disassembly and m2-hle2's CLAUDE.md where known); delete dead temporaries Ghidra invented; short comments only where it helps.
- Do not change behaviour: every memory write (address, width, order), every COP word, every register left behind, the condition code,
  and the returned R must stay as Gems has it. When unsure what Ghidra meant, read the PPC disassembly.
- C11, `static` functions only, header-only, no globals except in `sharc/state.h` (and `static` locals only if Gems has real state there).

## Checking
- `MINIZ_GEN=<m2-hle2 build>/miniz-gen ./check.sh <m2-hle2 checkout>`
  syntax-checks everything converted so far (all agents' files together; another agent's half-finished file may break it —
  then check yours alone by temporarily copying this repository's C to your scratch and deleting other files there).
  Zero warnings in your files.
- `lizard -l c -m -w fn sharc gems_impl.h`: no new warning, and no function's branch count raised beyond the original's (README, "Complexity").
- Final report (short): files written, anything you could not convert exactly and why, any wrong shared prototype.
