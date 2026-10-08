#pragma once
/* COP op 63 Fn_get_loc_pos */

static void gcop_63(void) {
    float x0 = gems_in_f(), y0 = gems_in_f(), z0 = gems_in_f();
    uint32_t ang = (uint16_t)gems_in_w();
    float x1 = gems_in_f(), y1 = gems_in_f(), z1 = gems_in_f();
    float s, c;
    gch_8001ead8(&s, &c, ang);
    float u = c * z1 + s * x0;
    float v = c * x1 + s * z1;
    u = u - s * x1;
    v = v - s * z0;
    float rz = u - c * z0;
    float rx = v - c * x0;
    gems_out_f(rx);
    gems_out_f(y1 - y0);
    gems_out_f(rz);
}
