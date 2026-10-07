/* Gems' COP helper functions, ported from its PowerPC code (Sonic Gems
 * Collection's stf.elf): FUN_8001xxxx / FUN_8002xxxx are its matrix, maths and
 * collision helpers, FUN_800axxxx the CodeWarrior runtime and MSL fdlibm.
 * Prototypes and conventions: helpers_protos.h.
 *
 * Floats are single precision, as on the PowerPC and the SHARC. The GC's
 * fused multiply-adds (fmadds a*c+b, fmsubs a*c-b, fnmadds, fnmsubs) are
 * written as a multiply and an add, each rounded on its own, because the
 * board's SHARC has no FMA (fpenv.h has its rounding). Operation order is the
 * GC's, never simplified, except where a sum is checked against the board's
 * firmware (cpres1.asm) and written in its order; those say so. */
#ifndef GEMS_SHARC_HELPERS_H
#define GEMS_SHARC_HELPERS_H

#include <math.h>
#include <stdint.h>
#include <string.h>
#include "helpers_protos.h"

/* ---- small tools ---- */

/* Gems' COP state as floats / words: S[k] is DM 0x30000 + k. */
static inline float    *gch_S(void)  { return gems_dmf(0x30000u); }
static inline uint32_t *gch_SU(void) { return gems_dm(0x30000u); }
/* The current matrix (state+0xCFC holds its index). */
static inline float    *gch_cur(void) { return gems_dmf(0x30000u + *gems_dm(0x3033Fu)); }

#define GCH_PI     3.14159274101257324f    /* 0x40490FDB */
#define GCH_PI_2   1.57079637050628662f    /* 0x3FC90FDB */
#define GCH_2PI    6.28318548202514648f    /* 0x40C90FDB */

static inline uint32_t gch_dhi(double d) { uint64_t u; memcpy(&u, &d, 8); return (uint32_t)(u >> 32); }
static inline uint32_t gch_dlo(double d) { uint64_t u; memcpy(&u, &d, 8); return (uint32_t)u; }
static inline double   gch_dmk(uint32_t hi, uint32_t lo) {
    uint64_t u = ((uint64_t)hi << 32) | lo; double d; memcpy(&d, &u, 8); return d;
}
static inline double gch_set_hi(double d, uint32_t hi) { return gch_dmk(hi, gch_dlo(d)); }
static inline double gch_set_lo(double d, uint32_t lo) { return gch_dmk(gch_dhi(d), lo); }

/* fctiwz: truncate toward zero, saturating; NaN is 0x80000000. */
static inline int32_t gch_fctiwz(double x) {
    if (x != x) return INT32_MIN;
    if (x >= 2147483648.0) return INT32_MAX;
    if (x <= -2147483649.0) return INT32_MIN;
    return (int32_t)x;
}

/* frsqrte, as the Broadway computes it (Dolphin's ApproximateReciprocalSquareRoot). */
static double gch_frsqrte(double val) {
    static const struct { int32_t base, dec; } tbl[32] = {
        {0x1a7e800, -0x568}, {0x17cb800, -0x4f3}, {0x1552800, -0x48d}, {0x130c000, -0x435},
        {0x10f2000, -0x3e7}, {0x0eff000, -0x3a2}, {0x0d2e000, -0x365}, {0x0b7c000, -0x32e},
        {0x09e5000, -0x2fc}, {0x0867000, -0x2d0}, {0x06ff000, -0x2a8}, {0x05ab800, -0x283},
        {0x046a000, -0x261}, {0x0339800, -0x243}, {0x0218800, -0x226}, {0x0105800, -0x20b},
        {0x3ffa000, -0x7a4}, {0x3c29000, -0x700}, {0x38aa000, -0x670}, {0x3572000, -0x5f2},
        {0x3279000, -0x584}, {0x2fb7000, -0x524}, {0x2d26000, -0x4cc}, {0x2ac0000, -0x47e},
        {0x2881000, -0x43a}, {0x2665000, -0x3fa}, {0x2468000, -0x3c2}, {0x2287000, -0x38e},
        {0x20c1000, -0x35e}, {0x1f12000, -0x332}, {0x1d79000, -0x30a}, {0x1bf4000, -0x2e6},
    };
    uint64_t bits; memcpy(&bits, &val, 8);
    const uint64_t qnan = 0x7FF8000000000000ull;
    uint64_t mantissa = bits & ((1ull << 52) - 1);
    uint64_t sign     = bits & (1ull << 63);
    int64_t  exponent = (int64_t)(bits & (0x7FFull << 52));
    double r;
    if ((bits & ~(1ull << 63)) == 0) return sign ? -INFINITY : INFINITY;
    if (exponent == (int64_t)(0x7FFull << 52)) {
        if (mantissa == 0) { if (!sign) return 0.0; memcpy(&r, &qnan, 8); return r; }
        bits |= 1ull << 51; memcpy(&r, &bits, 8); return r;
    }
    if (sign) { memcpy(&r, &qnan, 8); return r; }
    if (exponent == 0) {
        do { exponent -= 1ll << 52; mantissa <<= 1; } while (!(mantissa & (1ull << 52)));
        mantissa &= (1ull << 52) - 1;
        exponent += 1ll << 52;
    }
    int64_t exponent_lsb = exponent & (1ll << 52);
    exponent = ((0x3FFll << 52) - ((exponent - (0x3FEll << 52)) / 2)) & (0x7FFll << 52);
    uint64_t out = sign | (uint64_t)exponent;
    int i = (int)(((uint64_t)exponent_lsb | mantissa) >> 37);
    out |= (uint64_t)(int64_t)(tbl[i / 2048].base + tbl[i / 2048].dec * (i % 2048)) << 26;
    memcpy(&r, &out, 8);
    return r;
}

