#pragma once
/* COP op 6a Fn_glo_to_loc */

/* A world point into the current matrix's frame: M^T (p - T). */
static void gcop_6a(void) {
    float x = gems_in_f(), y = gems_in_f(), z = gems_in_f();
    const float *m = gch_cur();
    float dy = y - m[10];
    float dx = x - m[9];
    float dz = z - m[11];
    gems_out_f(m[2] * dz + (m[0] * dx + m[1] * dy));
    gems_out_f(m[5] * dz + (m[3] * dx + m[4] * dy));
    gems_out_f(m[8] * dz + (m[6] * dx + m[7] * dy));
}
