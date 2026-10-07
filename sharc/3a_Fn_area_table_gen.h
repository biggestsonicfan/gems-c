#pragma once
/* COP op 3a Fn_area_table_gen */

static void gcop_3a(void) {
    uint32_t mask_a = gems_in_w();
    uint32_t mask_b = gems_in_w();
    uint32_t kind_a = gems_in_w();
    uint32_t kind_b = gems_in_w();
    *gems_dm(0x3041Au) = kind_a;                        /* state+4200 */
    *gems_dm(0x3041Bu) = kind_b;                        /* state+4204 */
    for (uint32_t i = 0; i < 32; i++) gems_bram_wr(0x3E20u + i, 0xFFFFFFFFu);

    uint32_t *lo_tab = gems_dm(0x30340u);               /* state+0xD00, 64 words */
    uint32_t *hi_tab = gems_dm(0x30380u);               /* state+0xE00, 64 words */
    for (uint32_t axis = 0; axis < 3; axis++) {
        for (int j = 0; j < 128; j++) lo_tab[j] = 0;    /* memset 512 bytes */

        uint32_t bit = 1;
        for (uint32_t i = 0; i < 32; i++, bit <<= 1) {
            float lo = gems_bram_rdf(0x3F40u + axis + 3u * i);
            float hi = gems_bram_rdf(0x3F40u + axis + 3u * i + 96u);
            if (!(hi >= lo)) { float t = hi; hi = lo; lo = t; }
            float r = *gems_dmf(0x30600u + i);
            if (r == 0.0f) continue;
            uint32_t take = mask_a & 1u;
            mask_a >>= 1;
            if (take && kind_a != 4u) r = r * *gems_dmf(0x306F0u);   /* state+7104 */
            lo = lo - r;
            hi = hi + r;
            /* Signed, so a cell below 0 wraps to the top (-3 -> 61).  From the
             * PS2 build (0x15ff40: cvt.w.s, andi 0x3f), which matches the
             * board (r2=fix f8; r2 and 0x3f); the GC one makes it 0. */
            lo_tab[(uint32_t)gch_fctiwz((8.0f * lo + 32.0f) - 0.5f) & 63u] |= bit;
            hi_tab[(uint32_t)gch_fctiwz((8.0f * hi + 32.0f) - 0.5f) & 63u] |= bit;
        }
        for (int j = 1; j < 63; j++) lo_tab[j] |= lo_tab[j - 1];
        for (int j = 1; j < 63; j++) hi_tab[63 - j] |= hi_tab[64 - j];

        for (uint32_t i = 0; i < 32; i++) {
            float lo = gems_bram_rdf(0x7F40u + axis + 3u * i);
            float hi = gems_bram_rdf(0x7F40u + axis + 3u * i + 96u);
            if (!(hi >= lo)) { float t = hi; hi = lo; lo = t; }
            float r = *gems_dmf(0x30700u + i);
            if (r == 0.0f) { gems_bram_wr(0x3E20u + i, 0); continue; }
            uint32_t take = mask_b & 1u;
            mask_b >>= 1;
            if (take && kind_b != 4u) r = r * *gems_dmf(0x307F0u);   /* state+8128 */
            lo = lo - r;
            hi = hi + r;
            float v = 8.0f * lo + 32.0f;
            if (v < 0.0f || v >= 64.0f) { gems_bram_wr(0x3E20u + i, 0); continue; }
            uint32_t from_hi = hi_tab[gch_800a2544(v - 0.5f) & 63u];
            float w = 8.0f * hi + 32.0f;
            if (w < 0.0f || w >= 64.0f) { gems_bram_wr(0x3E20u + i, 0); continue; }
            uint32_t both = lo_tab[gch_800a2544(w - 0.5f) & 63u] & from_hi;
            gems_bram_wr(0x3E20u + i, both & gems_bram_rd(0x3E20u + i));
        }
    }
    for (uint32_t i = 0; i < 16; i++) gems_bram_wr(0x3D80u + i, 0);
}
