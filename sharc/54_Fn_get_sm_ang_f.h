#pragma once
/* COP op 54 Fn_get_sm_ang_f */

/* Identity, turned by nine angles (x y z z y x y x z), read back as angles. */
static void gcop_54(void) {
    uint32_t a[9];
    for (int k = 0; k < 9; k++) a[k] = (uint16_t)gems_in_w();
    gch_80025214();
    gch_8001e8b4(a[0], *gems_dm(0x3033Fu));
    gch_8001e808(a[1], *gems_dm(0x3033Fu));
    gch_8001e6f8(a[2], *gems_dm(0x3033Fu));
    gch_8001e6f8(a[3], *gems_dm(0x3033Fu));
    gch_8001e808(a[4], *gems_dm(0x3033Fu));
    gch_8001e8b4(a[5], *gems_dm(0x3033Fu));
    gch_8001e808(a[6], *gems_dm(0x3033Fu));
    gch_8001e8b4(a[7], *gems_dm(0x3033Fu));
    gch_8001e6f8(a[8], *gems_dm(0x3033Fu));
    gch_80023ccc();
}