/* The square-root core of d8e0..dbdc, e084, e174: s > 0. Three Newton steps
 * on frsqrte in double, then s * (1/sqrt s), rounded to single. */
#ifdef GEMS_HOST_MATH
/* A host with a single-precision square root (GEMS_HOST_SQRTF: the SH-4's
 * FSQRT) takes it, correctly rounded, in place of the Broadway's double
 * Newton steps, which a host without double hardware emulates. */
static float gch_sqrt_pos(float s) { return GEMS_HOST_SQRTF(s); }
#else
static float gch_sqrt_pos(float s) {
    double ds = s, g = gch_frsqrte(ds), t;
    for (int k = 0; k < 3; k++) {
        t = g * g;
        g = 0.5 * g;
        t = -fma(ds, t, -3.0);
        g = g * t;
    }
    return (float)(ds * g);
}
#endif
/* sqrt of s: 0 is s itself, negative or NaN is the NaN 0x7FFFFFFF. */
static float gch_sqrt_s(float s) {
    if (0.0f == s) return s;
    if (s > 0.0f) return gch_sqrt_pos(s);
    return gcs_nanf();
}
/* 1/sqrt of s: 0 is +0.0. */
static float gch_rsqrt_s(float s) {
    if (0.0f == s) return 0.0f;
    float r = s > 0.0f ? gch_sqrt_pos(s) : gcs_nanf();
    return 1.0f / r;
}
/* An angle in radians wrapped into [0, 2 pi) and made a binary angle. */
static int32_t gch_ang_word(float a) {
    while (a < 0.0f) a = a + GCH_2PI;
    while (a >= GCH_2PI) a = a - GCH_2PI;
#ifdef GEMS_HOST_MATH
    return (int32_t)((65536.0f * a) / GCH_2PI);      /* in [0, 65536]: no saturation */
#else
    return gch_fctiwz((double)((65536.0f * a) / GCH_2PI));
#endif
}
/* B.A: out's columns are b's 3x3 times a's columns, out's T is b's 3x3 times
 * a's T plus b's T (d4a0 and d688 share it, and store it differently). */
static void gch_compose(const float *a, const float *b, float *out) {
    float o[12], t;
    for (int c = 0; c < 4; c++)
        for (int j = 0; j < 3; j++) {
            t = b[3 + j] * a[3 * c + 1];
            t = b[j] * a[3 * c] + t;
            t = b[6 + j] * a[3 * c + 2] + t;
            o[3 * c + j] = c < 3 ? t : b[9 + j] + t;
        }
    memcpy(out, o, sizeof o);
}

/* ---- matrices ---- */

static void gch_8001d4a0(float *a, const float *b) { gch_compose(a, b, a); }
static void gch_8001d688(const float *a, float *b) { gch_compose(a, b, b); }
static void gch_8001d870(uint32_t *dst, const uint32_t *src) {
    uint32_t t[12];
    memcpy(t, src, sizeof t);
    memcpy(dst, t, sizeof t);
}

static void gch_8001e270(uint32_t idx) {
    const float *m = gems_dmf(0x30000u + idx);
    float *o = gems_dmf(0x30000u + 771u);       /* state + 0xC0C */
    float m0 = m[0], m1 = m[1], m2 = m[2], m3 = m[3], m4 = m[4], m5 = m[5];
    float m6 = m[6], m7 = m[7], m8 = m[8], m9 = m[9], m10 = m[10], m11 = m[11];
    o[0] = m4 * m8 - m5 * m7;
    o[3] = m5 * m6 - m3 * m8;
    o[6] = m3 * m7 - m4 * m6;
    o[1] = m7 * m2 - m8 * m1;
    o[4] = m8 * m0 - m6 * m2;
    o[7] = m6 * m1 - m7 * m0;
    o[2] = m1 * m5 - m2 * m4;
    o[5] = m2 * m3 - m0 * m5;
    o[8] = m0 * m4 - m1 * m3;
    o[9]  = -(m11 * o[6] + (m9 * o[0] + m10 * o[3]));
    o[10] = -(m11 * o[7] + (m9 * o[1] + m10 * o[4]));
    o[11] = -(m11 * o[8] + (m9 * o[2] + m10 * o[5]));
    float det = m2 * o[6] + (m0 * o[0] + m1 * o[3]);
    float inv = 1.0f / det;
    for (int k = 0; k < 12; k++) o[k] = o[k] * inv;
}

