/* dsp_pattern_new (i960 0x5DF4, Gems 80052964): copy pattern g0 whole into
 * tile RAM at g9, rows 0x80 bytes apart, adding the base tile word; g9 ends
 * past the first row. Pattern header (table 0x6480300): base (ldos), rows,
 * columns. */
static uint32_t gfn_dsp_pattern_new(int entry)
{
    if (GEMS_G(0) < 0x3FF) {
        uint32_t pat  = gems_ld32(GEMS_G(0) * 4 + 0x6480300);
        uint32_t base = gems_ld16(pat);                       /* ldos */
        uint32_t rc[2];                                       /* rows, columns */
        gems_ldn(rc, pat + 4, 2);
        uint32_t rows = rc[0], cols = rc[1];
        uint32_t src = pat + 12, dst = GEMS_G(9);
        do {
            uint32_t d = dst, n = cols;
            do {
                int32_t w = gems_ld16s(src);                    /* ldis */
                src += 2;
                gems_st16(d, base + (uint32_t)w);
                d += 2;
            } while (n-- > 1);
            dst += 0x80;
        } while (rows-- > 1);
        GEMS_G(9) += cols * 2;
    }
    if (entry) gems_i960_ret();
    return 0;
}
