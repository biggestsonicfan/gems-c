/* rob_ball_data_make (i960 0x2989C, Gems 8003921C): the fighter's 32 ball
 * records (x, y, z from one character table, radius word from another)
 * into rob (g7) + 0xD00, 16 bytes each. */
static uint32_t gfn_rob_ball_data_make(int entry)
{
    uint32_t chr   = gems_ld8(GEMS_G(7) + 4);              /* ldob */
    uint32_t pos   = gems_ld32(chr * 4 + 0x2E504);
    uint32_t radii = gems_ld32(chr * 4 + 0x2E50C);
    for (uint32_t i = 0; i < 32; i++) {
        uint32_t ball[4];
        gems_ldn(ball, pos + i * 12, 3);
        ball[3] = gems_ld32(radii + i * 4);
        gems_stn(GEMS_G(7) + i * 16 + 0xD00, ball, 4);
    }
    if (entry) gems_i960_ret();
    return 0;
}
