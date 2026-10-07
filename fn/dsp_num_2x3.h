/* dsp_num_2x3 (i960 0x62F8; Gems 800530D0): draw digit g0 as a 2x3 block of
 * tiles at g9 (tile RAM, 0x80 bytes a row), from font 13 of the table at
 * 0x6480334: a header of the base pattern (+0), a flag (+2), then the glyphs,
 * 8 to a 0x60-byte row, each 3 rows of 2 halfwords 0x20 apart. g9 ends one
 * glyph (4 bytes) to the right. */
#pragma once

static uint32_t gfn_dsp_num_2x3(int entry)
{
    const uint32_t row_bytes = 0x80;
    uint32_t font = gems_ld32(0x6480300u + 13u * 4u);
    uint32_t base = gems_ld16(font);
    (void)gems_ld16(font + 2);            /* read, as the i960 does; unused */
    font += 0xC;
    uint32_t ch = GEMS_G(0) + 0x28;
    uint32_t src = font + ((ch * 4u) & ~0x1Fu) * 3u + (ch - (ch & ~7u)) * 4u;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 2; col++) {
            gems_st16(GEMS_G(9), base + gems_ld16(src + 2u * (uint32_t)col));
            GEMS_G(9) += 2;
        }
        src += 0x20;
        GEMS_G(9) = GEMS_G(9) + row_bytes - 4;
    }
    GEMS_G(9) = GEMS_G(9) - 3 * row_bytes + 4;
    if (entry) gems_i960_ret();
    return 0;
}
