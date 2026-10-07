/* dented_cnt (i960 0x7F414, Gems 0x800396B8): a dented part's spring.
 * g0-g2 = the part's current (g3+0x24); unless the game flags at 0x508000
 * have both bits 2 and 5, or rob+0x219C is positive: the rest value at
 * g3+0x30 creeps down by 0.0025 while positive or NaN (all three get it), then
 * pendulum_3axis_cnt runs with g4-g6 = g3+0x3C and its g0-g2 go back to
 * g3+0x24. */
#pragma once

static uint32_t gfn_dented_cnt(int entry)
{
    uint32_t part = GEMS_G(3);
    gems_ldn(&GEMS_G(0), part + 0x24, 3);
    uint32_t gflags = gems_ld32(0x508000);
    if (!(gflags & 4) || !(gflags & 0x20)) {
        int32_t hold = (int32_t)gems_ld32(GEMS_G(7) + 0x219C);
        if (hold <= 0) {
            gems_ldn(&GEMS_G(0), part + 0x30, 3);
            float v = gems_u2f(GEMS_G(0));
            if (!(0.0f >= v)) {                     /* cmpr + bge: NaN goes on */
                GEMS_G(0) = gems_f2u(v - 0.0025f);   /* subr */
                GEMS_G(1) = GEMS_G(0);
                GEMS_G(2) = GEMS_G(0);
                gems_stn(part + 0x30, &GEMS_G(0), 3);
            }
            gems_ldn(&GEMS_G(4), part + 0x3C, 3);
            gfn_pendulum_3axis_cnt(0);
            gems_stn(part + 0x24, &GEMS_G(0), 3);
        }
    }
    if (entry) gems_i960_ret();
    return 0;
}
