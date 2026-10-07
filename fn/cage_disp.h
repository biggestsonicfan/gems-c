/* cage_disp (i960 0x24620, Gems 80050C3C). */
static uint32_t gfn_cage_disp(int entry)
{
    uint32_t flags = gems_ld32(0x500498);
    if ((flags & 0x80000000u) || !(gems_ld32(0x500498) & 2)) {
        gfn_cage_display(0);
        if (!(gems_ld32(0x508000) & 0x20))
            gfn_cage_time_manager(0);
    }
    if (entry) gems_i960_ret();
    return 0;
}
