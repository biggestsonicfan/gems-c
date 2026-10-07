/* get_frame_dat (i960 0x304C8, trap 0x6C; Gems 8003D548): evaluate the
 * fighter's motion at its frame (rob+0x1AA).
 *  - The motion's f-curves (get_fcurve_value_f, g5 = motion + 0x690, g6 = the
 *    frame as a float): at half a frame on when the slow-motion flag is up,
 *    or through the frame remap table at rob+0x854, which is pairs of
 *    (frame, mapped frame) bytes interpolated linearly; frame 1 of a motion
 *    takes its start values instead (get_start_value).
 *  - The blend at rob+0xBDD: the part angles (rob+0xBE8, 36 halfwords) and
 *    the rest (motion + 0x720, 24 words) fade the previous motion's offsets
 *    in or out over rob+0xBDE frames, or rear_smooth_int's turn-round.
 *  - Keeps the feet above the floor, copies the result into the rob, and
 *    turns the head towards the enemy (COP 0x63).
 * The remap search starts from the i960's r4/r7 as "previous key"; Gems uses
 * uninitialised stack words there, GEMS_R(4)/GEMS_R(7) are what the ROM has.
 *
 * Not Gems': the fade-out loop's multiply (i960 0x30608, r11 = the offset,
 * r12 = 24 - n) is where m2-hle2's sfight_console profile zeroes the head
 * tilt, as Sega's console DLL does. The loop runs that hook when the active
 * profile has one (gems_inner), so the trap serves both profiles. The
 * turn-round's loop (0x306A4) is a copy of its own, and nobody hooks it. */
#pragma once

/* motion+0x720 += k * src[n] for 24 words, k = (float)g6 from COP 0x17.
 * inner: a profile's hook on the loop's mulr, or NULL. */
static void get_frame_dat_blend_rest(uint32_t src, uint32_t motion, gems_inner_fn inner)
{
    uint32_t dst = motion + 0x720;
    gems_cop_w(0x0B801717);
    gems_cop_w(GEMS_G(6));
    float k = gems_cop_rf();
    for (int n = 0; n < 24; n++, src += 4, dst += 4) {
        float v = gems_ldf(src);
        float w = gems_ldf(dst);
        if (inner) {
            GEMS_R(10) = gems_f2u(w);
            GEMS_R(11) = gems_f2u(v);
            GEMS_R(12) = (uint32_t)(24 - n);
            gems_inner_call(inner);
            v = gems_u2f(GEMS_R(11));
        }
        float kv = k * v;
        float o = w + kv;
        gems_stf(dst, o);
    }
}

/* rob+0xBE8[n] = g6 * src[n] (36 signed halfwords), then the rest. */
static void get_frame_dat_blend(uint32_t rob, uint32_t src, uint32_t motion, gems_inner_fn inner)
{
    uint32_t dst = rob + 0xBE8;
    for (int n = 0; n < 36; n++, src += 4, dst += 2)
        gems_st16(dst, (uint16_t)(GEMS_G(6) * (uint32_t)gems_ld16s(src)));
    get_frame_dat_blend_rest(src, motion, inner);
}

/* COP 0x63: the head's turn towards target (12 bytes at tgt) -> rob+0xB0C. */
static void get_frame_dat_look(uint32_t rob, uint32_t ang, uint32_t tgt)
{
    uint32_t v[4];
    gems_cop_w(0x31806363);
    gems_ldn(v, rob + 0x18, 3);
    v[3] = ang;
    gems_cop_wn(v, 4);
    gems_ldn(v, tgt, 3);
    gems_cop_wn(v, 3);
    gems_cop_rn(v, 3);
    gems_stn(rob + 0xB0C, v, 3);
}