static void gch_8001e694(float s, float c, float *m) {
    for (int j = 0; j < 3; j++) {
        float c0 = m[j], c1 = m[3 + j];
        m[j]     = c0 * c - c1 * s;
        m[3 + j] = c0 * s + c1 * c;
    }
}
static void gch_8001e6f8(uint32_t ang, uint32_t idx) {
    if ((ang & 0xFFFFu) == 0) return;
    gch_8001e694(gcs_sin(ang), gcs_cos(ang), gems_dmf(0x30000u + idx));
}
static void gch_8001e7a4(float s, float c, float *m) {
    for (int j = 0; j < 3; j++) {
        float c0 = m[j], c2 = m[6 + j];
        m[6 + j] = c2 * c - c0 * s;
        m[j]     = c2 * s + c0 * c;
    }
}
static void gch_8001e808(uint32_t ang, uint32_t idx) {
    if ((ang & 0xFFFFu) == 0) return;
    gch_8001e7a4(gcs_sin(ang), gcs_cos(ang), gems_dmf(0x30000u + idx));
}
static void gch_8001e8b4(uint32_t ang, uint32_t idx) {
    if ((ang & 0xFFFFu) == 0) return;
    float s = gcs_sin(ang), c = gcs_cos(ang);
    float *m = gems_dmf(0x30000u + idx);
    for (int j = 0; j < 3; j++) {
        float c1 = m[3 + j], c2 = m[6 + j];
        m[3 + j] = c1 * c - c2 * s;
        m[6 + j] = c1 * s + c2 * c;
    }
}
static void gch_8001e960(float sx, float sy, float sz, uint32_t idx) {
    float *m = gems_dmf(0x30000u + idx);
    for (int j = 0; j < 3; j++) {
        m[j]     = m[j] * sx;
        m[3 + j] = m[3 + j] * sy;
        m[6 + j] = m[6 + j] * sz;
    }
}
/* T plus the 3x3 times (x, y, z), added onto T a term at a time, as the
 * board's _L20182 (Fn_trans) and _L20173 (Fn_point_trans) do:
 * ((T + x c0) + y c1) + z c2. The GC sums the products first. */
static void gch_8001e9dc(float x, float y, float z) {
    float *m = gch_cur();
    for (int j = 0; j < 3; j++) {
        float t = m[9 + j] + x * m[j];
        t = t + y * m[3 + j];
        m[9 + j] = t + z * m[6 + j];
    }
}
static void gch_8001ea5c(float x, float y, float z, uint32_t idx, float *ox, float *oy, float *oz) {
    const float *m = gems_dmf(0x30000u + idx);
    float r[3];
    for (int j = 0; j < 3; j++) {
        float t = m[9 + j] + x * m[j];
        t = t + y * m[3 + j];
        r[j] = t + z * m[6 + j];
    }
    *ox = r[0]; *oy = r[1]; *oz = r[2];
}

static void gch_8002443c(uint32_t n) {
    gch_8001d4a0(gch_cur(), gems_dmf(gcs_inner + n * 12u + 32u));
}
static void gch_80024558(uint32_t n) {
    gch_8001d870(gems_dm(gcs_inner + n * 12u + 32u), gems_dm(0x30000u + *gems_dm(0x3033Fu)));
}
static void gch_800245e4(void) {
    float *m = gch_cur();
    const float K = -1.0f;
    float inv = gch_8001e174(m[6] * m[6] + m[8] * m[8]);
    m[3] = m[6] * inv;
    m[5] = m[8] * inv;
    m[2] = K * m[3];
    m[0] = m[5];
}
static void gch_8002476c(void) {
    float *m = gch_cur();
    const float K = -1.0f;
    float inv = gch_8001e174(m[0] * m[0] + m[2] * m[2]);
    m[3] = m[0] * inv;
    m[5] = m[2] * inv;
    m[6] = K * m[5];
    m[8] = m[3];
}
static void gch_80024a34(uint32_t sel, uint32_t idx) {
    uint32_t off = (sel & 0xFFu) == 1u ? 1248u : 1056u;
    gch_8001d688(gems_dmf(0x30000u + idx + off), gch_cur());
}
static void gch_80025214(void) {
    float *m = gch_cur();
    for (int k = 0; k < 12; k++) m[k] = 0.0f;
    m[0] = m[4] = m[8] = 1.0f;
}
static void gch_800252e8(void) {
    uint32_t *depth = gems_dm(0x3033Cu), *cfc = gems_dm(0x3033Fu);
    uint32_t d = *depth;
    if (d == 0) return;
    *depth = d - 1u;
    if ((int32_t)d < 8) *cfc -= 12u;
}
static void gch_80025350(void) {
    uint32_t *depth = gems_dm(0x3033Cu), *cfc = gems_dm(0x3033Fu);
    if ((int32_t)*depth >= 7) return;
    *depth += 1u;
    gch_8001d870(gems_dm(0x30000u + *cfc + 12u), gems_dm(0x30000u + *cfc));
    *cfc += 12u;
}

/* ---- scalar maths ---- */

static float gch_8001d8e0(float x, float y)          { return gch_rsqrt_s(x * x + y * y); }
static float gch_8001d9e0(float x, float y, float z) { return gch_rsqrt_s(z * z + (x * x + y * y)); }
static float gch_8001dae4(float x, float y)          { return gch_sqrt_s(x * x + y * y); }
static float gch_8001dbdc(float x, float y, float z) { return gch_sqrt_s(z * z + (x * x + y * y)); }
static float gch_8001e084(float x) { return gch_sqrt_s(x); }
static float gch_8001e174(float x) { return gch_rsqrt_s(x); }

