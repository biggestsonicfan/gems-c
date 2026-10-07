#pragma once
/* COP op 85 Fn_zanzou_get_info */

/* Replies slot n's two words, its life and step, and one of its three model
 * numbers by how much life it has left. */
static void gcop_85(void) {
    const uint32_t *S = gems_dm(0x30000u);
    uint32_t w = 0x2300u + 32u * gems_in_w();
    gems_out_w(S[w]);
    gems_out_w(S[w + 1u]);
    uint32_t life = S[w + 2u];
    gems_out_w(life);
    uint32_t step = S[w + 3u];
    gems_out_w(step);
    if (step & 0x80000000u) step = 0u - step;
    uint32_t a = ((step >> 2) - 1u) * 7u + 40u;
    uint32_t b = (step >> 1) + step;
    uint32_t model = S[w + 4u];
    if (a >= life) {
        model = S[w + 5u];
        if (b >= life) model = S[w + 6u];
    }
    gems_out_w(model);
}
