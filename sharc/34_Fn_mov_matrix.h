#pragma once
/* COP op 34 Fn_mov_matrix */

static void gcop_34(void) {
    uint32_t idx = gems_in_w() >> 2;                    /* byte offset -> word */
    const float *m = gems_dmf(0x30000u + *gems_dm(0x3033Fu));
    for (int i = 0; i < 12; i++) gems_bram_wrf(idx++, m[i]);
    gems_out_w(0);
}
