#pragma once
/* COP op 05 Fn_get_matrix */

static void gcop_05(void) {
    const uint32_t *m = gems_dm(0x30000u + *gems_dm(0x3033Fu));
    for (int i = 0; i < 12; i++) gems_out_w(m[i]);
}
