#pragma once
/* COP op 68 Fn_ld_glb_mat */

/* The current matrix from bufferram at the word index given. */
static void gcop_68(void) {
    uint32_t b = gems_in_w();
    uint32_t *m = gems_dm(0x30000u + *gems_dm(0x3033Fu));
    for (uint32_t k = 0; k < 12; k++) m[k] = gems_bram_rd(b + k);
}
