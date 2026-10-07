/* dsp_character (i960 0x5F94, Gems 80037F60): copy a g3-wide window of
 * pattern g0 (starting at column offset g4 * g3) into tile RAM at g9, rows
 * 0x80 bytes apart, adding the pattern's base tile word to every entry.
 * Pattern header (table 0x6480300): base (ldis), rows, row stride (words).
 * For g0 >= 0x3FF the i960 branches straight to a ret (Gems returned 0
 * without i960_ret(); here it returns as the i960 does). */
static uint32_t gfn_dsp_character(int entry)
{
    if (GEMS_G(0) >= 0x3FF) {
        if (entry) gems_i960_ret();
        return 0;
    }
    uint32_t pat    = gems_ld32(GEMS_G(0) * 4 + 0x6480300);
    int32_t  base   = gems_ld16s(pat);                       /* ldis */
    uint32_t rows   = gems_ld32(pat + 4);
    int32_t  stride = (int32_t)gems_ld32(pat + 8);
    uint32_t src    = GEMS_G(4) * GEMS_G(3) * 2 + pat + 12;
    uint32_t dst    = GEMS_G(9);
    do {
        uint32_t s = src, d = dst, n = GEMS_G(3);
        do {
            uint32_t w = gems_ld16(s);                         /* ldos */
            s += 2;
            gems_st16(d, (uint32_t)base + w);
            d += 2;
        } while (n-- > 1);
        src += (uint32_t)(stride * 2);
        dst += 0x80;
    } while (rows-- > 1);
    if (entry) gems_i960_ret();
    return 0;
}
