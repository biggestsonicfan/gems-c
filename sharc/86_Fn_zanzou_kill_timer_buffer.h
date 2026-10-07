#pragma once
/* COP op 86 Fn_zanzou_kill_timer_buffer */

static void gcop_86(void) {
    uint32_t *S = gems_dm(0x30000u);
    uint32_t base = gems_in_w() ? 0x21F0u : 0x2190u;
    for (uint32_t k = 0; k < 16; k++) S[base + k] = 0;
}
