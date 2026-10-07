/* The SHARC's float mode for the COP handlers.
 *
 * The firmware sets MODE1 = 0x18000 at boot (cpres1.asm): TRUNCATE and RND32.
 * Every multiply and every add is rounded on its own to a 32-bit single, toward
 * zero; the ADSP-2106x has no fused multiply-add. It has no denormals either:
 * a denormal operand is taken as zero, and a result too small for a normal
 * single is flushed to zero (sign kept). An overflow gives the largest single,
 * as round toward zero does in IEEE.
 *
 * So each COP handler runs with the host in the same mode: gems_all.h's table
 * calls gcop_board_run(gcop_NN), which sets round toward zero and flush to
 * zero, runs the handler and puts the host's mode back. The rest of m2-hle2
 * never sees it. The handlers write each multiply and add apart (no fmaf), and
 * m2-hle2 builds with -ffp-contract=off, so the compiler fuses none.
 *
 * Hosts:
 *   x86-64 (SSE)   MXCSR: RC = toward zero, FTZ, DAZ
 *   AArch64        FPCR: RMode = toward zero, FZ (inputs and results)
 *   SH-4           FPSCR: RM = toward zero, DN (denormals are zero)
 *   other          fesetround(FE_TOWARDZERO) where <fenv.h> has it; no flush
 *   WebAssembly    has round-to-nearest only: the handlers run as before
 *
 * The GC's own libm (fdlibm, double: tan, asin) assumes round-to-nearest;
 * its callers run it under gcop_fp_nearest (the board has neither). */
#ifndef GEMS_SHARC_FPENV_H
#define GEMS_SHARC_FPENV_H

#include <stdint.h>

#if defined(__x86_64__) || defined(_M_X64) || ((defined(__i386__) || defined(_M_IX86)) && defined(__SSE2__))
#include <xmmintrin.h>
typedef unsigned int gcop_fp_t;
#define GCOP_FP_BOARD 1
static inline gcop_fp_t gcop_fp_get(void) { return _mm_getcsr(); }
static inline void gcop_fp_set(gcop_fp_t m) { _mm_setcsr(m); }
/* RC (bits 13-14) = 11 toward zero, FTZ bit 15, DAZ bit 6 */
static inline gcop_fp_t gcop_fp_board_of(gcop_fp_t m) { return m | 0x6000u | 0x8000u | 0x0040u; }
static inline gcop_fp_t gcop_fp_nearest_of(gcop_fp_t m) { return m & ~(0x6000u | 0x8000u | 0x0040u); }

#elif defined(__aarch64__)
typedef uint64_t gcop_fp_t;
#define GCOP_FP_BOARD 1
static inline gcop_fp_t gcop_fp_get(void) { uint64_t m; __asm__ volatile("mrs %0, fpcr" : "=r"(m)); return m; }
static inline void gcop_fp_set(gcop_fp_t m) { __asm__ volatile("msr fpcr, %0" :: "r"(m) : "memory"); }
/* RMode (bits 22-23) = 11 toward zero, FZ bit 24 */
static inline gcop_fp_t gcop_fp_board_of(gcop_fp_t m) { return m | (3u << 22) | (1u << 24); }
static inline gcop_fp_t gcop_fp_nearest_of(gcop_fp_t m) { return m & ~(uint64_t)((3u << 22) | (1u << 24)); }

#elif defined(__SH4__) || defined(__SH4_SINGLE__) || defined(__SH4_SINGLE_ONLY__)
typedef unsigned int gcop_fp_t;
#define GCOP_FP_BOARD 1
static inline gcop_fp_t gcop_fp_get(void) { return __builtin_sh_get_fpscr(); }
static inline void gcop_fp_set(gcop_fp_t m) { __builtin_sh_set_fpscr(m); }
/* RM (bits 0-1) = 01 toward zero, DN bit 18 */
static inline gcop_fp_t gcop_fp_board_of(gcop_fp_t m) { return (m & ~3u) | 1u | (1u << 18); }
static inline gcop_fp_t gcop_fp_nearest_of(gcop_fp_t m) { return m & ~(3u | (1u << 18)); }

#elif !defined(__wasm__) && !defined(__EMSCRIPTEN__)
#include <fenv.h>
#endif

#ifndef GCOP_FP_BOARD
#if defined(FE_TOWARDZERO) && defined(FE_TONEAREST)
typedef int gcop_fp_t;
#define GCOP_FP_BOARD 1
static inline gcop_fp_t gcop_fp_get(void) { return fegetround(); }
static inline void gcop_fp_set(gcop_fp_t m) { fesetround(m); }
static inline gcop_fp_t gcop_fp_board_of(gcop_fp_t m) { (void)m; return FE_TOWARDZERO; }
static inline gcop_fp_t gcop_fp_nearest_of(gcop_fp_t m) { (void)m; return FE_TONEAREST; }
#else
typedef int gcop_fp_t;
#define GCOP_FP_BOARD 0
static inline gcop_fp_t gcop_fp_get(void) { return 0; }
static inline void gcop_fp_set(gcop_fp_t m) { (void)m; }
static inline gcop_fp_t gcop_fp_board_of(gcop_fp_t m) { return m; }
static inline gcop_fp_t gcop_fp_nearest_of(gcop_fp_t m) { return m; }
#endif
#endif

/* -DGEMS_COP_NEAREST: the handlers keep the host's
 * round-to-nearest (the GC's rounding, without its FMA), to compare. */
#ifdef GEMS_COP_NEAREST
#undef GCOP_FP_BOARD
#define GCOP_FP_BOARD 0
#endif

/* Run one handler in the board's mode. The call goes through a volatile
 * pointer, so the compiler can neither inline the handler here nor move its
 * float work across the mode switch. */
static void gcop_board_run(void (*fn)(void)) {
    void (*volatile call)(void) = fn;
#if GCOP_FP_BOARD
    gcop_fp_t saved = gcop_fp_get();
    gcop_fp_set(gcop_fp_board_of(saved));
    call();
    gcop_fp_set(saved);
#else
    call();
#endif
}

/* The GC's fdlibm (double) under round-to-nearest, from inside a handler. */
static double gcop_fp_nearest(double (*f)(double), double x) {
    double (*volatile call)(double) = f;
#if GCOP_FP_BOARD
    gcop_fp_t saved = gcop_fp_get();
    gcop_fp_set(gcop_fp_nearest_of(saved));
    double r = call(x);
    gcop_fp_set(saved);
    return r;
#else
    return call(x);
#endif
}

#endif
