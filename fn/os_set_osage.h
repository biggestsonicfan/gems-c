/* os_set_osage (i960 0x67D28; Gems 8004D914; FN INDEX names 0x685FC, which
 * is the next routine): set up one sway chain's root. g13 = the fighter, g9 =
 * the chain's osage record, g8 = its output block, g7 = the character.
 *   - loads unit matrix (*g9) of the character's set (byte g7+4), turns it by
 *     g9+4..+0xC (z, y, x) and moves it by g9+0x10..+0x18;
 *   - g13+0xF4 = g13+0xE4 - the world y of that point;
 *   - g4..g6 = g13+0x138 with y lowered by g9+0x1C * the float at 0x50A000,
 *     transformed into that frame (and stored to g13+0x114 when bit 0 of
 *     *g13 is set); g0..g2 = 0x57's reply, stored to g13+0xE8;
 *   - writes the frame's matrix to g8+0x30, its matrix moved to g4..g6 to
 *     g8+0x60, smooths the record's matrix at g13 + *(g13+0x50) through 0x44
 *     (unless bit 0 of *g13: then from the base matrix), advancing +0x50 by
 *     0x30, and writes 0x45's matrix to g8. */
#pragma once

static inline void gfn_os_set_osage_get_matrix(uint32_t dst)
{
    gems_cop_w(0x02800505u);                    /* get_matrix */
    for (uint32_t i = 0; i < 12u; i++)
        gems_st32(dst + i * 4u, gems_cop_r());
}

static uint32_t gfn_os_set_osage(int entry)
{
    uint32_t tri[3];

    gems_cop_w(0x00800101u);                    /* push matrix */
    uint32_t unit = gems_ld32(GEMS_G(9));
    gems_cop_w(0x1B003636u);                    /* ld_unit_mat */
    gems_cop_w(gems_ld8(GEMS_G(7) + 4u));
    gems_cop_w(unit * 12u);
    gems_ldn(&GEMS_G(0), GEMS_G(9) + 4u, 3);
    gems_cop_w(0x05000A0Au);                    /* ang_z */
    gems_cop_w(GEMS_G(0));
    gems_cop_w(0x04800909u);                    /* ang_y */
    gems_cop_w(GEMS_G(1));
    gems_cop_w(0x04000808u);                    /* ang_x */
    gems_cop_w(GEMS_G(2));
    gems_ldn(&GEMS_G(0), GEMS_G(9) + 0x10u, 3);
    gems_cop_w(0x03000606u);                    /* trans */
    gems_cop_w(GEMS_G(0));
    gems_cop_w(GEMS_G(1));
    gems_cop_w(GEMS_G(2));
    gems_cop_w(0x21804343u);
    gems_cop_w(0u);
    gems_cop_w(0x07800F0Fu);                    /* get_point */
    GEMS_G(0) = gems_cop_r();
    GEMS_G(1) = gems_cop_r();
    GEMS_G(2) = gems_cop_r();
    GEMS_G(1) ^= 0x80000000u;
    GEMS_G(0) = gems_ld32(GEMS_G(13) + 0xE4u);
    GEMS_G(1) = gems_f2u(gems_u2f(GEMS_G(1)) + gems_u2f(GEMS_G(0)));
    gems_st32(GEMS_G(13) + 0xF4u, GEMS_G(1));

    gems_cop_w(0x06000C0Cu);
    gems_cop_w(0x09801313u);                    /* op 0x13, args = the command word */
    gems_cop_w(0x09801313u);
    gems_cop_w(0x09801313u);
    (void)gems_cop_r();
    gems_cop_w(0x21804343u);
    gems_cop_w(1u);

    gems_ldn(&GEMS_G(4), GEMS_G(13) + 0x138u, 3);
    float drop = gems_u2f(gems_ld32(0x50A000u) ^ 0x80000000u);
    float k = gems_ldf(GEMS_G(9) + 0x1Cu);
    drop = k * drop;
    GEMS_G(5) = gems_f2u(gems_u2f(GEMS_G(5)) + drop);
    gems_cop_w(0x00800101u);                    /* push matrix */
    gems_cop_w(0x06800D0Du);                    /* base_point */
    gems_cop_w(0x14802929u);                    /* point_trans */
    gems_cop_w(GEMS_G(4));
    gems_cop_w(GEMS_G(5));
    gems_cop_w(GEMS_G(6));
    GEMS_G(4) = gems_cop_r();
    GEMS_G(5) = gems_cop_r();
    GEMS_G(6) = gems_cop_r();
    if (gems_ld32(GEMS_G(13)) & 1u)
        gems_stn(GEMS_G(13) + 0x114u, &GEMS_G(4), 3);
    gems_cop_w(0x2B805757u);
    GEMS_G(0) = gems_cop_r();
    GEMS_G(1) = gems_cop_r();
    GEMS_G(2) = gems_cop_r();
    gems_stn(GEMS_G(13) + 0xE8u, &GEMS_G(0), 3);
    gems_cop_w(0x01000202u);                    /* pop matrix */

    uint32_t smooth = gems_ld32(GEMS_G(13) + 0x50u);
    if ((gems_ld32(GEMS_G(13)) & 1u) == 0) {
        uint32_t rec = GEMS_G(13) + smooth;
        gems_cop_w(0x05800B0Bu);                /* mul_matrix */
        for (uint32_t i = 0; i < 4u; i++) {
            gems_ldn(tri, rec + i * 0xCu, 3);
            gems_cop_wn(tri, 3);
        }
    } else {
        gems_cop_w(0x01800303u);                /* base matrix */
    }
    gfn_os_set_osage_get_matrix(GEMS_G(8) + 0x30u);
    gems_cop_w(0x07000E0Eu);                    /* load_point */
    gems_cop_w(GEMS_G(4));
    gems_cop_w(GEMS_G(5));
    gems_cop_w(GEMS_G(6));
    gfn_os_set_osage_get_matrix(GEMS_G(8) + 0x60u);
    gems_cop_w(0x22004444u);
    gems_cop_w(0u);
    gems_cop_w(0x02800505u);                    /* get_matrix */
    {
        uint32_t rec = GEMS_G(13) + smooth;
        for (uint32_t i = 0; i < 4u; i++) {
            gems_cop_rn(tri, 3);
            gems_stn(rec + i * 0xCu, tri, 3);
        }
    }
    gems_st32(GEMS_G(13) + 0x50u, smooth + 0x30u);
    gems_cop_w(0x01000202u);                    /* pop matrix */

    gems_cop_w(0x00800101u);                    /* push matrix */
    gems_cop_w(0x22804545u);
    gems_cop_w(0u);
    gfn_os_set_osage_get_matrix(GEMS_G(8));
    gems_cop_w(0x01000202u);                    /* pop matrix */

    if (entry) gems_i960_ret();
    return 0;
}
