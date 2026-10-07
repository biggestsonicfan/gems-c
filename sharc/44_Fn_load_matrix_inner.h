#pragma once
/* COP op 44 Fn_load_matrix_inner */

static void gcop_44(void) {
    uint32_t n = gems_in_w();
    gch_8001d870(gems_dm(0x30000u + *gems_dm(0x3033Fu)), gems_dm(gcs_inner + n * 12u + 32u));
}
