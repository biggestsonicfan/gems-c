/* rob_coli_ball_table_all_init (i960 0x1028, Gems 0x80053C3C): load a
 * character's collision-ball tables. g5 = the character's table set (from
 * 0xC9B30 by character rob+0x84C, +0xD0 for character 3, then by costume
 * rob+0x1B0). g6 names the destinations: [g6+4] = 30; 32 quads from [g5]
 * to [g6+8]; [g5+0xC] to [[g6+0xC]]; 16 longs from [g5+8] to [g6+0x10];
 * 32 words from [g5+4] to [g6+0x14]; 16 bytes from [[g6+0x1C]] to
 * [g6+0x18]. g0/g1 are left at the last copy's destination and source. */
#pragma once

static uint32_t gfn_rob_coli_ball_table_all_init(int entry)
{
    uint32_t rob = GEMS_G(7);
    uint32_t chr = gems_ld8(rob + 0x84C);
    GEMS_G(5) = gems_ld32(chr * 4 + 0xC9B30);
    if (chr == 3) GEMS_G(5) += 0xD0;
    uint32_t costume = gems_ld8(rob + 0x1B0);
    GEMS_G(5) = gems_ld32(GEMS_G(5) + costume * 4);
    uint32_t tbl = GEMS_G(5), dst = GEMS_G(6);
    uint32_t buf[4];

    gems_st32(gems_ld32(dst + 4), 30);

    GEMS_G(0) = gems_ld32(dst + 8);
    GEMS_G(1) = gems_ld32(tbl);
    for (uint32_t i = 0; i < 32; i++) {
        gems_ldn(buf, GEMS_G(1) + i * 16, 4);
        gems_stn(GEMS_G(0) + i * 16, buf, 4);
    }

    uint32_t w = gems_ld32(tbl + 0xC);
    gems_st32(gems_ld32(dst + 0xC), w);

    GEMS_G(0) = gems_ld32(dst + 0x10);
    GEMS_G(1) = gems_ld32(tbl + 8);
    for (uint32_t i = 0; i < 16; i++) {
        gems_ldn(buf, GEMS_G(1) + i * 8, 2);
        gems_stn(GEMS_G(0) + i * 8, buf, 2);
    }

    GEMS_G(0) = gems_ld32(dst + 0x14);
    GEMS_G(1) = gems_ld32(tbl + 4);
    for (uint32_t i = 0; i < 32; i++)
        gems_st32(GEMS_G(0) + i * 4, gems_ld32(GEMS_G(1) + i * 4));

    GEMS_G(0) = gems_ld32(dst + 0x18);
    GEMS_G(1) = gems_ld32(dst + 0x1C);
    GEMS_G(1) = gems_ld32(GEMS_G(1));
    for (uint32_t i = 0; i < 16; i++)
        gems_st8(GEMS_G(0) + i, gems_ld8(GEMS_G(1) + i));

    if (entry) gems_i960_ret();
    return 0;
}
