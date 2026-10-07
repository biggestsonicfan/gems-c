/* rob_disp_rdn_ps (trap at i960 0x1A378, inside rob_disp_rdn_loop's tail;
 * Gems 800460B0): the fighter's waist-stretch parts in mode 3 (kosi_nobi_put),
 * back to mode 1, then, when the fighter's flag bit 24 is up, the marker model
 * 6 at its world position (g7+0x1F4 / +0x1FC, y 0.1), turned by g7+0x616 and
 * scaled (1, 1, 3).
 * The i960 carries on at 0x1A438 after this (more of the fighter's drawing);
 * Gems returns from the function instead, as kept here. */
#pragma once

static uint32_t gfn_rob_disp_rdn_ps(int entry)
{
    uint32_t rob;
    GEMS_G(0) = 3;
    gfn_set_mmode(0);
    gfn_kosi_nobi_put(0);
    GEMS_G(0) = 1;
    gfn_set_mmode(0);
    rob = GEMS_G(7);
    GEMS_R(15) = gems_ld32(rob);
    if (GEMS_R(15) & 0x1000000u) {
        gems_cop_w(GEMS_R(15) = 0x00800101u);     /* push */
        gems_cop_w(GEMS_R(15) = 0x03000606u);     /* trans */
        gems_cop_w(GEMS_R(15) = gems_ld32(rob + 0x1F4));
        gems_cop_w(GEMS_R(15) = 0x3DCCCCCDu);     /* 0.1f */
        gems_cop_w(GEMS_R(15) = gems_ld32(rob + 0x1FC));
        gems_cop_w(GEMS_R(15) = 0x04800909u);     /* y_rot */
        gems_cop_w(GEMS_R(15) = (uint32_t)gems_ld16s(rob + 0x616));
        gems_cop_w(GEMS_R(15) = 0x03800707u);     /* scale */
        gems_cop_w(GEMS_R(15) = 0x3F800000u);
        gems_cop_w(GEMS_R(15) = 0x3F800000u);
        gems_cop_w(GEMS_R(15) = 0x40400000u);
        GEMS_R(3) = 0x4000;
        gems_cop_w(GEMS_R(15) = 0x04000808u);     /* x_rot */
        gems_cop_w(GEMS_R(3));
        GEMS_G(1) = 0;
        GEMS_G(0) = 6;
        gfn_set_obj();
        gems_cop_w(GEMS_R(15) = 0x01000202u);     /* pop */
    }
    if (entry) gems_i960_ret();
    return 0;
}
