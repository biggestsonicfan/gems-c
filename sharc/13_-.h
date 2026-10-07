#pragma once
/* COP op 13 - */

static void gcop_13(void) {
    float a = gems_in_f();
    float b = gems_in_f();
    gems_out_f(a + b);
}
