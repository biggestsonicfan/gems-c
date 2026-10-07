#pragma once
/* COP op 7f Fn_coli_copy_unit_matrix */

/* Keeps a fighter's 16 unit matrices (192 words) as last frame's, for the afterimages. */
static void gcop_7f(void) {
    uint32_t p = gems_in_w();
    const uint32_t *src = gems_dm(p == 1u ? 0x304E0u : 0x30420u);
    uint32_t *dst       = gems_dm(p == 1u ? 0x320C0u : 0x32000u);
    for (uint32_t k = 0; k < 192; k++) dst[k] = src[k];
}
