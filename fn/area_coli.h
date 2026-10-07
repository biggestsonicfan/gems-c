/* area_coli (i960 0x2DCA4; Gems 80043ED8): a fighter (g7) against the arena.
 *   - op 0x13, then 0x70 (Fn_area_coli) with the fighter's +0x1C, the arena
 *     at 0x50A010 / 0x50A00C, the floor height (0x50027C - 0.19, +10.0 unless
 *     the fighter is in state 2+ of +0x1A18 or bit 9 of +0x70C holds it) and
 *     the player number (bit 31 set in state 3). Its 22 replies go to
 *     g13+0x118..+0x124, +0x670, +0x710, +0x660, +0xA58, +0x650 and
 *     +0x614/+0x674/+0x624/+0x618 (+0x624 cleared off a slope, +0xA29 == 0).
 *   - turns the push-out (+0x660..+0x66C) around (state 3 takes +0xA58's),
 *     makes -0 plain 0, and sums it with the margin into +0x644 / +0x64C
 *     (cleared on stage 3 in mode 5); a non-zero one clears the limit on its
 *     side (+0x650 / +0x654, +0x658 / +0x65C).
 *   - op 0x2B (the 3D distance of +0x1F4) into +0x680.
 *   - the larger of |+0x644| and |+0x64C| names the wall: the facing at
 *     g13+0x1F0[p], the wall kind (ROM 0x2E260) at g13+0x1F4[p] and +0xAE2,
 *     2 at g13+0x23C[p], and bit 11 of +0xA08.
 *   - the smallest of each fighter's four limits (+0x650..+0x65C) and its
 *     index give g13+0x1F8[p], g13+0x204[p] and, through ROM 0x2E250 /
 *     0x2E230, g13+0x1FC[p] / +0x200[p].
 * Float compares follow the i960's cmpr (Gems' fcmpo had them backwards). The -1.0
 * multiplies are PowerPC fmuls (a NaN keeps its sign). */
#pragma once

/* cmpr a, b as the i960 sets the condition code: 4 less, 2 equal, 1 greater,
 * 0 unordered. Every site is `cmpr a,b; bl skip`, so `& 4u) == 0` runs the
 * block when !(a < b), a NaN included. */
static inline uint32_t gems_cmpr_area_coli(uint32_t a, uint32_t b)
{
    float fa = gems_u2f(a), fb = gems_u2f(b);
    if (fa < fb) return 4u;
    if (fa == fb) return 2u;
    return fa > fb ? 1u : 0u;
}

/* fmuls by -1.0: the sign flips, a NaN comes back quieted with its sign. */
static inline uint32_t gems_neg_area_coli(uint32_t v)
{
    if ((v & 0x7F800000u) == 0x7F800000u && (v & 0x007FFFFFu) != 0)
        return v | 0x00400000u;
    return v ^ 0x80000000u;
}

/* Smallest of the four words at base (in order, a later equal one wins);
 * *idx gets its index (0..3). */
static inline uint32_t gems_min4_area_coli(uint32_t base, uint32_t *idx)
{
    uint32_t r4 = gems_ld32(base), r10 = 0;
    for (uint32_t r11 = 3; r11 >= 1u; r11--) {
        uint32_t r6 = gems_ld32(base + (4u - r11) * 4u);
        if ((gems_cmpr_area_coli(r4, r6) & 4u) == 0) {
            r10 = 4u - r11;
            r4 = r6;
        }
    }
    *idx = r10;
    return r4;
}

