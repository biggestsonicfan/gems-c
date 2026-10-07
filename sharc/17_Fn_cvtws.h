#pragma once
/* COP op 17 Fn_cvtws */

static void gcop_17(void) {
    gems_out_f((float)(int32_t)gems_in_w());
}
