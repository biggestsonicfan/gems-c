#pragma once
/* COP op 70 Fn_area_coli */

/* PowerPC slw: a count of 32..63 gives 0. */
static inline uint32_t g70_shl(uint32_t v, uint32_t n) { n &= 63u; return n >= 32u ? 0u : v << n; }

/* The arena test for one fighter's 32 balls (world positions in bufferram,
 * radii in DM). Words are DM 0x30000 + k (S / Sf):
 *   0x800, 0x801  floor heights (lower/upper tolerance)   0x802  arena half width
 *   0x803  ball table in bufferram                       0x804  ceiling for the test
 *   0x805..0x809  ball masks: on floor, below floor, near 0x800, near 0x801, outside
 *   0x80A  highest ball top     0x80B..0x80E  push-out per side (outside)
 *   0x80F..0x812  push per side, 0x813..0x816  clearance per side (100 = none)
 * 22 replies: 0x805..0x816, then the masks 0x805, 0x809, 0x807, 0x808 as unit masks. */
static uint32_t g70_unit_mask(uint32_t balls) {
    const uint32_t *S = gems_dm(0x30000u);
    uint32_t tbl = (S[0x7F2] & 1u) ? 0x7A0u : 0x6A0u;
    uint32_t m = 0;
    for (uint32_t k = 0; k < 32; k++)
        if (balls & g70_shl(1u, k)) m |= g70_shl(1u, S[tbl + k]);
    return m;
}

static void gcop_70(void) {
    uint32_t *S  = gems_dm(0x30000u);
    float    *Sf = gems_dmf(0x30000u);

    S[0x801] = gems_in_w();
    S[0x800] = gems_in_w();
    S[0x802] = gems_in_w();
    S[0x804] = gems_in_w();
    uint32_t who = gems_in_w();
    S[0x7F2] = who & 0x7FFFFFFFu;

    uint32_t base, rad;
    if ((who & 0x7FFFFFFFu) == 0) {
        base = 16000u;
        rad  = 0x600u;
        for (uint32_t k = 0x817; k <= 0x826; k++) S[k] = 0;
    } else {
        base = 0x7E80u;
        rad  = 0x700u;
    }
    S[0x803] = base;
    for (uint32_t k = 0x805; k <= 0x816; k++) S[k] = 0;
    for (uint32_t k = 0x813; k <= 0x816; k++) Sf[k] = GCS_F_100;

    uint32_t mask = 0;
    for (uint32_t k = 0; k < 32; k++, base += 3u) {
        float x = gems_bram_rdf(base);
        float y = gems_bram_rdf(base + 1u);
        float z = gems_bram_rdf(base + 2u);
        float r = Sf[rad + k];
        if (0.0f == r) continue;

        float top = y + r;
        if (top > Sf[0x80A]) Sf[0x80A] = top;
        float bot = y - r;
        if (Sf[0x804] <= bot) continue;

        uint32_t bit = g70_shl(1u, k);
        mask |= bit;
        if (bot <= 0.05f) S[0x805] |= bit;
        if (top <= -0.1f) S[0x806] |= bit;
        if (bot <= 0.05f + Sf[0x800]) S[0x807] |= bit;
        if (bot <= 0.05f + Sf[0x801]) S[0x808] |= bit;

        /* the four sides: +x, -x, +z, -z */
        float w = Sf[0x802];
        for (uint32_t j = 0; j < 4; j++) {
            float v = (j & 2u) ? z : x;
            float d, e;
            if (j & 1u) { d = v - r; e = w + d; }
            else        { d = v + r; e = w - d; }
            uint32_t sgn = (j & 1u) ? 0x80000000u : 0u;
            float a = fabsf(d);
            if (((sgn ^ gems_f2u(d)) & 0x80000000u) == 0 && w <= a) {
                float out;
                if (w == a) {
                    out = 0.001f;
                } else {
                    out = a - w;
                    S[0x809] |= bit;
                }
                if (out >= Sf[0x80B + j]) Sf[0x80B + j] = out;
            } else {
                if (e < Sf[0x813 + j]) Sf[0x813 + j] = e;
            }
        }
    }

    uint32_t all_out = mask & S[0x809];
    if (all_out == mask && (who & 0x80000000u)) {
        S[0x80B] = all_out;
        S[0x80C] = all_out;
        S[0x80D] = all_out;
        S[0x80E] = all_out;
    }

    /* a side's push is the room left on the opposite side */
    float span = Sf[0x802] * 2.0f;
    for (uint32_t p = 0; p < 2; p++) {
        uint32_t c = 0x813u + 2u * p;
        if (!(GCS_F_100 == Sf[c])) {
            float f = span - Sf[c + 1u];
            if (0.0f == f) f = 0.001f;
            Sf[0x80F + 2u * p] = f;
        }
        if (!(GCS_F_100 == Sf[c + 1u])) {
            float f = span - Sf[c];
            if (0.0f == f) f = 0.001f;
            Sf[0x810 + 2u * p] = f;
        }
    }

    /* keep the smaller push of each pair (negatives as 0) */
    for (uint32_t p = 0x80F; p <= 0x811; p += 2u) {
        float a = Sf[p], b = Sf[p + 1u];
        if (!(a >= 0.0f)) a = 0.0f;
        if (!(b >= 0.0f)) b = 0.0f;
        if (a > b) a = 0.0f;
        else       b = 0.0f;
        Sf[p] = a;
        Sf[p + 1u] = b;
    }

    /* and then only the axis with the smaller total */
    float px0 = Sf[0x80F], px1 = Sf[0x810], pz0 = Sf[0x811], pz1 = Sf[0x812];
    if (!(px0 + px1 >= pz0 + pz1)) {
        pz0 = 0.0f;
        pz1 = 0.0f;
    } else {
        px0 = 0.0f;
        px1 = 0.0f;
    }
    Sf[0x80F] = px0;
    Sf[0x810] = px1;
    Sf[0x811] = pz0;
    Sf[0x812] = pz1;

    for (uint32_t k = 0x805; k <= 0x816; k++) gems_out_w(S[k]);
    gems_out_w(g70_unit_mask(S[0x805]));
    gems_out_w(g70_unit_mask(S[0x809]));
    gems_out_w(g70_unit_mask(S[0x807]));
    gems_out_w(g70_unit_mask(S[0x808]));
}
