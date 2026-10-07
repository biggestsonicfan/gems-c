/* os_set_coli (i960 0x6829C; Gems 8004D024): set up a sway chain's collision
 * record g8 from its osage record g9 and chain state g13.
 * With bit 0 of g13's flags: the radii and their squares (+0x1C..+0x3C) and
 * the two limit planes' cos / sin (COP 0x22 / 0x21 of the angle at +0x20 /
 * +0x24 turned by -/+ 0x4000) and their scaled forms (+0x40..+0x54).
 * Always: the two end points through the bone matrix (COP 0x36, then 0x29,
 * 0x44 1, 0x29) to +0x10 and +0x24, and for the two limits at +0x2C / +0x34
 * a point (+0x60 / +0x6C) and the ratio of the line crossing (+0x58 / +0x5C).
 * Leaves g0..g6 as Gems has them (g0..g3 the last crossing's floats). */
#pragma once

/* One limit: COP 0x5B / 0x59 of the angle at g9+arg, the stored point,
 * and the crossing with the plane at g8+plane. */
static void os_set_coli_limit(uint32_t arg, uint32_t add, uint32_t point,
                              uint32_t plane, uint32_t ratio,
                              uint32_t base0, uint32_t base1)
{
    gems_cop_w(0x2D805B5B);
    gems_cop_w(gems_ld32(GEMS_G(9) + arg));
    gems_cop_w(GEMS_G(4));
    gems_cop_w(GEMS_G(5));
    float p = gems_cop_rf();
    float q = gems_cop_rf();
    gems_cop_w(0x2C805959);
    gems_cop_wf(p);
    gems_cop_w(base0);
    gems_cop_wf(q);
    gems_cop_w(base1);
    float s = gems_cop_rf() + gems_ldf(GEMS_G(9) + add);
    gems_stf(GEMS_G(8) + point + 0, p);
    gems_stf(GEMS_G(8) + point + 4, q);
    gems_stf(GEMS_G(8) + point + 8, s);
    float a = gems_ldf(GEMS_G(8) + plane + 0);
    float b = gems_ldf(GEMS_G(8) + plane + 4);
    float c = gems_ldf(GEMS_G(8) + plane + 8);
    float g0 = c * p, g1 = s * a, g2 = b * p, g3 = q * a;
    g0 = g1 - g0;
    g2 = g3 - g2;
    GEMS_G(0) = gems_f2u(g0);
    GEMS_G(1) = gems_f2u(g1);
    GEMS_G(2) = gems_f2u(g2);
    GEMS_G(3) = gems_f2u(g3);
    gems_stf(GEMS_G(8) + ratio, g0 / g2);
}

/* One limit plane: cos and sin of the angle, then the radius-scaled cos. */
static void os_set_coli_plane(uint32_t angle, uint32_t plane)
{
    gems_cop_w(0x11002222);
    gems_cop_w(angle);
    gems_st32(GEMS_G(8) + plane + 0, gems_cop_r());
    gems_cop_w(0x10802121);
    gems_cop_w(angle);
    gems_st32(GEMS_G(8) + plane + 4, gems_cop_r());
    gems_stf(GEMS_G(8) + plane + 8,
             gems_ldf(GEMS_G(8) + plane) * gems_ldf(GEMS_G(8) + 0x38));
}

