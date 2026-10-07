/* set_coli_ball_data (i960 0x17C14, Gems 80038FF8): for part g0 of the
 * fighter at g7, send each of its collision balls to the COP: 0x39
 * (Fn_coli_set_ball: x, y, z, then 3 * (ball >> 2)), and unless
 * [[0x500828]] bit 31 is set, 0x29 on the same point (its three replies
 * are read and dropped, as the i960 does). */
static uint32_t gfn_set_coli_ball_data(int entry)
{
    uint32_t rob    = GEMS_G(7);
    uint32_t pos    = gems_ld32(rob + 0x1D0C);
    uint32_t radius = gems_ld32(rob + 0x1D00);
    uint32_t list   = gems_ld32(rob + 0x1D14) + GEMS_G(0) * 8;
    uint32_t count  = gems_ld8(gems_ld32(rob + 0x1D20) + GEMS_G(0));   /* ldob */
    uint32_t mode   = gems_ld32(gems_ld32(0x500828));
    for (uint32_t i = 0; count != 0 && i != count; i++) {
        uint32_t ball = gems_ld8(list + i);                     /* ldob */
        uint32_t p[4];
        gems_ldn(p, pos + ball * 4, 4);
        p[3] = gems_ld32(radius + ball * 4);
        gems_cop_w(0x1C803939);
        gems_cop_wn(p, 3);
        gems_cop_w((ball >> 2) * 3);
        if (!(mode & 0x80000000u)) {
            gems_cop_w(0x14802929);
            gems_cop_wn(p, 3);
            gems_cop_rn(p, 3);
        }
    }
    if (entry) gems_i960_ret();
    return 0;
}
