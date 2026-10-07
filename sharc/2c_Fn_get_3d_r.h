#pragma once
/* COP op 2c Fn_get_3d_r */

static void gcop_2c(void) {
    float a[6];
    for (int i = 0; i < 6; i++) a[i] = gems_in_f();
    gems_out_f(gch_8001dbdc(a[0] - a[1], a[2] - a[3], a[4] - a[5]));
}