static uint32_t gfn_area_coli(int entry)
{
    uint32_t g7 = GEMS_G(7), g13 = GEMS_G(13);
    uint32_t q[4], r9, r10, r12, v;

    gems_cop_w(0x09801313u);
    gems_cop_w(0x09801313u);
    gems_cop_w(0x09801313u);
    (void)gems_cop_r();

    gems_cop_w(0x38007070u);
    gems_cop_w(gems_ld32(g7 + 0x1Cu));
    gems_cop_w(gems_ld32(0x50A010u));
    gems_cop_w(gems_ld32(0x50A00Cu));
    v = gems_f2u(gems_ldf(0x50027Cu) - gems_u2f(0x3E428F5Cu));     /* - 0.19 */
    if (gems_ld8(g7 + 0x1A18u) < 2u && (gems_ld32(g7 + 0x70Cu) & 0x200u) == 0)
        v = gems_f2u(gems_u2f(0x41200000u) + gems_u2f(v));         /* + 10.0 */
    gems_cop_w(v);
    v = gems_ld8(g7 + 4u);
    if (gems_ld8(g7 + 0x1A18u) == 3u)
        v |= 0x80000000u;
    gems_cop_w(v);

    gems_st32(g13 + 0x118u, gems_cop_r());
    gems_st32(g13 + 0x11Cu, gems_cop_r());
    gems_st32(g13 + 0x120u, gems_cop_r());
    gems_st32(g13 + 0x124u, gems_cop_r());
    gems_st32(g7 + 0x670u, gems_cop_r());
    gems_st32(g7 + 0x710u, gems_cop_r());
    gems_cop_rn(q, 4);
    gems_stn(g7 + 0x660u, q, 4);
    gems_cop_rn(q, 4);
    gems_stn(g7 + 0xA58u, q, 4);
    gems_cop_rn(q, 4);
    gems_stn(g7 + 0x650u, q, 4);
    gems_cop_rn(q, 4);
    gems_st32(g7 + 0x614u, q[0]);
    gems_st32(g7 + 0x674u, q[1]);
    gems_st32(g7 + 0x624u, q[2]);
    gems_st32(g7 + 0x618u, q[3]);
    if (gems_ld8(g7 + 0xA29u) == 0)
        gems_st32(g7 + 0x624u, 0);

    if (gems_ld8(g7 + 0x1A18u) == 3u) {
        gems_ldn(q, g7 + 0xA58u, 4);
        gems_stn(g7 + 0x660u, q, 4);
        gems_st32(g7 + 0x660u, gems_neg_area_coli(gems_ld32(g7 + 0x660u)));
        gems_st32(g7 + 0x668u, gems_neg_area_coli(gems_ld32(g7 + 0x668u)));
    } else {
        gems_st32(g7 + 0x664u, gems_neg_area_coli(gems_ld32(g7 + 0x664u)));
        gems_st32(g7 + 0x66Cu, gems_neg_area_coli(gems_ld32(g7 + 0x66Cu)));
    }
    for (uint32_t o = 0x660u; o <= 0x66Cu; o += 4u)
        if (gems_ld32(g7 + o) == 0x80000000u)
            gems_st32(g7 + o, 0);

    gems_ldn(q, g7 + 0x660u, 2);
    gems_stf(g7 + 0x644u, gems_u2f(q[0]) + gems_u2f(q[1]));
    gems_ldn(q, g7 + 0x668u, 2);
    gems_stf(g7 + 0x64Cu, gems_u2f(q[0]) + gems_u2f(q[1]));
    if (gems_ld8(0x50002Bu) == 3u && gems_ld8(0x500031u) == 5u) {
        gems_st32(g7 + 0x644u, 0);
        gems_st32(g7 + 0x64Cu, 0);
    }

    v = gems_ld32(g7 + 0x644u);
    if (v != 0)
        gems_st32(g7 + 0x650u + (v >> 31) * 4u, 0);
    v = gems_ld32(g7 + 0x64Cu);
    if (v != 0)
        gems_st32(g7 + 0x658u + (v >> 31) * 4u, 0);

    gems_ldn(q, g7 + 0x1F4u, 3);
    gems_cop_w(0x15802B2Bu);
    gems_cop_w(q[0]);
    gems_cop_w(0);
    gems_cop_w(q[2]);
    gems_cop_w(0);
    gems_st32(g7 + 0x680u, gems_cop_r());

    uint32_t p = gems_ld8(g7 + 4u);
    uint32_t ax = gems_ld32(g7 + 0x644u) & 0x7FFFFFFFu;
    uint32_t az = gems_ld32(g7 + 0x64Cu) & 0x7FFFFFFFu;
    r9 = 0;
    r12 = az;
    int done = 0;

    /* A wall across x: |+0x644| is the larger (or +0x64C is 0). */
    if (ax != 0 &&
        (az == 0 || (gems_cmpr_area_coli(ax, az) & 4u) == 0)) {
        uint32_t r10x = gems_ld32(g7 + 0x644u);
        if ((r10x & 0x7FFFFFFFu) != 0) {
            uint32_t r4 = (uint32_t)gems_ld16s(g7 + 0x26u);
            gems_st16(g13 + 0x1F0u + p * 2u, r4 & 0xFFFFu);
            r12 = ((r4 ^ r10x) >> 28) & 8u;
            r4 &= 0x7000u;
            gems_st32(g7 + 0xA08u, gems_ld32(g7 + 0xA08u) | (r12 << 8));
            r4 = (r4 >> 12) | r12;
            r12 |= gems_ld8(0x2E260u + r4);
            gems_st16(g13 + 0x23Cu + p * 2u, 2);
            gems_st16(g13 + 0x1F4u + p * 2u, r12 & 0xFFFFu);
            gems_st16(g7 + 0xAE2u, (r12 & ~8u) & 0xFFFFu);
            done = 1;
        }
    }
    /* A wall across z: |+0x64C| at least |+0x644|. */
    if (!done) {
        uint32_t r10z = gems_ld32(g7 + 0x64Cu);
        uint32_t r11 = r10z & 0x7FFFFFFFu;
        if (r11 != 0 &&
            (gems_cmpr_area_coli(r11, gems_ld32(g7 + 0x644u) & 0x7FFFFFFFu) & 4u) == 0) {
            uint32_t r4 = (uint32_t)gems_ld16s(g7 + 0x26u);
            gems_st16(g13 + 0x1F0u + p * 2u, r4 & 0xFFFFu);
            r4 -= 0x4000u;
            r10z >>= 16;
            r9 = ((r10z ^ r4) >> 12) & 8u;
            r4 = (r4 & 0x7000u) >> 12;
            gems_st32(g7 + 0xA08u, gems_ld32(g7 + 0xA08u) | (r9 << 8));
            r4 |= r9;
            r9 |= gems_ld8(0x2E260u + r4);
            gems_st16(g13 + 0x23Cu + p * 2u, 2);
            gems_st16(g13 + 0x1F4u + p * 2u, r9 & 0xFFFFu);
            gems_st16(g7 + 0xAE2u, (r9 & ~8u) & 0xFFFFu);
        }
    }

    /* The nearest of this fighter's limits. */
    (void)gems_min4_area_coli(g7 + 0x650u, &r10);
    if ((r10 & 2u) == 0)
        r9 = r12;
    r9 = (r9 & 7u) | (r10 << 4);
    gems_st16(g13 + 0x1F8u + p * 2u, r9 & 0xFFFFu);

    /* And the other fighter's. */
    uint32_t r7 = gems_ld8(g7 + 4u);
    uint32_t other = gems_ld32(0x500804u + ((~r7) & 1u) * 4u);
    uint32_t lim = gems_min4_area_coli(other + 0x650u, &r10);
    gems_st32(g13 + 0x204u + p * 4u, lim);
    if (r7 & 1u)
        r10 = (r10 & 2u) | ((~r10) & 1u);
    uint32_t a = gems_ld32(g13 + 0x1E0u);
    r10 |= (((a << 1) ^ a) >> 12) & 4u;
    r10 |= gems_ld16(0x2E250u + (a >> 13) * 2u);
    uint32_t r11 = gems_ld16(0x2E230u + r10 * 2u);
    gems_st16(g13 + 0x1FCu + p * 2u, r10 & 0xFFFFu);
    gems_st16(g13 + 0x200u + p * 2u, r11 & 0xFFFFu);

    if (entry) gems_i960_ret();
    return 0;
}
