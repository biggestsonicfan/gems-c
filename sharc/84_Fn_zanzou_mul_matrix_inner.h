#pragma once
/* COP op 84 Fn_zanzou_mul_matrix_inner */

/* current = current (x) afterimage slot n's matrix */
static void gcop_84(void) {
    uint32_t n = gems_in_w();
    gch_8001d688(gems_dmf(0x30000u + 0x2314u + 32u * n), gch_cur());
}
