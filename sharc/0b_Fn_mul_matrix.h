#pragma once
/* COP op 0b Fn_mul_matrix */

static void gcop_0b(void) {
    float m[12];
    for (int i = 0; i < 12; i++) m[i] = gems_in_f();
    gch_8001d688(m, gems_dmf(0x30000u + *gems_dm(0x3033Fu)));
}
