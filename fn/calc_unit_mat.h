/* calc_unit_mat (i960 0x16C44, Gems 8003B244): one fighter's (g7) unit
 * matrices. Walks the skeleton on the COP's matrix stack (0x01 push, 0x02
 * pop, 0x03 translate, 0x08/0x09/0x0A turn, 0x07 scale, 0x3F turn by three
 * angles, 0x6B two-bone IK, 0x13 the 0x12/0x11 view save/restore through
 * 0x501024), stores each of the 16 parts with 0x35 (TGP slot 12 * part),
 * reads its world position back with 0x0F into rob+0x1F4 + 12 * part and
 * sends that part's collision balls (set_coli_ball_data). Scales are
 * dented_cnt + scale_parts_cnt + the global scale at 0x509518.
 *
 * Outside the pause (0x508000 bit 5): first copies the last unit matrix to
 * the COP (0x7F), and while rob+0x70C bit 23 is set eases the held body
 * position rob+0x1E28 towards rob+0x1F4 (frames 14-23 of rob+0x1AA) or
 * lets it go (24 on); afterwards keeps the previous position at
 * rob+0x6FC / 0x1E0C and asks the COP for the step length (0x2D). */
#pragma once

/* A turn: the command, then the angle as the i960 loads it (ldis). */
static inline void calc_unit_mat_ang(uint32_t cmd, uint32_t off)
{
    gems_cop_w(cmd);
    gems_cop_w((uint32_t)gems_ld16s(GEMS_G(7) + off));
}

/* 0x06 Fn_trans by the three words at rob+off. */
static inline void calc_unit_mat_trans(uint32_t off)
{
    uint32_t v[3];
    gems_cop_w(0x03000606);
    gems_ldn(v, GEMS_G(7) + off, 3);
    gems_cop_wn(v, 3);
}

/* 0x13 with the command word as both arguments, as the i960 sends it; the
 * reply is dropped. */
static inline void calc_unit_mat_13(void)
{
    gems_cop_w(0x09801313);
    gems_cop_w(0x09801313);
    gems_cop_w(0x09801313);
    (void)gems_cop_r();
}

/* 0x6B IK: the slot (P1 0x3A00, P2 0x3B00, by rob+4 bit 0) + off, then the
 * angle at rob+0x26. */
static inline void calc_unit_mat_ik(uint32_t off)
{
    gems_cop_w(0x34806969);
    gems_cop_w(((gems_ld8(GEMS_G(7) + 4) & 1) ? 0x3B00u : 0x3A00u) + off);   /* ldob */
    gems_cop_w((uint32_t)gems_ld16s(GEMS_G(7) + 0x26));                     /* ldis */
}

/* 0x3F: the three angles at rob+a, b, c (ldis); its reply is dropped. */
static inline void calc_unit_mat_3f(uint32_t a, uint32_t b, uint32_t c)
{
    uint32_t v[3];
    gems_cop_w(0x1F803F3F);
    v[0] = (uint32_t)gems_ld16s(GEMS_G(7) + a);
    v[1] = (uint32_t)gems_ld16s(GEMS_G(7) + b);
    v[2] = (uint32_t)gems_ld16s(GEMS_G(7) + c);
    gems_cop_wn(v, 3);
    (void)gems_cop_r();
}

/* 0x11 with the nine words 0x13/0x12 saved at 0x501024. */
static inline void calc_unit_mat_11(void)
{
    uint32_t v[3];
    gems_cop_w(0x08801111);
    for (uint32_t k = 0; k < 3; k++) {
        gems_ldn(v, 0x501024u + 12u * k, 3);
        gems_cop_wn(v, 3);
    }
}

/* Store part n: 0x35 into the fighter's slot 12 * n, read the world
 * position (0x0F) into rob+0x1F4 + 12 * n, then the part's balls. */
static inline void calc_unit_mat_part(uint32_t n)
{
    uint32_t v[3];
    gems_cop_w(0x1A803535);
    gems_cop_w(gems_ld8(GEMS_G(7) + 4));                                    /* ldob */
    gems_cop_w(12u * n);
    gems_cop_w(0x07800F0F);
    gems_cop_rn(v, 3);
    gems_stn(GEMS_G(7) + 0x1F4u + 12u * n, v, 3);
    GEMS_G(0) = n;
    gfn_set_coli_ball_data(0);
}

