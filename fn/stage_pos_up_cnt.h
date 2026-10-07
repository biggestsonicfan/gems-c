/* stage_pos_up_cnt (i960 0x26E38, Gems 8004F6FC): on stages 0x1A / 0x1B,
 * step the rising-stage counter at 0x50049E; each step asks the COP for
 * a value (0x17) and moves by half of it in y (0x06, Fn_trans). */
static uint32_t gfn_stage_pos_up_cnt(int entry)
{
    uint32_t stage = gems_ld8(0x500031);
    if (stage == 0x1A || stage == 0x1B) {
        uint32_t flags = gems_ld32(0x500498);
        if (!(flags & 0x40) && (flags & 1) && (flags & 8)) {
            uint32_t n = gems_ld16(0x50049E);                   /* ldos */
            if (n == 0x46) {
                gems_st16(0x50049E, 0);
                gems_st32(0x500498, gems_ld32(0x500498) | 0x40);
                gems_st32(0x500498, gems_ld32(0x500498) | 0x20);
            } else {
                if (n == 0x32)
                    gems_st32(0x500498, gems_ld32(0x500498) | 0x10);
                gems_cop_w(0x0B801717);
                gems_cop_w(n);
                float h = 0.5f * gems_cop_rf();                 /* fmuls */
                gems_cop_w(0x03000606);
                gems_cop_w(0);
                gems_cop_wf(h);
                gems_cop_w(0);
                if (!(gems_ld32(0x508000) & 0x20))
                    gems_st16(0x50049E, (n + 1) & 0xFFFF);
            }
        }
    }
    if (entry) gems_i960_ret();
    return 0;
}
