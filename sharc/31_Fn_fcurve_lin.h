#pragma once
/* COP op 31 Fn_fcurve_lin */

static void gcop_31(void) {
    float v0 = gems_in_f();
    float v1 = gems_in_f();
    float t = gems_in_f();
    float span = gems_in_f();
    gems_out_f(v0 + ((t * (v1 - v0)) / span));
}
