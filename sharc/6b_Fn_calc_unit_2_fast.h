#pragma once
/* COP op 6b Fn_calc_unit_2_fast */

/* Clamp to [-1, 1] the way Gems does (NaN comes out as 1). */
static inline float g6b_clamp(float v) {
    float o = 1.0f;
    if (1.0f - v > 0.0f) {
        o = -1.0f;
        if (-1.0f - v < 0.0f) o = v;
    }
    return o;
}

static inline void g6b_store(uint32_t b) {
    const uint32_t *m = gems_dm(0x30000u + *gems_dm(0x3033Fu));
    for (uint32_t k = 0; k < 12; k++) gems_bram_wr(b + k, m[k]);
}

/* Two-bone IK: place and turn the root, aim it at the target, and when the
 * target is in reach bend the two bones, storing each bone's matrix into
 * bufferram. The working values live in gcs_7c60..gcs_7c98, as in Gems. */
static void gcop_6b(void) {
    float tx = gems_in_f(), ty = gems_in_f(), tz = gems_in_f();
    uint32_t a[6];
    for (int k = 0; k < 6; k++) a[k] = (uint16_t)gems_in_w();
    float px = gems_in_f(), py = gems_in_f(), pz = gems_in_f();
    float len1 = gems_in_f(), len2 = gems_in_f();
    uint32_t bram0 = gems_in_w();
    uint32_t bram1 = gems_in_w();
    uint32_t flag  = gems_in_w();

    gch_8001e9dc(tx, ty, tz);
    gch_8001e6f8(a[0], *gems_dm(0x3033Fu));
    gch_8001e808(a[1], *gems_dm(0x3033Fu));
    gch_8001e8b4(a[2], *gems_dm(0x3033Fu));
    gch_8001e808(a[3], *gems_dm(0x3033Fu));
    gch_8001e8b4(a[4], *gems_dm(0x3033Fu));
    gch_8001e6f8(a[5], *gems_dm(0x3033Fu));

    float *G = gcs_7c60;
    G[0] = len1;
    G[2] = len2;
    gcs_7c90 = (int32_t)bram0;
    gcs_7c94 = (int32_t)bram1;
    gcs_7c98 = (int32_t)flag;

    /* the target in the root's frame (y negated) */
    const float *m = gch_cur();
    float dy = py - m[10];
    float dx = px - m[9];
    float dz = pz - m[11];
    G[3] = m[2] * dz + (m[0] * dx + m[1] * dy);
    float qx = G[3] * G[3];
    G[4] = -(m[5] * dz + (m[3] * dx + m[4] * dy));
    float q = qx + G[4] * G[4];
    G[5] = m[8] * dz + (m[6] * dx + m[7] * dy);
    G[1] = gch_8001e084(q + G[5] * G[5]);           /* distance to the target */
    float r = gch_8001e174(q);
    G[8] = 1.0f / G[1];
    G[7] = r;
    G[6] = r * q;

    /* aim: about Z, then about Y */
    gch_8001e694(g6b_clamp(r * G[4]), g6b_clamp(r * G[3]), gch_cur());
    gch_8001e7a4(g6b_clamp(G[8] * G[5]), g6b_clamp(G[8] * G[6]), gch_cur());

    if ((G[2] + G[0]) - G[1] > 0.0f) {
        float l1 = G[0], l2 = G[2], d = G[1];
        G[9]  = l1 * l1;
        G[10] = d * d;
        G[11] = l2 * l2;
        /* cosine at the root */
        float cs = ((G[10] + G[9]) - G[11]) / (2.0f * (d * l1));
        float sn = gch_8001e084(1.0f - cs * cs);
        if (flag == 0) sn = -sn;
        gch_8001e694(g6b_clamp(sn), g6b_clamp(cs), gch_cur());
        g6b_store((uint32_t)gcs_7c90);

        /* cosine at the joint */
        float cj = ((G[9] + G[11]) - G[10]) / (2.0f * (G[0] * G[2]));
        cs = -cj;
        sn = gch_8001e084(1.0f - cj * cj);
        if (flag != 0) sn = -sn;
        gch_8001e694(g6b_clamp(sn), g6b_clamp(cs), gch_cur());
        g6b_store((uint32_t)gcs_7c94);
        gems_out_f(0.0f);
    } else {
        /* out of reach: both bones straight along the aim */
        g6b_store((uint32_t)gcs_7c90);
        g6b_store((uint32_t)gcs_7c94);
        gems_out_f(0.0f);
    }
}
