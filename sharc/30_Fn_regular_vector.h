#pragma once
/* COP op 30 Fn_regular_vector */

static void gcop_30(void) {
    float x = gems_in_f();
    float y = gems_in_f();
    float z = gems_in_f();
    float r = gch_8001d9e0(x, y, z);
    gems_out_f(r * x);
    gems_out_f(r * y);
    gems_out_f(r * z);
}
