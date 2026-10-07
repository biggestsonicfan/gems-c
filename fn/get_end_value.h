/* get_end_value (i960 0x30B54, Gems 0x8003FB20): unpack a motion's end key
 * into 60 words at g5. The motion record is [rob+0xBD8]: kinds from +0x78C
 * (g4), key indexes from [+0x784] (g3), values from [+0x780] (g2). Per
 * channel: kind 4 takes the next value; 5 skips that many values and takes
 * the last; above 5 the same in 3-word keys; 3 is zero; below 3 skips one.
 * Then the 36 angle words are converted with cvtri (rounded by the AC mode;
 * out of range or NaN gives 0x80000000) and stored as halfwords (stis) over
 * their own low halves, and
 * a mirrored fighter (bit 6 of rob+0) goes through set_mirror. g1 ends 0,
 * g2-g5 where the walk left them (g5 back at the start). */
#pragma once

static uint32_t gfn_get_end_value(int entry)
{
    uint32_t mot = gems_ld32(GEMS_G(7) + 0xBD8);
    GEMS_G(4) = mot + 0x78C;
    GEMS_G(3) = gems_ld32(mot + 0x784);
    GEMS_G(2) = gems_ld32(mot + 0x780);
    GEMS_G(1) = 60;
    do {
        uint32_t kind = gems_ld8(GEMS_G(4));
        if (kind == 4) {
            gems_st32(GEMS_G(5), gems_ld32(GEMS_G(2)));
            GEMS_G(2) += 4;
        } else if (kind > 4) {
            uint32_t keys = gems_ld8(GEMS_G(3));
            GEMS_G(3) += 1;
            if (kind == 5) {
                GEMS_G(2) += keys * 4;
                gems_st32(GEMS_G(5), gems_ld32(GEMS_G(2) - 4));
            } else {
                GEMS_G(2) += keys * 12;
                gems_st32(GEMS_G(5), gems_ld32(GEMS_G(2) - 12));
            }
        } else if (kind == 3) {
            gems_st32(GEMS_G(5), 0);
        } else {
            GEMS_G(2) += 4;
        }
        GEMS_G(4) += 1;
        GEMS_G(5) += 4;
    } while (GEMS_G(1)-- > 1);

    GEMS_G(5) -= 0xF0;
    for (int i = 0; i < 36; i++) {
        gems_st16(GEMS_G(5), gems_cvtri(gems_ld32(GEMS_G(5))));
        GEMS_G(5) += 4;
    }
    GEMS_G(5) -= 0x90;
    if (gems_ld32(GEMS_G(7)) & 0x40)
        gfn_set_mirror(0);
    if (entry) gems_i960_ret();
    return 0;
}