static float gch_8001dcd8(float x) {
    if (x <= -1.0f) return -GCH_PI_2;
    if (x >= 1.0f) return GCH_PI_2;
    return (float)gcop_fp_nearest(gch_800aea98, (double)x);
}
static int32_t gch_8001dd2c(float x) {
    if (x <= -1.0f) return 0xC000;
    if (x >= 1.0f) return 0x4000;
    return gch_ang_word((float)gcop_fp_nearest(gch_800aea98, (double)x));
}

/* The rational arctangent of t, |t| <= 1. */
static float gch_atan_p(float t) {
    const float A = 0.174415379762649536f;  /* 0x3E3299F2 */
    const float B = 5.73102188110351562f;   /* 0x40B76488 */
    const float C = 11.5545005798339844f;   /* 0x4138DF3C */
    const float D = 22.9397144317626953f;   /* 0x41B78489 */
    const float E = 29.7766838073730469f;   /* 0x41EE36A6 */
    const float F = 20.5109291076660156f;   /* 0x41A41662 */
    float s = t * t;
    float p = A * s + B;
    float q = C + s;
    p = s * p + D;
    q = s * q + E;
    float num = s * p + F;
    float den = s * q + F;
    return (t * num) / den;
}
static float gch_8001ddcc(float y, float x) {
    if (y >= x) {
        if (y >= -x) {
            if (0.0f == y) return 0.0f;
            return gch_atan_p(x / y);
        }
        if (0.0f == x) return -GCH_PI_2;
        return -GCH_PI_2 - gch_atan_p(y / x);
    }
    if (y >= -x) {
        if (0.0f == x) return GCH_PI_2;
        return GCH_PI_2 - gch_atan_p(y / x);
    }
    if (0.0f == y) return GCH_PI;
    if (x > 0.0f) return GCH_PI + gch_atan_p(x / y);
    return -GCH_PI + gch_atan_p(x / y);
}
static int32_t gch_8001dfd0(float y, float x) { return gch_ang_word(gch_8001ddcc(y, x)); }

static void gch_8001e038(float *a) {
    while (*a < 0.0f) *a = *a + GCH_2PI;
    while (*a >= GCH_2PI) *a = *a - GCH_2PI;
}

static void  gch_8001ead8(float *s, float *c, uint32_t ang) { *s = gcs_sin(ang); *c = gcs_cos(ang); }
static float gch_8001eb18(uint32_t ang) { return gcs_cos(ang); }
static float gch_8001eb40(uint32_t ang) { return gcs_sin(ang); }

/* ---- collision ---- */

/* Squared distance from the ball (prev position at +0x60 words, Gems'
 * Fn_coli_dist test point at state+0x104C) along a swept segment. */
static float gch_8001f9ac(uint32_t ball_idx, float *d, float *p) {
    const float *S = gch_S();
    float b0 = S[ball_idx + 96], b1 = S[ball_idx + 97], b2 = S[ball_idx + 98];
    float e0 = S[1043] - b0, e1 = S[1044] - b1, e2 = S[1045] - b2;
    float f6 = p[0] - e0, f7 = p[1] - e1, f8 = p[2] - e2;
    p[0] = d[0]; p[1] = d[1]; p[2] = d[2];
    float g = -(e2 * f8 + (e0 * f6 + e1 * f7));
    if (g <= 0.0f) return e2 * e2 + (e0 * e0 + e1 * e1);
    float L = f8 * f8 + (f6 * f6 + f7 * f7);
    if (L <= g) return d[2] * d[2] + (d[0] * d[0] + d[1] * d[1]);
    float t = g / L;
    float a = f7 * t + e1;
    float b = f6 * t + e0;
    float c = f8 * t + e2;
    return c * c + (b * b + a * a);
}

static void gch_8001faa4(float r, int32_t ball, uint32_t unit, int32_t *n_near, int32_t *n_hit,
                         uint32_t *mask, float *push, float *push_y, int32_t kind,
                         uint32_t m10, uint32_t m11, uint32_t m12, uint32_t m13,
                         uint32_t bit, uint32_t m15) {
    float *S = gch_S();
    uint32_t *SU = gch_SU();
    float R = S[ball + 1568];
    if (R == 0.0f) return;
    (*n_near)++;
    if ((m10 & bit) && SU[1050] != 4u) R = R * S[1776];
    uint32_t j = SU[1634 + ball] - 0x30000u;
    float L20[3], L8[3];
    L20[0] = L8[0] = S[1040] - S[j];
    L20[1] = L8[1] = S[1041] - S[j + 1];
    L20[2] = L8[2] = S[1042] - S[j + 2];
    float dist = (bit & m11) ? gch_8001f9ac(j, L20, L8)
                             : (L20[2] * L20[2] + (L20[0] * L20[0] + L20[1] * L20[1]));
    float rr = r + R;
    float q = rr * rr;
    if (dist >= q) { *mask &= ~bit; return; }
    (*n_hit)++;
    SU[2035] = (uint32_t)ball;
    SU[2036] = unit;
    if (kind == 4 || kind == 5) {
        float w = ((q - L20[0] * L8[0])) - L20[2] * L8[2];
        float v = kind == 4 ? gch_8001e084(w) - L20[1] : gch_8001e084(w) + L20[1];
        if (!(v <= *push_y)) *push_y = v;
        return;
    }
    if (m11 & bit) return;
    float f2 = rr - gch_8001e084(dist);
    float v = !(m12 & bit) ? 1.0f * f2 : !(m13 & m15) ? 1.0f * f2 : 0.0f * f2;
    if (v >= *push) *push = v;
}

