/* scale_parts_cnt (i960 0x7F648; Gems 80038E7C): part g3's scale animation.
 * Each 0x1C-byte record at rob+0x20F0 (g7) is { count, scale[3], step[3] }.
 * Returns the scale in g0-g2 (0 when the count has run out); while the count
 * is positive and the game is not paused (0x508000 bits 2 and 5 both set),
 * it counts down and the scale moves on by one step. */
#pragma once

static uint32_t gfn_scale_parts_cnt(int entry)
{
    uint32_t rec = GEMS_G(7) + 0x20F0u + GEMS_G(3) * 0x1Cu;
    int32_t count = (int32_t)gems_ld32(rec);
    GEMS_G(0) = 0;
    GEMS_G(1) = 0;
    GEMS_G(2) = 0;
    if (count > 0) {
        gems_ldn(&GEMS_G(0), rec + 4u, 3);
        uint32_t flags = gems_ld32(0x508000u);
        if ((flags & 4u) == 0 || (flags & 0x20u) == 0) {
            uint32_t step[3];
            gems_st32(rec, (uint32_t)(count - 1));
            gems_ldn(step, rec + 0x10u, 3);
            for (int k = 0; k < 3; k++)
                GEMS_G(k) = gems_f2u(gems_u2f(step[k]) + gems_u2f(GEMS_G(k)));
            gems_stn(rec + 4u, &GEMS_G(0), 3);
        }
    }
    if (entry) gems_i960_ret();
    return 0;
}
