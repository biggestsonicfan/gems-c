/* cps_1 (i960 0x5DA4, Gems 800533B8): fill a g0 x g1 block of tile words
 * at g9 (rows 0x80 bytes apart) with `value`. Leaves g9 past the block and
 * g1 counted down, as the i960 does. A count of 0 runs once (cmpdeco). */
static uint32_t gfn_cps_1(int32_t entry, int16_t value)
{
    do {
        uint32_t a = GEMS_G(9);
        uint32_t n = GEMS_G(0);
        do {
            gems_st16(a, (uint16_t)value);
            a += 2;
        } while (n-- > 1);
        GEMS_G(9) += 0x80;
    } while (GEMS_G(1)-- > 1);
    if (entry) gems_i960_ret();
    return 0;
}
