#pragma once
/* COP op 11 Fn_load_3x3 */

static void gcop_11(void) {
    float *m = gems_dmf(0x30000u + *gems_dm(0x3033Fu));
    for (int i = 0; i < 9; i++) m[i] = gems_in_f();
}
