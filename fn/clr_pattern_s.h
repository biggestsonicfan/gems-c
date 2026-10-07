/* clr_pattern_s (i960 0x5D98, Gems 0x80053394): fill g1 rows of g0 tile
 * words from g9 with 0x20, through cps_1. */
#pragma once

static uint32_t gfn_clr_pattern_s(int entry)
{
    return gfn_cps_1(entry, 0x20);
}
