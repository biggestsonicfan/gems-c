/* coli_recalc_pos (i960 0x2D67C, a `bal` leaf of coli_cont_cop; Gems
 * 80041704): turn the fight collision's push-out into each fighter's step.
 * g7/g8 = the robs at 0x500804 / 0x500808, g13 the fight work area, g4/g5/g6
 * what the narrow phase (0x3B) left.
 *   - picks the two steps (r8..r10 for g7, r12..r14 for g8): nothing (no
 *     contact, g13+0x164 == 0), one fighter pushed up a slope (g13+0x88 == 5
 *     or 4: g5 onto the rob's +0x678, capped at 0.15), or the push-out along
 *     the contact normal at g13+0xD4..+0xDC, split by g13+0xCC / +0xD0 and
 *     scaled by g4 (capped at 0.3) after the rob's approach speed is added;
 *     g6 bit 0 (both locked) instead pools the two robs' +0x644 / +0x64C;
 *   - stores them to g13+0xE0..+0xF4, drops the axis a wall (g13+0x23C /
 *     +0x23E) holds, lets a fighter against the arena edge (+0x644 / +0x64C)
 *     hand the push to the other, clamps to the rob's limits (+0x650..+0x65C),
 *     takes the edge offsets off again, and writes the steps to +0xA68/+0xA6C;
 *   - sends 0x3D (push-out onto the unit matrices) and 0x72 for each rob
 *     (reply byte to +0xA29), moves each rob's +0x18/+0x20, and decays +0x678
 *     by 0.03.
 * The scratch is the caller's local registers r3..r15 (Gems passes its
 * scratch block in r3). The "contact normal" branch reads the caller's r4, r6,
 * r8 and r10 before writing them: coli_cont_cop's ldt of +0x1F4 from its
 * entry g7 and g8 (fighter 0's x, z and fighter 1's x, z). Those four come in
 * through gfn_coli_recalc_pos_at; gfn_coli_recalc_pos(0) re-reads them from
 * the robs at 0x500804 / 0x500808, which matches only while nothing between
 * coli_cont_cop's entry and this call has moved a +0x1F4. No gems_i960_ret:
 * it is a leaf on the i960 as well. */
#pragma once

/* Gems' compare pattern: fcmpo a, b; a <= b (ordered) gives 1 (less) or 2
 * (equal), anything else 4. */
static inline uint32_t crp_le(float a, float b)
{
    if (a <= b) return a == b ? 2u : 1u;
    return 4u;
}

#define CRP_F(x) gems_u2f(x)
#define CRP_U(x) gems_f2u(x)

/* The two arena-edge hand-offs share this: the factor is 1.0, or 0.9 unless
 * the other's body unit at g13+0x1E8 is at or beyond g13+0x1D8 (then 0). */
static inline uint32_t crp_edge_factor(uint32_t other)
{
    uint32_t f = 0x3F800000u;                   /* 1.0f */
    if ((gems_ld32(other + 0x1A4u) & 0x100u) == 0) {
        f = 0x3F666666u;                        /* 0.9f */
        uint32_t idx = gems_ld8(other + 4u);
        uint32_t a = gems_ld32(GEMS_G(13) + idx * 4u + 0x1E8u);
        uint32_t b = gems_ld32(GEMS_G(13) + 0x1D8u);
        if (!(CRP_F(b) < CRP_F(a)))            /* cmpr b,a; bl: a NaN zeroes too */
            f = 0;
    }
    return f;
}

