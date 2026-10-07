#pragma once
/* COP op 2e Fn_get_3d_len */

static void gcop_2e(void) {
    float x = gems_in_f();
    float y = gems_in_f();
    float z = gems_in_f();
    gems_out_f(gch_8001dbdc(x, y, z));
}
