/* spin_attack_cnt_esp_dsp (i960 0x81914; Gems 80047BCC): draw Espio's spin
 * attack. The spin's scale (0x509508) times the vector g0-g2 (COP 0x5E)
 * gives three floats; the first and third, converted with cvtri (rounded by
 * the AC mode; out of range or NaN gives 0x80000000), are the x and z angles. COP 0x25 of each against 1.5 and the word
 * at 0x509518 go to rob+0x2074 (rob = g7) as { 0x25(z), [0x509518], 0x25(x) }.
 * Then turn by z and x, translate (0, -[rob+0x20EC], 0), turn by y
 * (halfword rob+0x2080, or +0x2082 on odd frames: bit 0 of 0x500020) and
 * draw model rob+0x2084 (or +0x2086) with set_obj (g0 = model, g1 = 0). */
#pragma once

static uint32_t gfn_spin_attack_cnt_esp_dsp(int entry)
{
    uint32_t rob = GEMS_G(7);
    const uint32_t one_half = 0x3FC00000u;   /* 1.5f */
    uint32_t v[3];

    gems_cop_w(0x2F005E5Eu);                 /* scalar * vector */
    gems_cop_w(gems_ld32(0x509508u));
    gems_cop_w(GEMS_G(0));
    gems_cop_w(GEMS_G(1));
    gems_cop_w(GEMS_G(2));
    uint32_t rx = gems_cop_r();
    (void)gems_cop_r();
    uint32_t rz = gems_cop_r();
    uint32_t ang_x = gems_cvtri(rx);
    uint32_t ang_z = gems_cvtri(rz);

    gems_cop_w(0x12802525u);
    gems_cop_w(ang_z);
    gems_cop_w(one_half);
    v[0] = gems_cop_r();
    v[1] = gems_ld32(0x509518u);
    gems_cop_w(0x12802525u);
    gems_cop_w(ang_x);
    gems_cop_w(one_half);
    v[2] = gems_cop_r();
    gems_stn(rob + 0x2074u, v, 3);

    gems_cop_w(0x05000A0Au);                 /* ang_z */
    gems_cop_w(ang_z);
    gems_cop_w(0x04000808u);                 /* ang_x */
    gems_cop_w(ang_x);
    gems_cop_w(0x03000606u);                 /* translate */
    gems_cop_w(0);
    gems_cop_w(gems_ld32(rob + 0x20ECu) | 0x80000000u);
    gems_cop_w(0);

    uint32_t odd = gems_ld32(0x500020u) & 1u;
    gems_cop_w(0x04800909u);                 /* ang_y */
    gems_cop_w(gems_ld16(rob + (odd ? 0x2082u : 0x2080u)));
    uint32_t model = gems_ld16(rob + (odd ? 0x2086u : 0x2084u));
    GEMS_G(1) = 0;
    GEMS_G(0) = model;
    gfn_set_obj();
    if (entry) gems_i960_ret();
    return 0;
}
