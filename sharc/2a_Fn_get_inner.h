#pragma once
/* COP op 2a Fn_get_inner */

static void gcop_2a(void) {
    float a[6];
    for (int i = 0; i < 6; i++) a[i] = gems_in_f();
    float t = a[2] * a[3];
    t = a[0] * a[1] + t;
    gems_out_f(a[4] * a[5] + t);
}
