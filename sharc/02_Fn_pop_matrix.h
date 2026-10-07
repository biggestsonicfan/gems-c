#pragma once
/* COP op 02 Fn_pop_matrix */

static void gcop_02(void) {
    uint32_t depth = *gems_dm(0x3033Cu);
    if (depth == 0u) return;
    *gems_dm(0x3033Cu) = depth - 1u;
    if (depth >= 8u) return;
    *gems_dm(0x3033Fu) -= 0xCu;
}