static uint32_t gfn_os_set_coli(int entry)
{
    if (gems_ld32(GEMS_G(13)) & 1) {
        static const uint32_t rad[3][2] = { { 0x0C, 0x1C }, { 0x18, 0x30 }, { 0x1C, 0x38 } };
        for (int i = 0; i < 3; i++) {
            float r = gems_ldf(GEMS_G(9) + rad[i][0]);
            gems_stf(GEMS_G(8) + rad[i][1], r);
            gems_stf(GEMS_G(8) + rad[i][1] + 4, r * r);
        }
        os_set_coli_plane(gems_ld32(GEMS_G(9) + 0x20) - 0x4000, 0x40);
        os_set_coli_plane(gems_ld32(GEMS_G(9) + 0x24) + 0x4000, 0x4C);
        gems_st32(GEMS_G(8) + 0x54, gems_ld32(GEMS_G(8) + 0x54) ^ 0x80000000u);
    }
    gems_cop_w(0x00800101);                    /* push */
    uint32_t bone = gems_ld32(GEMS_G(9));
    gems_cop_w(0x1B003636);
    gems_cop_w(gems_ld8(GEMS_G(7) + 4));
    gems_cop_w(bone * 0xC);
    uint32_t x = gems_ld32(GEMS_G(9) + 4), z = gems_ld32(GEMS_G(9) + 8);
    gems_cop_w(0x14802929);
    gems_cop_w(x);
    gems_cop_w(0);
    gems_cop_w(z);
    GEMS_G(4) = gems_cop_r();
    GEMS_G(5) = gems_cop_r();
    GEMS_G(6) = gems_cop_r();
    x = gems_ld32(GEMS_G(9) + 0x10);
    z = gems_ld32(GEMS_G(9) + 0x14);
    gems_cop_w(0x14802929);
    gems_cop_w(x);
    gems_cop_w(0);
    gems_cop_w(z);
    GEMS_G(0) = gems_cop_r();
    GEMS_G(1) = gems_cop_r();
    GEMS_G(2) = gems_cop_r();
    gems_cop_w(0x22004444);
    gems_cop_w(1);
    gems_cop_w(0x14802929);
    gems_cop_w(GEMS_G(4));
    gems_cop_w(GEMS_G(5));
    gems_cop_w(GEMS_G(6));
    GEMS_G(4) = gems_cop_r();
    GEMS_G(5) = gems_cop_r();
    GEMS_G(6) = gems_cop_r();
    gems_stn(GEMS_G(8) + 0x10, &GEMS_G(4), 3);
    gems_cop_w(0x14802929);
    gems_cop_w(GEMS_G(0));
    gems_cop_w(GEMS_G(1));
    gems_cop_w(GEMS_G(2));
    GEMS_G(0) = gems_cop_r();
    GEMS_G(1) = gems_cop_r();
    GEMS_G(2) = gems_cop_r();
    gems_stn(GEMS_G(8) + 0x24, &GEMS_G(0), 3);

    bone = gems_ld32(GEMS_G(9) + 0x28);
    gems_cop_w(0x1B803737);
    gems_cop_w(gems_ld8(GEMS_G(7) + 4));
    gems_cop_w(bone * 0xC);
    gems_cop_w(0x2B005656);
    GEMS_G(4) = gems_cop_r();
    GEMS_G(5) = gems_cop_r();
    (void)gems_cop_r();
    gems_cop_w(0x07800F0F);                    /* read_world_pos */
    uint32_t base0 = gems_cop_r();
    uint32_t base1 = gems_cop_r();
    (void)gems_cop_r();
    gems_cop_w(0x2D005A5A);
    gems_cop_w(GEMS_G(4));
    gems_cop_w(GEMS_G(5));
    GEMS_G(4) = gems_cop_r();
    GEMS_G(5) = gems_cop_r();
    os_set_coli_limit(0x2C, 0x30, 0x60, 0x40, 0x58, base0, base1);
    os_set_coli_limit(0x34, 0x38, 0x6C, 0x4C, 0x5C, base0, base1);
    gems_cop_w(0x01000202);                    /* pop */

    uint32_t keep[3];
    gems_ldn(keep, GEMS_G(13) + 0xE8, 3);
    uint32_t w = gems_ld32(GEMS_G(13) + 0xF4);
    gems_stn(GEMS_G(8), keep, 3);
    gems_st32(GEMS_G(8) + 0xC, w);
    if (entry) gems_i960_ret();
    return 0;
}
