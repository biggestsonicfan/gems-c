/* cage_time_manager (i960 0x24CDC; Gems 80051A10): the cage's four sides each
 * have a countdown; a side whose countdown has run out restarts it at 0x1F
 * when its request bit in 0x50A1E4 is up. Each request bit is cleared after
 * its side is looked at. */
#pragma once

static uint32_t gfn_cage_time_manager(int entry)
{
    static const struct { uint32_t timer, bit; } side[4] = {
        { 0x50A1EE, 1 }, { 0x50A1EA, 2 }, { 0x50A1E8, 8 }, { 0x50A1EC, 4 },
    };
    for (int i = 0; i < 4; i++) {
        uint32_t t = gems_ld16(side[i].timer);
        if (t == 0) {
            if (gems_ld32(0x50A1E4) & side[i].bit) gems_st16(side[i].timer, 0x1F);
        } else {
            gems_st16(side[i].timer, t - 1);
        }
        gems_st32(0x50A1E4, gems_ld32(0x50A1E4) & ~side[i].bit);
    }
    if (entry) gems_i960_ret();
    return 0;
}
