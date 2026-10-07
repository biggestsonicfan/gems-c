#pragma once
/* COP op 0e Fn_load_point */

static void gcop_0e(void) {
    float *m = gems_dmf(0x30000u + *gems_dm(0x3033Fu));
    for (int i = 9; i < 12; i++) m[i] = gems_in_f();
}
