#pragma once
/* COP op 57 Fn_get_y_axis */

static void gcop_57(void) {
    const float *m = gch_cur();
    gems_out_f(m[3]);
    gems_out_f(m[4]);
    gems_out_f(m[5]);
}
