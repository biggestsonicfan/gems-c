/* dsp_mes_2x3 (i960 0x619C; Gems 80052B54): write the string at g0 into the
 * tilemap at g9 in the 2x3-tile font. The font is font table entry 13
 * (0x6480334): a halfword tile base, then from +0xC the glyphs, eight to a
 * 0x60-byte row of 3 lines x 32 bytes. Each character maps to a glyph through
 * the byte table at 0x8E60C; its 2x3 tile numbers + base go to the map
 * (lines 0x80 bytes apart), and g9 moves on 4 bytes. Leaves g0 on the NUL. */
#pragma once

static uint32_t gfn_dsp_mes_2x3(int entry)
{
    const uint32_t pitch = 0x80u;
    uint32_t font = gems_ld32(0x6480300u + 13u * 4u);
    uint32_t base = gems_ld16(font);
    (void)gems_ld16(font + 2u);
    uint32_t glyphs = font + 0xCu;
    for (;;) {
        uint32_t c = gems_ld8(GEMS_G(0));
        if (c == 0) break;
        GEMS_G(0) += 1u;
        uint32_t code = gems_ld8(0x8E60Cu + c);
        uint32_t p = glyphs + (code & ~7u) * 12u + (code & 7u) * 4u;
        for (int line = 0; line < 3; line++) {
            for (int col = 0; col < 2; col++) {
                gems_st16(GEMS_G(9), base + gems_ld16(p));
                p += 2u;
                GEMS_G(9) += 2u;
            }
            p += 28u;
            GEMS_G(9) += pitch;
            GEMS_G(9) -= 4u;
        }
        GEMS_G(9) -= pitch;
        GEMS_G(9) -= pitch;
        GEMS_G(9) -= pitch;
        GEMS_G(9) += 4u;
    }
    if (entry) gems_i960_ret();
    return 0;
}
