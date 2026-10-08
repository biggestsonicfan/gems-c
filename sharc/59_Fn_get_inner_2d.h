#pragma once
/* COP op 59 Fn_get_inner_2d */

static void gcop_59(void) {
    float a = gems_in_f(), b = gems_in_f();
    float c = gems_in_f(), d = gems_in_f();
    gems_out_f(a * b + c * d);
}
