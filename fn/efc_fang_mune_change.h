/* efc_fang_mune_change (i960 0x333B0; Gems 80048458): Fang's chest model
 * during motions 0x169 and 0x16A (halfword rob+0x1A8, rob = g7). The model
 * number for frame rob+0x1AA comes from a halfword table picked by the motion
 * and by whether rob+0x1B0 is 0x1E, and goes to rob+0x44 sign-extended. */
#pragma once

static uint32_t gfn_efc_fang_mune_change(int entry)
{
    uint32_t rob = GEMS_G(7);
    uint32_t motion = gems_ld16(rob + 0x1A8u);
    uint32_t table = 0;
    if (motion == 0x169u)
        table = gems_ld8(rob + 0x1B0u) == 0x1Eu ? 0x33548u : 0x33488u;
    else if (motion == 0x16Au)
        table = gems_ld8(rob + 0x1B0u) == 0x1Eu ? 0x335B8u : 0x334F8u;
    if (table) {
        uint32_t frame = gems_ld16(rob + 0x1AAu);
        gems_st32(rob + 0x44u, (uint32_t)gems_ld16s(table + (frame - 1u) * 2u));
    }
    if (entry) gems_i960_ret();
    return 0;
}
