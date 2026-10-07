#pragma once
/* COP op 75 Fn_kage_flag */

/* Two masks over the fighter's 16 shadow balls: near (above the floor, within
 * the inner reach) and far (beyond the outer reach). */
static void gcop_75(void) {
    uint32_t id   = gems_in_w();
    uint32_t mode = gems_in_w();
    float dx    = gems_in_f();
    float dz    = gems_in_f();
    float inner = gems_in_f();
    float outer = gems_in_f();
    const uint32_t *S = gems_dm(0x30000u);
    uint32_t base = id ? 0x7E80u : 16000u;
    uint32_t tbl  = id ? 0x7E0u : 0x6E0u;
    uint32_t near_m = 0, far_m = 0;
    for (uint32_t k = 0; k < 16; k++) {
        uint32_t bi = S[tbl + k] * 3u + base;
        float x = gems_bram_rdf(bi);
        float y = gems_bram_rdf(bi + 1u);
        float z = gems_bram_rdf(bi + 2u);
        float X = dx * y;
        X = X + x;
        float Z = dz * y;
        Z = Z + z;
        float l;
        if (mode) {
            float xx = X * X;
            float zz = Z * Z;
            l = 0.7071059942245483f * gch_8001e084(xx + zz);
        } else {
            l = fabsf(X);
            if (!(l >= fabsf(Z))) l = fabsf(Z);
        }
        if (!(gems_f2u(y) & 0x80000000u) && l <= inner) near_m |= 1u << k;
        else if (l >= outer)                             far_m  |= 1u << k;
    }
    gems_out_w(near_m);
    gems_out_w(far_m);
}