/* Fn_parts_oidasi's per-fighter test: the 32 balls at bufferram bram_idx
 * against the sphere (x, y, z, r); skipped when ball 13 is over 3.0 away.
 * Gems' push factor is 1 - d/(R+r) (the firmware's is 1 - 2d/(R+r)). */
static uint32_t gch_800217b0(float x, float y, float z, float r, uint32_t bram_idx, uint32_t radius_idx,
                             float *push, float *push_x, float *push_z, uint32_t *last_ball) {
    uint32_t mask = 0, i = bram_idx;
    *push = 0.0f;
    float cx = gems_bram_rdf(i + 39), cy = gems_bram_rdf(i + 40), cz = gems_bram_rdf(i + 41);
    if (!(gch_8001dbdc(cx - x, cy - y, cz - z) <= 3.0f)) return 0;
    for (uint32_t k = 0; k < 32; k++) {
        float dx = gems_bram_rdf(i++) - x;
        float dy = gems_bram_rdf(i++) - y;
        float dz = gems_bram_rdf(i++) - z;
        float rad = gch_S()[radius_idx + k];
        if (0.0f == rad) continue;
        float d = gch_8001dbdc(dx, dy, dz);
        float rr = rad + r;
        if (!(d <= rr)) continue;
        float f = 1.0f - d / rr;
        mask |= 1u << k;
        *push_x = *push_x + dx * f;
        *push_z = *push_z + dz * f;
        *push = f;
        *last_ball = k;
    }
    return mask;
}

/* ---- afterimages ---- */

/* Lay copies of one body part's matrix into the 128-slot ring at DM 0x32300,
 * interpolated from last frame's matrix (prev) toward this one (cur) every
 * `step` of the way. Departure: Gems has no bound on the loop (a step too
 * small to move t never ends it); this caps it at 1 << 20 copies. */
static void gch_80021d88(float step, const float *cur, const float *prev, uint32_t obj_idx,
                         uint32_t life_idx, uint32_t turn, uint32_t life_step, uint32_t player,
                         uint32_t part) {
    float *S = gch_S();
    uint32_t *SU = gch_SU();
    uint32_t o0 = SU[obj_idx + part], o1 = SU[obj_idx + part + 16], o2 = SU[obj_idx + part + 32];
    for (int k = 0; k < 12; k++) S[0x2184 + k] = cur[part * 12 + k] - prev[part * 12 + k];
    uint32_t li = life_idx + part;
    uint32_t h = SU[0x2180];
    uint32_t v = SU[li] ? SU[li] - 1u : SU[0x2182];
    uint32_t flags = player | (turn ? 0x80000000u : 0u);
    float t = 0.0f;
    for (uint32_t guard = 0; guard < (1u << 20); guard++) {
        uint32_t b = 0x2300u + h * 32u;
        SU[b] = part;
        SU[b + 1] = flags;
        SU[b + 2] = v++;
        SU[b + 3] = life_step;
        SU[b + 4] = o0;
        SU[b + 5] = o1;
        SU[b + 6] = o2;
        for (int k = 0; k < 12; k++) S[b + 0x14 + k] = S[0x2184 + k] * t + prev[part * 12 + k];
        h = (h + 1u) & 0x7Fu;
        if (!(0.0f < step)) break;
        t = t + step;
        if (t >= 1.0f) break;
    }
    SU[0x2180] = h;
    SU[li] = v;
}

/* ---- reply FIFO, get_sm_ang readback ---- */

static void gch_80022e7c(uint32_t w) { gems_out_w(w); }

static void gch_80023ccc(void) {
    const float *m = gch_cur();
    float a0 = gch_8001ddcc(m[8], m[6]);
    float a1 = gch_8001dcd8(m[7]);
    float a2 = gch_8001ddcc(m[4], m[1]);
    float r0 = a0 < 0.0f ? GCH_PI + a0 : a0 - GCH_PI;
    float r1 = a1 < 0.0f ? -GCH_PI - a1 : GCH_PI - a1;
    float r2 = a2 < 0.0f ? GCH_PI + a2 : a2 - GCH_PI;
    float orig = fabsf(a0) + (fabsf(a1) + fabsf(a2));
    float alts = -1.0f + (fabsf(r1) + fabsf(r2));   /* sic: |r0| is -1.0 in Gems */
    if (alts > orig) { r0 = a0; r1 = a1; r2 = a2; }
    gch_8001e038(&r0);
    gch_8001e038(&r1);
    gch_8001e038(&r2);
    gems_out_w((uint32_t)gch_fctiwz((double)((65536.0f * r0) / GCH_2PI)) & 0xFFFFu);
    gems_out_w((uint32_t)gch_fctiwz((double)((65536.0f * r1) / GCH_2PI)) & 0xFFFFu);
    gems_out_w((uint32_t)gch_fctiwz((double)((65536.0f * r2) / GCH_2PI)) & 0xFFFFu);
}

/* ---- CodeWarrior runtime / MSL fdlibm (double) ---- */

static uint32_t gch_800a2544(double x) {
    if (x < 0.0) return 0;
    if (x >= 4294967296.0) return 0xFFFFFFFFu;
    if (x < 2147483648.0) return (uint32_t)gch_fctiwz(x);
    return (uint32_t)gch_fctiwz(x - 2147483648.0) + 0x80000000u;
}
static void gch_800a25cc(void) {}
static void gch_800a2618(void) {}

