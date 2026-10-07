/* efc_eggrob_mune_chg (i960 0x33758, Gems 80048044): in actions 0x0B and
 * 0x25 (rob+0x1B0), draw Robotnik's chest: model 0x1022 with rob+0x1A4 bit 14,
 * otherwise one of 16 from the table at 0x34E96, stepped by the frame count. */
static uint32_t gfn_efc_eggrob_mune_chg(int entry)
{
    uint32_t rob = GEMS_G(7);
    if (!(gems_ld32(rob) & 0x20000000u)) {
        uint32_t act = gems_ld8(rob + 0x1B0);                   /* ldob */
        if (act == 0x0B || act == 0x25) {
            if (!(gems_ld32(rob + 0x1A4) & 0x4000)) {
                uint32_t k = (gems_ld32(0x500020) >> 1) & 0xF;
                GEMS_G(1) = 0;
                GEMS_G(0) = gems_ld16(k * 2 + 0x34E96);          /* ldos */
                gfn_set_obj();
            } else {
                GEMS_G(1) = 0;
                GEMS_G(0) = 0x1022;
                gfn_set_obj();
            }
        }
    }
    if (entry) gems_i960_ret();
    return 0;
}
