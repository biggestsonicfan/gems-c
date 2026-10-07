#pragma once
/* COP op 2d Fn_get_2d_len */

static void gcop_2d(void) {
    float x = gems_in_f();
    float y = gems_in_f();
    gems_out_f(gch_8001dae4(x, y));
}
