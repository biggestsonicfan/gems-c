#pragma once
/* COP op 3e Fn_coli_trans_xz */

static void gcop_3e(void) {
    float dx = gems_in_f();
    float dz = gems_in_f();
    float px = gems_in_f();
    float py = gems_in_f();
    float pz = gems_in_f();
    float len = gch_8001e084(dx * dx + dz * dz);
    float c = dx / len;
    float s = dz / len;
    static const uint32_t src[4] = { 16000u, 0x3EE0u, 0x7E80u, 0x7EE0u };
    static const uint32_t dst[4] = { 0x3F40u, 0x3FA0u, 0x7F40u, 0x7FA0u };
    for (int b = 0; b < 4; b++) {
        uint32_t si = src[b], di = dst[b];
        for (uint32_t i = 0; i < 32; i++, si += 3u, di += 3u) {
            float x = gems_bram_rdf(si);
            float y = gems_bram_rdf(si + 1u);
            float z = gems_bram_rdf(si + 2u) - pz;
            x = x - px;
            gems_bram_wrf(di, (x * c + z * s));
            gems_bram_wrf(di + 1u, y - py);
            gems_bram_wrf(di + 2u, (z * c - x * s));
        }
    }
}
