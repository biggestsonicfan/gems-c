#pragma once
/* COP op 46 Fn_mul_matrix_inner_rev */

static void gcop_46(void) {
    uint32_t n = gems_in_w();
    gch_8001d4a0(gch_cur(), gems_dmf(gcs_inner + n * 12u + 32u));
}
