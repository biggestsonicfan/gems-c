#pragma once
/* COP op 06 Fn_trans */

static void gcop_06(void) {
    float x = gems_in_f();
    float y = gems_in_f();
    float z = gems_in_f();
    gch_8001e9dc(x, y, z);
}
