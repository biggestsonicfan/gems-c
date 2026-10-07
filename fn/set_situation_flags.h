/* set_situation_flags (i960 0x6878C; Gems 8004E1E0): bit 1 of the sway
 * record's (g13) flags. Set when the word at g13+0xF8 is negative, unless the
 * fighter's (g7) state byte rob+0x1B1 is 3 and flag 0x200 is up; else clear. */
#pragma once

static uint32_t gfn_set_situation_flags(int entry)
{
    uint32_t rec = GEMS_G(13);
    bool set = false;
    if (gems_ld8(GEMS_G(7) + 0x1B1u) != 3u || (gems_ld32(rec) & 0x200u) == 0)
        set = (int32_t)gems_ld32(rec + 0xF8u) < 0;
    uint32_t flags = gems_ld32(rec);
    gems_st32(rec, set ? flags | 2u : flags & ~2u);
    if (entry) gems_i960_ret();
    return 0;
}
