#pragma once
/* COP op 51 Fn_get_glo_ang */

/* The current matrix's three angles: atan2(col2.x, col2.z), asin(col2.y),
 * atan2(col0.y, col1.y), each sent as a zero-extended u16. */
static void gcop_51(void) {
    const float *m = gch_cur();
    gems_out_w((uint16_t)gch_8001dfd0(m[8], m[6]));
    gems_out_w((uint16_t)gch_8001dd2c(m[7]));
    gems_out_w((uint16_t)gch_8001dfd0(m[4], m[1]));
}
