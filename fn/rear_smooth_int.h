/* rear_smooth_int (i960 0x2F878; Gems 8003E964): start the smoothing of a
 * fighter's turn-round.  The pose the motion ends on goes to 0x50E0F0
 * (get_end_value), the rear pose (motion + 0x1E0/0x2D0/0x3C0/0x5A0/0x4B0 by
 * the kind at rob+0x814) to 0x50E000; both are made relative to their entry
 * 12, mirrored and turned as the fighter is, and the per-part angle and
 * translation steps go to motion + 0xF0.
 *
 * rob+0xBE4 == 1 only clears the state.  The i960 stores its r3 to rob+0xBE5
 * there (Gems stores an uninitialised stack byte); GEMS_R(3) is used. */
#pragma once

/* 8 entries of 12 bytes from base: x -= x0 and z -= z0 through COP 0x14
 * (skipped when the base's own x and z are both 0). */
static void rear_smooth_int_rebase(uint32_t base)
{
    uint32_t x0 = gems_ld32(base);
    uint32_t z0 = gems_ld32(base + 8);
    if ((x0 | z0) == 0)
        return;
    uint32_t p = base;
    for (int n = 0; n < 8; n++, p += 12) {
        uint32_t v = gems_ld32(p);
        gems_cop_w(0x0A001414);
        gems_cop_w(v);
        gems_cop_w(x0);
        gems_st32(p, gems_cop_r());
        v = gems_ld32(p + 8);
        gems_cop_w(0x0A001414);
        gems_cop_w(v);
        gems_cop_w(z0);
        gems_st32(p + 8, gems_cop_r());
    }
}

