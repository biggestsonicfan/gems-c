#pragma once
/* COP op 47 Fn_mul_matrix_rev */

static void gcop_47(void) {
    float m[12];
    for (int k = 0; k < 12; k++) m[k] = gems_in_f();
    gch_8001d4a0(gch_cur(), m);
}
