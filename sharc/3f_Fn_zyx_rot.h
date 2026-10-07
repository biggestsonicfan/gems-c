#pragma once
/* COP op 3f Fn_zyx_rot */

static void gcop_3f(void) {
    uint32_t az = (uint16_t)gems_in_w();
    uint32_t ay = (uint16_t)gems_in_w();
    uint32_t ax = (uint16_t)gems_in_w();
    gch_8001e6f8(az, *gems_dm(0x3033Fu));
    gch_8001e808(ay, *gems_dm(0x3033Fu));
    gch_8001e8b4(ax, *gems_dm(0x3033Fu));
    gems_out_w(0);
}
