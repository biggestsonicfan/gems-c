#pragma once
/* COP op 5c - */

/* Two vectors, interleaved (x0 x1 y0 y1 z0 z1): their sum. */
static void gcop_5c(void) {
    float x0 = gems_in_f(), x1 = gems_in_f();
    float y0 = gems_in_f(), y1 = gems_in_f();
    float z0 = gems_in_f(), z1 = gems_in_f();
    gems_out_f(x0 + x1);
    gems_out_f(y0 + y1);
    gems_out_f(z0 + z1);
}
