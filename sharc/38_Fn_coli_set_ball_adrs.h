#pragma once
/* COP op 38 Fn_coli_set_ball_adrs */

static void gcop_38(void) {
    *gems_dm(0x3033Eu) = (gems_in_w() == 1u) ? 0x7E80u : 16000u;   /* ball base (state+0xCF8) */
}