static uint32_t gfn_get_frame_dat(int entry)
{
    uint32_t rob = GEMS_G(7);
    GEMS_G(6) = gems_ld16(rob + 0x1AA);
    uint32_t motion = gems_ld32(rob + 0xBD8);
    GEMS_G(5) = motion + 0x690;

    /* ---- the f-curves ---- */
    if ((gems_ld32(0x500068) & 0x20000) && (gems_ld16(0x500092) & 1)
        && GEMS_G(6) != gems_ld16(rob + 0x800)) {
        float f = (float)GEMS_G(6);
        f = f + 0.5f;
        GEMS_G(6) = gems_f2u(f);
        gfn_get_fcurve_value_f(0);
    } else {
        uint32_t p = gems_ld32(rob + 0x854);
        bool exact = (p == 0);
        if (!exact) {
            uint32_t key = GEMS_R(4), val = GEMS_R(7), prev_key, prev_val;
            for (;;) {
                prev_key = key;
                key = gems_ld8(p);
                if (key == GEMS_G(6)) {
                    exact = true;
                    break;
                }
                prev_val = val;
                val = gems_ld8(p + 1);
                p += 2;
                if (!(key < GEMS_G(6)))
                    break;
            }
            if (!exact) {
                uint32_t span = key - prev_key;
                uint32_t num  = (val - prev_val) * (GEMS_G(6) - prev_key) + span * prev_val;
                float fn = (float)num;
                float fd = (float)span;
                GEMS_G(6) = gems_f2u(fn / fd);
                gfn_get_fcurve_value_f(0);
            }
        }
        if (exact) {
            if (GEMS_G(6) == 1 && gems_ld8(rob + 0x19F) != 20) {
                if (!(gems_ld8(rob + 0xBDD) & 1))
                    gfn_get_start_value(0);
            } else {
                GEMS_G(6) = gems_f2u((float)GEMS_G(6));
                gfn_get_fcurve_value_f(0);
            }
        }
    }

    /* ---- the blend ---- */
    GEMS_G(6) = gems_ld16(rob + 0x1AA);
    if (gems_ld32(rob + 0x804) & 0x20010) {
        GEMS_G(6) = gems_ld16(rob + 0xBE2) + 1;
        gems_st16(rob + 0xBE2, (uint16_t)GEMS_G(6));
    }

    uint32_t mode = gems_ld8(rob + 0xBDD);
    bool cleared = false;
    if (mode == 0) {
        cleared = true;
    } else {
        uint32_t len = 0;
        if (mode != 2)
            len = gems_ld16(rob + 0xBDE);
        if (mode != 2 && len > GEMS_G(6)) {
            /* fading out the previous motion's offsets */
            GEMS_G(6) = len - GEMS_G(6);
            get_frame_dat_blend(rob, motion, motion, gems_inner(0x00030608));
        } else {
            uint32_t frame = gems_ld16(rob + 0x1AA);
            uint32_t start = 0;
            if (mode != 1)
                start = gems_ld16(rob + 0xBE0);
            if (mode == 1 || start >= frame) {
                cleared = true;
            } else {
                /* the turn-round */
                uint32_t st = gems_ld8(rob + 0xBE5);
                if (!(st & 1)) {
                    gfn_rear_smooth_int(0);
                    st |= 1;
                }
                GEMS_G(6) = frame - start;
                if (gems_ld16(rob + 0xBDE) - 1 == GEMS_G(6))
                    st |= 2;
                gems_st8(rob + 0xBE5, (uint8_t)st);
                get_frame_dat_blend(rob, motion + 0xF0, motion, NULL);
            }
        }
    }
    if (cleared) {
        static const uint32_t zero[4] = { 0, 0, 0, 0 };
        uint32_t dst = rob + 0xBE8;
        for (int n = 0; n < 4; n++, dst += 16)
            gems_stn(dst, zero, 4);
        gems_stn(dst, zero, 2);
    }

    /* ---- keep the feet above the floor ---- */
    if (gems_ld32(rob) & 4) {
        float dy = gems_ldf(rob + 0x1C) - gems_ldf(motion + 0x724);
        uint32_t x = gems_ld32(rob + 0x18);
        uint32_t z = gems_ld32(rob + 0x20);
        float floor_y = gems_ldf(0x50A010);
        float lim     = gems_ldf(0x50A00C);
        if (gems_u2f(x & 0x7FFFFFFFu) < lim && gems_u2f(z & 0x7FFFFFFFu) < lim)
            floor_y = 0.0f;
        float lo = 0.1f + floor_y;
        uint32_t p = motion + 0x730;
        for (int n = 0; n < 7; n++, p += 12) {
            float v = gems_ldf(p);
            float y = v + dy;
            if (!(lo <= y)) {
                float d = lo - y;
                gems_stf(p, v + d);
            }
        }
    }

    /* ---- copy the evaluated motion into the rob ---- */
    uint32_t src = motion + 0x690;
    uint32_t t[3];
    for (uint32_t i = 0; i < 4; i++) {
        uint32_t a = rob + gems_ld32(0x316D4 + i * 4);
        gems_ldn(t, src, 3);
        src += 12;
        gems_st16(a,     (uint16_t)t[0]);
        gems_st16(a + 2, (uint16_t)t[1]);
        gems_st16(a + 4, (uint16_t)t[2]);
    }
    gems_ldn(t, src, 3);
    src += 12;
    gems_st16(rob + 0x140, (uint16_t)t[0]);
    gems_st16(rob + 0x142, (uint16_t)t[1]);
    gems_st16(rob + 0x144, (uint16_t)t[2]);
    for (GEMS_G(5) = rob + 0xBA8; GEMS_G(5) != rob + 0xBA8 + 7 * 6; GEMS_G(5) += 6) {
        gems_ldn(t, src, 3);
        src += 12;
        gems_st16(GEMS_G(5),     (uint16_t)t[0]);
        gems_st16(GEMS_G(5) + 2, (uint16_t)t[1]);
        gems_st16(GEMS_G(5) + 4, (uint16_t)t[2]);
    }
    gems_ldn(t, src, 3);
    src += 12;
    gems_stn(rob + 0x80, t, 3);
    for (GEMS_G(5) = rob + 0xB00; GEMS_G(5) != rob + 0xB00 + 7 * 12; GEMS_G(5) += 12) {
        gems_ldn(t, src, 3);
        src += 12;
        gems_stn(GEMS_G(5), t, 3);
    }

    /* ---- the head looks at the enemy ---- */
    gems_st8(rob + 0x7D3, 0);
    if (gems_ld32(rob) & 0x80000) {
        uint32_t enemy = gems_ld32(0x500814);
        get_frame_dat_look(rob, (uint32_t)gems_ld16s(rob + 0x26), enemy + 0x41C);
    } else if (!(gems_ld8(0x50002C) & 0x80)
               && !(gems_ld32(0x500034) & 0x3033C030u)
               && !(gems_ld32(rob) & 0xA00000)
               && !(gems_ld32(rob) & 0x100)
               && !(gems_ld32(rob + 0x1A4) & 0x0802C119u)) {
        gems_st8(rob + 0x7D3, 1);
        uint32_t enemy = gems_ld32(0x500814);
        uint32_t e_dir = gems_ld16(enemy + 0x26);
        uint32_t m_dir = gems_ld16(rob + 0x26);
        int32_t  q     = (int32_t)((m_dir - e_dir) << 16) / 4;
        uint32_t off   = (uint32_t)(q >> 16);       /* shri: arithmetic, as the i960 */
        uint32_t ang   = gems_ld16(rob + 0x26) - off;
        get_frame_dat_look(rob, ang, GEMS_G(8) + 0x20C);
        if (gems_ld32(rob) & 0x80000) {
            uint32_t en = gems_ld32(0x500814);
            get_frame_dat_look(rob, (uint32_t)gems_ld16s(rob + 0x26), en + 0x41C);
        } else if (gems_ld32(rob) & 0x20000) {
            uint32_t en = gems_ld32(0x500814);
            uint32_t a[3], b[3], m[3];
            gems_ldn(a, en + 0x18, 3);
            gems_ldn(b, GEMS_G(8) + 0x20C, 3);
            for (int i = 0; i < 3; i++) {
                float s = gems_u2f(a[i]) + gems_u2f(b[i]);
                m[i] = gems_f2u(s * 0.5f);
            }
            gems_stn(en + 0x41C, m, 3);
            get_frame_dat_look(rob, (uint32_t)gems_ld16s(rob + 0x26), en + 0x41C);
        }
    }

    if (entry) gems_i960_ret();
    return 0;
}
