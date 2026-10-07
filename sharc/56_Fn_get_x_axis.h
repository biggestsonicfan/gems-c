#pragma once
/* COP op 56 Fn_get_x_axis */

static void gcop_56(void) {
    const float *m = gch_cur();
    gems_out_f(m[0]);
    gems_out_f(m[1]);
    gems_out_f(m[2]);
}
