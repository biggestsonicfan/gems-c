/* coli_cont_cop (i960 0x2D36C, Gems 8004114C): one frame of fight collision.
 * g7/g8 are the two fighters' records on entry; the robs come back from
 * 0x500804 / 0x500808 as the chain goes on. g13 is the fight work area. */

/* The record whose action counter at +0x1AA has just entered the 15-frame
 * window that starts at +0x80A. */
static int coli_cont_cop_in_window(uint32_t rob)
{
    uint32_t count = gems_ld16(rob + 0x1AA);
    uint32_t start = gems_ld16(rob + 0x80A);
    if (count < start) return 0;
    return count < start + 15;
}

static uint32_t gfn_coli_cont_cop(int entry)
{
    uint32_t p0[3], p1[3];
    gems_ldn(p0, GEMS_G(7) + 0x1F4, 3);
    gems_ldn(p1, GEMS_G(8) + 0x1F4, 3);
    float dx = gems_u2f(p1[0]) - gems_u2f(p0[0]);
    float dz = gems_u2f(p1[2]) - gems_u2f(p0[2]);
    gems_cop_w(0x1F003E3E);                       /* into the P0 -> P1 frame */
    gems_cop_wf(dx);
    gems_cop_wf(dz);
    gems_cop_w(p0[0]);
    gems_cop_w(p0[1]);
    gems_cop_w(p0[2]);

    GEMS_G(7) = gems_ld32(0x500804);
    gfn_calc_attack_flag(0);
    uint32_t attack0 = GEMS_G(3);
    GEMS_G(7) = gems_ld32(0x500808);
    gfn_calc_attack_flag(0);
    uint32_t attack1 = GEMS_G(3);
    GEMS_G(7) = gems_ld32(0x500804);
    uint32_t chr0 = gems_ld8(GEMS_G(7) + 0x821);
    GEMS_G(7) = gems_ld32(0x500808);
    uint32_t chr1 = gems_ld8(GEMS_G(7) + 0x821);
    gems_cop_w(0x1D003A3A);                       /* broad phase */
    gems_cop_w(attack0);
    gems_cop_w(attack1);
    gems_cop_w(chr0);
    gems_cop_w(chr1);

    gfn_coli_ram_init(0);
    GEMS_G(7) = gems_ld32(0x500804);
    GEMS_G(8) = gems_ld32(0x500808);
    gfn_area_coli(0);
    GEMS_G(7) = gems_ld32(0x500808);
    GEMS_G(8) = gems_ld32(0x500804);
    gfn_area_coli(0);
    gfn_decide_coli_kind(0);

    int recalc_only = 0;
    GEMS_G(7) = gems_ld32(0x500804);
    if (gems_ld32(GEMS_G(7) + 0x1A4) & 0x20000000) {
        gfn_decide_dir(0);
    } else {
        GEMS_G(8) = gems_ld32(0x500808);
        if (gems_ld32(GEMS_G(8) + 0x1A4) & 0x20000000) {
            gfn_decide_dir(0);
        } else {
            uint32_t kind = GEMS_G(6);
            if (kind & 1)
                recalc_only = 1;
            else if (kind & 0x20)
                recalc_only = coli_cont_cop_in_window(GEMS_G(7));
            else if (kind & 0x80)
                recalc_only = coli_cont_cop_in_window(GEMS_G(8));
        }
    }
    if (recalc_only) {
        gfn_decide_dir(0);
        GEMS_G(14) = 0x2D488;                     /* bal coli_recalc_pos's return */
        gfn_coli_recalc_pos_at(p0[0], p0[2], p1[0], p1[2]);
        if (entry) gems_i960_ret();
        return 0;
    }

    GEMS_G(7) = gems_ld32(0x500804);
    uint32_t r0 = gems_ld32(GEMS_G(7) + 0x768);
    GEMS_G(8) = gems_ld32(0x500808);
    uint32_t r1 = gems_ld32(GEMS_G(8) + 0x768);
    uint32_t w = GEMS_G(13);
    uint32_t a88 = gems_ld32(w + 0x88);
    uint32_t ac0 = gems_ld32(w + 0xC0);
    uint32_t ac4 = gems_ld32(w + 0xC4);
    gems_cop_w(0x1D803B3B);                       /* narrow phase */
    gems_cop_w(a88);
    gems_cop_w(r0);
    gems_cop_w(r1);
    gems_cop_w(ac0);
    gems_cop_w(ac4);
    uint32_t a264 = gems_ld32(w + 0x264);
    uint32_t a268 = gems_ld32(w + 0x268);
    gems_cop_w(a264);
    gems_cop_w(a268);
    uint32_t rep0 = gems_cop_r();
    uint32_t rep1 = gems_cop_r();
    GEMS_G(4) = gems_cop_r();
    GEMS_G(5) = gems_cop_r();
    gems_st32(w + 0x160, rep0);
    gems_st32(w + 0x164, rep1);

    /* Four-frame running mean of g5: history at +0xF8..+0x100. */
    uint32_t h100 = gems_ld32(w + 0x100);
    uint32_t hfc = gems_ld32(w + 0xFC);
    float sum = gems_u2f(hfc) + gems_u2f(h100);
    gems_st32(w + 0x100, hfc);
    uint32_t hf8 = gems_ld32(w + 0xF8);
    sum = gems_u2f(hf8) + sum;
    gems_st32(w + 0xFC, hf8);
    sum = gems_u2f(GEMS_G(5)) + sum;
    gems_st32(w + 0xF8, GEMS_G(5));
    GEMS_G(5) = gems_f2u(sum / 4.0f);
    GEMS_G(14) = 0x2D530;
    gfn_coli_recalc_pos_at(p0[0], p0[2], p1[0], p1[2]);
    if (entry) gems_i960_ret();
    return 0;
}
