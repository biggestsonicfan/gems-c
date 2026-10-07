/* osage_copro (i960 0x687C4; Gems 8004E33C): run Fn_osage (COP 0x4A) over the
 * sway-chain stream at g13+0x4C, then walk the records it answers: g9 (the
 * record stream) and g8 step by the record type's sizes from the table at
 * 0x67520; a segment record (type 5) takes its 12 matrix words
 * (os_set_osage_after). Type 0 ends the stream. Each record's echo word is
 * read and dropped, as the i960 does. */
#pragma once

static uint32_t gfn_osage_copro(int entry)
{
    gems_cop_w(0x25004A4Au);
    gems_cop_w(gems_ld32(GEMS_G(13) + 0x4C));
    GEMS_G(9) = gems_ld32(GEMS_G(13) + 0x44);
    GEMS_G(8) = gems_ld32(GEMS_G(13) + 0x48);
    for (;;) {
        (void)gems_cop_r();
        uint32_t type = gems_ld32(GEMS_G(9));
        GEMS_G(9) += 4;
        GEMS_G(8) += 4;
        if (type == 0) break;
        if (type == 5) gfn_os_set_osage_after(0);
        uint32_t step[2];
        gems_ldn(step, type * 8u + 0x67520u, 2);
        GEMS_G(9) += step[0];
        GEMS_G(8) += step[1];
    }
    if (entry) gems_i960_ret();
    return 0;
}
