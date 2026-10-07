#pragma once
/* COP op 41 Fn_kage_leave_x_axis */

static void gcop_41(void) {
    uint32_t cur = *gems_dm(0x3033Fu);
    float *m = gems_dmf(0x30000u + cur);
    float r = gch_8001e174(m[0] * m[0] + m[2] * m[2]);
    m[3] = m[0] * r;
    m[5] = m[2] * r;
    m[6] = -1.0f * m[5];
    *gems_dm(0x30000u + cur + 8u) = *gems_dm(0x30000u + cur + 3u);
}