static double gch_800ab620(double x) {
    const double pio2_hi = gcs_d(0x3FF921FB54442D18ull), pio2_lo = gcs_d(0x3C91A62633145C07ull);
    const double huge = 1.0e300, one = 1.0, pio4_hi = gcs_d(0x3FE921FB54442D18ull);
    const double pS0 = gcs_d(0x3FC5555555555555ull), pS1 = gcs_d(0xBFD4D61203EB6F7Dull);
    const double pS2 = gcs_d(0x3FC9C1550E884455ull), pS3 = gcs_d(0xBFA48228B5688F3Bull);
    const double pS4 = gcs_d(0x3F49EFE07501B288ull), pS5 = gcs_d(0x3F023DE10DFDF709ull);
    const double qS1 = gcs_d(0xC0033A271C8A2D4Bull), qS2 = gcs_d(0x40002AE59C598AC8ull);
    const double qS3 = gcs_d(0xBFE6066C1B8D0159ull), qS4 = gcs_d(0x3FB3B8C5B12E9282ull);
    int32_t hx = (int32_t)gch_dhi(x);
    int32_t ix = hx & 0x7FFFFFFF;
    double t = 0.0, w, p, q, c, r, s;
    if (ix >= 0x3FF00000) {
        if (((ix - 0x3FF00000) | (int32_t)gch_dlo(x)) == 0) return fma(pio2_hi, x, x * pio2_lo);
        return (double)gcs_nanf();
    }
    if (ix < 0x3FE00000) {
        if (ix < 0x3E400000) {
            if (huge + x > one) return x;
            /* Gems reads an uninitialised t here; unreachable (huge + x > one always). */
        }
        t = x * x;
        p = t * fma(t, fma(t, fma(t, fma(t, fma(pS5, t, pS4), pS3), pS2), pS1), pS0);
        q = fma(t, fma(t, fma(t, fma(qS4, t, qS3), qS2), qS1), one);
        w = p / q;
        return fma(x, w, x);
    }
    w = one - fabs(x);
    t = w * 0.5;
    p = t * fma(t, fma(t, fma(t, fma(t, fma(pS5, t, pS4), pS3), pS2), pS1), pS0);
    q = fma(t, fma(t, fma(t, fma(qS4, t, qS3), qS2), qS1), one);
    s = gch_800aede4(t);
    if (ix >= 0x3FEF3333) {
        w = p / q;
        double f = fma(s, w, s);
        t = pio2_hi - fma(2.0, f, -pio2_lo);
    } else {
        double df = gch_set_lo(s, 0);
        c = -fma(df, df, -t) / (s + df);
        r = p / q;
        double pp = -fma(2.0, c, -pio2_lo);
        double qq = -fma(2.0, df, -pio4_hi);
        pp = fma(2.0 * s, r, -pp);
        t = pio4_hi - (pp - qq);
    }
    return hx > 0 ? t : -t;
}

static int32_t gch_800acc04(double x, double *y) {
    const double zero = 0.0, half = 0.5, two24 = 16777216.0;
    const double invpio2 = gcs_d(0x3FE45F306DC9C883ull);
    const double pio2_1  = gcs_d(0x3FF921FB54400000ull), pio2_1t = gcs_d(0x3DD0B4611A626331ull);
    const double pio2_2  = gcs_d(0x3DD0B4611A600000ull), pio2_2t = gcs_d(0x3BA3198A2E037073ull);
    const double pio2_3  = gcs_d(0x3BA3198A2E000000ull), pio2_3t = gcs_d(0x397B839A252049C1ull);
    double z, w, t, r, fn, tx[3];
    int32_t e0, i, j, nx, n;
    int32_t hx = (int32_t)gch_dhi(x);
    int32_t ix = hx & 0x7FFFFFFF;
    if (ix <= 0x3FE921FB) { y[0] = x; y[1] = 0; return 0; }
    if (ix < 0x4002D97C) {
        if (hx > 0) {
            z = x - pio2_1;
            if (ix != 0x3FF921FB) {
                y[0] = z - pio2_1t;
                y[1] = (z - y[0]) - pio2_1t;
            } else {
                z -= pio2_2;
                y[0] = z - pio2_2t;
                y[1] = (z - y[0]) - pio2_2t;
            }
            return 1;
        }
        z = x + pio2_1;
        if (ix != 0x3FF921FB) {
            y[0] = z + pio2_1t;
            y[1] = (z - y[0]) + pio2_1t;
        } else {
            z += pio2_2;
            y[0] = z + pio2_2t;
            y[1] = (z - y[0]) + pio2_2t;
        }
        return -1;
    }
    if (ix <= 0x413921FB) {
        t = fabs(x);
        n = gch_fctiwz(fma(invpio2, t, half));
        fn = (double)n;
        r = -fma(pio2_1, fn, -t);
        w = pio2_1t * fn;
        if (n < 32 && ix != gcs_npio2_hw[n - 1]) {
            y[0] = r - w;
        } else {
            j = ix >> 20;
            y[0] = r - w;
            i = j - (int32_t)((gch_dhi(y[0]) >> 20) & 0x7FF);
            if (i > 16) {
                t = r;
                w = fn * pio2_2;
                r = t - w;
                w = fma(pio2_2t, fn, -((t - r) - w));
                y[0] = r - w;
                i = j - (int32_t)((gch_dhi(y[0]) >> 20) & 0x7FF);
                if (i > 49) {
                    t = r;
                    w = fn * pio2_3;
                    r = t - w;
                    w = fma(pio2_3t, fn, -((t - r) - w));
                    y[0] = r - w;
                }
            }
        }
        y[1] = (r - y[0]) - w;
        if (hx < 0) { y[0] = -y[0]; y[1] = -y[1]; return -n; }
        return n;
    }
    if (ix >= 0x7FF00000) { y[0] = y[1] = x - x; return 0; }
    e0 = (ix >> 20) - 1046;
    z = gch_set_hi(x, (uint32_t)(ix - (e0 << 20)));
    for (i = 0; i < 2; i++) {
        tx[i] = (double)gch_fctiwz(z);
        z = two24 * (z - tx[i]);
    }
    tx[2] = z;
    nx = 3;
    while (tx[nx - 1] == zero) nx--;
    n = gch_800ad098(tx, y, e0, nx, 2, gcs_ipio2);
    if (hx < 0) { y[0] = -y[0]; y[1] = -y[1]; return -n; }
    return n;
}

