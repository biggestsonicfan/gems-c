#pragma once
/* COP op 00 Fn_initialize */

static void gcop_00(void) {
    *gems_dm(0x3033Cu) = 0;          /* stack depth (state+0xCF0) */
    *gems_dm(0x3033Fu) = 0x5A0u;     /* current matrix (state+0xCFC) */
}
