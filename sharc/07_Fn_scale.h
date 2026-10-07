#pragma once
/* COP op 07 Fn_scale */

static void gcop_07(void) {
    float x = gems_in_f();
    float y = gems_in_f();
    float z = gems_in_f();
    gch_8001e960(x, y, z, *gems_dm(0x3033Fu));
}
