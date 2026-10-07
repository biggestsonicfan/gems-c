#pragma once
/* COP op 35 Fn_st_unit_mat */

static void gcop_35(void) {
    uint32_t sel = gems_in_w();
    uint32_t n = gems_in_w();
    uint32_t slot = ((sel & 0xFFu) == 1u) ? n + 0x4E0u : n + 0x420u;
    gch_8001d870(gems_dm(0x30000u + slot), gems_dm(0x30000u + *gems_dm(0x3033Fu)));
}
