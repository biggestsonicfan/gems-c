#pragma once
/* COP op 0c Fn_inv_matrix */

static void gcop_0c(void) {
    gch_8001e270(*gems_dm(0x3033Fu));                  /* inverse -> state+0xC0C */
    gch_8001d870(gems_dm(0x30000u + *gems_dm(0x3033Fu)), gems_dm(0x30303u));
}