/* Scale for a dented part: dented_cnt(part at rob+dent) + scale_parts_cnt(
 * part) + the global scale vector at 0x509518, left in g0-g2 and sent as
 * 0x07. */
static inline void calc_unit_mat_dent_scale(uint32_t dent, uint32_t part)
{
    float s[3];
    GEMS_G(3) = GEMS_G(7) + dent;
    gfn_dented_cnt(0);
    for (int k = 0; k < 3; k++) s[k] = gems_u2f(GEMS_G(k));
    GEMS_G(3) = part;
    gfn_scale_parts_cnt(0);
    for (int k = 0; k < 3; k++) s[k] = s[k] + gems_u2f(GEMS_G(k));          /* fadds */
    gems_ldn(&GEMS_G(0), 0x509518, 3);
    for (int k = 0; k < 3; k++) GEMS_G(k) = gems_f2u(s[k] + gems_u2f(GEMS_G(k)));
    gems_cop_w(0x03800707);
    gems_cop_wn(&GEMS_G(0), 3);
}

/* Scale for a limb: scale_parts_cnt(part) + the one word at 0x509518 on
 * every axis. part is base -/+ rob+0 bit 6 (the fighter faces the other
 * way: left and right swap). */
static inline void calc_unit_mat_limb_scale(uint32_t part)
{
    GEMS_G(3) = part;
    gfn_scale_parts_cnt(0);
    float s = gems_ldf(0x509518);
    for (int k = 0; k < 3; k++) GEMS_G(k) = gems_f2u(s + gems_u2f(GEMS_G(k)));   /* fadds */
    gems_cop_w(0x03800707);
    gems_cop_wn(&GEMS_G(0), 3);
}

