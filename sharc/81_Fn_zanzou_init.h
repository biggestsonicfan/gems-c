#pragma once
/* COP op 81 Fn_zanzou_init */

static void gcop_81(void) {
    uint32_t *S = gems_dm(0x30000u);
    for (uint32_t k = 0; k < 4480; k++) S[0x2180u + k] = 0;
}
