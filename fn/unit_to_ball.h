/* unit_to_ball (i960 0x2D328, Gems 0x80043CC4): turn a mask of units (g0)
 * into the mask of their collision balls. Each unit from the highest set bit
 * down to bit 1 has an 8-byte list at g1 + unit*8: ball numbers << 2, the
 * first always taken, then up to a zero. The result is left in g0. */
#pragma once

static uint32_t gfn_unit_to_ball(int entry)
{
    if (GEMS_G(0) != 0) {
        uint32_t units = GEMS_G(0);
        GEMS_G(0) = 0;
        for (;;) {
            int unit = -1;   /* scanbit */
            for (int b = 31; b >= 0; b--)
                if (units & (1u << b)) { unit = b; break; }
            if (unit < 1) break;
            uint32_t list = GEMS_G(1) + (uint32_t)unit * 8;
            for (uint32_t i = 0;; i++) {
                uint32_t ball = gems_ld8(list + i) >> 2;
                if (i != 0 && ball == 0) break;
                GEMS_G(0) |= ball < 32 ? 1u << ball : 0;   /* PowerPC slw */
            }
            units &= ~(1u << unit);
        }
    }
    if (entry) gems_i960_ret();
    return 0;
}
