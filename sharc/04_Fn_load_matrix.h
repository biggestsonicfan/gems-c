#pragma once
/* COP op 04 Fn_load_matrix */

static void gcop_04(void) {
    float *m = gems_dmf(0x30000u + *gems_dm(0x3033Fu));
    for (int i = 0; i < 12; i++) m[i] = gems_in_f();
}
