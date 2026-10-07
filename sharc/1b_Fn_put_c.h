#pragma once
/* COP op 1b Fn_put_c */

static void gcop_1b(void) {
    *gems_dmf(gcs_c) = gems_in_f();
}
