#pragma once
/* COP op 72 Fn_outside_ball */

/* Counts the fighter's balls outside the arena's half width (DM 0x30802),
 * after moving them by the push-out (dx, dz) the i960 sends.  From the PS2
 * build (handler 0x160940), which matches the board (PM 0x20D6F): the GC one
 * (0x80020920) reads dx and dz but drops them.  The compares are written so a
 * NaN counts as outside, as the board's comp/if gt/if lt does. */
static void gcop_72(void) {
    uint32_t id = gems_in_w();
    float dx = gems_in_f();
    float dz = gems_in_f();
    uint32_t b = id ? 0x7E80u : 16000u;
    float w = *gems_dmf(0x30802u);
    uint32_t n = 0;
    for (uint32_t k = 0; k < 32; k++, b += 3u) {
        float x = gems_bram_rdf(b) + dx;
        float z = gems_bram_rdf(b + 2u) + dz;
        if (!(fabsf(x) <= w) || !(fabsf(z) < w)) n++;
    }
    gems_out_w(n);
}
