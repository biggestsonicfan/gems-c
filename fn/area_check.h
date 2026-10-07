/* area_check (i960 0x288A8; Gems 80049BCC): which edges of the square arena
 * the point (g0, g2) is outside of. The half-width h is the float at
 * 0x50A00C; the coordinates are compared as words, signed against h and
 * unsigned against -h (h with the sign bit set), as the i960 does.
 * Returns g0: 8 x > h, 0x10 x < -h, 2 z > h, 4 z < -h. */
#pragma once

static uint32_t gfn_area_check(int entry)
{
    uint32_t h = gems_ld32(0x50A00Cu);
    uint32_t neg_h = h | 0x80000000u;
    uint32_t x = GEMS_G(0), z = GEMS_G(2);
    uint32_t out = 0;
    if ((int32_t)x > (int32_t)h) out = 8;
    if (x > neg_h) out |= 0x10u;
    if ((int32_t)z > (int32_t)h) out |= 2u;
    if (z > neg_h) out |= 4u;
    GEMS_G(0) = out;
    if (entry) gems_i960_ret();
    return 0;
}
