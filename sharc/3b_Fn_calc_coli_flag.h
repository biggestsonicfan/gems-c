#pragma once
/* COP op 3b Fn_calc_coli_flag */

static void gcop_3b(void) {
    uint32_t kind = gems_in_w();
    uint32_t m1 = gems_in_w();
    uint32_t m2 = gems_in_w();
    uint32_t m3 = gems_in_w();
    uint32_t m4 = gems_in_w();
    uint32_t ball_mask = gems_in_w();
    uint32_t unit_mask = gems_in_w();
    int32_t n_near = 0, n_hit = 0;
    float push = 0.0f, push_y = 0.0f;

    for (uint32_t j = 0; j < 96; j++) *gems_dm(0x30340u + j) = gems_bram_rd(0x3F40u + j);
    for (uint32_t j = 0; j < 96; j++) *gems_dm(0x303A0u + j) = gems_bram_rd(0x3FA0u + j);

    uint32_t bit = 1;
    for (int32_t unit = 32; unit >= 1; unit--, bit <<= 1) {
        uint32_t idx = 0x3E20u + (uint32_t)(32 - unit);
        uint32_t mask = gems_bram_rd(idx);
        if (mask != 0) {
            float r = *gems_dmf(0x30720u + (uint32_t)unit);
            if (r != 0.0f) {
                uint32_t m2bit = m2 & bit;
                if (m2bit && *gems_dm(0x3041Bu) != 4u) r = r * *gems_dmf(0x307F0u);
                uint32_t b = *gems_dm(0x30741u + (uint32_t)unit);
                *gems_dm(0x30410u) = gems_bram_rd(b + 0x7F40u);
                *gems_dm(0x30411u) = gems_bram_rd(b + 0x7F41u);
                *gems_dm(0x30412u) = gems_bram_rd(b + 0x7F42u);
                *gems_dm(0x30413u) = gems_bram_rd(b + 0x7FA0u);
                *gems_dm(0x30414u) = gems_bram_rd(b + 0x7FA1u);
                *gems_dm(0x30415u) = gems_bram_rd(b + 0x7FA2u);
                uint32_t m11 = m2bit ? 0xFFFFFFFFu : m1;
                if (unit_mask & bit) {
                    uint32_t bbit = 1;
                    for (int32_t ball = 32; ball >= 1; ball--, bbit <<= 1) {
                        if ((ball_mask & bbit) && (mask & bbit))
                            gch_8001faa4(r, ball, (uint32_t)unit, &n_near, &n_hit, &mask, &push, &push_y,
                                         (int32_t)kind, m1, m11, m3, m4, bbit, bit);
                    }
                }
            }
        }
        gems_bram_wr(idx + 0x4000u, mask);
    }

    for (uint32_t i = 0; i < 32; i++) {
        uint32_t sh = *gems_dm(0x307A0u + i);
        if (sh == 0) continue;
        uint32_t hits = gems_bram_rd(0x7E20u + i);
        uint32_t m2i = m2 & (1u << i);
        uint32_t flag = (sh & 32u) ? 0u : (1u << (sh & 31u));   /* slw */
        for (uint32_t j = 0; j < 32; j++) {
            uint32_t jb = 1u << j;
            if (!(hits & jb)) continue;
            uint32_t a = *gems_dm(0x306A0u + j);
            if (a == 0) continue;
            if (m2i && j - 30u <= 1u) continue;
            if ((m1 & jb) && i - 30u <= 1u) continue;
            a += 0x3D80u;
            gems_bram_wr(a, gems_bram_rd(a) | flag);
        }
    }
    gems_out_w((uint32_t)n_near);
    gems_out_w((uint32_t)n_hit);
    gems_out_f(push);
    gems_out_f(push_y);
}
