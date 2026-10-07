#pragma once
/* COP op 1f Fn_mul_c */

static void gcop_1f(void) {
    float v = gems_in_f();
    float *c = gems_dmf(gcs_c);
    *c = *c * v;
}
