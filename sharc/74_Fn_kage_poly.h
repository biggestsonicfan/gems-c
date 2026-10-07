#pragma once
/* COP op 74 Fn_kage_poly */

/* Gems pushes the matrix stack here and never pops it (as the decompile and PPC have it). */
static void gcop_74(void) {
    gch_80025350();
    uint32_t sel = gems_in_w();
    uint32_t idx = gems_in_w();
    gch_80024a34(sel, idx);
    uint32_t flags = gems_in_w();
    float x = gems_in_f();
    float y = gems_in_f();
    float z = gems_in_f();
    if (flags & 4u) gch_8001e9dc(x, y, z);
    if (flags & 2u)      gch_800245e4();
    else if (flags & 1u) gch_8002476c();
    gch_8002443c(gems_in_w());
}
