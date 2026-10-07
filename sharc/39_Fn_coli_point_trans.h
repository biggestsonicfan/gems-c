#pragma once
/* COP op 39 Fn_coli_point_trans */

static void gcop_39(void) {
    float x = gems_in_f();
    float y = gems_in_f();
    float z = gems_in_f();
    uint32_t n = gems_in_w();
    float o[3];
    gch_8001ea5c(x, y, z, *gems_dm(0x3033Fu), &o[0], &o[1], &o[2]);
    uint32_t i = *gems_dm(0x3033Eu) + n;
    for (int k = 0; k < 3; k++, i++) {
        gems_bram_wr(i + 0x60u, gems_bram_rd(i));      /* keep the previous position */
        gems_bram_wrf(i, o[k]);
    }
}
