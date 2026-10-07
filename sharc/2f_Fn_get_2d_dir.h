#pragma once
/* COP op 2f Fn_get_2d_dir */

static void gcop_2f(void) {
    float a[4];
    for (int i = 0; i < 4; i++) a[i] = gems_in_f();
    gems_out_w((uint16_t)gch_8001dfd0(a[1] - a[0], a[3] - a[2]));
}
