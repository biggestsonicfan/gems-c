#pragma once
/* COP op 1e Fn_sub_c */

static void gcop_1e(void) {
    float v = gems_in_f();
    float *c = gems_dmf(gcs_c);
    *c = *c - v;
}
