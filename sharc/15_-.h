#pragma once
/* COP op 15 - */

static void gcop_15(void) {
    float a = gems_in_f();
    float b = gems_in_f();
    gems_out_f(a * b);
}