static int32_t gch_800ad098(const double *x, double *y, int32_t e0, int32_t nx,
                            int32_t prec, const int32_t *ipio2) {
    const double zero = 0.0, one = 1.0, two24 = 16777216.0, twon24 = 5.9604644775390625e-08;
    int32_t jz, jx, jv, jp, jk, carry, n, iq[20], i, j, k, m, q0, ih;
    double z, fw, f[20], fq[20], q[20];
    gch_800a25cc();
    jk = gcs_init_jk[prec];
    jp = jk;
    jx = nx - 1;
    jv = (e0 - 3) / 24; if (jv < 0) jv = 0;
    q0 = e0 - 24 * (jv + 1);
    j = jv - jx; m = jx + jk;
    for (i = 0; i <= m; i++, j++) f[i] = (j < 0) ? zero : (double)ipio2[j];
    for (i = 0; i <= jk; i++) {
        for (j = 0, fw = 0.0; j <= jx; j++) fw = fma(x[j], f[jx + i - j], fw);
        q[i] = fw;
    }
    jz = jk;
recompute:
    for (i = 0, j = jz, z = q[jz]; j > 0; i++, j--) {
        fw = (double)gch_fctiwz(twon24 * z);
        iq[i] = gch_fctiwz(-fma(two24, fw, -z));
        z = q[j - 1] + fw;
    }
    z = gch_800ae688(z, q0);
    z = -fma(8.0, gch_800ae4b4(z * 0.125), -z);
    n = gch_fctiwz(z);
    z -= (double)n;
    ih = 0;
    if (q0 > 0) {
        i = (iq[jz - 1] >> (24 - q0)); n += i;
        iq[jz - 1] -= i << (24 - q0);
        ih = iq[jz - 1] >> (23 - q0);
    } else if (q0 == 0) ih = iq[jz - 1] >> 23;
    else if (z >= 0.5) ih = 2;
    if (ih > 0) {
        n += 1; carry = 0;
        for (i = 0; i < jz; i++) {
            j = iq[i];
            if (carry == 0) {
                if (j != 0) { carry = 1; iq[i] = 0x1000000 - j; }
            } else iq[i] = 0xFFFFFF - j;
        }
        if (q0 > 0) {
            switch (q0) {
            case 1: iq[jz - 1] &= 0x7FFFFF; break;
            case 2: iq[jz - 1] &= 0x3FFFFF; break;
            }
        }
        if (ih == 2) {
            z = one - z;
            if (carry != 0) z -= gch_800ae688(one, q0);
        }
    }
    if (z == zero) {
        j = 0;
        for (i = jz - 1; i >= jk; i--) j |= iq[i];
        if (j == 0) {
            for (k = 1; iq[jk - k] == 0; k++);
            for (i = jz + 1; i <= jz + k; i++) {
                f[jx + i] = (double)ipio2[jv + i];
                for (j = 0, fw = 0.0; j <= jx; j++) fw = fma(x[j], f[jx + i - j], fw);
                q[i] = fw;
            }
            jz += k;
            goto recompute;
        }
    }
    if (z == 0.0) {
        jz -= 1; q0 -= 24;
        while (iq[jz] == 0) { jz--; q0 -= 24; }
    } else {
        z = gch_800ae688(z, -q0);
        if (z >= two24) {
            fw = (double)gch_fctiwz(twon24 * z);
            iq[jz] = gch_fctiwz(-fma(two24, fw, -z));
            jz += 1; q0 += 24;
            iq[jz] = gch_fctiwz(fw);
        } else iq[jz] = gch_fctiwz(z);
    }
    fw = gch_800ae688(one, q0);
    for (i = jz; i >= 0; i--) { q[i] = fw * (double)iq[i]; fw *= twon24; }
    for (i = jz; i >= 0; i--) {
        for (fw = 0.0, k = 0; k <= jp && k <= jz - i; k++) fw = fma(gcs_PIo2(k), q[i + k], fw);
        fq[jz - i] = fw;
    }
    switch (prec) {
    case 0:
        fw = 0.0;
        for (i = jz; i >= 0; i--) fw += fq[i];
        y[0] = (ih == 0) ? fw : -fw;
        break;
    case 1:
    case 2:
        fw = 0.0;
        for (i = jz; i >= 0; i--) fw += fq[i];
        y[0] = (ih == 0) ? fw : -fw;
        fw = fq[0] - fw;
        for (i = 1; i <= jz; i++) fw += fq[i];
        y[1] = (ih == 0) ? fw : -fw;
        break;
    case 3:
        for (i = jz; i > 0; i--) { fw = fq[i - 1] + fq[i]; fq[i] += fq[i - 1] - fw; fq[i - 1] = fw; }
        for (i = jz; i > 1; i--) { fw = fq[i - 1] + fq[i]; fq[i] += fq[i - 1] - fw; fq[i - 1] = fw; }
        for (fw = 0.0, i = jz; i >= 2; i--) fw += fq[i];
        if (ih == 0) { y[0] = fq[0]; y[1] = fq[1]; y[2] = fw; }
        else { y[0] = -fq[0]; y[1] = -fq[1]; y[2] = -fw; }
        break;
    }
    gch_800a2618();
    return n & 7;
}

