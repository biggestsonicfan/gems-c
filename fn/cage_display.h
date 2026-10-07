/* cage_display (i960 0x2468C; Gems 80050CD0): the cage's four walls. Unless
 * bit 15 of the word at g13 is set: push the matrix, move to
 * (h / 6, y / 3.1, h / 6) (h the arena half-width at 0x50A00C, y the float at
 * 0x50027C), then for each wall g1 = 0..3 draw it with cage_clip_m (g4 = the
 * halfword at 0x50A1E8 + 2 * g1) when bit g1 of the byte at 0x500288 is set,
 * and turn a quarter (ang_y 0x4000); pop the matrix. Leaves g1 = 4. */
#pragma once

static uint32_t gfn_cage_display(int entry)
{
    uint32_t walls = gems_ld8(0x500288u);
    if ((gems_ld32(GEMS_G(13)) & 0x8000u) == 0) {
        gems_cop_w(0x00800101u);                            /* push matrix */
        float h = gems_ldf(0x50A00Cu) / 6.0f;
        float y = gems_ldf(0x50027Cu) / gems_u2f(0x40466666u);  /* 3.1f */
        gems_cop_w(0x03800707u);                            /* translate */
        gems_cop_wf(h);
        gems_cop_wf(y);
        gems_cop_wf(h);
        GEMS_G(1) = 0;
        for (;;) {
            uint32_t wall = GEMS_G(1);
            if (walls & (1u << wall)) {
                GEMS_G(4) = gems_ld16(0x50A1E8u + wall * 2u);
                gfn_cage_clip_m(0);
                GEMS_G(1) = wall;
            }
            gems_cop_w(0x04800909u);                        /* ang_y */
            gems_cop_w(0x4000u);
            wall = GEMS_G(1);
            GEMS_G(1) = wall + 1u;
            if (wall >= 3u) break;
        }
        gems_cop_w(0x01000202u);                            /* pop matrix */
    }
    if (entry) gems_i960_ret();
    return 0;
}
