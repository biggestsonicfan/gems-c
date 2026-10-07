#pragma once
/* COP op 5a Fn_regular_vector_2d */

static void gcop_5a(void) {
    float x = gems_in_f(), y = gems_in_f();
    float k = gch_8001d8e0(x, y);
    gems_out_f(k * x);
    gems_out_f(k * y);
}
