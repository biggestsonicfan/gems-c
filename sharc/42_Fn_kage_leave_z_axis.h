#pragma once
/* COP op 42 Fn_kage_leave_z_axis */

static void gcop_42(void) {
    uint32_t cur = *gems_dm(0x3033Fu);
    float *m = gems_dmf(0x30000u + cur);
    float r = gch_8001e174(m[6] * m[6] + m[8] * m[8]);
    m[3] = m[6] * r;
    m[5] = m[8] * r;
    m[2] = -1.0f * m[3];
    *gems_dm(0x30000u + cur) = *gems_dm(0x30000u + cur + 5u);
}
