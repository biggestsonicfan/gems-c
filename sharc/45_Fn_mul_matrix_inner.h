#pragma once
/* COP op 45 Fn_mul_matrix_inner */

static void gcop_45(void) {
    uint32_t n = gems_in_w();
    gch_8001d688(gems_dmf(gcs_inner + n * 12u + 32u), gch_cur());
}
