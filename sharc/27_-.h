#pragma once
/* COP op 27 - */

static void gcop_27(void) {
    float y = gems_in_f();
    float x = gems_in_f();
    gems_out_w((uint16_t)gch_8001dfd0(y, x));
}
