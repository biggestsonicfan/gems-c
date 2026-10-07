#pragma once
/* COP op 3d Fn_coli_trans_mat */

static void gcop_3d(void) {
    float a[6];
    for (int i = 0; i < 6; i++) a[i] = gems_in_f();
    for (uint32_t k = 0, w = 0x30429u; k < 16; k++, w += 12u) {
        *gems_dmf(w)          = *gems_dmf(w)          + a[0];
        *gems_dmf(w + 0xC0u)  = *gems_dmf(w + 0xC0u)  + a[3];
        *gems_dmf(w + 1u)     = *gems_dmf(w + 1u)     + a[1];
        *gems_dmf(w + 0xC1u)  = *gems_dmf(w + 0xC1u)  + a[4];
        *gems_dmf(w + 2u)     = *gems_dmf(w + 2u)     + a[2];
        *gems_dmf(w + 0xC2u)  = *gems_dmf(w + 0xC2u)  + a[5];
    }
}
