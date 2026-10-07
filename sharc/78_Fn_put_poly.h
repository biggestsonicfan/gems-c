#pragma once
/* COP op 78 Fn_put_poly */

/* Writes a GEO list fragment at the byte offset in arg 0: a matrix command
 * (0x05800B0B + the current matrix), then a one-polygon command with args 2..5.
 * m2-hle2 keeps its own 0x78; this is Gems' version for reference. */
static void gcop_78(void) {
    uint32_t a = gems_in_w() >> 2;
    (void)gems_in_w();
    uint32_t w3 = gems_in_w();
    uint32_t w4 = gems_in_w();
    uint32_t w5 = gems_in_w();
    uint32_t w6 = gems_in_w();
    const uint32_t *m = gems_dm(0x30000u + *gems_dm(0x3033Fu));
    gems_bram_wr(a, 0x05800B0Bu);
    for (uint32_t k = 0; k < 12; k++) gems_bram_wr(a + 1u + k, m[k]);
    uint32_t w7 = gems_in_w();
    uint32_t w8 = gems_in_w();
    uint32_t b = a + 13u;
    gems_bram_wr(b, 0x00800101u);
    gems_bram_wr(b + 1u, w3);
    gems_bram_wr(b + 2u, w4);
    gems_out_w(w7);
    gems_out_w(w8);
    gems_bram_wr(b + 3u, w5);
    gems_bram_wr(b + 4u, w6);
}
