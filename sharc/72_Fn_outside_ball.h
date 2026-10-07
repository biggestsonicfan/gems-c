#pragma once
/* COP op 72 Fn_outside_ball */

/* Counts the fighter's balls outside the arena's half width (DM 0x30802). */
static void gcop_72(void) {
    uint32_t id = gems_in_w();
    (void)gems_in_f();
    (void)gems_in_f();
    uint32_t b = id ? 0x7E80u : 16000u;
    float w = *gems_dmf(0x30802u);
    uint32_t n = 0;
    for (uint32_t k = 0; k < 32; k++, b += 3u) {
        float x = gems_bram_rdf(b);
        float z = gems_bram_rdf(b + 2u);
        if (fabsf(x) > w || fabsf(z) >= w) n++;
    }
    gems_out_w(n);
}
