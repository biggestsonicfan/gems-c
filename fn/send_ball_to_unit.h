/* send_ball_to_unit (i960 0x13F0, Gems 800535C0): for each of 32 balls,
 * a Fn_nop (0x13) handshake and two Fn_write_ram (0x49) words: DM 0x306A0+i
 * and 0x307A0+i from the tables at 0x506580 / 0x506600. */
static uint32_t gfn_send_ball_to_unit(int entry)
{
    uint32_t dm_a = 0x306A0, dm_b = 0x307A0;
    uint32_t src_a = 0x506580, src_b = 0x506600;
    for (int i = 0; i < 32; i++) {
        uint32_t a = gems_ld32(src_a);
        uint32_t b = gems_ld32(src_b);
        gems_cop_w(0x09801313);
        gems_cop_w(0x09801313);
        gems_cop_w(0x09801313);
        (void)gems_cop_r();
        gems_cop_w(0x24804949);
        gems_cop_w(dm_a);
        gems_cop_w(a);
        gems_cop_w(0x24804949);
        gems_cop_w(dm_b);
        gems_cop_w(b);
        dm_a++; dm_b++;
        src_a += 4; src_b += 4;
    }
    if (entry) gems_i960_ret();
    return 0;
}
