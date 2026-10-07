/* select_enemy_command (i960 0x3C214, Gems 80051EB4): clear the CPU command
 * interpreter's debug history and carry on at 0x3C27C (site + 0xD0 / 2).
 * Gems skips the clearing; the i960 does it (0x50F600 = -1, then zeros over
 * 0x50F604-0x50F68F), and so does this, registers and condition code too. */
static uint32_t gfn_select_enemy_command(int entry)
{
    (void)entry;
    gems_st32(0x50F600, 0xFFFFFFFFu);
    gems_st32(0x50F604, 0);
    gems_st32(0x50F608, 0);
    for (uint32_t a = 0x50F610; a < 0x50F690; a += 4) gems_st32(a, 0);
    gems_st32(0x50F614, 0);
    gems_st32(0x50F618, 0);
    gems_st32(0x50F61C, 0);
    GEMS_R(13) = 0;
    GEMS_R(14) = 0x50F690;
    GEMS_R(15) = 0;
    GEMS_AC = (GEMS_AC & ~7u) | 2u;          /* the loop's last cmpdeco: 1 == 1 */
    return 0xD0;
}
