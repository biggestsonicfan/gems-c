/* efc_fang_gun_disp (i960 0x336DC, Gems 0x800481D0): Fang's gun. Translate
 * (COP 0x06, Fn_trans) by the position its effect record holds at +4, then
 * draw object rob+0x2A90 with set_obj (g0 = object, g1 = 0). */
#pragma once

static uint32_t gfn_efc_fang_gun_disp(int entry)
{
    uint32_t rob = GEMS_G(7);
    uint32_t efc = gems_ld32(rob + 0x2A8C);
    uint32_t pos[3];
    gems_ldn(pos, efc + 4, 3);
    gems_cop_w(0x03000606);
    gems_cop_w(pos[0]);
    gems_cop_w(pos[1]);
    gems_cop_w(pos[2]);
    uint32_t obj = gems_ld16(rob + 0x2A90);   /* ldos */
    GEMS_G(1) = 0;
    GEMS_G(0) = obj;
    gfn_set_obj();
    if (entry) gems_i960_ret();
    return 0;
}