static uint32_t gfn_calc_unit_mat(int entry)
{
    uint32_t rob = GEMS_G(7);

    if (!(gems_ld32(0x508000) & 0x20)) {
        gems_cop_w(0x3F807F7F);                                             /* 0x7F */
        gems_cop_w(gems_ld8(rob + 4));                                      /* ldob */
        if (gems_ld32(rob + 0x70C) & 0x800000) {
            uint32_t t = gems_ld16(rob + 0x1AA);                            /* ldos */
            if (t < 2) {
                uint32_t v[3];
                gems_ldn(v, rob + 0x1F4, 3);
                gems_stn(rob + 0x1E28, v, 3);
            } else if (t >= 14) {
                if (t >= 24) {
                    gems_st32(rob + 0x70C, gems_ld32(rob + 0x70C) & ~0x800000u);
                } else {
                    uint32_t nu[3], ou[3];
                    gems_ldn(nu, rob + 0x1F4, 3);
                    gems_ldn(ou, rob + 0x1E28, 3);
                    float n0 = gems_u2f(nu[0]), n1 = gems_u2f(nu[1]);
                    float o0 = gems_u2f(ou[0]), o1 = gems_u2f(ou[1]), o2 = gems_u2f(ou[2]);
                    float d0 = n0 - o0, d1 = n1 - o1;
                    float d2 = o2 - o2;          /* the ROM's subr r10,r10,r6: z holds */
                    float m0 = 0.4f * d0, m1 = 0.4f * d1, m2 = 0.4f * d2;
                    nu[0] = gems_f2u(o0 + m0);
                    nu[1] = gems_f2u(o1 + m1);
                    nu[2] = gems_f2u(o2 + m2);
                    gems_stn(rob + 0x1E28, nu, 3);
                }
            }
        }
    }

    /* The body: 0x38 for this fighter, then the root at (0, rob+0x678, 0). */
    gems_cop_w(0x1C003838);
    gems_cop_w(gems_ld8(rob + 4));                                          /* ldob */
    calc_unit_mat_13();
    gems_cop_w(0x00800101);
    gems_cop_w(0x01800303);
    gems_cop_w(0x07000E0E);
    gems_cop_w(0);
    gems_cop_w(gems_ld32(rob + 0x678));
    gems_cop_w(0);
    calc_unit_mat_trans(0x18);
    calc_unit_mat_ang(0x04800909, 0x26);
    calc_unit_mat_13();
    {
        uint32_t v[3];
        gems_cop_w(0x09001212);
        for (uint32_t k = 0; k < 3; k++) {
            gems_cop_rn(v, 3);
            gems_stn(0x501024u + 12u * k, v, 3);
        }
    }

    /* Part 0, the waist. */
    if (!(gems_ld32(rob) & 4)) {
        uint32_t v[3];
        gems_cop_w(0x03000606);
        gems_ldn(v, rob + 0x80, 3);
        v[1] = gems_f2u(gems_ldf(rob + 0xC50) + gems_u2f(v[1]));           /* fadds */
        gems_cop_wn(v, 3);
    } else if (gems_ld8(rob + 0x83F) & 1) {                                 /* ldob */
        gems_cop_w(0x03000606);
        gems_cop_w(gems_ld32(rob + 0x80));
        gems_cop_w(0);
        gems_cop_w(gems_ld32(rob + 0x88));
    }
    calc_unit_mat_13();
    calc_unit_mat_ik(0x00);
    calc_unit_mat_part(0);

    /* Part 1, the chest. */
    gems_cop_w(0x00800101);
    calc_unit_mat_trans(0x8C);
    calc_unit_mat_ik(0x0C);
    calc_unit_mat_ang(0x04000808, 0xBE6);
    calc_unit_mat_dent_scale(0x1FEC, 1);
    calc_unit_mat_part(1);

    /* Part 2, the head. */
    gems_cop_w(0x00800101);
    calc_unit_mat_trans(0x98);
    calc_unit_mat_3f(0xBB2, 0xBB0, 0xBAE);
    calc_unit_mat_ang(0x04800909, 0xC0E);
    calc_unit_mat_ang(0x04000808, 0xC0C);
    calc_unit_mat_ang(0x05000A0A, 0xC10);
    calc_unit_mat_3f(0x150, 0x14E, 0x14C);
    calc_unit_mat_ang(0x04000808, 0xBE6);
    calc_unit_mat_dent_scale(0x1F88, 0);
    calc_unit_mat_part(2);
    gems_cop_w(0x01000202);

    /* Parts 3-5, one arm. */
    gems_cop_w(0x00800101);
    calc_unit_mat_trans(0xA4);
    calc_unit_mat_ik(0x24);
    calc_unit_mat_part(3);
    calc_unit_mat_trans(0xB0);
    calc_unit_mat_ik(0x30);
    calc_unit_mat_part(4);
    calc_unit_mat_trans(0xBC);
    calc_unit_mat_ang(0x05000A0A, 0x15E);
    calc_unit_mat_ang(0x04800909, 0x15C);
    calc_unit_mat_ang(0x04000808, 0x15A);
    calc_unit_mat_ang(0x04800909, 0xBEA);
    calc_unit_mat_ang(0x04000808, 0xBE8);
    calc_unit_mat_ang(0x05000A0A, 0xBEC);
    calc_unit_mat_limb_scale(3u - ((gems_ld32(rob) >> 6) & 1));
    calc_unit_mat_part(5);
    gems_cop_w(0x01000202);

    /* Parts 6-8, the other arm. */
    calc_unit_mat_trans(0xC8);
    calc_unit_mat_ik(0x48);
    calc_unit_mat_part(6);
    calc_unit_mat_trans(0xD4);
    calc_unit_mat_ik(0x54);
    calc_unit_mat_part(7);
    calc_unit_mat_trans(0xE0);
    calc_unit_mat_ang(0x05000A0A, 0x16C);
    calc_unit_mat_ang(0x04800909, 0x16A);
    calc_unit_mat_ang(0x04000808, 0x168);
    calc_unit_mat_ang(0x04800909, 0xBF0);
    calc_unit_mat_ang(0x04000808, 0xBEE);
    calc_unit_mat_ang(0x05000A0A, 0xBF2);
    calc_unit_mat_limb_scale(2u + ((gems_ld32(rob) >> 6) & 1));
    calc_unit_mat_part(8);
    gems_cop_w(0x01000202);

    /* Back to the waist: the held position while rob+0x70C bit 23. */
    if (gems_ld32(rob + 0x70C) & 0x800000) {
        uint32_t v[3];
        gems_ldn(v, rob + 0x1E28, 3);
        gems_cop_w(0x07000E0E);
        gems_cop_wn(v, 3);
    }
    calc_unit_mat_13();

    /* Part 9, the hips. */
    calc_unit_mat_trans(0xEC);
    calc_unit_mat_3f(0xBC4, 0xBC2, 0xBC0);
    calc_unit_mat_ang(0x04800909, 0xC20);
    calc_unit_mat_ang(0x04000808, 0xC1E);
    calc_unit_mat_ang(0x05000A0A, 0xC22);
    calc_unit_mat_ang(0x05000A0A, 0x172);
    calc_unit_mat_ang(0x04800909, 0x170);
    calc_unit_mat_part(9);
    /* (the ROM loads rob+0x190 into r3 here and never uses it) */
    calc_unit_mat_13();
    gems_cop_w(0x00800101);
    calc_unit_mat_13();

    /* Parts 10-12, one leg. */
    calc_unit_mat_trans(0xF8);
    calc_unit_mat_ik(0x78);
    calc_unit_mat_part(10);
    calc_unit_mat_trans(0x104);
    calc_unit_mat_ik(0x84);
    calc_unit_mat_part(11);
    calc_unit_mat_13();
    calc_unit_mat_trans(0x110);
    calc_unit_mat_11();
    calc_unit_mat_ang(0x05000A0A, 0x180);
    calc_unit_mat_ang(0x04800909, 0x17E);
    calc_unit_mat_ang(0x04000808, 0x17C);
    calc_unit_mat_ang(0x04800909, 0xBF6);
    calc_unit_mat_ang(0x04000808, 0xBF4);
    calc_unit_mat_ang(0x05000A0A, 0xBF8);
    calc_unit_mat_limb_scale(5u - ((gems_ld32(rob) >> 6) & 1));
    calc_unit_mat_part(12);
    gems_cop_w(0x01000202);

    /* Parts 13-15, the other leg. */
    calc_unit_mat_trans(0x11C);
    calc_unit_mat_ik(0x9C);
    calc_unit_mat_part(13);
    calc_unit_mat_trans(0x128);
    calc_unit_mat_ik(0xA8);
    calc_unit_mat_part(14);
    calc_unit_mat_13();
    calc_unit_mat_trans(0x134);
    calc_unit_mat_11();
    calc_unit_mat_ang(0x05000A0A, 0x18E);
    calc_unit_mat_ang(0x04800909, 0x18C);
    calc_unit_mat_ang(0x04000808, 0x18A);
    calc_unit_mat_ang(0x04800909, 0xBFC);
    calc_unit_mat_ang(0x04000808, 0xBFA);
    calc_unit_mat_ang(0x05000A0A, 0xBFE);
    calc_unit_mat_limb_scale(4u + ((gems_ld32(rob) >> 6) & 1));
    calc_unit_mat_part(15);
    gems_cop_w(0x01000202);

    gfn_rob_ball_data_make(0);

    if (!(gems_ld32(0x508000) & 0x20)) {
        uint32_t nu[3], ou[3];
        gems_ldn(nu, rob + 0x1F4, 3);
        gems_ldn(ou, rob + 0x6FC, 3);
        gems_stn(rob + 0x1E0C, ou, 3);
        gems_stn(rob + 0x6FC, nu, 3);
        float dx = gems_u2f(nu[0]) - gems_u2f(ou[0]);                       /* fsubs */
        float dz = gems_u2f(nu[2]) - gems_u2f(ou[2]);
        /* (the ROM loads rob+0x1E00 here and never uses it) */
        if (gems_ld8(0x50002B) == 9 && gems_ld8(0x500031) == 9               /* ldob */
            && (gems_ld32(rob) & 0x80)) {
            gems_cop_w(0x16802D2D);
            gems_cop_wf(dx);
            gems_cop_wf(dz);
            (void)gems_cop_r();
            /* The ROM then tests the step against 0.7 and rob+0x1A4's bits,
             * and only its condition code comes of it; Gems leaves the AC alone. */
        }
    }

    if (entry) gems_i960_ret();
    return 0;
}
