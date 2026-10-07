/* fill_pattern_s (i960 0x5DA0; Gems 80053358): fill a tile rectangle with the
 * pattern in g2 (cps_1 does the work). */
#pragma once

static uint32_t gfn_fill_pattern_s(int entry)
{
    return gfn_cps_1(entry, (int16_t)GEMS_G(2));
}