static uint32_t gfn_coli_recalc_pos_at(uint32_t r4, uint32_t r6, uint32_t r8, uint32_t r10)
{
    uint32_t r3, r7, r9 = 0, r11, r12 = 0, r13 = 0, r14 = 0, r15;
    uint32_t g7 = gems_ld32(0x500804u);
    uint32_t g8 = gems_ld32(0x500808u);
    GEMS_G(7) = g7;
    GEMS_G(8) = g8;

    int pooled = 0;
    if ((gems_ld32(g7 + 0x1A4u) & 0x20000000u) == 0 &&
        (gems_ld32(g8 + 0x1A4u) & 0x20000000u) == 0) {
        if (GEMS_G(6) & 1u) {
            pooled = 1;
        } else if (GEMS_G(6) & 8u) {
            gfn_decide_dir(0);
        }
    }

    if (pooled) {
        r8 = r9 = r10 = 0;
        r12 = r13 = r14 = 0;
        gems_st32(GEMS_G(13) + 0xE0u, r8);
        gems_st32(GEMS_G(13) + 0xE4u, r9);
        gems_st32(GEMS_G(13) + 0xE8u, r10);
        gems_st32(GEMS_G(13) + 0xECu, r12);
        gems_st32(GEMS_G(13) + 0xF0u, r13);
        gems_st32(GEMS_G(13) + 0xF4u, r14);
        r3 = gems_ld32(g7 + 0x1A4u);
        r3 |= r3 << 1;
        if (r3 & 0x80000u) {
            r3 = gems_ld32(g8 + 0x1A4u);
            r3 |= r3 << 1;
            if (r3 & 0x80000u) {
                uint32_t a = gems_ld32(g7 + 0x644u), b = gems_ld32(g8 + 0x644u);
                uint32_t s = CRP_U(CRP_F(a) + CRP_F(b));
                gems_st32(g7 + 0x644u, s);
                gems_st32(g8 + 0x644u, s);
                a = gems_ld32(g7 + 0x64Cu);
                b = gems_ld32(g8 + 0x64Cu);
                s = CRP_U(CRP_F(a) + CRP_F(b));
                gems_st32(g7 + 0x64Cu, s);
                gems_st32(g8 + 0x64Cu, s);
            }
        }
    } else {
        if (gems_ld32(GEMS_G(13) + 0x164u) == 0) {
            gfn_decide_dir(0);
            r8 = r9 = r10 = 0;
            r12 = r13 = r14 = 0;
        } else {
            uint32_t kind = gems_ld32(GEMS_G(13) + 0x88u);
            if (kind == 5u || kind == 4u) {
                uint32_t rob = kind == 5u ? g7 : g8;
                r12 = gems_ld32(rob + 0x678u);
                float v = CRP_F(GEMS_G(5)) + CRP_F(r12);
                r14 = 0x3E19999Au;              /* 0.15f */
                if ((crp_le(v, CRP_F(r14)) & 1u) == 0) {
                    v = CRP_F(r14);
                    GEMS_G(5) = CRP_U(CRP_F(r14) - CRP_F(r12));
                }
                gems_st32(rob + 0x678u, CRP_U(v));
                r8 = r9 = r10 = 0;
                r12 = r13 = r14 = 0;
                if (kind == 5u)
                    r9 = GEMS_G(5);
                else
                    r13 = GEMS_G(5);
            } else {
                r4 = CRP_U(CRP_F(r8) - CRP_F(r4));      /* p1.x - p0.x */
                r6 = CRP_U(CRP_F(r10) - CRP_F(r6));     /* p1.z - p0.z */
                r8 = gems_ld32(GEMS_G(13) + 0xD4u);
                r10 = gems_ld32(GEMS_G(13) + 0xDCu);
                r15 = 0x3E99999Au;                      /* 0.3f */
                r7 = gems_ld32(g7 + 0x804u);
                r11 = gems_ld32(g7 + 0x1A4u);
                r13 = r7 & r11;
                if ((r13 & 0x100u) == 0) {
                    r3 = gems_ld32(g8 + 0x804u);
                    r12 = gems_ld32(g8 + 0x1A4u);
                    r14 = r3 & r12;
                    if ((r14 & 0x100u) == 0) {
                        GEMS_G(0) = 0x2100u;
                        r13 = GEMS_G(0) & r7;
                        r14 = GEMS_G(0) & r3;
                        r13 |= r14;
                        if (r13 == GEMS_G(0)) {
                            r15 = 0x3F333333u;          /* 0.7f */
                        } else if (((r7 >> 6) & r11 & 4u) != 0 ||
                                   ((r3 >> 6) & r12 & 4u) != 0) {
                            r15 = 0x3F266666u;          /* 0.65f */
                        }
                    }
                }
                gems_st32(g7 + 0x67Cu, r15);
                gems_st32(g8 + 0x67Cu, r15);
                r9 = CRP_U(CRP_F(r8) * CRP_F(r15));
                r11 = CRP_U(CRP_F(r10) * CRP_F(r15));
                r4 = CRP_U(CRP_F(r4) - CRP_F(r9));
                r6 = CRP_U(CRP_F(r6) - CRP_F(r11));
                r4 = CRP_U(CRP_F(r4) * CRP_F(r8));
                r6 = CRP_U(CRP_F(r6) * CRP_F(r10));
                r4 = CRP_U(CRP_F(r4) + CRP_F(r6));
                if (r4 & 0x80000000u) {
                    r4 ^= 0x80000000u;
                    GEMS_G(4) = CRP_U(CRP_F(GEMS_G(4)) + CRP_F(r4));
                }
                r15 = 0x3E99999Au;                      /* 0.3f */
                if ((GEMS_G(4) & 0x80000000u) == 0 &&
                    (crp_le(CRP_F(GEMS_G(4)), CRP_F(r15)) & 3u) == 0)
                    GEMS_G(4) = r15;
                r12 = gems_ld32(GEMS_G(13) + 0xD4u);
                r13 = gems_ld32(GEMS_G(13) + 0xD8u);
                r14 = gems_ld32(GEMS_G(13) + 0xDCu);
                r3 = gems_ld32(GEMS_G(13) + 0xCCu) ^ 0x80000000u;
                r3 = CRP_U(CRP_F(GEMS_G(4)) * CRP_F(r3));
                r8 = CRP_U(CRP_F(r3) * CRP_F(r12));
                r9 = CRP_U(CRP_F(r3) * CRP_F(r13));
                r10 = CRP_U(CRP_F(r3) * CRP_F(r14));
                r3 = gems_ld32(GEMS_G(13) + 0xD0u);
                r3 = CRP_U(CRP_F(GEMS_G(4)) * CRP_F(r3));
                r12 = CRP_U(CRP_F(r3) * CRP_F(r12));
                r13 = CRP_U(CRP_F(r3) * CRP_F(r13));
                r14 = CRP_U(CRP_F(r3) * CRP_F(r14));
            }
        }
        gems_st32(GEMS_G(13) + 0xE0u, r8);
        gems_st32(GEMS_G(13) + 0xE4u, r9);
        gems_st32(GEMS_G(13) + 0xE8u, r10);
        gems_st32(GEMS_G(13) + 0xECu, r12);
        gems_st32(GEMS_G(13) + 0xF0u, r13);
        gems_st32(GEMS_G(13) + 0xF4u, r14);
    }

    /* A wall holds one axis: bit 14 of (angle ^ angle << 1) picks which. */
    if (gems_ld16(GEMS_G(13) + 0x23Cu) != 0) {
        uint32_t a = (uint32_t)gems_ld16s(g7 + 0x26u);
        if ((((a << 1) ^ a) & 0x4000u) == 0) r8 = 0;
        else r10 = 0;
    }
    if (gems_ld16(GEMS_G(13) + 0x23Eu) != 0) {
        uint32_t a = (uint32_t)gems_ld16s(g8 + 0x26u);
        if ((((a << 1) ^ a) & 0x4000u) == 0) r12 = 0;
        else r14 = 0;
    }

    r7 = 0;
    {
        uint32_t a = gems_ld32(g7 + 0x680u), b = gems_ld32(g8 + 0x680u);
        if ((crp_le(CRP_F(b), CRP_F(a)) & 3u) == 0)
            r7 = 1;
    }

    r3 = 0;
    if (gems_ld32(g7 + 0x644u) != 0 && r7 == 0) {
        r15 = crp_edge_factor(g8);
        r15 = CRP_U(CRP_F(r8) * CRP_F(r15));
        r12 = CRP_U(CRP_F(r12) - CRP_F(r15));
        r8 = 0;
        r10 = 0;
        r3 |= 1u;
    }
    if (gems_ld32(g7 + 0x64Cu) != 0 && r7 == 0) {
        r15 = crp_edge_factor(g8);
        r15 = CRP_U(CRP_F(r10) * CRP_F(r15));
        r14 = CRP_U(CRP_F(r14) - CRP_F(r15));
        r10 = 0;
        r8 = 0;
        r3 |= 2u;
    }
    if (gems_ld32(g8 + 0x644u) != 0 && r7 == 1u) {
        r15 = crp_edge_factor(g7);
        r15 = CRP_U(CRP_F(r14) * CRP_F(r15));  /* r14, not r12: as Gems and the ROM */
        r8 = CRP_U(CRP_F(r8) - CRP_F(r15));
        r12 = 0;
        r14 = 0;
        r3 |= 4u;
    }
    if (gems_ld32(g8 + 0x64Cu) != 0 && r7 == 1u) {
        r15 = crp_edge_factor(g7);
        r15 = CRP_U(CRP_F(r14) * CRP_F(r15));
        r10 = CRP_U(CRP_F(r10) - CRP_F(r15));
        r14 = 0;
        r12 = 0;
        r3 |= 8u;
    }
    gems_st32(GEMS_G(13) + 0xE0u, r8);
    gems_st32(GEMS_G(13) + 0xE4u, r9);
    gems_st32(GEMS_G(13) + 0xE8u, r10);
    gems_st32(GEMS_G(13) + 0xECu, r12);
    gems_st32(GEMS_G(13) + 0xF0u, r13);
    gems_st32(GEMS_G(13) + 0xF4u, r14);

    /* Clamp each step to its rob's limits: +0x650 / +0x658 above, minus
     * +0x654 / +0x65C below. */
    if ((r3 & 1u) == 0) {
        if ((r8 & 0x80000000u) == 0) {
            r15 = gems_ld32(g7 + 0x650u);
            if ((crp_le(CRP_F(r8), CRP_F(r15)) & 3u) == 0) r8 = r15;
        } else {
            r15 = gems_ld32(g7 + 0x654u) ^ 0x80000000u;
            if (!(CRP_F(r15) <= CRP_F(r8))) r8 = r15;   /* cmpr r15,r8; ble */
        }
    }
    if ((r3 & 2u) == 0) {
        r15 = gems_ld32(g7 + 0x658u);
        if ((r10 & 0x80000000u) == 0) {
            if ((crp_le(CRP_F(r10), CRP_F(r15)) & 3u) == 0) r10 = r15;
        } else {
            r15 = gems_ld32(g7 + 0x65Cu) ^ 0x80000000u;
            if (!(CRP_F(r15) <= CRP_F(r10))) r10 = r15;   /* cmpr r15,r10; ble */
        }
    }
    if ((r3 & 4u) == 0) {
        if ((r12 & 0x80000000u) == 0) {
            r15 = gems_ld32(g8 + 0x650u);
            if ((crp_le(CRP_F(r12), CRP_F(r15)) & 3u) == 0) r12 = r15;
        } else {
            r15 = gems_ld32(g8 + 0x654u) ^ 0x80000000u;
            if (!(CRP_F(r15) <= CRP_F(r12))) r12 = r15;   /* cmpr r15,r12; ble */
        }
    }
    if ((r3 & 8u) == 0) {
        r15 = gems_ld32(g8 + 0x658u);
        if ((r14 & 0x80000000u) == 0) {
            if ((crp_le(CRP_F(r14), CRP_F(r15)) & 3u) == 0) r14 = r15;
        } else {
            r15 = gems_ld32(g8 + 0x65Cu) ^ 0x80000000u;
            if (!(CRP_F(r15) <= CRP_F(r14))) r14 = r15;   /* cmpr r15,r14; ble */
        }
    }

    r8 = CRP_U(CRP_F(r8) - gems_ldf(g7 + 0x644u));
    r10 = CRP_U(CRP_F(r10) - gems_ldf(g7 + 0x64Cu));
    r12 = CRP_U(CRP_F(r12) - gems_ldf(g8 + 0x644u));
    r14 = CRP_U(CRP_F(r14) - gems_ldf(g8 + 0x64Cu));
    gems_st32(g7 + 0xA68u, r8);
    gems_st32(g7 + 0xA6Cu, r10);
    gems_st32(g8 + 0xA68u, r12);
    gems_st32(g8 + 0xA6Cu, r14);

    gems_cop_w(0x1E803D3Du);                    /* push-out onto the unit matrices */
    gems_cop_w(r8);
    gems_cop_w(r9);
    gems_cop_w(r10);
    gems_cop_w(r12);
    gems_cop_w(r13);
    gems_cop_w(r14);
    gems_cop_w(0x39007272u);
    gems_cop_w(gems_ld8(g7 + 4u));
    gems_cop_w(r8);
    gems_cop_w(r10);
    gems_st8(g7 + 0xA29u, gems_cop_r() & 0xFFu);
    gems_cop_w(0x39007272u);
    gems_cop_w(gems_ld8(g8 + 4u));
    gems_cop_w(r12);
    gems_cop_w(r14);
    gems_st8(g8 + 0xA29u, gems_cop_r() & 0xFFu);

    gems_stf(g7 + 0x18u, gems_ldf(g7 + 0x18u) + CRP_F(r8));
    gems_stf(g7 + 0x20u, gems_ldf(g7 + 0x20u) + CRP_F(r10));
    gems_stf(g8 + 0x18u, gems_ldf(g8 + 0x18u) + CRP_F(r12));
    gems_stf(g8 + 0x20u, gems_ldf(g8 + 0x20u) + CRP_F(r14));

    r15 = CRP_U(gems_ldf(g7 + 0x678u) - CRP_F(0x3CF5C28Fu));   /* - 0.03f */
    if (r15 & 0x80000000u) r15 = 0;
    gems_st32(g7 + 0x678u, r15);
    r15 = CRP_U(gems_ldf(g8 + 0x678u) - CRP_F(0x3CF5C28Fu));
    if (r15 & 0x80000000u) r15 = 0;
    gems_st32(g8 + 0x678u, r15);
    return 0;
}

static uint32_t gfn_coli_recalc_pos(int entry)
{
    (void)entry;                                /* a leaf: never returns through the i960 */
    uint32_t p0 = gems_ld32(0x500804u), p1 = gems_ld32(0x500808u);
    return gfn_coli_recalc_pos_at(gems_ld32(p0 + 0x1F4u), gems_ld32(p0 + 0x1FCu),
                                  gems_ld32(p1 + 0x1F4u), gems_ld32(p1 + 0x1FCu));
}

#undef CRP_F
#undef CRP_U
