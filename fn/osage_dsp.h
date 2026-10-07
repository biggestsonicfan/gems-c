/* osage_dsp (i960 0x67640, Gems 8004B67C): one fighter's sway chains
 * (osage) for this frame. g13 is the osage work, g7 the fighter (g13+0x40).
 * Loads the camera into the COP when the game mode asks for it (0x04 from
 * 0x501128 / 0x501158 by player), works out the flags in [g13], opens a
 * bufferram display list block for the fighter, runs the parameter stream
 * (g9 -> g8, from g13+0x44 / +0x48) through the per-record setters, then
 * Fn_osage (osage_copro) and set_situation_flags; in mode 2 it also draws the
 * list of matrices + models queued at 0x50E400. Bumps the frame count at
 * g13+6 and times the first frame from the timer at 0xF0000C.
 *
 * Gems leaves out the i960's check at 0x679C0 that prints
 * "ose_parameter_over" / "limit" (balx 0x65D0) when g8 ran more than 0x800
 * bytes past g13+0x48, and calls only types 1..5 of the record table at
 * 0x679A8 (the i960 callx's whatever the table holds for any other type). */

/* i960 bbs / bbc: the bit number is taken mod 32 (Gems' slw gave 0 for
 * 32..63). */
static inline uint32_t osage_dsp_bit(uint32_t n)
{
    return 1u << (n & 0x1F);
}

/* g13 word: read, change, write back (each is its own ld / st). */
static inline void osage_dsp_flags(uint32_t clr, uint32_t set)
{
    uint32_t w = gems_ld32(GEMS_G(13));
    gems_st32(GEMS_G(13), (w & ~clr) | set);
}

/* (0xFFFFF - (timer & 0xFFFFF) - 18) / 25, from ldl 0xF00008 (both words read). */
/* The meter at g13+0x120..0x128 is the only thing --gems-verify finds different:
 * the C takes no board time, so timer 2 reads what it read at the trap. */
static inline uint32_t osage_dsp_timer(void)
{
    uint32_t t[2];
    gems_ldn(t, 0xF00008, 2);
    return (0xFFFFFu - (t[1] & 0xFFFFFu) - 18u) / 25u;
}

/* Open the fighter's display list block, as the i960 does: g10 gets 0
 * (g14 parked at 0x50EFFC meanwhile), 0x501800 the block's start. */
static inline void osage_dsp_open_block(void)
{
    uint32_t start = gems_ld32(GEMS_G(10) + 0x2008);
    gems_st32(0x50EFFC, GEMS_G(14));
    GEMS_G(14) = 0;
    gems_st32(GEMS_G(10), GEMS_G(14));
    GEMS_G(14) = gems_ld32(0x50EFFC);
    gems_st32(0x501800, start);
}

/* Link the block to the slot pair at `slot` (start, link): an empty link
 * keeps the start in the slot, else the link word in bufferram points here. */
static inline void osage_dsp_link(uint32_t slot)
{
    uint32_t link = gems_ld32(slot + 4);
    uint32_t here = gems_ld32(GEMS_G(10) + 0x2008);
    if (link == 0)
        gems_st32(slot, here);
    else
        gems_st32(link + 0x900000, here | 0x80000000u);
}

/* Close the block: the slot's link becomes this point, g10 jumps to the
 * slot's start, and the block's head (at 0x501800) points here. */
static inline void osage_dsp_close_block(uint32_t slot)
{
    uint32_t start = gems_ld32(slot);
    gems_st32(slot + 4, gems_ld32(GEMS_G(10) + 0x2008));
    gems_st32(GEMS_G(10), start | 0x80000000u);
    uint32_t here = gems_ld32(GEMS_G(10) + 0x2008);
    uint32_t head = gems_ld32(0x501800);
    gems_st32(head + 0x900000, here | 0x80000000u);
}

