/* smooth_int (i960 0x2F2B0, Gems 0x80054D50): start a motion blend.  Picks
 * the blend mode (rob+0xBDD) and length (rob+0xBDE/0xBE0), snapshots the
 * start pose, takes the start/end angle differences through the COP
 * (0x54 get_sm_ang_f) and lays per-frame steps at the blend buffer
 * (rob+0xBD8).
 *
 * Gems' shifts of the angle steps are logical (srw); the ROM's shri is
 * arithmetic, and a negative step's top bits are left in g0-g2. These are
 * the ROM's.  Gems never writes the condition code. */
#pragma once

static void gsi__bail(uint32_t rob, uint32_t state)
{
    gems_st8(rob + 0xBE5, state);
    gems_st8(rob + 0xBDD, 0);
    gems_st8(rob + 0xBE4, 0);
}

/* Push two lerp keys (a, b) through op 0x14 over 8 (x, ?, z) triples. */
static void gsi__keys(uint32_t start)
{
    uint32_t a = gems_ld32(start), b = gems_ld32(start + 8);
    if ((a | b) == 0) return;
    for (uint32_t p = start, i = 0; i < 8; i++, p += 12) {
        uint32_t v = gems_ld32(p);
        gems_cop_w(0x0A001414); gems_cop_w(v); gems_cop_w(a);
        gems_st32(p, gems_cop_r());
        v = gems_ld32(p + 8);
        gems_cop_w(0x0A001414); gems_cop_w(v); gems_cop_w(b);
        gems_st32(p + 8, gems_cop_r());
    }
}