static uint32_t gfn_rear_smooth_int(int entry)
{
    uint32_t rob = GEMS_G(7);

    if (gems_ld8(rob + 0xBE4) == 1) {
        gems_st8(rob + 0xBE5, (uint8_t)GEMS_R(3));
        gems_st8(rob + 0xBDD, 0);
        gems_st8(rob + 0xBE4, 0);
        if (entry) gems_i960_ret();
        return 0;
    }

    uint32_t flags   = gems_ld32(rob + 0x1A4);
    uint32_t motion  = gems_ld32(rob + 0xBD8);
    uint32_t start_t = gems_ld16(rob + 0xBE0);

    GEMS_G(5) = 0x50E0F0;
    gfn_get_end_value(0);

    switch (gems_ld8(rob + 0x814)) {
    case 0x00: GEMS_G(0) = motion + 0x1E0; break;
    case 0x80: GEMS_G(0) = motion + 0x2D0; break;
    case 0x03: GEMS_G(0) = motion + 0x3C0; break;
    case 0x04: GEMS_G(0) = motion + 0x5A0; break;
    case 0x83: GEMS_G(0) = motion + 0x4B0; break;
    default:
        gems_st8(rob + 0xBDD, (uint8_t)(gems_ld8(rob + 0xBDD) & ~2u));
        if (entry) gems_i960_ret();
        return 0;
    }

    /* The rear pose: 20 triples to 0x50E000. */
    uint32_t dst = 0x50E000;
    for (int n = 0; n < 20; n++) {
        uint32_t t[3];
        gems_ldn(t, GEMS_G(0), 3);
        gems_stn(dst, t, 3);
        GEMS_G(0) += 12;
        dst += 12;
    }

    rear_smooth_int_rebase(0x50E180);
    rear_smooth_int_rebase(0x50E090);

    uint32_t mirror = 0;
    if (gems_ld32(rob + 0x804) & 0x200000) mirror ^= 1;
    if (gems_ld32(rob) & 0x40)             mirror ^= 1;
    if (mirror & 1) {
        GEMS_G(5) = 0x50E000;
        gfn_set_mirror(0);
    }

    if (flags & 0x40) {
        int32_t ang = gems_ld16s(rob + 0x812);
        gems_cop_w(0x00800101);                    /* push */
        gems_cop_w(0x01800303);
        gems_cop_w(0x04800909);                    /* y_rot */
        gems_cop_w((uint32_t)ang);
        uint32_t p = 0x50E090;
        for (GEMS_G(0) = 8; GEMS_G(0) != 0; GEMS_G(0)--, p += 12) {
            uint32_t x = gems_ld32(p);
            uint32_t z = gems_ld32(p + 8);
            gems_cop_w(0x14802929);                /* transform point */
            gems_cop_w(x);
            gems_cop_w(0);
            gems_cop_w(z);
            x = gems_cop_r();
            (void)gems_cop_r();
            z = gems_cop_r();
            gems_st32(p, x);
            gems_st32(p + 8, z);
        }
        gems_cop_w(0x01000202);                    /* pop */
        gems_st16(0x50E034, (uint16_t)(ang + gems_ld16s(0x50E034)));
        gems_st16(0x50E028, (uint16_t)(ang + gems_ld16s(0x50E028)));
        gems_st16(0x50E01C, (uint16_t)(ang + gems_ld16s(0x50E01C)));
    }

    gfn_unit_smooth_cancel_rear(0, 0x50E000, 0x50E0F0, gems_ld32(rob + 0x860));

    GEMS_G(6) = gems_ld16(rob + 0x800);
    uint32_t span = GEMS_G(6) - start_t;
    uint32_t shift = span == 8 ? 3 : span == 4 ? 2 : 1;

    gems_cop_w(0x00800101);                        /* push */
    uint32_t end  = 0x50E0F0;
    uint32_t rear = 0x50E000;
    uint32_t out  = motion + 0xF0;
    uint32_t base_ang = rob + 0xBE8;
    GEMS_G(4) = gems_ld32(rob);
    (void)gems_ld32(rob + 0x1A4);
    (void)gems_ld32(rob + 0xC30);
    GEMS_G(3) = 0;

    /* 12 parts: the angle step from the end pose to the rear pose. */
    for (int n = 0; n < 12; n++) {
        gems_cop_w(0x2A805555);                    /* get_sm_ang_r */
        GEMS_G(0) = (uint32_t)gems_ld16s(base_ang + 4);
        GEMS_G(1) = (uint32_t)gems_ld16s(base_ang);
        GEMS_G(2) = (uint32_t)gems_ld16s(base_ang + 2);
        gems_cop_wn(&GEMS_G(0), 3);
        GEMS_G(0) = 0u - (uint32_t)gems_ld16s(end);
        GEMS_G(1) = 0u - (uint32_t)gems_ld16s(end + 4);
        GEMS_G(2) = 0u - (uint32_t)gems_ld16s(end + 8);
        gems_cop_wn(&GEMS_G(0), 3);
        GEMS_G(0) = (uint32_t)gems_ld16s(rear + 8);
        GEMS_G(1) = (uint32_t)gems_ld16s(rear + 4);
        GEMS_G(2) = (uint32_t)gems_ld16s(rear);
        gems_cop_wn(&GEMS_G(0), 3);
        GEMS_G(0) = (uint32_t)(int16_t)gems_cop_r();
        GEMS_G(1) = (uint32_t)(int16_t)gems_cop_r();
        GEMS_G(2) = (uint32_t)(int16_t)gems_cop_r();
        GEMS_G(0) = (uint32_t)((int32_t)(0u - GEMS_G(0)) >> shift);   /* shri */
        gems_st16(out + 4, (uint16_t)GEMS_G(0));
        GEMS_G(1) = (uint32_t)((int32_t)GEMS_G(1) >> shift);
        gems_st16(out, (uint16_t)GEMS_G(1));
        GEMS_G(2) = (uint32_t)((int32_t)(0u - GEMS_G(2)) >> shift);
        gems_st16(out + 8, (uint16_t)GEMS_G(2));
        out += 12;
        rear += 12;
        end += 12;
        base_ang += 6;
    }

    /* The rest (24 words): (rear - end) / 2^shift, float. */
    gems_cop_w(0x0B801717);
    gems_cop_w(1u << shift);
    float inv = 1.0f / gems_cop_rf();
    for (int n = 0; n < 24; n++) {
        float e = gems_ldf(end);
        float r = gems_ldf(rear);
        gems_stf(out, inv * (r - e));
        out += 4;
        rear += 4;
        end += 4;
    }
    gems_cop_w(0x01000202);                        /* pop */

    if (entry) gems_i960_ret();
    return 0;
}
