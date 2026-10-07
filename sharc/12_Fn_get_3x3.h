#pragma once
/* COP op 12 Fn_get_3x3 */

static void gcop_12(void) {
    const uint32_t *m = gems_dm(0x30000u + *gems_dm(0x3033Fu));
    for (int i = 0; i < 9; i++) gems_out_w(m[i]);
}