static uint32_t gfn_smooth_int(int entry)
{
    uint32_t rob = GEMS_G(7);
    uint32_t state = 0;

    gems_st16(rob + 0xBE2, 0);
    if (gems_ld8(0x508008) != 0 ||
        !(gems_ld32(rob) & 0x4000000) ||
        (gems_ld32(rob + 0x70C) & 2) ||
        (gems_ld32(rob + 0x860) & 4) ||
        gems_ld8(rob + 0xBE4) == 1) {
        gsi__bail(rob, state);
        goto out;
    }

    uint32_t n = gems_ld16(rob + 0x800);
    uint32_t f = gems_ld32(rob + 0x1A4);
    if (f & 0x20000) {
        state = 2;
    } else if (gems_ld32(rob + 0x804) & 0x40000) {
        state = 3;
    } else if (gems_ld32(rob + 0x804) & 0x80000) {
        state = 1;
    } else {
        if ((gems_ld32(rob + 0xC30) & 0x8040) == 0x8040) { gsi__bail(rob, state); goto out; }
        uint32_t k = gems_ld8(rob + 0x814);
        if (k == 4) { gsi__bail(rob, state); goto out; }
        if (k == 0x7F)               state = 1;
        else if (f & 0x4000000)      state = 3;
        else if (f & 0x6600)         state = 1;
        else                         state = 3;
    }
    if (n <= 2) { gsi__bail(rob, state); goto out; }

    uint32_t step = (n <= 16) ? 4 : 8;
    uint32_t m = n - step;
    if (gems_ld8(rob + 0xA00) != 0 && m < gems_ld16(rob + 0x808))
        state &= ~2u;
    if ((gems_ld8(rob + 0xBE5) & 2) && (gems_ld8(rob + 0xBDD) & 2))
        state &= ~1u;
    gems_st8(rob + 0xBDD, state);
    gems_st16(rob + 0xBDE, step);
    gems_st16(rob + 0xBE0, m);
    gems_st8(rob + 0xBE5, 0);
    if (state == 2) goto out;

    uint32_t base = gems_ld32(rob + 0xBD8);
    GEMS_G(5) = 0x50E000;
    gfn_get_start_value(0);

    /* 20 triples: old pose -> 0x50E0F0, new start pose -> blend buffer. */
    uint32_t t[3];
    for (uint32_t i = 0; i < 20; i++) {
        gems_ldn(t, base + 0x690 + i * 12, 3);
        gems_stn(0x50E0F0 + i * 12, t, 3);
    }
    for (uint32_t i = 0; i < 20; i++) {
        gems_ldn(t, 0x50E000 + i * 12, 3);
        gems_stn(base + 0x690 + i * 12, t, 3);
    }
    gsi__keys(0x50E090);
    gsi__keys(0x50E180);

    if ((f & 0x100000) || (gems_ld32(rob + 0xC30) & 0x40)) {
        gems_cop_w(0x00800101);
        gems_cop_w(0x01800303);
        gems_cop_w(0x1F803F3F);
        gems_cop_w((uint32_t)gems_ld16s(0x50E038));
        gems_cop_w((uint32_t)gems_ld16s(0x50E034));
        gems_cop_w((uint32_t)gems_ld16s(0x50E030));
        gems_cop_r();
        gems_cop_w(0x05000A0A); gems_cop_w((uint32_t)-gems_ld16s(rob + 0xC04));
        gems_cop_w(0x04000808); gems_cop_w((uint32_t)-gems_ld16s(rob + 0xC00));
        gems_cop_w(0x04800909); gems_cop_w((uint32_t)-gems_ld16s(rob + 0xC02));
        gems_cop_w(0x04000808); gems_cop_w((uint32_t)-gems_ld16s(rob + 0x140));
        gems_cop_w(0x04800909); gems_cop_w((uint32_t)-gems_ld16s(rob + 0x142));
        gems_cop_w(0x05000A0A); gems_cop_w((uint32_t)-gems_ld16s(rob + 0x144));
        for (uint32_t p = 0x50E180, i = 0; i < 8; i++, p += 12) {
            uint32_t x = gems_ld32(p), z = gems_ld32(p + 8);
            gems_cop_w(0x14802929);
            gems_cop_w(x); gems_cop_w(0); gems_cop_w(z);
            x = gems_cop_r();
            gems_cop_r();
            z = gems_cop_r();
            gems_st32(p, x);
            gems_st32(p + 8, z);
        }
        GEMS_G(0) = 0;
        gems_cop_w(0x01000202);
    }
    (void)gems_ld32(rob + 0xC30);
    gfn_unit_smooth_cancel_front(0);

    uint32_t sh = (step == 8) ? 3 : (step == 4) ? 2 : 1;

    gems_cop_w(0x00800101);
    uint32_t start = 0x50E000, ang = rob + 0xBE8, end = 0x50E0F0, dst = base;
    GEMS_G(4) = gems_ld32(rob);
    uint32_t f2 = gems_ld32(rob + 0x1A4);
    uint32_t c30 = gems_ld32(rob + 0xC30);
    GEMS_G(3) = 0;
    for (uint32_t cnt = 12; cnt >= 1; cnt--) {
        if (((f2 & 0x100000) || (c30 & 0x40)) && cnt >= 8) {
            gems_st16(dst, GEMS_G(3));
            gems_st16(dst + 4, GEMS_G(3));
            gems_st16(dst + 8, GEMS_G(3));
        } else {
            gems_cop_w(0x2A005454);
            GEMS_G(0) = (uint32_t)-gems_ld16s(start);
            GEMS_G(1) = (uint32_t)-gems_ld16s(start + 4);
            GEMS_G(2) = (uint32_t)-gems_ld16s(start + 8);
            gems_cop_wn(&GEMS_G(0), 3);
            GEMS_G(0) = (uint32_t)gems_ld16s(end + 8);
            GEMS_G(1) = (uint32_t)gems_ld16s(end + 4);
            GEMS_G(2) = (uint32_t)gems_ld16s(end);
            gems_cop_wn(&GEMS_G(0), 3);
            GEMS_G(0) = (uint32_t)gems_ld16s(ang + 2);
            GEMS_G(1) = (uint32_t)gems_ld16s(ang);
            GEMS_G(2) = (uint32_t)gems_ld16s(ang + 4);
            gems_cop_wn(&GEMS_G(0), 3);
            GEMS_G(0) = (uint32_t)(int32_t)(int16_t)gems_cop_r();
            GEMS_G(1) = (uint32_t)(int32_t)(int16_t)gems_cop_r();
            GEMS_G(2) = (uint32_t)(int32_t)(int16_t)gems_cop_r();
            GEMS_G(0) = (uint32_t)((int32_t)(0u - GEMS_G(0)) >> sh);
            gems_st16(dst + 4, GEMS_G(0));
            GEMS_G(1) = (uint32_t)((int32_t)GEMS_G(1) >> sh);
            gems_st16(dst, GEMS_G(1));
            GEMS_G(2) = (uint32_t)((int32_t)(0u - GEMS_G(2)) >> sh);
            gems_st16(dst + 8, GEMS_G(2));
        }
        dst += 12; end += 12; start += 12; ang += 6;
    }

    /* Position steps: (end - start) / 2^sh, the divide done by the COP. */
    gems_cop_w(0x0B801717);
    gems_cop_w(1u << sh);
    float inv = 1.0f / gems_cop_rf();
    for (uint32_t i = 0; i < 24; i++, dst += 4, end += 4, start += 4) {
        float d = gems_ldf(end) - gems_ldf(start);
        gems_stf(dst, inv * d);
    }
    gems_cop_w(0x01000202);

out:
    if (entry) gems_i960_ret();
    return 0;
}
