/* spin_attack_cnt_nml_dsp (i960 0x81848; Gems 80047E74): the spin-attack
 * ball, scaled by (g0, g1, g2) plus the pulse at 0x509518 (kept at g7+0x2074),
 * turned by g7+0x2080 and drawn as model g7+0x2084 -- or, on odd frames
 * (0x500020 bit 0), turned by g7+0x2082 with no half turn and drawn as
 * g7+0x2086. */
#pragma once

static uint32_t gfn_spin_attack_cnt_nml_dsp(int entry)
{
    uint32_t rob = GEMS_G(7);
    float lift = gems_ldf(0x509518);
    uint32_t pos[3] = {
        gems_f2u(lift + gems_u2f(GEMS_G(0))),
        gems_f2u(lift + gems_u2f(GEMS_G(1))),
        gems_f2u(lift + gems_u2f(GEMS_G(2))),
    };
    gems_stn(rob + 0x2074, pos, 3);
    gems_cop_w(0x04000808u);              /* x_rot */
    gems_cop_w(0xFFFFE000u);
    gems_cop_w(0x03800707u);              /* scale */
    gems_cop_wn(pos, 3);
    uint32_t model;
    if ((gems_ld32(0x500020) & 1) == 0) {
        gems_cop_w(0x04000808u);
        gems_cop_w(gems_ld16(rob + 0x2080));
        gems_cop_w(0x04800909u);          /* y_rot: a half turn */
        gems_cop_w(0x8000u);
        model = gems_ld16(rob + 0x2084);
    } else {
        gems_cop_w(0x04000808u);
        gems_cop_w(gems_ld16(rob + 0x2082));
        model = gems_ld16(rob + 0x2086);
    }
    GEMS_G(1) = 0;
    GEMS_G(0) = model;
    gfn_set_obj();
    if (entry) gems_i960_ret();
    return 0;
}
