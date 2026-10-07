/* efc_fang_mune_init (i960 0x33438, Gems 0x8004828C): Fang's chest part.
 * With rob+0x1B1 == 4 and rob+0x84C != 3, rob+0x44 gets an object number
 * from the table at 0x33608 (0x3360C when rob+0x1B0 == 30), entry 1 when
 * bit 29 of rob+0 is set, else entry 0. */
#pragma once

static uint32_t gfn_efc_fang_mune_init(int entry)
{
    uint32_t rob = GEMS_G(7);
    if (gems_ld8(rob + 0x1B1) == 4 && gems_ld8(rob + 0x84C) != 3) {
        uint32_t table = gems_ld8(rob + 0x1B0) == 30 ? 0x3360C : 0x33608;
        uint32_t idx = (gems_ld32(rob) & 0x20000000) ? 1 : 0;
        gems_st32(rob + 0x44, (uint32_t)gems_ld16s(table + idx * 2));   /* ldis */
    }
    if (entry) gems_i960_ret();
    return 0;
}
