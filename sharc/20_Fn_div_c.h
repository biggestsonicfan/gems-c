#pragma once
/* COP op 20 Fn_div_c */

static void gcop_20(void) {
    float v = gems_in_f();
    float *c = gems_dmf(gcs_c);
    *c = *c / v;
}
