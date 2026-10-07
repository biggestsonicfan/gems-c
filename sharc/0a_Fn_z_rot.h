#pragma once
/* COP op 0a Fn_z_rot */

static void gcop_0a(void) {
    uint32_t ang = (uint16_t)gems_in_w();
    gch_8001e6f8(ang, *gems_dm(0x3033Fu));
}
