/* rob_area_check (i960 0x1AF3C; Gems 80049D68): run area_check on each of the
 * fighter's (g7) 16 world points at rob+0x1F4 (12 bytes each) and keep its
 * byte result at rob+0x6B8 + i. */
#pragma once

static uint32_t gfn_rob_area_check(int entry)
{
    uint32_t rob = GEMS_G(7);
    for (uint32_t i = 0; i < 16u; i++) {
        gems_ldn(&GEMS_G(0), rob + 0x1F4u + i * 12u, 3);
        gfn_area_check(0);
        gems_st8(rob + 0x6B8u + i, GEMS_G(0));
    }
    if (entry) gems_i960_ret();
    return 0;
}
