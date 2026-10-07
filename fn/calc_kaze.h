/* calc_kaze (i960 0x68AA4): the stage's wind on fighter g7, work at g13.
 * Picks a direction (g13+0x134) and the gust/still flag (bit 10) by stage,
 * advances the gust phase (g13+0x130) and stores the wind vector at
 * g13+0x138. */
static uint32_t gfn_calc_kaze(int entry)
{
    uint32_t w = GEMS_G(13);
    int windy = 0;

    if (!(gems_ld32(w) & 0x10) && !(gems_ld32(w) & 0x40)) {
        gems_st32(w + 0x134, 0xF000);
        gems_st32(w, gems_ld32(w) | 0x400);
        uint32_t stage = gems_ld8(0x500064);
        windy = 1;
        if (stage == 8) {
            gems_st32(w + 0x134, 0x7000);
            gems_st32(w, gems_ld32(w) | 0x400);
        } else if (stage == 0 || stage == 0xD || stage == 6) {
            gems_st32(w + 0x134, 0xF000);
            gems_st32(w, gems_ld32(w) | 0x400);
        } else if (stage == 2) {
            gems_st32(w + 0x134, 0xC000);
            gems_st32(w, gems_ld32(w) & ~0x400u);
        } else {
            /* other stages: only in game modes 14/15, and only for the
             * fighter named by 0x500065 when its 0x1B1 is not 1.
             * PowerPC slw: the shift's low 6 bits, 32-63 give 0. */
            uint32_t mode = gems_ld8(0x500031) & 63;
            uint32_t bit = mode < 32 ? 1u << mode : 0;
            if (!(bit & 0xC000u) || gems_ld8(0x500065) != gems_ld8(GEMS_G(7) + 4)
                || gems_ld8(GEMS_G(7) + 0x1B1) == 1)
                windy = 0;
        }
    }

    if (!windy) {
        gems_st32(w, gems_ld32(w) & ~0x200u);
        gems_st32(w + 0x130, 0);
        uint32_t zero[3] = { 0, 0, 0 };
        gems_stn(w + 0x138, zero, 3);
    } else {
        gems_st32(w, gems_ld32(w) | 0x200);
        uint32_t tab = gems_ld32(0x68914 + gems_ld8(GEMS_G(7) + 0x1B0) * 4);
        if (!(gems_ld32(w) & 0x400))
            tab += 8;
        GEMS_G(0) = gems_ld16(tab);                         /* ldos */
        GEMS_G(2) = gems_ld32(tab + 4);
        uint32_t phase = gems_ld32(w + 0x130) + GEMS_G(0);
        gems_st32(w + 0x130, phase);

        gems_cop_w(0x10802121);                              /* sin(phase) */
        gems_cop_w(phase);
        float s = gems_cop_rf() + 2.0f;
        float strength = gems_u2f(GEMS_G(2)) * s;
        strength = strength * gems_ldf(0x50A000);
        uint32_t dir = gems_ld32(w + 0x134);

        uint32_t v[3];
        gems_cop_w(0x12802525);
        gems_cop_w(dir);
        gems_cop_wf(strength);
        v[0] = gems_cop_r();
        gems_cop_w(0x12002424);
        gems_cop_w(dir);
        gems_cop_wf(strength);
        v[2] = gems_cop_r();
        float lim = gems_ldf(0x50A000);
        v[1] = (lim <= strength && strength != lim) ? gems_f2u(lim)
                                                    : gems_f2u(strength);
        gems_stn(w + 0x138, v, 3);
    }

    if (entry)
        gems_i960_ret();
    return 0;
}
