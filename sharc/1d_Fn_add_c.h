#pragma once
/* COP op 1d Fn_add_c */

static void gcop_1d(void) {
    float v = gems_in_f();
    float *c = gems_dmf(gcs_c);
    *c = *c + v;
}
