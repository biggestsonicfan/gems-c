#pragma once
/* COP op 32 Fn_fcurve_spl */

static void gcop_32(void) {
    const float K = 0.03333299979567528f;   /* 0x3D08882F */
    float span = gems_in_f();
    float t = gems_in_f();
    float v0 = gems_in_f();
    float v1 = gems_in_f();
    float m0 = gems_in_f();
    float m1 = gems_in_f();
    float msum = m0 + m1;
    float scale = K * span;
    float inv = 1.0f / scale;
    float p0 = v0 * inv;
    float k = K * inv;
    float dv = -fmaf(v1, inv, -p0);          /* fnmsubs: p0 - v1*inv */
    float u = k * t;
    float dv2 = dv + dv;
    float c3 = msum + dv2;
    float r = fmaf(c3, u, -dv2);              /* fmsubs */
    r = r - dv;
    r = r - msum;
    r = r - m0;
    r = fmaf(u, r, m0);
    r = fmaf(u, r, p0);
    gems_out_f(scale * r);
}
