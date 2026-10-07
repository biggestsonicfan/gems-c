#pragma once
/* COP op 28 Fn_tri_shin */

static void gcop_28(void) {
    float a = gems_in_f();
    float b = gems_in_f();
    float c = gems_in_f();
    float a2 = a * a;
    float b2 = b * b;
    float c2 = c * c;
    int32_t ang = gch_8001dd2c((c2 + (a2 - b2)) / ((2.0f * a) * c));
    gems_out_w((uint16_t)(0x4000 - ang));
    float x = ((c2 + b2) - a2) / (2.0f * b);
    gems_out_f(x);
    gems_out_f(gch_8001e084(-fmaf(x, x, -c2)));   /* fnmsubs */
}
