#pragma once
/* COP op 58 Fn_get_z_axis */

static void gcop_58(void) {
    const float *m = gch_cur();
    gems_out_f(m[6]);
    gems_out_f(m[7]);
    gems_out_f(m[8]);
}
