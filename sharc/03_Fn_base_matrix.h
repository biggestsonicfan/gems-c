#pragma once
/* COP op 03 Fn_base_matrix */

static void gcop_03(void) {
    uint32_t *m = gems_dm(0x30000u + *gems_dm(0x3033Fu));
    for (int i = 0; i < 12; i++) m[i] = 0;
    m[0] = m[4] = m[8] = 0x3F800000u;   /* 1.0f */
}
