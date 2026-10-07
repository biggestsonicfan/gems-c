#pragma once
/* COP op 67 Fn_st_glb_mat */

/* The current matrix into bufferram at the word index given. */
static void gcop_67(void) {
    uint32_t b = gems_in_w();
    const uint32_t *m = gems_dm(0x30000u + *gems_dm(0x3033Fu));
    for (uint32_t k = 0; k < 12; k++) gems_bram_wr(b + k, m[k]);
}
