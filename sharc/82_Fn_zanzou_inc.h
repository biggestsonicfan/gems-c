#pragma once
/* COP op 82 Fn_zanzou_inc */

/* Ages the 128 afterimage slots by their life step; replies how many are alive. */
static void gcop_82(void) {
    uint32_t *S = gems_dm(0x30000u);
    uint32_t n = 0;
    for (uint32_t j = 0; j < 128; j++) {
        uint32_t w = 0x2300u + 32u * j;
        uint32_t life = S[w + 2u];
        if (life) {
            n++;
            life += S[w + 3u];
            if (life & 0x80000000u) {
                life = 0;
                n--;
            }
            S[w + 2u] = life;
        }
    }
    gems_out_w(n);
}
