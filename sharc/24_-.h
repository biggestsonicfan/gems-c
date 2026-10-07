#pragma once
/* COP op 24 - */

static void gcop_24(void) {
    uint32_t ang = (uint16_t)gems_in_w();
    float s = gems_in_f();
    gems_out_f(s * gch_8001eb40(ang));
}
