#pragma once
/* COP op 43 Fn_get_matrix_inner */

static void gcop_43(void) {
    uint32_t n = gems_in_w();
    gch_8001d870(gems_dm(gcs_inner + n * 12u + 32u), gems_dm(0x30000u + *gems_dm(0x3033Fu)));
}
