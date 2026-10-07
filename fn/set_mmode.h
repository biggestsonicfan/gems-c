/* set_mmode (i960 0x4C94, Gems 80038E08): GEO command 07 (mode word).
 * Gems leaves g14 = 0x707 and writes nothing at 0x50EFFC; the i960 parks g14
 * there while it carries the command and takes it back, and its callers
 * (rob_disp_rdn_loop, rob_kage_disp_test) read g14 after. This is the i960's. */
static uint32_t gfn_set_mmode(int entry)
{
    gems_st32(0x50EFFC, GEMS_G(14));
    GEMS_G(14) = 0x707;
    gems_st32(GEMS_G(10) + 0x70, GEMS_G(14));
    GEMS_G(14) = gems_ld32(0x50EFFC);
    gems_st32(GEMS_G(10) + GEMS_G(12), GEMS_G(0));
    if (entry) gems_i960_ret();
    return 0;
}
