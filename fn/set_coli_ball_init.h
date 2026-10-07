/* set_coli_ball_init (i960 0x114C, Gems 0x80053AB4): copy word 3 of each of
 * the 32 collision balls (16 bytes each) of both fighters, 0x506000 to
 * 0x50A204 and 0x506200 to 0x50A28C. */
#pragma once

static uint32_t gfn_set_coli_ball_init(int entry)
{
    for (uint32_t i = 0; i < 32; i++)
        gems_st32(0x50A204 + i * 4, gems_ld32(0x506000 + i * 16 + 0xC));
    for (uint32_t i = 0; i < 32; i++)
        gems_st32(0x50A28C + i * 4, gems_ld32(0x506200 + i * 16 + 0xC));
    if (entry) gems_i960_ret();
    return 0;
}
