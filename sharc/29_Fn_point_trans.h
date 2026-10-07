#pragma once
/* COP op 29 Fn_point_trans */

static void gcop_29(void) {
    float x = gems_in_f();
    float y = gems_in_f();
    float z = gems_in_f();
    float ox, oy, oz;
    gch_8001ea5c(x, y, z, *gems_dm(0x3033Fu), &ox, &oy, &oz);
    gems_out_f(ox);
    gems_out_f(oy);
    gems_out_f(oz);
}
