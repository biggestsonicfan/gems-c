/* get_start_value (i960 0x30A8C; Gems 8003FEB8): fill the 60 start values of a
 * motion at g5 from its key stream. rob+0xBD8 is the motion; +0x78C its 60
 * channel kinds (g4), +0x784 the key counts (g3), +0x780 the key data (g2).
 * Then the first 36 values are converted with cvtri (rounded by the AC mode;
 * out of range or NaN gives 0x80000000), stored as halfwords (stis) over
 * themselves. Leaves g1 = 0, g2..g4 advanced, g5 back at its start. */
#pragma once

static uint32_t gfn_get_start_value(int entry)
{
    uint32_t mot = gems_ld32(GEMS_G(7) + 0xBD8);
    GEMS_G(4) = mot + 0x78C;
    GEMS_G(3) = gems_ld32(mot + 0x784);
    GEMS_G(2) = gems_ld32(mot + 0x780);
    GEMS_G(1) = 60;
    for (;;) {
        uint32_t kind = gems_ld8(GEMS_G(4));
        if (kind == 4) {                       /* constant: one word */
            gems_st32(GEMS_G(5), gems_ld32(GEMS_G(2)));
            GEMS_G(2) += 4;
        } else if (kind > 4) {                 /* keyed: first key's value */
            uint32_t keys = gems_ld8(GEMS_G(3));
            GEMS_G(3) += 1;
            gems_st32(GEMS_G(5), gems_ld32(GEMS_G(2)));
            GEMS_G(2) += kind == 5 ? keys * 4 : keys * 12;
        } else if (kind == 3) {                /* zero */
            gems_st32(GEMS_G(5), 0);
        } else {                               /* skipped word */
            GEMS_G(2) += 4;
        }
        GEMS_G(4) += 1;
        GEMS_G(5) += 4;
        uint32_t n = GEMS_G(1)--;
        if (n <= 1) break;
    }
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
