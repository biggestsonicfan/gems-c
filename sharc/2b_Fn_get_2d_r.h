#pragma once
/* COP op 2b Fn_get_2d_r */

static void gcop_2b(void) {
    float a[4];
    for (int i = 0; i < 4; i++) a[i] = gems_in_f();
    float d1 = a[2] - a[3];
    float d0 = a[0] - a[1];
    gems_out_f(gch_8001e084(d0 * d0 + d1 * d1));
}
