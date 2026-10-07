/* calc_rob_angle_cont (i960 0x2FF2C, a trap site; Gems 80039844): the
 * fighter g7's base and chest matrices, then its four limbs by two-bone IK.
 *
 * 0x62 (scale, angles 0x144/0x142/0x140, 0xC02/0xC00/0xC04), then 0x67 into
 * the waist slot, the leg/arm parts through calc_unit_1, and for each limb
 * 0x6B (Fn_ik: the record at g6, the angles at g5 and g3, the point at g4,
 * the two bone lengths, the lower and upper slot, the side flag) with its one
 * reply dropped. Each IK steps g6/g5/g4/g3/g1/g0 to the next limb, as the
 * i960 does. Last, the move's reach scale (table 0x31354) into +0xC50 and
 * +0x1A8 copied to +0xC4C.
 *
 * The i960 waits for the COP before the matrices, each calc_unit_1 and
 * each IK (twice) with a 0x09801313 whose words are the command itself and
 * whose reply it drops. Gems sends none; this sends them, as the i960 does. */

static inline void calc_rob_angle_cont_sync(void)
{
    gems_cop_w(0x09801313u);
    gems_cop_w(0x09801313u);
    gems_cop_w(0x09801313u);
    (void)gems_cop_r();
}

/* One limb's Fn_ik (0x35806B6B). `slots` is the lower slot << 16 | upper
 * slot for player 1; player 2's are 0x100 on (ldob 4(g7) bit 0). */
static void gfn_calc_rob_angle_cont_ik(uint32_t slots, uint32_t side)
{
    uint32_t rob = GEMS_G(7);
    uint32_t w[3];

    calc_rob_angle_cont_sync();
    calc_rob_angle_cont_sync();
    gems_cop_w(0x35806B6Bu);
    w[0] = gems_ld32(GEMS_G(6));
    w[1] = gems_ld32(GEMS_G(6) + 4);
    w[2] = gems_ld32(GEMS_G(6) + 8);
    gems_cop_wn(w, 3);
    w[0] = (uint32_t)gems_ld16s(GEMS_G(5) + 4);   /* ldis */
    w[1] = (uint32_t)gems_ld16s(GEMS_G(5) + 2);
    w[2] = (uint32_t)gems_ld16s(GEMS_G(5));
    gems_cop_wn(w, 3);
    w[0] = (uint32_t)gems_ld16s(GEMS_G(3) + 2);   /* ldis */
    w[1] = (uint32_t)gems_ld16s(GEMS_G(3));
    w[2] = (uint32_t)gems_ld16s(GEMS_G(3) + 4);
    gems_cop_wn(w, 3);
    w[0] = gems_ld32(GEMS_G(4));
    w[1] = gems_ld32(GEMS_G(4) + 4);
    w[2] = gems_ld32(GEMS_G(4) + 8);
    gems_cop_wn(w, 3);
    w[0] = gems_ld32(GEMS_G(6) + 0x18);
    w[1] = gems_ld32(GEMS_G(6) + 0xC);
    gems_cop_wn(w, 2);
    uint32_t p2 = (gems_ld8(rob + 4) & 1) ? 0x100u : 0;   /* ldob */
    gems_cop_w((slots >> 16) + p2);
    gems_cop_w((slots & 0xFFFFu) + p2);
    gems_cop_w(side);
    GEMS_G(6) += 0x24;
    GEMS_G(5) += 6;
    GEMS_G(4) += 0xC;
    GEMS_G(3) += 6;
    GEMS_G(1) += 1;
    GEMS_G(0) += 0xE;
    (void)gems_cop_r();
}

static uint32_t gfn_calc_rob_angle_cont(int entry)
{
    uint32_t rob = GEMS_G(7);
    uint32_t w[3];

    calc_rob_angle_cont_sync();
    gems_cop_w(0x31006262u);
    w[0] = gems_ld32(rob + 0x80);
    w[1] = gems_ld32(rob + 0x84);
    w[2] = gems_ld32(rob + 0x88);
    gems_cop_wn(w, 3);
    w[0] = (uint32_t)gems_ld16s(rob + 0x144);   /* ldis */
    w[1] = (uint32_t)gems_ld16s(rob + 0x142);
    w[2] = (uint32_t)gems_ld16s(rob + 0x140);
    gems_cop_wn(w, 3);
    w[0] = (uint32_t)gems_ld16s(rob + 0xC02);   /* ldis */
    w[1] = (uint32_t)gems_ld16s(rob + 0xC00);
    w[2] = (uint32_t)gems_ld16s(rob + 0xC04);
    gems_cop_wn(w, 3);

    gems_cop_w(0x33806767u);
    gems_cop_w((gems_ld8(rob + 4) & 1) ? 0x3B00u : 0x3A00u);

    /* The limb records: per character (+0x84C), per costume (+0x1B0). */
    GEMS_G(6) = gems_ld32(0xC2058u + gems_ld8(rob + 0x84C) * 4);   /* ldob */
    GEMS_G(6) = gems_ld32(GEMS_G(6) + gems_ld8(rob + 0x1B0) * 4);  /* ldob */
    if (gems_ld32(rob + 0x860) & 0x1000u)
        GEMS_G(6) = rob + 0x8C;
    GEMS_G(5) = rob + 0xBA8;
    GEMS_G(4) = rob + 0xB00;
    GEMS_G(1) = 1;
    GEMS_G(0) = rob + 0x146;
    GEMS_G(3) = rob + 0xC06;
    gems_cop_w(0x00800101u);
    calc_rob_angle_cont_sync();
    gfn_calc_unit_1(0);

    gems_cop_w(0x33806767u);
    gems_cop_w((gems_ld8(rob + 4) & 1) ? 0x3B0Cu : 0x3A0Cu);
    gems_cop_w(0x00800101u);
    calc_rob_angle_cont_sync();
    gfn_calc_unit_1(0);

    gems_cop_w(0x01000202u);
    gems_cop_w(0x00800101u);
    gfn_calc_rob_angle_cont_ik(0x3A303A24u, 0);
    gems_cop_w(0x01000202u);
    gfn_calc_rob_angle_cont_ik(0x3A543A48u, 0);
    gems_cop_w(0x01000202u);
    calc_rob_angle_cont_sync();
    gfn_calc_unit_1(0);
    gems_cop_w(0x00800101u);
    gfn_calc_rob_angle_cont_ik(0x3A843A78u, 1);
    gems_cop_w(0x01000202u);
    gfn_calc_rob_angle_cont_ik(0x3AA83A9Cu, 1);
    gems_cop_w(0x01000202u);

    /* The reach scale: 0 stays 0 (an integer test, as the i960's cmpobne),
     * otherwise 0.85 * scale * the fighter's scale (+0x84). */
    uint32_t tbl   = gems_ld32(0x31354u + gems_ld8(rob + 0x84C) * 4);   /* ldob */
    uint32_t reach = gems_ld32(tbl + gems_ld8(rob + 0x1B0) * 4);        /* ldob */
    if (reach == 0) {
        gems_st32(rob + 0xC50, reach);
    } else {
        float f = 0.85f * gems_u2f(reach);          /* fmuls */
        f = f * gems_ldf(rob + 0x84);               /* fmuls */
        gems_stf(rob + 0xC50, f);
    }
    gems_st16(rob + 0xC4C, gems_ld16(rob + 0x1A8));   /* ldos / stos */
    if (entry) gems_i960_ret();
    return 0;
}
