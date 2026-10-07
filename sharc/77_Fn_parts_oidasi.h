#pragma once
/* COP op 77 Fn_parts_oidasi */

/* PowerPC slw: a count of 32..63 gives 0. */
static inline uint32_t g77_shl(uint32_t v, uint32_t n) { n &= 63u; return n >= 32u ? 0u : v << n; }

static uint32_t g77_unit_mask(uint32_t balls, uint32_t tbl) {
    const uint32_t *S = gems_dm(0x30000u);
    uint32_t m = 0;
    for (uint32_t k = 0; k < 32; k++)
        if (balls & g77_shl(1u, k)) m |= g77_shl(1u, S[tbl + k]);
    return m;
}

/* A projectile (x, y, z, radius) against both fighters' balls. 9 replies: push-out x/z,
 * the last fighter hit (-1 none), its ball and unit, then P0 ball/unit masks, P1 ball/unit masks. */
static void gcop_77(void) {
    float x = gems_in_f();
    float y = gems_in_f();
    float z = gems_in_f();
    float r = gems_in_f();
    const uint32_t *S = gems_dm(0x30000u);
    float push = 0.0f, push_x = 0.0f, push_z = 0.0f;
    uint32_t last = 0, unit = 0, who = 0xFFFFFFFFu;

    uint32_t balls0 = gch_800217b0(x, y, z, r, 16000u, 0x600u, &push, &push_x, &push_z, &last);
    uint32_t units0 = g77_unit_mask(balls0, 0x6A0u);
    if (!(0.0f == push)) {
        who  = 0;
        unit = S[0x6A0u + last];
    }

    uint32_t balls1 = gch_800217b0(x, y, z, r, 0x7E80u, 0x700u, &push, &push_x, &push_z, &last);
    uint32_t units1 = g77_unit_mask(balls1, 0x7A0u);
    if (!(0.0f == push)) {
        who  = 1;
        unit = S[0x7A0u + last];
    }

    gems_out_f(push_x);
    gems_out_f(push_z);
    gems_out_w(who);
    gems_out_w(last);
    gems_out_w(unit);
    gems_out_w(balls0);
    gems_out_w(units0);
    gems_out_w(balls1);
    gems_out_w(units1);
}
