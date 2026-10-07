#pragma once
/* COP op 23 - */

static void gcop_23(void) {
    uint32_t a = (uint16_t)gems_in_w();
    if (a == 0x4000u || a == 0xC000u) {     /* tan's poles */
        gems_out_w(0);
        return;
    }
    float rad = (6.2831854820251465f * (float)a) / 65536.0f;
    gems_out_f((float)gch_800aea20((double)rad));
}