static double gch_800adf8c(double x, double y, int32_t iy) {
    const double one = 1.0, pio4 = gcs_d(0x3FE921FB54442D18ull), pio4lo = gcs_d(0x3C81A62633145C07ull);
    double z, r, v, w, s, a, t;
    int32_t hx = (int32_t)gch_dhi(x);
    int32_t ix = hx & 0x7FFFFFFF;
    if (ix < 0x3E300000) {
        if (gch_fctiwz(x) == 0) {
            if (((ix | (int32_t)gch_dlo(x)) | (iy + 1)) == 0) return one / fabs(x);
            if (iy == 1) return x;
            return -1.0 / x;
        }
    }
    if (ix >= 0x3FE59428) {
        if (hx < 0) { x = -x; y = -y; }
        z = pio4 - x;
        w = pio4lo - y;
        x = z + w;
        y = 0.0;
    }
    z = x * x;
    w = z * z;
    r = fma(w, fma(w, fma(w, fma(w, fma(w, gcs_tan_T(11), gcs_tan_T(9)), gcs_tan_T(7)),
                          gcs_tan_T(5)), gcs_tan_T(3)), gcs_tan_T(1));
    v = z * fma(w, fma(w, fma(w, fma(w, fma(w, gcs_tan_T(12), gcs_tan_T(10)), gcs_tan_T(8)),
                              gcs_tan_T(6)), gcs_tan_T(4)), gcs_tan_T(2));
    s = z * x;
    r = fma(gcs_tan_T(0), s, fma(z, fma(s, r + v, y), y));
    w = x + r;
    if (ix >= 0x3FE59428) {
        v = (double)iy;
        return (double)(1 - ((hx >> 30) & 2)) * (-fma(2.0, x - (w * w / (w + v) - r), -v));
    }
    if (iy == 1) return w;
    z = gch_set_lo(w, 0);
    v = r - (z - x);
    a = -1.0 / w;
    t = gch_set_lo(a, 0);
    s = fma(t, z, 1.0);
    return fma(a, fma(t, v, s), t);
}

static double gch_800ae4b4(double x) { return floor(x); }

static double gch_800ae688(double x, int32_t n) {
    const double two54 = 1.80143985094819840000e+16, twom54 = 5.55111512312578270212e-17;
    const double huge = 1.0e300, tiny = 1.0e-300;
    int32_t hx = (int32_t)gch_dhi(x);
    uint32_t lx = gch_dlo(x);
    int32_t k = (hx & 0x7FF00000) >> 20;
    if (k == 0) {
        if ((lx | (uint32_t)(hx & 0x7FFFFFFF)) == 0) return x;
        x *= two54;
        hx = (int32_t)gch_dhi(x);
        k = ((hx & 0x7FF00000) >> 20) - 54;
        if (n < -50000) return tiny * x;
    }
    if (k == 0x7FF) return x + x;
    k = k + n;
    if (k > 0x7FE) return huge * copysign(huge, x);
    if (k > 0) return gch_set_hi(x, (uint32_t)((hx & (int32_t)0x800FFFFF) | (k << 20)));
    if (k <= -54) {
        if (n > 50000) return huge * copysign(huge, x);
        return tiny * copysign(tiny, x);
    }
    k += 54;
    x = gch_set_hi(x, (uint32_t)((hx & (int32_t)0x800FFFFF) | (k << 20)));
    return x * twom54;
}

static double gch_800aea20(double x) {
    double y[2];
    int32_t ix = (int32_t)gch_dhi(x) & 0x7FFFFFFF;
    if (ix <= 0x3FE921FB) return gch_800adf8c(x, 0.0, 1);
    if (ix >= 0x7FF00000) return x - x;
    int32_t n = gch_800acc04(x, y);
    return gch_800adf8c(y[0], y[1], 1 - ((n & 1) << 1));
}

static double gch_800aea98(double x) { return gch_800ab620(x); }

static double gch_800aede4(double x) {
    if (((gch_dhi(x) >> 20) & 0x7FFu) == 0x7FFu) return fma(x, x, x);
    if (x <= 0.0) {
        if (x == 0.0) return x;
        return (double)gcs_nanf();
    }
    return sqrt(x);
}

#endif
