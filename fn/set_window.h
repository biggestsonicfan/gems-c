/* set_window (i960 0x5564, Gems 0x8003819C): GEO command 0x303 (window) at
 * g10+0x30, then six window words: for each (x, y) halfword pair at g0,
 * (y << 16) + (0x17F - x) + [0x5013F0], stored at g10 + g12. g0 is left
 * past the six pairs. g14 is put back as it was. */
#pragma once

static uint32_t gfn_set_window(int entry)
{
    gems_st32(0x50EFFC, GEMS_G(14));
    GEMS_G(14) = 0x303;
    gems_st32(GEMS_G(10) + 0x30, GEMS_G(14));
    GEMS_G(14) = gems_ld32(0x50EFFC);

    uint32_t base = gems_ld32(0x5013F0);
    for (int i = 0; i < 6; i++) {
        int32_t x = gems_ld16s(GEMS_G(0));       /* ldis */
        int32_t y = gems_ld16s(GEMS_G(0) + 2);   /* ldis */
        uint32_t w = base + (uint32_t)(0x17F - x) + (uint32_t)y * 0x10000u;
        gems_st32(GEMS_G(10) + GEMS_G(12), w);
        GEMS_G(0) += 4;
    }
    if (entry) gems_i960_ret();
    return 0;
}
