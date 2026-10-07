#pragma once
/* COP op 01 Fn_push_matrix */

static void gcop_01(void) {
    uint32_t depth = *gems_dm(0x3033Cu);
    if (depth >= 7u) return;
    *gems_dm(0x3033Cu) = depth + 1u;
    uint32_t cur = *gems_dm(0x3033Fu);
    gch_8001d870(gems_dm(0x30000u + cur + 0xCu), gems_dm(0x30000u + cur));
    *gems_dm(0x3033Fu) = cur + 0xCu;
}
