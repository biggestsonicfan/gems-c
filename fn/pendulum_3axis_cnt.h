/* pendulum_3axis_cnt (i960 0x89C24, Gems 80039360): a three-axis pendulum.
 * g3 is its record (+0x00 scale, +0x0C amplitude, +0x18 rate, +0x48 frame
 * count), g4..g6 the axes' periods; the result goes back in g0..g2.
 * The COP's angles are converted with cvtri (rounded by the AC mode; out of
 * range or NaN gives 0x80000000), the frame count with cvtir (signed). */

static uint32_t gfn_pendulum_3axis_cnt(int entry)
{
    uint32_t rec = GEMS_G(3);
    uint32_t frame = gems_ld32(rec + 0x48) + 1;
    gems_st32(rec + 0x48, frame);
    float t = (float)(int32_t)frame * gems_u2f(0x4622F984);   /* cvtir (signed), mulr */

    gems_cop_w(0x2F005E5E);                                   /* scalar * vector */
    gems_cop_wf(t);
    gems_cop_w(GEMS_G(4));
    gems_cop_w(GEMS_G(5));
    gems_cop_w(GEMS_G(6));
    float ang[3];
    for (int i = 0; i < 3; i++) ang[i] = gems_cop_rf();

    uint32_t amp[3], rate[3];
    gems_ldn(amp, rec + 0x0C, 3);
    gems_ldn(rate, rec + 0x18, 3);
    for (int i = 0; i < 3; i++) {
        uint32_t a = gems_cvtri(gems_f2u(ang[i]));
        gems_cop_w(0x12802525);                               /* sin */
        gems_cop_w(a);
        gems_cop_w(amp[i]);
        amp[i] = gems_cop_r();
        float r = gems_u2f(rate[i]) / gems_u2f(GEMS_G(4 + i));    /* divr */
        gems_cop_w(0x12002424);                               /* cos */
        gems_cop_w(a);
        gems_cop_wf(r);
        rate[i] = gems_cop_r();
    }
    gems_cop_w(0x2E005C5C);
    for (int i = 0; i < 3; i++) {
        gems_cop_w(rate[i]);
        gems_cop_w(amp[i]);
    }
    float v[3];
    for (int i = 0; i < 3; i++) v[i] = gems_cop_rf();

    uint32_t sw[3];
    gems_ldn(sw, rec, 3);
    float s[3];
    for (int i = 0; i < 3; i++) {
        s[i] = gems_u2f(GEMS_G(i)) * gems_u2f(sw[i]);         /* mulr */
        sw[i] = gems_f2u(s[i]);
    }
    gems_stn(rec, sw, 3);
    for (int i = 0; i < 3; i++)
        GEMS_G(i) = gems_f2u(s[i] * v[i]);
    if (entry) gems_i960_ret();
    return 0;
}
