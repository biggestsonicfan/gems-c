/* dsp_char_2x3 (i960 0x6254, Gems 0x80052E40): put character g0 of the 2x3
 * font (pattern set 13 of the table at 0x6480300) at g9 in the tilemap: three
 * rows of two tile words, each the font's word plus the set's base. The font
 * holds eight characters a row, 16 words per character row. g9 is left two
 * tiles on from where it started. */
#pragma once

static uint32_t gfn_dsp_char_2x3(int entry)
{
    const uint32_t row_bytes = 0x80;   /* one tilemap row */
    uint32_t set = gems_ld32(0x6480334);
    uint32_t base = gems_ld16(set);             /* ldos */
    uint32_t pat = set + 0xC;
    uint32_t ch = gems_ld8(GEMS_G(0) + 0x8E60C);
    uint32_t src = pat + (ch & ~7u) * 4u * 3u + (ch & 7u) * 4u;

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 2; col++) {
            gems_st16(GEMS_G(9), base + gems_ld16(src + 2u * (uint32_t)col));
            GEMS_G(9) += 2;
        }
        src += 0x20;
        GEMS_G(9) += row_bytes - 4;
    }
    GEMS_G(9) = GEMS_G(9) - 3 * row_bytes + 4;
    if (entry) gems_i960_ret();
    return 0;
}
