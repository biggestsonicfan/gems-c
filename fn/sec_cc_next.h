/* sec_cc_next (i960 0x3C284, Gems 0x80051E3C): the CPU command interpreter's
 * dispatch. r3 = the command byte at g6; r4 = its handler from the table at
 * 0x3E230, and the i960 carries on at the jump (site + 0x44). A command past
 * the table goes to 0x3E1E8 (Gems' branch takes a doubled IP: 0x7C3D0).
 *
 * Gems drops the debug history (the command pointer at 0x50F608, the last 128
 * command bytes at 0x50F610 indexed by 0x50F604) and the condition code; the
 * i960 keeps both, and so does this. */
#pragma once

static uint32_t gfn_sec_cc_next(int entry)
{
    (void)entry;
    gems_st32(0x50F608, GEMS_G(6));
    GEMS_R(3) = gems_ld8(GEMS_G(6));
    GEMS_R(13) = 0x93;
    GEMS_AC = (GEMS_AC & ~7u) | (GEMS_R(13) < GEMS_R(3) ? 4u : GEMS_R(13) == GEMS_R(3) ? 2u : 1u);
    if (GEMS_R(3) >= 0x93) {
        gems_branch(0x3E1E8);
        return 0;
    }
    GEMS_R(5) = gems_ld32(0x50F604);
    gems_st8(0x50F610 + GEMS_R(5), GEMS_R(3));
    GEMS_R(13) = 0x7F;
    GEMS_R(5) = (GEMS_R(5) + 1u) & GEMS_R(13);
    gems_st32(0x50F604, GEMS_R(5));
    GEMS_R(4) = gems_ld32(GEMS_R(3) * 4u + 0x3E230u);
    return 0x88;
}
