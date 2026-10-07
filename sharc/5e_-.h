#pragma once
/* COP op 5e - */

/* Scalar times vector. */
static void gcop_5e(void) {
    float s = gems_in_f();
    float x = gems_in_f(), y = gems_in_f(), z = gems_in_f();
    gems_out_f(s * x);
    gems_out_f(s * y);
    gems_out_f(s * z);
}
