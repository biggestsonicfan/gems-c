#pragma once
/* COP op 0f Fn_get_point */

static void gcop_0f(void) {
    const uint32_t *m = gems_dm(0x30000u + *gems_dm(0x3033Fu));
    for (int i = 9; i < 12; i++) gems_out_w(m[i]);
}
