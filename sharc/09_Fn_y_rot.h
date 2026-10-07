#pragma once
/* COP op 09 Fn_y_rot */

static void gcop_09(void) {
    uint32_t ang = (uint16_t)gems_in_w();
    gch_8001e808(ang, *gems_dm(0x3033Fu));
}
