#pragma once
/* COP op 5b Fn_rot_2d */

static void gcop_5b(void) {
    uint32_t ang = (uint16_t)gems_in_w();
    float x = gems_in_f(), y = gems_in_f();
    float s, c;
    gch_8001ead8(&s, &c, ang);
    gems_out_f(c * x - s * y);
    gems_out_f(s * x + c * y);
}
