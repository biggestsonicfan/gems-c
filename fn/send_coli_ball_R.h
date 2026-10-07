/* send_coli_ball_R (i960 0x11A8, Gems 0x80053730): write both fighters'
 * collision-ball radii into the COP's DM with Fn_write_ram (0x49): P1's from
 * 0x50A204 to DM 0x30600.., P2's from 0x50A28C to 0x30700... Balls 0-31 go
 * to slots 0x00-0x1F, a zero to slot 0x20, then balls 31..0 again to slots
 * 0x21-0x40, then ball 33's to slot 0xF1. Each pair of writes is preceded by
 * a COP 0x13 (args 0x09801313 twice) whose one reply is dropped. */
#pragma once

static void gfn_send_coli_ball_R__pair(uint32_t slot1, uint32_t r1, uint32_t slot2, uint32_t r2)
{
    gems_cop_w(0x09801313);
    gems_cop_w(0x09801313);
    gems_cop_w(0x09801313);
    (void)gems_cop_r();
    gems_cop_w(0x24804949);
    gems_cop_w(slot1);
    gems_cop_w(r1);
    gems_cop_w(0x24804949);
    gems_cop_w(slot2);
    gems_cop_w(r2);
}

static uint32_t gfn_send_coli_ball_R(int entry)
{
    uint32_t dm1 = 0x30600, dm2 = 0x30700;
    for (uint32_t i = 0; i < 32; i++, dm1++, dm2++)
        gfn_send_coli_ball_R__pair(dm1, gems_ld32(0x50A204 + i * 4),
                                   dm2, gems_ld32(0x50A28C + i * 4));
    gfn_send_coli_ball_R__pair(0x30620, 0, 0x30720, 0);
    dm1 = 0x30621;
    dm2 = 0x30721;
    for (int32_t i = 31; i >= 0; i--, dm1++, dm2++)
        gfn_send_coli_ball_R__pair(dm1, gems_ld32(0x50A204 + (uint32_t)i * 4),
                                   dm2, gems_ld32(0x50A28C + (uint32_t)i * 4));
    gems_cop_w(0x24804949);
    gems_cop_w(0x306F1);
    gems_cop_w(gems_ld32(0x50A288));
    gems_cop_w(0x24804949);
    gems_cop_w(0x307F1);
    gems_cop_w(gems_ld32(0x50A310));
    if (entry) gems_i960_ret();
    return 0;
}
