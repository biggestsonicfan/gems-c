#pragma once
/* COP op 16 - */

static void gcop_16(void) {
    float a = gems_in_f();
    float b = gems_in_f();
    gems_out_f(a / b);
}