static uint32_t gfn_osage_dsp(int entry)
{
    if (gems_ld32(GEMS_G(13)) & 0x08)
        goto out;
    GEMS_G(7) = gems_ld32(GEMS_G(13) + 0x40);

    uint32_t mode = gems_ld8(0x50002B);                                 /* ldob */
    if (osage_dsp_bit(mode) & 0xC0) {
        if (!(gems_ld32(gems_ld32(0x500838)) & 0x80000000u))
            goto out;
        osage_dsp_flags(0, 0x10);
        osage_dsp_flags(0x02, 0);
        uint32_t cam = gems_ld8(GEMS_G(7) + 4) == 0 ? 0x501128 : 0x501158;   /* ldob */
        gems_cop_w(0x02000404);                                         /* Fn_load_matrix */
        for (uint32_t i = 0; i < 12; i++)
            gems_cop_w(gems_ld32(cam + i * 4));
    } else {
        uint32_t w = gems_ld32(GEMS_G(13)) & ~0x10u;
        gems_st32(GEMS_G(13), w);
        /* As the ROM: bbc tests bit (the flag word) of 0x300. */
        if ((osage_dsp_bit(w) & 0x300) && (osage_dsp_bit(gems_ld8(0x500031)) & 3))
            goto out;
        if (!(gems_ld32(GEMS_G(7)) & 0x80))
            goto out;
    }

    osage_dsp_flags(0x100, 0);
    uint32_t pad = gems_ld32(0x500814);
    uint32_t held = gems_ld8(GEMS_G(7) + 4) == 0                        /* ldob */
                  ? gems_ld32(pad) & 0x02 : gems_ld32(pad) & 0x04;
    if (held)
        osage_dsp_flags(0x20, 0);
    else
        osage_dsp_flags(0, 0x20);

    uint32_t w = gems_ld32(GEMS_G(13));
    if (!(w & 4) && !(w & 1))
        gems_st32(GEMS_G(13) + 0x120, osage_dsp_timer());

    osage_dsp_open_block();
    uint32_t slot = 0x501804 + gems_ld8(GEMS_G(7) + 4) * 8;             /* ldob */
    osage_dsp_link(slot);
    gems_st32(0x50E400, 0x50E404);

    /* g13+0xE4: -999.9 with flag 0x10, else -0.5 when the larger of |x|, |z|
     * at g7+0x20C passes [0x50A00C] (compared as integer bits), else 0. */
    uint32_t lift;
    if (gems_ld32(GEMS_G(13)) & 0x10) {
        lift = 0xC479F99Au;
    } else {
        uint32_t v[3];
        gems_ldn(v, GEMS_G(7) + 0x20C, 3);                              /* ldt */
        int32_t ax = (int32_t)(v[0] & 0x7FFFFFFFu);
        int32_t az = (int32_t)(v[2] & 0x7FFFFFFFu);
        if (!(ax > az)) ax = az;
        lift = ax > (int32_t)gems_ld32(0x50A00C) ? 0xBF000000u : 0;
    }
    gems_st32(GEMS_G(13) + 0xE4, lift);
    float level = gems_u2f(lift) + 0.5f;                                /* fadds */
    if (gems_ldf(GEMS_G(7) + 0x210) < level) {
        osage_dsp_flags(0, 0x40);
        osage_dsp_flags(0, 0x80);
    } else {
        osage_dsp_flags(0x40, 0);
        osage_dsp_flags(0x80, 0);
    }

    gfn_calc_kaze(0);
    gems_st32(GEMS_G(13) + 0x144, 0);
    gems_st32(GEMS_G(13) + 0x50, 0x54);
    GEMS_G(9) = gems_ld32(GEMS_G(13) + 0x44);
    GEMS_G(8) = gems_ld32(GEMS_G(13) + 0x48);

    /* The parameter stream: copy each record's type word, let its setter
     * fill the record, step g9 / g8 by the type's sizes at 0x67520. */
    for (;;) {
        uint32_t type = gems_ld32(GEMS_G(9));
        GEMS_G(9) += 4;
        gems_st32(GEMS_G(8), type);
        GEMS_G(8) += 4;
        if (type == 0)
            break;
        switch ((int32_t)type) {
        case 1: gfn_os_set_osage(0);   break;
        case 2: gfn_os_set_coli(0);    break;
        case 3: {                      /* i960 0x68758, inline in Gems */
            gems_st32(GEMS_G(8), gems_ld32(GEMS_G(9)));
            uint32_t v = gems_ld32(GEMS_G(9) + 4);
            if (gems_ld32(GEMS_G(13)) & 0x40)
                v = 0x3F4CCCCDu;       /* 0.8f */
            gems_st32(GEMS_G(8) + 4, v);
            if (gems_ld32(GEMS_G(13)) & 0x10)
                gems_st32(GEMS_G(8), 1);
            break;
        }
        case 4: gfn_os_set_tsukene(0); break;
        case 5: gfn_os_set_matrix(0);  break;
        default: break;
        }
        uint32_t step[2];
        gems_ldn(step, 0x67520 + type * 8, 2);                          /* ldl */
        GEMS_G(9) += step[0];
        GEMS_G(8) += step[1];
    }

    gems_cop_w(0x00800101);                                             /* Fn_push_matrix */
    gfn_osage_copro(0);
    gems_cop_w(0x01000202);                                             /* Fn_pop_matrix */
    gfn_set_situation_flags(0);
    osage_dsp_close_block(0x501804 + gems_ld8(GEMS_G(7) + 4) * 8);      /* ldob */

    if (gems_ld8(0x500064) == 2) {                                      /* ldob */
        osage_dsp_open_block();
        osage_dsp_link(0x501834);
        gems_cop_w(0x00800101);
        gems_cop_w(0x06000C0C);
        gems_cop_w(0x09801313);
        gems_cop_w(0x09801313);
        gems_cop_w(0x09801313);
        (void)gems_cop_r();
        gems_cop_w(0x21804343);
        gems_cop_w(0);
        gems_cop_w(0x01000202);
        gems_cop_w(0x00800101);
        gems_cop_w(0x03800707);                                         /* scale 1, -1, 1 */
        gems_cop_w(0x3F800000u);
        gems_cop_w(0xBF800000u);
        gems_cop_w(0x3F800000u);
        gems_cop_w(0x22804545);
        gems_cop_w(0);

        /* The queued draws at 0x50E404..[0x50E400]: 12 matrix words and a
         * model number each. The 0x13 before each test passes on the
         * previous reply, as the i960 does with r4. */
        uint32_t cur = 0x50E404;
        uint32_t end = gems_ld32(0x50E400);
        uint32_t arg = 0x3F800000u;
        for (;;) {
            gems_cop_w(0x09801313);
            gems_cop_w(arg);
            gems_cop_w(arg);
            arg = gems_cop_r();
            if (cur == end)
                break;
            gems_cop_w(0x00800101);
            gems_cop_w(0x05800B0B);                                     /* Fn_mul_matrix */
            for (uint32_t i = 0; i < 12; i++)
                gems_cop_w(gems_ld32(cur + i * 4));
            cur += 0x30;
            GEMS_G(0) = gems_ld32(cur);
            cur += 4;
            if (GEMS_G(0) != 0) {
                GEMS_G(1) = gems_ld8(GEMS_G(7) + 4);                    /* ldob */
                gfn_set_obj();
            }
            gems_cop_w(0x01000202);
        }
        gems_cop_w(0x01000202);
        osage_dsp_close_block(0x501834);
    }

    gems_st16(GEMS_G(13) + 6, gems_ld16(GEMS_G(13) + 6) + 1);           /* ldos / stos */
    w = gems_ld32(GEMS_G(13));
    if (!(w & 4) && !(w & 1) && gems_ld8(0x500000) == 0) {
        gems_st32(GEMS_G(13) + 0x124, osage_dsp_timer());
        uint32_t t0 = gems_ld32(GEMS_G(13) + 0x120);
        uint32_t t1 = gems_ld32(GEMS_G(13) + 0x124);
        gems_st32(GEMS_G(13) + 0x128, t1 - t0);
        osage_dsp_flags(0, 4);
    }
    osage_dsp_flags(1, 0);
out:
    if (entry) gems_i960_ret();
    return 0;
}
