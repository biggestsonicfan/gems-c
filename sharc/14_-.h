#pragma once
/* COP op 14 - */

static void gcop_14(void) {
    float a = gems_in_f();
    float b = gems_in_f();
    gems_out_f(a - b);
}
