/* os_set_matrix (i960 0x685FC, Gems 8004C8F8): the type-5 record of the
 * osage parameter stream, called from osage_dsp's loop with g9 = the source
 * record, g8 = the copy, g13 = the osage work. (FN/INDEX.md's 0x67D28 is the
 * type-1 handler, os_set_osage; 0x685FC is entry 5 of the table at 0x679A8.)
 *
 * Flag 1 in [g13] (the first frame): moves the point at g13+0x108 by the
 * record's offset turned into world space (0x5C rotate by g13+0x114, 0x30,
 * 0x5E scale by g9+0x24, 0x5C add) and copies the record into g8, leaving
 * g0..g2 the COP's last direction. Otherwise only the copy's g8+0x20 is set:
 * zero with flag 0x80, else the record's g9+0x28 (or +0x34 with flag 2),
 * scaled by g13+0x144 when that is not zero. */
static uint32_t gfn_os_set_matrix(int entry)
{
    if (gems_ld32(GEMS_G(13)) & 1) {
        uint32_t p[3], q[3];
        gems_ldn(&GEMS_G(0), GEMS_G(13) + 0x114, 3);                    /* ldt */
        gems_ldn(p, GEMS_G(9) + 0x28, 3);
        gems_cop_w(0x2E005C5C);
        gems_cop_w(p[0]); gems_cop_w(GEMS_G(0));
        gems_cop_w(p[1]); gems_cop_w(GEMS_G(1));
        gems_cop_w(p[2]); gems_cop_w(GEMS_G(2));
        GEMS_G(0) = gems_cop_r();
        GEMS_G(1) = gems_cop_r();
        GEMS_G(2) = gems_cop_r();

        gems_cop_w(0x18003030);
        gems_cop_w(GEMS_G(0));
        gems_cop_w(GEMS_G(1));
        gems_cop_w(GEMS_G(2));
        GEMS_G(0) = gems_cop_r();
        GEMS_G(1) = gems_cop_r();
        GEMS_G(2) = gems_cop_r();

        uint32_t scale = gems_ld32(GEMS_G(9) + 0x24);
        gems_cop_w(0x2F005E5E);
        gems_cop_w(scale);
        gems_cop_w(GEMS_G(0));
        gems_cop_w(GEMS_G(1));
        gems_cop_w(GEMS_G(2));
        GEMS_G(0) = gems_cop_r();
        GEMS_G(1) = gems_cop_r();
        GEMS_G(2) = gems_cop_r();

        gems_ldn(p, GEMS_G(13) + 0x108, 3);
        gems_cop_w(0x2E005C5C);
        gems_cop_w(GEMS_G(0)); gems_cop_w(p[0]);
        gems_cop_w(GEMS_G(1)); gems_cop_w(p[1]);
        gems_cop_w(GEMS_G(2)); gems_cop_w(p[2]);
        p[0] = gems_cop_r();
        p[1] = gems_cop_r();
        p[2] = gems_cop_r();
        gems_stn(GEMS_G(13) + 0x108, p, 3);                             /* stt */
        gems_stn(GEMS_G(8), p, 3);

        gems_ldn(q, GEMS_G(9) + 0x0C, 3);
        gems_stn(GEMS_G(8) + 0x0C, q, 3);
        gems_st32(GEMS_G(8) + 0x18, gems_ld32(GEMS_G(9) + 0x18));
        gems_st32(GEMS_G(8) + 0x1C, gems_ld32(GEMS_G(9) + 0x24));
        gems_ldn(q, GEMS_G(9) + 0x28, 3);
        gems_stn(GEMS_G(8) + 0x20, q, 3);
    } else {
        uint32_t w = gems_ld32(GEMS_G(13));
        uint32_t scale = gems_ld32(GEMS_G(13) + 0x144);
        uint32_t p[3] = { 0, 0, 0 };
        if (!(w & 0x80)) {
            gems_ldn(p, GEMS_G(9) + ((w & 2) ? 0x34 : 0x28), 3);
            if (scale != 0) {
                gems_cop_w(0x2F005E5E);
                gems_cop_w(scale);
                gems_cop_w(p[0]);
                gems_cop_w(p[1]);
                gems_cop_w(p[2]);
                p[0] = gems_cop_r();
                p[1] = gems_cop_r();
                p[2] = gems_cop_r();
            }
        }
        gems_stn(GEMS_G(8) + 0x20, p, 3);
    }
    if (entry) gems_i960_ret();
    return 0;
}
