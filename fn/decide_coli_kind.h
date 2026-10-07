/* decide_coli_kind (i960 0x2CFBC): picks the collision-kind table entry for
 * the two fighters (g7 = P1, g8 = P2) and stores its ball masks, radii and
 * flags into the collision work at g13. g6 is left holding the kind bits. */
static void gfn_decide_coli_kind__masks(uint32_t entry_addr, uint32_t off,
                                        uint32_t dst_ofs, int invert)
{
    GEMS_G(1) = gems_ld32(gems_ld32(off) + 0x1D14);
    GEMS_G(0) = gems_ld32(entry_addr);
    gfn_unit_to_ball(0);
    if (invert)
        GEMS_G(0) = ~GEMS_G(0);
    gems_st32(GEMS_G(13) + dst_ofs, GEMS_G(0));
}

/* The "is the other fighter a soft grab target" test (0x2D018 / 0x2D064):
 * returns nonzero when entry `kind` applies. */
static int gfn_decide_coli_kind__grab(uint32_t me, uint32_t you,
                                      uint32_t my_st, uint32_t your_st)
{
    if (!(your_st & 8))
        return 0;
    if ((gems_ld32(me + 0xC58) & 0x40000) && !(my_st & 0x10))
        return 0;
    if (gems_ld16(me + 0x1AA) > 4)                 /* ldos */
        return 0;
    if (!(my_st & 2) && !(gems_ld32(you + 0x5B8) & 0x200)
        && !(gems_ld32(you + 0x5B8) & 0x100)
        && gems_ldf(you + 0x5F4) > 0.4f)
        return 0;
    return 1;
}

static uint32_t gfn_decide_coli_kind(int entry)
{
    uint32_t p1 = gems_ld32(0x500804), p2 = gems_ld32(0x500808);
    GEMS_G(7) = p1;
    GEMS_G(8) = p2;
    uint32_t st1 = gems_ld32(p1 + 0x1A4), st2 = gems_ld32(p2 + 0x1A4);
    uint32_t st_or = st1 | st2, st_and = st1 & st2, st_xor = st1 ^ st2;
    uint32_t ex_xor = gems_ld32(p1 + 0x70C) ^ gems_ld32(p2 + 0x70C);
    uint32_t kind = 0x2CD50;
    int chosen = 0;

    GEMS_G(6) = 0;
    if (st_or & 0x40000) {
        GEMS_G(6) |= 1;
    } else if (!(ex_xor & 0x40000) && !(st_xor & 0x100)) {
        if (st_and & 0x100)
            GEMS_G(6) |= 4;
    } else {
        if ((ex_xor & 0x40000) && (st_xor & 8)) {
            /* whichever fighter is not in state 0x40000 at +0x70C is "me" */
            if (!(gems_ld32(p1 + 0x70C) & 0x40000)) {
                if (gfn_decide_coli_kind__grab(p2, p1, st2, st1)) {
                    kind = 0x2CEF0;
                    chosen = 1;
                }
            } else if (gfn_decide_coli_kind__grab(p1, p2, st1, st2)) {
                kind = 0x2CF10;
                chosen = 1;
            }
        }
        if (!chosen && gems_ld8(p1 + 0x820) != 0x29
            && gems_ld8(p2 + 0x820) != 0x29)
            GEMS_G(6) |= 2;
    }

    if (!chosen) {
        if (st_or & 0x4000)
            GEMS_G(6) |= 8;
        uint32_t g6 = GEMS_G(6);
        if (!(g6 & 1) && !(g6 & 4)) {
            int tail = 1;                  /* 0x2D20C */
            if (st1 & 0x10000) {
                if (!(st1 & 0x8000) && (st2 & 0x400000)) {
                    uint32_t w = gems_ld32(p2 + 0x804);
                    if ((w & 0x1000) || (gems_ld32(p2 + 0x804) & 0x10)) {
                        GEMS_G(6) |= 0x40;
                        kind = 0x2CE30;
                        tail = 0;
                    } else {
                        GEMS_G(6) |= 0x80;
                    }
                }
            } else if (st2 & 0x10000) {
                if (!(st2 & 0x8000) && (st1 & 0x400000)) {
                    uint32_t w = gems_ld32(p1 + 0x804);
                    if ((w & 0x1000) || (gems_ld32(p1 + 0x804) & 0x10)) {
                        GEMS_G(6) |= 0x10;
                        kind = 0x2CE50;
                        tail = 0;
                    } else {
                        GEMS_G(6) |= 0x20;
                    }
                }
            } else {
                tail = 0;
                if (g6 & 2) {
                    /* table by g13+0x1E0 bits 13..15: a fighter's ball radius */
                    uint32_t sel = (gems_ld32(GEMS_G(13) + 0x1E0) >> 13) & 7;
                    if (st1 & 0x100) {
                        kind = 0x2CDB0;
                        uint32_t o = gems_ld32(0x2E290 + sel * 4);
                        if (!(gems_ldf(p2 + 0x650 + o) > 0.1f))
                            kind = 0x2CDF0;
                    } else {
                        kind = 0x2CDD0;
                        uint32_t o = gems_ld32(0x2E270 + sel * 4);
                        if (!(gems_ldf(p1 + 0x650 + o) > 0.1f))
                            kind = 0x2CE10;
                    }
                } else if (st_xor & 0x10) {
                    kind = (st1 & 0x10) ? 0x2CE70 : 0x2CE90;
                }
            }
            if (tail) {
                if (!(st1 & 0x10000))
                    kind = (st2 & 0x8000) ? 0x2CED0 : 0x2CD90;
                else if (!(st2 & 0x10000))
                    kind = (st1 & 0x8000) ? 0x2CEB0 : 0x2CD70;
            }
        }
    }

    gfn_decide_coli_kind__masks(kind, 0x500804, 0xC0, 1);
    gfn_decide_coli_kind__masks(kind + 4, 0x500808, 0xC4, 1);

    uint32_t r1 = gems_ld32(kind + 8), r2 = gems_ld32(kind + 0xC);
    if (gems_u2f(r1) == 9.9f) {          /* 9.9: radii from g13+0x27C/0x280 */
        r1 = gems_ld32(GEMS_G(13) + 0x27C) & 0x7FFFFFFFu;
        r2 = gems_ld32(GEMS_G(13) + 0x280) & 0x7FFFFFFFu;
        if (r1 | r2) {
            gems_cop_w(0x2D005A5A);
            gems_cop_w(r1);
            gems_cop_w(r2);
            r1 = gems_cop_r();
            r2 = gems_cop_r();
        }
    }
    gems_st32(GEMS_G(13) + 0xCC, r1);
    gems_st32(GEMS_G(13) + 0xD0, r2);
    gems_st32(GEMS_G(13) + 0xC8, gems_ld32(kind + 0x10));
    gems_st32(GEMS_G(13) + 0x88, gems_ld32(kind + 0x14));

    gfn_decide_coli_kind__masks(kind + 0x18, 0x500804, 0x264, 0);
    gfn_decide_coli_kind__masks(kind + 0x1C, 0x500808, 0x268, 0);

    if (entry)
        gems_i960_ret();
    return 0;
}
