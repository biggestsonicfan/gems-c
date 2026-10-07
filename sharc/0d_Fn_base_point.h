#pragma once
/* COP op 0d Fn_base_point */

static void gcop_0d(void) {
    uint32_t *m = gems_dm(0x30000u + *gems_dm(0x3033Fu));
    m[9] = 0;
    m[10] = 0;
    m[11] = 0;
}
