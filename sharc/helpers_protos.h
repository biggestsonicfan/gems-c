/* Gems' COP helper functions (FUN_8001xxxx / FUN_800axxxx), prototypes.
 * Bodies in sharc/helpers.h. Signatures are the PowerPC's real ones, not
 * Ghidra's: single-precision values are float, the MSL fdlibm routines
 * (0x800a/0x800ae) are double.
 *
 * Conventions:
 *  - "idx" is a DM word offset from Gems' state pointer (the value kept at
 *    state+0xCFC is one): its words are gems_dm(0x30000 + idx).
 *  - Pointer params point at real floats/words: pass gems_dmf(0x30000 + idx)
 *    for state memory, or a local array.
 *  - A matrix is 12 floats: col0 (3), col1 (3), col2 (3), T (3).
 *  - Angles are 16-bit binary angles (0x10000 = 2 pi); only the low 16 bits count. */
#ifndef GEMS_SHARC_HELPERS_PROTOS_H
#define GEMS_SHARC_HELPERS_PROTOS_H

#include <stdint.h>

/* ---- matrices ---- */
static void     gch_8001d4a0(float *a, const float *b);          /* a = b (x) a (b's 3x3 on a's columns, T = b3x3 a_T + b_T), into a */
static void     gch_8001d688(const float *a, float *b);          /* b = b (x) a, the same product as d4a0, into b */
static void     gch_8001d870(uint32_t *dst, const uint32_t *src);/* copy one matrix (12 words) */
static void     gch_8001e270(uint32_t idx);                      /* inverse of matrix idx -> state+0xC0C */
static void     gch_8001e694(float s, float c, float *m);        /* turn m's col0/col1 by (s, c) */
static void     gch_8001e6f8(uint32_t ang, uint32_t idx);        /* rotate matrix idx about Z */
static void     gch_8001e7a4(float s, float c, float *m);        /* turn m's col2/col0 by (s, c) */
static void     gch_8001e808(uint32_t ang, uint32_t idx);        /* rotate matrix idx about Y */
static void     gch_8001e8b4(uint32_t ang, uint32_t idx);        /* rotate matrix idx about X */
static void     gch_8001e960(float sx, float sy, float sz, uint32_t idx); /* scale columns */
static void     gch_8001e9dc(float x, float y, float z);         /* current: T += M v */
static void     gch_8001ea5c(float x, float y, float z, uint32_t idx,
                             float *ox, float *oy, float *oz);   /* o = M v + T */
static void     gch_8002443c(uint32_t n);                        /* current = current (x) inner slot n */
static void     gch_80024558(uint32_t n);                        /* inner slot n = current */
static void     gch_800245e4(void);                              /* kage: normalise (leave axis) */
static void     gch_8002476c(void);                              /* kage: normalise (other axis) */
static void     gch_80024a34(uint32_t sel, uint32_t idx);        /* current = current (x) unit matrix (sel's low byte == 1: +0x4E0, else +0x420) */
static void     gch_80025214(void);                              /* current = identity, T = 0 */
static void     gch_800252e8(void);                              /* pop */
static void     gch_80025350(void);                              /* push */

/* ---- scalar maths (single precision) ---- */
static float    gch_8001d8e0(float x, float y);                  /* 1/sqrt(x*x + y*y) */
static float    gch_8001d9e0(float x, float y, float z);         /* 1/sqrt(x*x + y*y + z*z) */
static float    gch_8001dae4(float x, float y);                  /* sqrt(x*x + y*y) */
static float    gch_8001dbdc(float x, float y, float z);         /* sqrt(x*x + y*y + z*z) */
static float    gch_8001dcd8(float x);                           /* asin, +-pi/2 at |x| >= 1 */
static int32_t  gch_8001dd2c(float x);                           /* asin as a binary angle */
static float    gch_8001ddcc(float y, float x);                  /* atan2 (rational approximation) */
static int32_t  gch_8001dfd0(float y, float x);                  /* atan2 as a binary angle */
static void     gch_8001e038(float *a);                          /* wrap *a into [0, 2 pi) */
static float    gch_8001e084(float x);                           /* sqrt */
static float    gch_8001e174(float x);                           /* 1/sqrt */
static void     gch_8001ead8(float *s, float *c, uint32_t ang);  /* sin and cos from the table */
static float    gch_8001eb18(uint32_t ang);                      /* cos from the table */
static float    gch_8001eb40(uint32_t ang);                      /* sin from the table */

/* ---- collision ---- */
static float    gch_8001f9ac(uint32_t ball_idx, float *d, float *p);
static void     gch_8001faa4(float r, int32_t ball, uint32_t unit, int32_t *n_near, int32_t *n_hit,
                             uint32_t *mask, float *push, float *push_y, int32_t kind,
                             uint32_t m10, uint32_t m11, uint32_t m12, uint32_t m13,
                             uint32_t bit, uint32_t m15);
static uint32_t gch_800217b0(float x, float y, float z, float r, uint32_t bram_idx, uint32_t radius_idx,
                             float *push, float *push_x, float *push_z, uint32_t *last_ball);

/* ---- afterimages ---- */
static void     gch_80021d88(float step, const float *cur, const float *prev, uint32_t obj_idx,
                             uint32_t life_idx, uint32_t turn, uint32_t life_step, uint32_t player,
                             uint32_t part);

/* ---- reply FIFO, get_sm_ang readback ---- */
static void     gch_80022e7c(uint32_t w);                        /* queue one reply word */
static void     gch_80023ccc(void);                              /* reply the current matrix's 3 angles (u16 x3) */

/* ---- CodeWarrior runtime / MSL fdlibm (double) ---- */
static uint32_t gch_800a2544(double x);                          /* __cvt_fp2unsigned */
static void     gch_800a25cc(void);                              /* _savefpr (no-op) */
static void     gch_800a2618(void);                              /* _restfpr (no-op) */
static double   gch_800ab620(double x);                          /* __ieee754_asin */
static int32_t  gch_800acc04(double x, double *y);               /* __ieee754_rem_pio2 */
static int32_t  gch_800ad098(const double *x, double *y, int32_t e0, int32_t nx,
                             int32_t prec, const int32_t *ipio2); /* __kernel_rem_pio2 */
static double   gch_800adf8c(double x, double y, int32_t k);     /* __kernel_tan */
static double   gch_800ae4b4(double x);                          /* floor */
static double   gch_800ae688(double x, int32_t n);               /* scalbn */
static double   gch_800aea20(double x);                          /* tan */
static double   gch_800aea98(double x);                          /* asin */
static double   gch_800aede4(double x);                          /* sqrt */

#endif
