#pragma once
/* COP op 08 Fn_x_rot */

static void gcop_08(void) {
    uint32_t ang = (uint16_t)gems_in_w();
    gch_8001e8b4(ang, *gems_dm(0x3033Fu));
}
